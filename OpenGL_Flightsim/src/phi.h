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

// RigidBody::update() can be marked virtual
#if 0
#define RB_VIRTUAL_UPDATE
#else
#define RB_VIRTUAL_UPDATE virtual
#endif

#define DISABLE 0
#define ENABLE  1

#define COLLIDERS DISABLE

namespace phi
{

typedef float Seconds;

class RigidBody;
struct Collider;

// constants
constexpr float EPSILON = 1e-8f;
constexpr float EARTH_GRAVITY = 9.80665f;
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

struct Transform {
  glm::vec3 position;
  glm::quat rotation;

  constexpr Transform() : position(0.0f), rotation(1.0f, 0.0f, 0.0f, 0.0f) {}
  constexpr Transform(const glm::vec3& p, const glm::quat& o) : position(p), rotation(o) {}

  // get transform matrix
  glm::mat4 matrix() const { return glm::translate(glm::mat4(1.0f), position) * glm::mat4(rotation); }

  // transform direction from body space to world space
  inline glm::vec3 transform_direction(const glm::vec3& direction) const { return rotation * direction; }

  // transform direction from world space to body space
  inline glm::vec3 inverse_transform_direction(const glm::vec3& direction) const
  {
    return glm::inverse(rotation) * direction;
  }

  // get body direction in world space
  inline glm::vec3 up() const { return transform_direction(phi::UP); }
  inline glm::vec3 right() const { return transform_direction(phi::RIGHT); }
  inline glm::vec3 forward() const { return transform_direction(phi::FORWARD); }
};

struct CollisionInfo {
  glm::vec3 point;    // contact point
  glm::vec3 normal;   // contact normal
  float penetration;  // penetration depth
  RigidBody *a, *b;   // the rigidbodies involved
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
const float INFINITE_RB_MASS = std::numeric_limits<float>::infinity();
const glm::quat DEFAULT_RB_ORIENTATION = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
const glm::mat3 DEFAULT_RB_INERTIA = inertia::tensor(inertia::sphere(DEFAULT_RB_MASS, 1.0f));

struct RigidBodyParams {
  float mass = DEFAULT_RB_MASS;
  glm::mat3 inertia = DEFAULT_RB_INERTIA;
  glm::vec3 position = glm::vec3(0);
  glm::quat rotation = DEFAULT_RB_ORIENTATION;
  glm::vec3 velocity = glm::vec3(0);
  glm::vec3 angular_velocity = glm::vec3(0);
  bool apply_gravity = true;
  Collider* collider = nullptr;
};

class RigidBody : public Transform
{
 private:
  glm::vec3 m_force{};   // force vector in world space
  glm::vec3 m_torque{};  // torque vector in body space

 public:
  float mass = DEFAULT_RB_MASS;                  // rigidbody mass, kg
  glm::vec3 velocity = glm::vec3(0.0f);          // velocity in world space, m/s
  glm::vec3 angular_velocity = glm::vec3(0.0f);  // object space, (x = roll, y = yaw, z = pitch), rad/s
  bool apply_gravity = true;
  bool sleep = false;
  bool detect_collision = true;
  Collider* collider = nullptr;
  glm::mat3 inertia = glm::mat3(0.0f);
  glm::mat3 inverse_inertia = glm::mat3(0.0f);  // inertia tensor

  RigidBody() : RigidBody({DEFAULT_RB_MASS, DEFAULT_RB_INERTIA}) {}

  RigidBody(const RigidBodyParams& params)
      : Transform(params.position, params.rotation),
        mass(params.mass),
        velocity(params.velocity),
        inertia(params.inertia),
        apply_gravity(params.apply_gravity),
        angular_velocity(params.angular_velocity),
        inverse_inertia(glm::inverse(params.inertia)),
        collider(params.collider)
  {
  }

  // get velocity of relative point in body space
  inline glm::vec3 get_point_velocity(const glm::vec3& point) const
  {
    return get_body_velocity() + glm::cross(angular_velocity, point);
  }

  // get velocity in body space
  inline glm::vec3 get_body_velocity() const { return inverse_transform_direction(velocity); }

  // self explantory
  inline float get_inverse_mass() const { return 1.0f / mass; };

  // force and point vectors are in body space
  inline void add_force_at_point(const glm::vec3& force, const glm::vec3& point)
  {
    m_force += transform_direction(force);
    m_torque += glm::cross(point, force);
  }
  // set inertia tensor
  inline void set_inertia(const glm::mat3& tensor) { inertia = tensor, inverse_inertia = glm::inverse(tensor); }

  // set inertia tensor from moment of inertia
  inline void set_inertia(const glm::vec3& moment_of_inertia) { set_inertia(inertia::tensor(moment_of_inertia)); }

  // linear impulse in world space
  inline void add_linear_impulse(const glm::vec3& impulse) { velocity += impulse / mass; }

  // linear impulse in body space
  inline void add_relative_linear_impulse(const glm::vec3& impulse) { velocity += transform_direction(impulse) / mass; }

  // angular impulse in world space
  inline void add_angular_impulse(const glm::vec3& impulse)
  {
    angular_velocity += inverse_transform_direction(impulse) * inverse_inertia;
  }

  // angular impulse in body space
  inline void add_relative_angular_impulse(const glm::vec3& impulse) { angular_velocity += impulse * inverse_inertia; }

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

  // integrate RigidBody
  RB_VIRTUAL_UPDATE void update(phi::Seconds dt)
  {
    if (sleep) return;

    glm::vec3 acceleration = m_force / mass;

    if (apply_gravity) acceleration.y -= EARTH_GRAVITY;

    velocity += acceleration * dt;
    position += velocity * dt;

    angular_velocity += inverse_inertia * (m_torque - glm::cross(angular_velocity, inertia * angular_velocity)) * dt;
    rotation += (rotation * glm::quat(0.0f, angular_velocity)) * (0.5f * dt);
    rotation = glm::normalize(rotation);

    // reset accumulators
    m_force = glm::vec3(0.0f), m_torque = glm::vec3(0.0f);
  }

  // restitution_coeff:  0 = perfectly inelastic, 1 = perfectly elastic
  // impulse collision response without angular effects
  static void linear_impulse_collision_response(RigidBody* a, RigidBody* b, const CollisionInfo& collision,
                                                float restitution_coeff = 0.5f)
  {
    float total_inverse_mass = a->get_inverse_mass() + b->get_inverse_mass();

    // move objects so they are no longer colliding. heavier object gets moved less
    a->position -= collision.normal * collision.penetration * (a->get_inverse_mass() / total_inverse_mass);
    b->position += collision.normal * collision.penetration * (b->get_inverse_mass() / total_inverse_mass);

    auto relative_velocity = b->velocity - a->velocity;

    // force is highest in head on collision
    float impulse_force = glm::dot(relative_velocity, collision.normal);

    // magnitude of impulse
    float j = (-(1 + restitution_coeff) * impulse_force) / (total_inverse_mass);

    auto impulse = j * collision.normal;

    // apply linear impulse
    a->add_linear_impulse(-impulse);
    b->add_linear_impulse(+impulse);
  }

  // impulse collision response
  static void impulse_collision_response(RigidBody* a, RigidBody* b, const CollisionInfo& collision,
                                         float restitution_coeff = 0.5f)
  {
    float total_inverse_mass = a->get_inverse_mass() + b->get_inverse_mass();

    // move objects so they are no longer colliding. heavier object gets moved less
    a->position -= collision.normal * collision.penetration * (a->get_inverse_mass() / total_inverse_mass);
    b->position += collision.normal * collision.penetration * (b->get_inverse_mass() / total_inverse_mass);

    // location of collision point relative to rigidbody
    auto a_relative = collision.point - a->position;
    auto b_relative = collision.point - b->position;

    // get velocity at this point (body space)
    auto a_velocity = a->get_point_velocity(a_relative);
    auto b_velocity = b->get_point_velocity(b_relative);

    // relative velocity in world space
    auto relative_velocity = b->transform_direction(b_velocity) - a->transform_direction(a_velocity);

    // force is highest in a head on collision
    float impulse_force = glm::dot(relative_velocity, collision.normal);

    auto a_inertia = glm::cross(a->inertia * glm::cross(a_relative, collision.normal), a_relative);
    auto b_inertia = glm::cross(b->inertia * glm::cross(b_relative, collision.normal), b_relative);
    float angular_effect_1 = glm::dot(a_inertia + b_inertia, collision.normal);

    float angular_effect_2 =
        glm::dot(collision.normal, glm::cross((glm::cross(a_relative, collision.normal) / a->inertia), a_relative)) +
        glm::dot(collision.normal, glm::cross((glm::cross(b_relative, collision.normal) / b->inertia), b_relative));

    // TODO: find correct implementation
    printf("a_1 = %f, a_2 = %f\n", angular_effect_1, angular_effect_2);
    assert(std::abs(angular_effect_1 - angular_effect_2) < phi::EPSILON);

    // magnitude of impulse
    float j = (-(1 + restitution_coeff) * impulse_force) / (total_inverse_mass + angular_effect_1);

    auto impulse = j * collision.normal;

    // apply linear impulse
    a->add_linear_impulse(-impulse);
    b->add_linear_impulse(+impulse);

    // apply angular impulse at position
    a->add_angular_impulse(glm::cross(a_relative, -impulse));
    b->add_angular_impulse(glm::cross(b_relative, +impulse));
  }
};

struct Collider {
  virtual float volume() const = 0;
  virtual glm::vec3 inertia(float mass) const = 0;
  virtual bool collision(const Transform* t0, const Collider* c1, const Transform* t1) const = 0;
};

#if (COLLIDERS == ENABLE)
struct Sphere : public Collider {
  const float radius;
  Sphere(float radius_) : radius(radius_) {}
  float volume() const override { return (4.0f / 3.0f) * PI * cb(radius); }
  glm::vec3 inertia(float mass) const override { return inertia::sphere(mass, radius); }
  bool collision(const Transform* t0, const Collider* c1, const Transform* t1) const override;
};

struct Cuboid : public Collider {
  const glm::vec3 size;
  Cuboid(const glm::vec3& size_) : size(size_) {}
  float volume() const override { return size.x * size.y * size.z; }
  glm::vec3 inertia(float mass) const override { return inertia::cuboid(mass, size); }
  bool collision(const Transform* t0, const Collider* c1, const Transform* t1) const override;
};

bool Sphere::collision(const Transform* t0, const Collider* c1, const Transform* t1) const
{
  return c1->collision(t1, this, t0);
}

bool Cuboid::collision(const Transform* t0, const Collider* c1, const Transform* t1) const
{
  return c1->collision(t1, this, t0);
}
#endif

// collision detection system
namespace collision
{
// narrowphase collision detection. this algorithm is O(n^2) -> very slow
template <typename RB>
std::vector<CollisionInfo> detection(std::vector<RB>& objects, phi::Seconds dt)
{
  std::vector<CollisionInfo> collisions;

  for (std::size_t i = 0; i < objects.size(); i++) {
    for (std::size_t j = i + 1; j < objects.size(); j++) {
      // TODO: test collision
    }
  }

  return collisions;
}

// resolve collision events
inline void resolution(std::vector<CollisionInfo>& collisions)
{
  for (auto& collision : collisions) {
    phi::RigidBody::impulse_collision_response(collision.a, collision.b, collision);
  }
}

};  // namespace collision

template <typename RB>
void step_physics(std::vector<RB>& objects, phi::Seconds dt)
{
  for (auto& object : objects) {
    object.update(dt);
  }

  auto collisions = collision::detection(objects, dt);

  if (collisions.size() > 0) {
    // std::cout << "found collisions, resolving...\n";
    collision::resolution(collisions);
  } else {
    // std::cout << "found no collisions...\n";
  }
}

};  // namespace phi
