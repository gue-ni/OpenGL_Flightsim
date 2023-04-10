/*
    copyright (c) 2023 jakob maier
    'phi.h' is a simple, header-only rigidbody physics library.
*/
#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <vector>

// RigidBody::update() can be marked virtual
#if 0
#define RB_VIRTUAL_UPDATE
#else
#define RB_VIRTUAL_UPDATE virtual
#endif

namespace phi
{

typedef float Seconds;
typedef float Radians;
typedef float Degrees;

// constants
constexpr float EPSILON = 1e-8f;
constexpr float EARTH_GRAVITY = 9.80665f;
constexpr float PI = 3.141592653589793f;

// directions in body space
constexpr glm::vec3 UP = {0.0f, 1.0f, 0.0f};
constexpr glm::vec3 DOWN = {0.0f, -1.0f, 0.0f};
constexpr glm::vec3 RIGHT = {0.0f, 0.0f, 1.0f};
constexpr glm::vec3 LEFT = {0.0f, 0.0f, -1.0f};
constexpr glm::vec3 FORWARD = {1.0f, 0.0f, 0.0f};
constexpr glm::vec3 BACKWARD = {-1.0f, 0.0f, 0.0f};

constexpr glm::vec3 X_AXIS = {1.0f, 0.0f, 0.0f};
constexpr glm::vec3 Y_AXIS = {0.0f, 1.0f, 0.0f};
constexpr glm::vec3 Z_AXIS = {0.0f, 0.0f, 1.0f};

// utility functions
template <typename T>
constexpr inline T sq(T a)
{
  return a * a;
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

template <typename T>
inline T move_towards(T current, T target, T speed)
{
  if (std::abs(target - current) <= speed) {
    return target;
  }
  return current + glm::sign(target - current) * speed;
}

// inertia tensor calculations
// formulas according to https://en.wikipedia.org/wiki/List_of_moments_of_inertia
namespace inertia
{
struct Element {
  glm::vec3 size;
  glm::vec3 position;  // position in design coordinates
  glm::vec3 inertia;
  glm::vec3 offset;  // offset from center of gravity
  float mass;

  float volume() const { return size.x * size.y * size.z; }
};

// solid sphere
constexpr glm::vec3 sphere(float radius, float mass) { return glm::vec3((2.0f / 5.0f) * mass * sq(radius)); }

// cube with side length 'size'
constexpr glm::vec3 cube(float size, float mass) { return glm::vec3((1.0f / 6.0f) * mass * sq(size)); }

constexpr glm::vec3 cuboid(const glm::vec3& size, float mass)
{
  const float C = (1.0f / 12.0f) * mass;
  return glm::vec3(sq(size.y) + sq(size.z), sq(size.x) + sq(size.z), sq(size.x) + sq(size.y)) * C;
}

constexpr glm::vec3 cylinder(float radius, float length, float mass)
{
  const float C = (1.0f / 12.0f) * mass;
  glm::vec3 I(0.0f);
  I.x = (0.5f) * mass * sq(radius);
  I.y = I.z = C * (3.0f * sq(radius) + sq(length));
  return I;
}

// inertia tensor from moment of inertia
constexpr glm::mat3 tensor(const glm::vec3& moment_of_inertia)
{
  return {
      moment_of_inertia.x, 0.0f, 0.0f, 0.0f, moment_of_inertia.y, 0.0f, 0.0f, 0.0f, moment_of_inertia.z,
  };
}

// helper function for the creation of a cuboid mass element
constexpr Element cube(const glm::vec3& position, const glm::vec3& size, float mass)
{
  return {size, position, cuboid(size, mass), position, mass};
}

// distribute mass among elements depending on element volume
void compute_uniform_mass(std::vector<Element>& elements, float total_mass)
{
  float total_volume = 0.0f;
  for (const auto& element : elements) {
    total_volume += element.volume();
  }

  for (auto& element : elements) {
    element.mass = (element.volume() / total_volume) * total_mass;
  }
}

// calculate inertia tensor for a collection of connected masses
glm::mat3 tensor(std::vector<Element>& elements, bool precomputed_offset = false, glm::vec3* cg = nullptr)
{
  float Ixx = 0, Iyy = 0, Izz = 0;
  float Ixy = 0, Ixz = 0, Iyz = 0;

  float mass = 0;
  glm::vec3 moment(0.0f);

  for (const auto& element : elements) {
    mass += element.mass;
    moment += element.mass * element.position;
  }

  const auto center_of_gravity = moment / mass;

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

  return {Ixx, -Ixy, -Ixz, -Ixy, Iyy, -Iyz, -Ixz, -Iyz, Izz};
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
};  // namespace units

struct CollisionInfo {
  glm::vec3 point;
  glm::vec3 normal;
  float penetration;
};

// default rigid body is a sphere with radius 1m and a mass of 10kg
constexpr float DEFAULT_RB_MASS = 10.0f;
constexpr glm::mat3 DEFAULT_RB_INERTIA = inertia::tensor(inertia::sphere(1.0f, DEFAULT_RB_MASS));
constexpr glm::quat DEFAULT_RB_ORIENTATION = glm::quat{0.0f, 0.0f, 0.0f, 1.0f};

struct RigidBodyParams {
  float mass = DEFAULT_RB_MASS;
  glm::mat3 inertia = DEFAULT_RB_INERTIA;
  glm::vec3 position{};
  glm::vec3 velocity{};
  glm::vec3 angular_velocity{};
  glm::quat orientation = DEFAULT_RB_ORIENTATION;
  bool apply_gravity = true;
};

class RigidBody
{
 private:
  glm::vec3 m_force{};   // force vector in world space
  glm::vec3 m_torque{};  // torque vector in body space

 public:
  float mass;                              // rigidbody mass in kg
  glm::vec3 position{};                    // position in world space
  glm::quat orientation{};                 // orientation in world space
  glm::vec3 velocity{};                    // velocity in world space
  glm::vec3 angular_velocity{};            // angular velocity in object space, x
                                           // represents rotation around x axis
  glm::mat3 inertia{}, inverse_inertia{};  // inertia tensor
  bool apply_gravity = true;
  bool active = true;

  RigidBody() : RigidBody({DEFAULT_RB_MASS, DEFAULT_RB_INERTIA}) {}

  RigidBody(const RigidBodyParams& params)
      : mass(params.mass),
        position(params.position),
        velocity(params.velocity),
        inertia(params.inertia),
        orientation(params.orientation),
        apply_gravity(params.apply_gravity),
        angular_velocity(params.angular_velocity),
        inverse_inertia(glm::inverse(params.inertia))
  {
  }

  // get angular velocity of relative point in body space
  inline glm::vec3 get_point_angular_velocity(const glm::vec3& point) const
  {
    return glm::cross(angular_velocity, point);
  }

  // get velocity of relative point in body space
  inline glm::vec3 get_point_velocity(const glm::vec3& point) const
  {
    return inverse_transform_direction(velocity) + get_point_angular_velocity(point);
  }

  // get velocity in body space
  inline glm::vec3 get_body_velocity() const { return inverse_transform_direction(velocity); }

  inline float get_inverse_mass() const { return 1.0f / mass; };

  // force and point vectors are in body space
  inline void add_force_at_point(const glm::vec3& force, const glm::vec3& point)
  {
    m_force += transform_direction(force);
    m_torque += glm::cross(point, force);
  }

  // transform direction from body space to world space
  inline glm::vec3 transform_direction(const glm::vec3& direction) const { return orientation * direction; }

  // transform direction from world space to body space
  inline glm::vec3 inverse_transform_direction(const glm::vec3& direction) const
  {
    return glm::inverse(orientation) * direction;
  }

  // set inertia tensor
  inline void set_inertia(const glm::mat3& tensor) { inertia = tensor, inverse_inertia = glm::inverse(tensor); }

  // set moment of inertia
  inline void set_inertia(const glm::vec3& moment) { set_inertia(inertia::tensor(moment)); }

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

  // get speed
  inline float get_speed() const { return glm::length(velocity); }

  // get torque in body space
  inline glm::vec3 get_torque() const { return m_torque; }

  // get torque in world space
  inline glm::vec3 get_force() const { return m_force; }

  // get forward direction in world space
  inline glm::vec3 forward() const { return transform_direction(phi::FORWARD); }

  // get up direction in world space
  inline glm::vec3 up() const { return transform_direction(phi::UP); }

  // get right direction in world space
  inline glm::vec3 right() const { return transform_direction(phi::RIGHT); }

  // integrate RigidBody
  RB_VIRTUAL_UPDATE void update(Seconds dt)
  {
    if (!active) return;

    glm::vec3 acceleration = m_force / mass;

    if (apply_gravity) acceleration.y -= EARTH_GRAVITY;

    velocity += acceleration * dt;
    position += velocity * dt;

    angular_velocity += inverse_inertia * (m_torque - glm::cross(angular_velocity, inertia * angular_velocity)) * dt;
    orientation += (orientation * glm::quat(0.0f, angular_velocity)) * (0.5f * dt);
    orientation = glm::normalize(orientation);

    // reset accumulators
    m_force = glm::vec3(0.0f), m_torque = glm::vec3(0.0f);
  }

  // no angular effects
  // restitution:  0 = perfectly inelastic, 1 = perfectly elastic
  static void linear_impulse_collision_response(RigidBody* a, RigidBody* b, const CollisionInfo& collision,
                                                float restitution_coeff = 0.66f)
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

  // simple impulse collision response
  // restitution:  0 = perfectly inelastic, 1 = perfectly elastic
  static void impulse_collision_response(RigidBody* a, RigidBody* b, const CollisionInfo& collision,
                                         float restitution_coeff = 0.66f)
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

struct ForceGenerator {
  virtual void apply_forces(phi::RigidBody* rigid_body, phi::Seconds dt) = 0;
};

};  // namespace phi