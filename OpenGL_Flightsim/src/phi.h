/*
  Version: v0.1

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
  SOFTWARE.
*/
#pragma once

#include <glm/glm.hpp>
#include <numeric>
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
  
class RigidBody;
  
struct Collider;  
struct Plane;
struct OBB;


// constants
constexpr float EPSILON       = 1e-8f;
constexpr float EARTH_GRAVITY = 9.80665f;
constexpr float PI            = 3.141592653589793f;

// directions in body space
constexpr glm::vec3 X_AXIS = {1.0f, 0.0f, 0.0f};
constexpr glm::vec3 Y_AXIS = {0.0f, 1.0f, 0.0f};
constexpr glm::vec3 Z_AXIS = {0.0f, 0.0f, 1.0f};

constexpr glm::vec3 FORWARD  = X_AXIS;
constexpr glm::vec3 UP       = Y_AXIS;
constexpr glm::vec3 RIGHT    = Z_AXIS;
constexpr glm::vec3 BACKWARD = -X_AXIS;
constexpr glm::vec3 DOWN     = -Y_AXIS;
constexpr glm::vec3 LEFT     = -Z_AXIS;

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
// mass element used for inertia tensor calculation
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

// cuboid with side length 'size'
constexpr glm::vec3 cuboid(const glm::vec3& size, float mass)
{
  const float C = (1.0f / 12.0f) * mass;
  return glm::vec3(sq(size.y) + sq(size.z), sq(size.x) + sq(size.z), sq(size.x) + sq(size.y)) * C;
}

// cylinder oriented along the x direction
constexpr glm::vec3 cylinder(float radius, float length, float mass)
{
  const float C = (1.0f / 12.0f) * mass;
  glm::vec3 I(0.0f);
  I.x = (0.5f) * mass * sq(radius);
  I.y = I.z = C * (3.0f * sq(radius) + sq(length));
  return I;
}

// helper function for the creation of a cuboid mass element
constexpr Element cube(const glm::vec3& position, const glm::vec3& size, float mass = 0.0f)
{
  return {size, position, cuboid(size, mass), position, mass};
}

// inertia tensor from moment of inertia
constexpr glm::mat3 tensor(const glm::vec3& moment_of_inertia)
{
  // clang-format off
  return {
      moment_of_inertia.x, 0.0f, 0.0f, 
      0.0f, moment_of_inertia.y, 0.0f, 
      0.0f, 0.0f, moment_of_inertia.z,
  };
  // clang-format on
}

// distribute mass among elements depending on element volume, to be called before passing elements to tensor()
void compute_uniform_mass(std::vector<Element>& elements, float total_mass)
{
  auto f             = [](float s, auto& e) { return s + e.volume(); };
  float total_volume = std::accumulate(elements.begin(), elements.end(), 0.0f, f);

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

  // clang-format off
  return {
      Ixx, -Ixy, -Ixz, 
      -Ixy, Iyy, -Iyz, 
      -Ixz, -Iyz, Izz
  };
  // clang-format on
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

struct CollisionInfo {
  glm::vec3 point;
  glm::vec3 normal;
  float penetration;
  RigidBody *a, *b;
};
#if 0

namespace collision 
{




  
namespace primitive {
  bool test(const Plane* plane, const OBB* obb) {
    return false;
  } 
};

struct Collider 
{
  
  virtual void update(const RigidBody* rb) = 0;
  virtual bool test_collision(const Collider* other) const = 0;
  virtual bool test_collision(const OBB* other) const = 0;
  virtual bool test_collision(const Plane* other) const = 0;
  //virtual bool test_collision(const Sphere* other) const = 0;
  
   

};






struct Plane : public Collider 
{
  glm::vec3 origin, normal;
  
  void update(const RigidBody* rb) override 
  {
    origin = rb->position;
  } 
  
  bool test_collision(const Collider *other) const override
  {  return other->test_collision(this);} 
  
  bool test_collision(const OBB* other) const override
  { 
    //return primitive::test(this, other);
    return false;
  } 
};

 




  





struct OBB : public Collider
{
  glm::vec3 origin, size;
  glm::quat orientation;
  
  void update(const RigidBody *rb) override 
  {
    origin      = rb->position;
    orientation = rb->orientation;
  } 
  
  bool test_collision(const Collider* other) const override {
    return other->test_collision(this);
  }
  
  bool test_collision(const Plane* other) const override 
  { 
    //return primitive::test(other, this);
    return false;
  } 

};

template <typename RB>
std::vector<CollisionInfo> narrowphase(std::vector<RB>& objects, phi::Seconds dt) 
{
  std::vector<CollisionInfo> collisions;
  
  for(int i = 0; i < objects.size(); i++) 
  {
    for(int j = i + 1; j < objects.size(); j++)
    {
      if(objects[i].collider && objects[j].collider)
      {
        auto a = objects[i].collider;
        auto b = objects[j].collider;
        
        // test for collison
        if(a->test_collision(b))
        {
          // TODO
        } 
      } 
    } 
  }
  
  return collisions;
} 

void resolve(std::vector<CollisionInfo>& collisions) 
{
  for(auto& collision : collisions) 
  {
    // TODO
  } 
} 

};

#endif

// default rigid body is a sphere with radius 1 meter and a mass of 100 kg
constexpr float DEFAULT_RB_MASS            = 100.0f;
constexpr float INFINITE_RB_MASS            = std::numeric_limits<float>::infinity();
constexpr glm::mat3 DEFAULT_RB_INERTIA     = inertia::tensor(inertia::sphere(1.0f, DEFAULT_RB_MASS));
constexpr glm::quat DEFAULT_RB_ORIENTATION = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

struct RigidBodyParams {
  float mass        = DEFAULT_RB_MASS;
  glm::mat3 inertia = DEFAULT_RB_INERTIA;
  glm::vec3 position{};
  glm::vec3 velocity{};
  glm::vec3 angular_velocity{};
  glm::quat orientation = DEFAULT_RB_ORIENTATION;
  bool apply_gravity    = true;
  Collider *collider    = nullptr;
};

class RigidBody
{
 private:
  glm::vec3 m_force{};   // force vector in world space
  glm::vec3 m_torque{};  // torque vector in body space

 public:
  float mass;                              // rigidbody mass, kg
  glm::vec3 position{};                    // position in world space, m
  glm::quat orientation{};                 // orientation in world space
  glm::vec3 velocity{};                    // velocity in world space, m/s
  glm::vec3 angular_velocity{};            // angular velocity in object space, (x = roll, y = yaw, z = pitch), rad/s
  glm::mat3 inertia{}, inverse_inertia{};  // inertia tensor
  bool apply_gravity = true;
  bool active        = true;
  Collider* collider = nullptr;

  RigidBody() : RigidBody({DEFAULT_RB_MASS, DEFAULT_RB_INERTIA}) {}

  RigidBody(const RigidBodyParams& params)
      : mass(params.mass),
        position(params.position),
        velocity(params.velocity),
        inertia(params.inertia),
        orientation(params.orientation),
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

  // add terrain friction
  glm::vec3 add_friction(const glm::vec3& normal, const glm::vec3& sliding_direction, float friction_coeff)
  {
    // https://en.wikipedia.org/wiki/Normal_force
    // https://en.wikipedia.org/wiki/Friction
    float weight      = mass * EARTH_GRAVITY;
    auto normal_force = weight * std::max(glm::dot(normal, UP), 0.0f);
    return -sliding_direction * normal_force * friction_coeff;
  }

  // get speed
  inline float get_speed() const { return glm::length(velocity); }

  // get euler angles in radians
  inline glm::vec3 get_euler_angles() const { return glm::eulerAngles(orientation); }

  // get torque in body space
  inline glm::vec3 get_torque() const { return m_torque; }

  // get torque in world space
  inline glm::vec3 get_force() const { return m_force; }

  // get rigidbody forward direction in world space
  inline glm::vec3 forward() const { return transform_direction(phi::FORWARD); }

  // get rigidbody up direction in world space
  inline glm::vec3 up() const { return transform_direction(phi::UP); }

  // get rigidbody right direction in world space
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
#if 0
    if(collider != nullptr) {
      // update collider transform
      collider->update(this);
    } 
#endif
    // reset accumulators
    m_force = glm::vec3(0.0f), m_torque = glm::vec3(0.0f);
  }

  // restitution_coeff:  0 = perfectly inelastic, 1 = perfectly elastic
  // impulse collision response without angular effects
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

  // impulse collision response
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

    auto a_inertia         = glm::cross(a->inertia * glm::cross(a_relative, collision.normal), a_relative);
    auto b_inertia         = glm::cross(b->inertia * glm::cross(b_relative, collision.normal), b_relative);
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







  

  
   
bool collision_primitive(const Plane *plane, const OBB *obb);
  
  



struct Collider 
{
  
  virtual void update(const RigidBody* rb) = 0;
  virtual bool test_collision(const Collider* other) 
  {
    return other->test_collision(this) ;
  }
  
  virtual bool test_collision(const OBB* other) const = 0;
  virtual bool test_collision(const Plane* other) const = 0;
  virtual bool test_collision(const Heightmap* hm) const = 0;

  
   

};






struct Plane : public Collider 
{
  glm::vec3 origin, normal;
  
  void update(const RigidBody* rb) override 
  {
    origin = rb->position;
  } 
  
  bool test_collision(const Collider *other) const override
  {  
    return other->test_collision(this);
  } 
  
  bool test_collision(const OBB* other) const override
  { 
    return collision_primitive(this, other);
  } 
};

 

struct Heightmap : public Collider{
  const uint8_t* data;
  const int width, height, channels;
  const float scale = 3000.0f, shift = 0.0f;

  float magnification = 25000.0f;

  Heightmap(const uint8_t* data, int width, int height, int channels)
      : data(data), width(width), height(height), channels(channels)
  {
  }

  glm::vec3 sample(const glm::vec2& coord) const
  {
    assert(0.0f <= coord.x && coord.x <= 1.0f);
    assert(0.0f <= coord.y && coord.y <= 1.0f);

    int x = static_cast<int>(width * coord.x);
    int y = static_cast<int>(height * coord.y);

    int index = (y * width + x) * channels;

    auto color = glm::vec3{static_cast<float>(data[index + 0]), static_cast<float>(data[index + 1]),
                           static_cast<float>(data[index + 2])};
    color /= 255.0f;
    return color;
  }

  Plane get_plane(const glm::vec2& coord) const
  {
    glm::vec2 tmp = glm::clamp(coord / magnification, glm::vec2(-1.0f), glm::vec2(1.0f));
    auto uv       = phi::scale(tmp, glm::vec2(-1.0f), glm::vec2(1.0f), glm::vec2(0.0f), glm::vec2(1.0f));
    float value   = sample(uv).r;
    float height  = scale * value + shift;
    return { glm::vec3(coord.x, height, coord.y), phi::UP };
  }
  
  bool test_collision(const OBB* obb) override
  {
    auto plane = get_plane({ obb->origin.x, obb->origin.z});
    return collison_primitive(&plane, obb);
  } 
  
}


  





struct OBB : public Collider
{
  glm::vec3 origin, size;
  glm::quat orientation;
  
  void update(const RigidBody *rb) override 
  {
    origin      = rb->position;
    orientation = rb->orientation;
  } 
  
  bool test_collision(const Collider* other) const override 
  {
    return other->test_collision(this);
  }
  
  bool test_collision(const Plane* other) const override 
  { 
    return collision_primitive(other, this);
  } 

};
  

bool collision_primitive(const Plane* plane, const OBB* obb) {
  return false;
} 


    



template <typename RB>
std::vector<CollisionInfo> collision_narrowphase(std::vector<RB>& objects, phi::Seconds dt) 
{
  std::vector<CollisionInfo> collisions;
  
  for(int i = 0; i < objects.size(); i++) 
  {
    for(int j = i + 1; j < objects.size(); j++)
    {
      if(objects[i].collider && objects[j].collider)
      {
        auto a = objects[i].collider;
        auto b = objects[j].collider;
        
        // test for collison
        if(a->test_collision(b))
        {
          // TODO
          CollisionInfo info;
          info.a = a;
          info.b = b;
          collisions.push_back(info);
        } 
      } 
    } 
  }
  
  return collisions;
} 

void collision_resolution(std::vector<CollisionInfo>& collisions) 
{
  for(auto& c : collisions) 
  {

    phi::RigidBody::impulse_collision_response(c.a, c.b, c);
  } 
} 



  
struct ForceGenerator {
  virtual void apply_forces(phi::RigidBody* rigid_body, phi::Seconds dt) = 0;
};
  
struct Joint {
  phi::RigidBody *a, *b;
  virtual void update(phi::Seconds dt) = 0;
};

template <typename RB>
void step_physics(std::vector<RB>& objects, phi::Seconds dt) 
{
  // update
  for(auto& object : objects) 
  {
    object.update(dt);
    if(object.collider != nullptr) {
      object.collider->update(&object);
    } 
  } 
  
  // collision detection
  auto collisions = collision_narrowphase(objects, dt);
  
  // collision resolution 
  if(collisions.size() > 0)
  {
    collision_resolution(collisions);
  } 
  
} 

};  // namespace phi
