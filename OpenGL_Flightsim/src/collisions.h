/*
    Algorithms based on Christer_Ericson's Real-Time Collision Detection
*/
#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <tuple>

#define RUN_COLLISION_UNITTESTS 0

namespace collisions {

constexpr float EPSILON = 1e-8f;

template <typename T>
constexpr inline T sq(T x) {
  return x * x;
}

typedef void CollisionCallback(const glm::vec3& point, const glm::vec3& normal);

struct Collider {
  CollisionCallback* on_collision = nullptr;
};

// axis aligned bounding box
struct AABB : public Collider {
  glm::vec3 center, size;

  glm::vec3 min() const { return center - size / 2.0f; }

  glm::vec3 max() const { return center + size / 2.0f; }
};

// bounding sphere
struct Sphere : public Collider {
  glm::vec3 center;
  float radius;
};

// ray = origin + direction * t
struct Ray : public Collider {
  glm::vec3 origin, direction;
};

// test collision between a ray and a sphere
bool test_collision(const Ray& r, const Sphere& s, float* t) {
  // page 178
  assert(std::abs(glm::length(r.direction) - 1.0f) < EPSILON);

  auto m = r.origin - s.center;
  auto b = glm::dot(m, r.direction);
  auto c = glm::dot(m, m) - sq(s.radius);

  if (c > 0.0f && b > 0.0f) return false;

  auto discr = sq(b) - c;

  if (discr < 0.0f) return false;

  *t = std::max(-b - std::sqrt(discr), 0.0f);
  return true;
}

// test collision between two spheres
bool test_collision(const Sphere& s0, const Sphere& s1) {
  return glm::length(s0.center - s1.center) < (s0.radius + s1.radius);
}

// test collision between two axis aligned bounding boxes
bool test_collision(const AABB& a, const AABB& b) {
  auto a_min = a.min(), a_max = a.max();
  auto b_min = b.min(), b_max = b.max();
  return ((a_max.x < b_min.x || a_min.x > b_max.x) &&
          (a_max.y < b_min.y || a_min.y > b_max.y) &&
          (a_max.z < b_min.z || a_min.z > b_max.z));
}

// test collision of two moving spheres
bool test_moving_collision(const Sphere& s0, const glm::vec3& velocity0,
                           const Sphere& s1, const glm::vec3& velocity1,
                           float* t) {
  // Christer_Ericson-Real-Time_Collision_Detection.pdf#page=264
#if 0
    auto direction = s1.center - s0.center;
    auto radius_sum = s0.radius + s1.radius;
    auto relative_velocity = velocity1 - velocity0;

    float c = glm::dot(direction, direction) - sq(radius_sum);

    if (c < 0.0f) {
        *t = 0.0f;
        return true; // spheres are intersecting
    }

    float a = glm::dot(relative_velocity, relative_velocity);

    if (a < EPSILON)
        return false; // spheres not moving relative to each other

    float b = glm::dot(relative_velocity, direction);

    if (b >= 0.0f)
        return false; // spheres moving away from each other

    float d = sq(b) - a * c;

    if (d < 0.0f)
        return false; // spheres are not intersecting

    *t = (-b - std::sqrt(d)) / a;
    return true;
#else
  // page 226
  auto v = velocity0 - velocity1;
  auto vlen = glm::length(v);

  Ray ray = {.origin = s0.center, .direction = v / vlen};
  Sphere sphere = {.center = s1.center, .radius = s0.radius + s1.radius};

  if (test_collision(ray, sphere, t))
    return *t <= vlen;
  else
    return false;
#endif
}
};  // namespace collisions
