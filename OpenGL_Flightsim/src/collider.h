/*
    Algorithms based on Christer_Ericson's Real-Time Collision Detection
*/
#pragma once

#include <glm/glm.hpp>

#include "phi.h"

namespace collider
{

typedef void CollisionCallback(const glm::vec3& point, const glm::vec3& normal);

struct Collider {
  CollisionCallback* on_collision = nullptr;
};

struct Contact {
  Collider *a, *b;  // the objects in contact
  float restitution_coeff;
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
  Sphere() : Sphere(glm::vec3(0.0f), 1.0f) {}
  Sphere(const glm::vec3& center, float radius) : center(center), radius(radius) {}
};

struct Ray : public Collider {
  glm::vec3 origin, direction;
  Ray() : Ray(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)) {}
  Ray(const glm::vec3& origin, const glm::vec3& direction) : origin(origin), direction(direction) {}
  inline glm::vec3 point_at(float t) const { return origin + direction * t; }
};

struct Heightmap {
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

  float get_height(const glm::vec2& coord) const
  {
    glm::vec2 tmp = glm::clamp(coord / magnification, glm::vec2(-1.0f), glm::vec2(1.0f));
    auto uv = phi::scale(tmp, glm::vec2(-1.0f), glm::vec2(1.0f), glm::vec2(0.0f), glm::vec2(1.0f));
    float value = sample(uv).r;
    float height = scale * value + shift;
    return height;
  }
};

// test collision between a ray and a sphere
bool test_collision(const Ray& r, const Sphere& s, float* t)
{
  // Christer_Ericson-Real-Time_Collision_Detection.pdf#page=178
  auto m = r.origin - s.center;
  auto b = glm::dot(m, r.direction);
  auto c = glm::dot(m, m) - phi::sq(s.radius);

  if (c > 0.0f && b > 0.0f) return false;

  auto discr = phi::sq(b) - c;

  if (discr < 0.0f) return false;

  *t = std::max(-b - std::sqrt(discr), 0.0f);
  return true;
}

// test collision between two spheres
bool test_collision(const Sphere& a, const Sphere& b, phi::CollisionInfo* info)
{
  float distance = glm::length(a.center - b.center);
  float radius_sum = a.radius + b.radius;

  if (distance < radius_sum) {
    info->normal = glm::normalize(b.center - a.center);
    info->penetration = radius_sum - distance;
    info->point = a.center + a.radius * info->normal;
    return true;
  } else {
    return false;
  }
}

// test collision between two axis aligned bounding boxes
bool test_collision(const AABB& a, const AABB& b)
{
  auto a_min = a.min(), a_max = a.max();
  auto b_min = b.min(), b_max = b.max();
  return ((a_max.x < b_min.x || a_min.x > b_max.x) && (a_max.y < b_min.y || a_min.y > b_max.y) &&
          (a_max.z < b_min.z || a_min.z > b_max.z));
}

bool test_collision(const Heightmap& heightmap, const glm::vec3& point, float* height)
{
  *height = heightmap.get_height({point.x, point.z});
  return point.y <= *height;
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
};  // namespace collider
