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

namespace col
{

constexpr float EPSILON = 1e-8f;

template <typename T>
constexpr inline T sq(T x)
{
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
  Sphere(const glm::vec3& center, float radius) : center(center), radius(radius) {}
};

// ray = origin + direction * t
struct Ray : public Collider {
  glm::vec3 origin, direction;
  Ray(const glm::vec3& origin, const glm::vec3& direction) : origin(origin), direction(direction) {}
};

struct Heightmap {
  const char* heightmap_data;
  float scale = 3000.0f, shift = 0.0f, magnification = 1.0f;

  float get_height(const glm::vec2& coords) const
  {
    auto uv_coord = glm::clamp(coords / magnification, glm::vec2(-1.0f), glm::vec2(1.0f));
    float sampled_value = 0.0f;

    // TODO: map uv to [0, 1]
    // TODO: sample and interpolate height values

    return scale * sampled_value + shift;
  }
};

// test collision between a ray and a sphere
bool test_collision(const Ray& r, const Sphere& s, float* t)
{
  // Christer_Ericson-Real-Time_Collision_Detection.pdf#page=178
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
bool test_collision(const Sphere& s0, const Sphere& s1)
{
  float distance = glm::length(s0.center - s1.center);
  float radius_sum = s0.radius + s1.radius;
  return distance < radius_sum;
}

// test collision between two axis aligned bounding boxes
bool test_collision(const AABB& a, const AABB& b)
{
  auto a_min = a.min(), a_max = a.max();
  auto b_min = b.min(), b_max = b.max();
  return ((a_max.x < b_min.x || a_min.x > b_max.x) && (a_max.y < b_min.y || a_min.y > b_max.y) &&
          (a_max.z < b_min.z || a_min.z > b_max.z));
}

bool test_collision(const Heightmap& heightmap, const glm::vec3& point)
{
  float height = heightmap.get_height({point.x, point.z});
  return point.y <= height;
}

// test collision of two moving spheres
bool test_moving_collision(const Sphere& s0, const glm::vec3& velocity0, const Sphere& s1, const glm::vec3& velocity1,
                           float* t = nullptr)
{
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
  // Christer_Ericson-Real-Time_Collision_Detection.pdf#page=226
  float tmp_t = 0.0f;
  auto v = velocity0 - velocity1;
  auto vlen = glm::length(v);

  Ray ray(s0.center, v / vlen);
  Sphere sphere(s1.center, s0.radius + s1.radius);

  if (test_collision(ray, sphere, &tmp_t)) {
    if (t != nullptr) {
      *t = tmp_t;
    }
    return tmp_t <= vlen;
  } else {
    return false;
  }
#endif
}
};  // namespace col
