/*
Version: v0.2

'phi.h' is a simple, header-only 3D rigidbody physics library based
on 'Physics for Game Developers, 2nd Edition' by David Bourg and Bryan Bywalec.

All units are SI, x is forward, y is up and z is right.

MIT License

Copyright (c) 2023 jakob maier

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */
#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <numeric>
#include <type_traits>
#include <variant>
#include <vector>

// defined externally
struct Collider;

namespace phi
{

using Seconds = float;

class RigidBody;

// constants
constexpr float EPSILON = 1e-8f;
constexpr float EARTH_GRAVITY = 9.80665f;
constexpr float EARTH_RADIUS = 6371000.0f;
constexpr float PI = 3.141592653589793f;

// directions in body space
constexpr glm::vec3 X_AXIS = {1.0f, 0.0f, 0.0f};
constexpr glm::vec3 Y_AXIS = {0.0f, 1.0f, 0.0f};
constexpr glm::vec3 Z_AXIS = {0.0f, 0.0f, 1.0f};

constexpr glm::vec3 FORWARD = +X_AXIS;
constexpr glm::vec3 UP = +Y_AXIS;
constexpr glm::vec3 RIGHT = +Z_AXIS;
constexpr glm::vec3 BACKWARD = -X_AXIS;
constexpr glm::vec3 DOWN = -Y_AXIS;
constexpr glm::vec3 LEFT = -Z_AXIS;

// utility functions

// x^2
template <typename T>
constexpr inline T sq(T x)
{
  return x * x;
}

// x^3
template <typename T>
constexpr inline T cb(T x)
{
  return x * x * x;
}

template <typename T>
constexpr inline T scale(T input, T in_min, T in_max, T out_min, T out_max)
{
  input = glm::clamp(input, in_min, in_max);
  return (input - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

template <typename T>
constexpr inline T lerp(T a, T b, float t)
{
  t = glm::clamp(t, 0.0f, 1.0f);
  return a + t * (b - a);
}

template <typename T>
constexpr inline float inverse_lerp(T a, T b, T v)
{
  v = glm::clamp(v, a, b);
  return (v - a) / (b - a);
}

//
struct Transform {
  glm::vec3 position;
  glm::quat rotation;

  constexpr Transform() : position(0.0f), rotation(1.0f, 0.0f, 0.0f, 0.0f) {}
  constexpr Transform(const glm::vec3& p, const glm::quat& r) : position(p), rotation(r) {}

  // get transform matrix
  glm::mat4 matrix() const { return glm::translate(glm::mat4(1.0f), position) * glm::mat4(rotation); }

  // transform direction from body space to world space
  inline glm::vec3 transform_direction(const glm::vec3& direction) const { return rotation * direction; }

  // transform direction from world space to body space
  inline glm::vec3 inverse_transform_direction(const glm::vec3& direction) const
  {
    return glm::inverse(rotation) * direction;
  }

  // transform vector from body space to world space (includes translation)
  inline glm::vec3 transform_vector(const glm::vec3& vector) const
  {
    return glm::vec3(matrix() * glm::vec4(vector, 1.0f));
  }

  // transform vector from world space to body space (includes translation)
  inline glm::vec3 inverse_transform_vector(const glm::vec3& vector) const
  {
    return glm::vec3(glm::inverse(matrix()) * glm::vec4(vector, 1.0f));
  }

  // get body direction in world space
  inline glm::vec3 up() const { return transform_direction(phi::UP); }
  inline glm::vec3 right() const { return transform_direction(phi::RIGHT); }
  inline glm::vec3 forward() const { return transform_direction(phi::FORWARD); }
};

// information needed to resolve a collision
struct CollisionInfo {
  float restitution_coeff = 0.75f;  // coefficient of restitution, 0 = perfectly inelastic, 1 = perfectly elastic
  glm::vec3 point;                  // contact point
  glm::vec3 normal;                 // contact normal
  float penetration;                // penetration depth
  RigidBody *a, *b;                 // the rigidbodies involved
};

// inertia tensor calculations
namespace inertia
{
// mass element used for inertia tensor calculation
struct Element {
  glm::vec3 size;
  glm::vec3 position;  // position in design coordinates
  glm::vec3 inertia;   // moment of inertia
  glm::vec3 offset;    // offset from center of gravity
  float mass;
  float volume() const { return size.x * size.y * size.z; }
};

// cuboid moment of inertia
inline glm::vec3 cuboid(float mass, const glm::vec3& size)
{
  float x = size.x, y = size.y, z = size.z;
  return glm::vec3(sq(y) + sq(z), sq(x) + sq(z), sq(x) + sq(y)) * (1.0f / 12.0f) * mass;
}

// sphere moment of inertia
inline glm::vec3 sphere(float mass, float radius) { return glm::vec3((2.0f / 5.0f) * mass * sq(radius)); }

// inertia tensor from moment of inertia
inline glm::mat3 tensor(const glm::vec3& moment_of_inertia) { return glm::diagonal3x3(moment_of_inertia); }

// distribute mass among elements depending on element volume, to be called before passing elements to tensor()
inline void set_uniform_density(std::vector<Element>& elements, float total_mass)
{
  auto f = [](float s, auto& e) { return s + e.volume(); };
  float total_volume = std::accumulate(elements.begin(), elements.end(), 0.0f, f);

  for (auto& element : elements) {
    element.mass = (element.volume() / total_volume) * total_mass;
  }
}

// calculate inertia tensor for a collection of connected masses
inline glm::mat3 tensor(std::vector<Element>& elements, bool precomputed_offset = false, glm::vec3* cg = nullptr)
{
  float Ixx = 0, Iyy = 0, Izz = 0;
  float Ixy = 0, Ixz = 0, Iyz = 0;

  float mass = 0;
  glm::vec3 moment_of_inertia(0.0f);

  for (const auto& element : elements) {
    mass += element.mass;
    moment_of_inertia += element.mass * element.position;
  }

  const auto center_of_gravity = moment_of_inertia / mass;

  for (auto& element : elements) {
    if (!precomputed_offset) {
      element.offset = element.position - center_of_gravity;
    } else {
      element.offset = element.position;
    }

    const auto offset = element.offset;

    Ixx += element.inertia.x + element.mass * (sq(offset.y) + sq(offset.z));
    Iyy += element.inertia.y + element.mass * (sq(offset.z) + sq(offset.x));
    Izz += element.inertia.z + element.mass * (sq(offset.x) + sq(offset.y));
    Ixy += element.mass * (offset.x * offset.y);
    Ixz += element.mass * (offset.x * offset.z);
    Iyz += element.mass * (offset.y * offset.z);
  }

  if (cg != nullptr) {
    *cg = center_of_gravity;
  }

  // clang-format off
  return {
      Ixx, -Ixy, -Ixz, 
      -Ixy, Iyy, -Iyz, 
      -Ixz, -Iyz, Izz
  };
  // clang-format on
}

// helper function for the creation of a cuboid mass element
inline Element cube(const glm::vec3& position, const glm::vec3& size, float mass = 0.0f)
{
  glm::vec3 inertia = cuboid(mass, size);
  return {size, position, inertia, position, mass};
}

};  // namespace inertia

// unit conversions
namespace units
{
constexpr inline float knots(float meter_per_second) { return meter_per_second * 1.94384f; }

constexpr inline float meter_per_second(float kilometer_per_hour) { return kilometer_per_hour / 3.6f; }

constexpr inline float kilometer_per_hour(float meter_per_second) { return meter_per_second * 3.6f; }

constexpr inline float kelvin(float celsius) { return celsius - 273.15f; }

constexpr inline float watts(float horsepower) { return horsepower * 745.7f; }

constexpr inline float mile_to_kilometre(float mile) { return mile * 1.609f; }

constexpr inline float feet_to_meter(float feet) { return feet * 0.3048f; }

};  // namespace units

// various useful formulas
namespace calc
{
// power in watts
constexpr inline float torque(float power, float rpm) { return 30.0f * power / (2.0f * rpm); }

// the time it takes to full from a certain height
float fall_time(float height, float acceleration = EARTH_GRAVITY) { return sqrt((2 * height) / acceleration); }

};  // namespace calc

// default rigid body is a sphere with radius 1 meter and a mass of 100 kg
const float DEFAULT_RB_MASS = 100.0f;
const float INFINITE_RB_MASS = std::numeric_limits<float>::max();
const glm::quat DEFAULT_RB_ORIENTATION = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
const glm::mat3 DEFAULT_RB_INERTIA = inertia::tensor(inertia::sphere(DEFAULT_RB_MASS, 1.0f));

struct RigidBodyParams {
  float mass = DEFAULT_RB_MASS;
  glm::mat3 inertia = DEFAULT_RB_INERTIA;
  glm::vec3 position = glm::vec3(0);
  glm::quat rotation = DEFAULT_RB_ORIENTATION;
  glm::vec3 velocity = glm::vec3(0);
  glm::vec3 angular_velocity = glm::vec3(0);
  Collider* collider = nullptr;
};

//
class RigidBody : public Transform
{
 private:
  glm::vec3 m_force{};   // force vector in world space
  glm::vec3 m_torque{};  // torque vector in body space

 public:
  float mass = DEFAULT_RB_MASS;        // rigidbody mass, kg
  glm::vec3 velocity;                  // velocity in world space, m/s
  glm::vec3 angular_velocity;          // object space, (x = roll, y = yaw, z = pitch), rad/s
  glm::mat3 inertia, inverse_inertia;  // inertia tensor
  bool apply_gravity, active;
  Collider* collider;

  RigidBody() : RigidBody({DEFAULT_RB_MASS, DEFAULT_RB_INERTIA}) {}

  RigidBody(const RigidBodyParams& params)
      : Transform(params.position, params.rotation),
        mass(params.mass),
        velocity(params.velocity),
        angular_velocity(params.angular_velocity),
        inertia(params.inertia),
        inverse_inertia(glm::inverse(params.inertia)),
        apply_gravity(true),
        active(true),
        collider(params.collider)
  {
  }

  // get velocity of relative point in body space
  inline glm::vec3 get_point_velocity(const glm::vec3& point) const
  {
    return get_body_velocity() + glm::cross(angular_velocity, point);
  }

  // get velocity of point in world space
  inline glm::vec3 get_point_world_velocity(const glm::vec3& point) const
  {
    glm::vec3 relative_point = inverse_transform_direction(point - position);
    return transform_direction(get_point_velocity(relative_point));
  }

  // get velocity in body space
  inline glm::vec3 get_body_velocity() const { return inverse_transform_direction(velocity); }

  // self explantory
  inline float get_inverse_mass() const { return 1.0f / mass; };

  // force and point vectors are in body space
  inline void add_force_at_point(const glm::vec3& force, const glm::vec3& relative_point)
  {
    add_relative_force(force);
    add_relative_torque(glm::cross(relative_point, force));
  }

  // impulse and point vectors are in body space
  inline void add_impulse_at_point(const glm::vec3& impulse, const glm::vec3& relative_point)
  {
    add_relative_impulse(impulse);
    add_relative_angular_impulse(glm::cross(relative_point, impulse));
  }

  // force and point vectors are in world space
  inline void add_force_at_world_point(const glm::vec3& force, const glm::vec3& point)
  {
    add_force(force);
    add_torque(glm::cross((point - position), force));
  }

  // impulse and point vectors are in world space
  inline void add_impulse_at_world_point(const glm::vec3& impulse, const glm::vec3& point)
  {
    add_impulse(impulse);
    add_angular_impulse(glm::cross((point - position), impulse));
  }

  // set inertia tensor
  inline void set_inertia(const glm::mat3& tensor) { inertia = tensor, inverse_inertia = glm::inverse(tensor); }

  // set inertia tensor from moment of inertia
  inline void set_inertia(const glm::vec3& moment_of_inertia) { set_inertia(inertia::tensor(moment_of_inertia)); }

  // linear impulse in world space
  inline void add_impulse(const glm::vec3& impulse)
  {
    if (active) {
      velocity += impulse / mass;
    }
  }

  // linear impulse in body space
  inline void add_relative_impulse(const glm::vec3& impulse)
  {
    if (active) {
      velocity += transform_direction(impulse) / mass;
    }
  }

  // angular impulse in world space
  inline void add_angular_impulse(const glm::vec3& impulse)
  {
    if (active) {
      angular_velocity += inverse_transform_direction(impulse) * inverse_inertia;
    }
  }

  // angular impulse in body space
  inline void add_relative_angular_impulse(const glm::vec3& impulse)
  {
    if (active) {
      angular_velocity += impulse * inverse_inertia;
    }
  }

  // force vector in world space
  inline void add_force(const glm::vec3& force) { m_force += force; }

  // force vector in body space
  inline void add_relative_force(const glm::vec3& force) { m_force += transform_direction(force); }

  // torque vector in world space
  inline void add_torque(const glm::vec3& torque) { m_torque += inverse_transform_direction(torque); }

  // torque vector in body space
  inline void add_relative_torque(const glm::vec3& torque) { m_torque += torque; }

  // get transform
  inline Transform get_transform() const { return {position, rotation}; }

  // get speed
  inline float get_speed() const { return glm::length(velocity); }

  // get euler angles in radians
  inline glm::vec3 get_euler_angles() const { return glm::eulerAngles(rotation); }

  // get torque in body space
  inline glm::vec3 get_torque() const { return m_torque; }

  // get torque in world space
  inline glm::vec3 get_force() const { return m_force; }

  // get inverse inertia tensor in world space
  inline glm::mat3 get_rotated_inverse_inertia() const
  {
    return glm::mat3(rotation) * inverse_inertia * glm::mat3(glm::inverse(rotation));
  }

  // integrate RigidBody
  virtual void update(phi::Seconds dt)
  {
    if (active) {
      glm::vec3 acceleration = m_force / mass;

      if (apply_gravity) acceleration.y -= EARTH_GRAVITY;

      velocity += acceleration * dt;
      position += velocity * dt;

      angular_velocity += inverse_inertia * (m_torque - glm::cross(angular_velocity, inertia * angular_velocity)) * dt;
      rotation += (rotation * glm::quat(0.0f, angular_velocity)) * (0.5f * dt);
      rotation = glm::normalize(rotation);
    }

    // reset accumulators
    m_force = glm::vec3(0.0f), m_torque = glm::vec3(0.0f);
  }

  // impulse collision response without angular effects
  static void linear_impulse_collision(const CollisionInfo& collision)
  {
    RigidBody *a = collision.a, *b = collision.b;

    glm::vec3 relative_velocity = b->velocity - a->velocity;

    float total_mass = a->mass + b->mass;
    float total_inverse_mass = a->get_inverse_mass() + b->get_inverse_mass();

    // move objects so they are no longer colliding. heavier object gets moved less
    a->position -= collision.normal * collision.penetration * (b->mass / total_mass);
    b->position += collision.normal * collision.penetration * (a->mass / total_mass);

    // magnitude of impulse
    float impulse_force = glm::dot(glm::normalize(relative_velocity), collision.normal);
    float j = (-(1.0f + collision.restitution_coeff) * impulse_force) / (total_inverse_mass);

    glm::vec3 impulse = j * collision.normal;

    // apply linear impulse
    a->add_impulse(-impulse);
    b->add_impulse(+impulse);
  }

  // impulse collision response with angular effects
  static void impulse_collision(const CollisionInfo& collision)
  {
    RigidBody *a = collision.a, *b = collision.b;

    float total_mass = a->mass + b->mass;
    float total_inverse_mass = a->get_inverse_mass() + b->get_inverse_mass();

    // move objects so they are no longer colliding. heavier object gets moved less
    a->position -= collision.normal * collision.penetration * (b->mass / total_mass);
    b->position += collision.normal * collision.penetration * (a->mass / total_mass);

    // location of collision point relative to rigidbody
    glm::vec3 a_relative = collision.point - a->position;
    glm::vec3 b_relative = collision.point - b->position;

    // get velocity at collision point
    glm::vec3 a_velocity = a->get_point_world_velocity(collision.point);
    glm::vec3 b_velocity = b->get_point_world_velocity(collision.point);

    // relative velocity in world space
    glm::vec3 relative_velocity = b_velocity - a_velocity;

    // force is highest in a head on collision
    float impulse_force = glm::dot(glm::normalize(relative_velocity), collision.normal);

    glm::mat3 a_tensor = a->get_rotated_inverse_inertia();
    glm::mat3 b_tensor = b->get_rotated_inverse_inertia();

    glm::vec3 a_inertia = glm::cross(a_tensor * glm::cross(a_relative, collision.normal), a_relative);
    glm::vec3 b_inertia = glm::cross(b_tensor * glm::cross(b_relative, collision.normal), b_relative);

    float angular_effect = glm::dot(a_inertia + b_inertia, collision.normal);

    float j = (-(1 + collision.restitution_coeff) * impulse_force) / (total_inverse_mass + angular_effect);

    glm::vec3 impulse = j * collision.normal;

    a->add_impulse_at_world_point(-impulse, collision.point);
    b->add_impulse_at_world_point(+impulse, collision.point);
  }
};

template <typename RB>
void step_physics(std::vector<RB>& objects, phi::Seconds dt)
{
  for (RB& object : objects) {
    object.update(dt);
  }
}

};  // namespace phi

// debug print
std::ostream& operator<<(std::ostream& os, const phi::RigidBody& rb)
{
  return os << "RigidBody { p = " << rb.position << ", r = " << rb.get_euler_angles() << ", v = " << rb.velocity
            << ", av = " << rb.angular_velocity << " }";
}
