#pragma once
#include "phi.h"

struct Collider;
struct Sphere;
struct Heightmap;
struct LandingGear;

#define COLLISION_NOT_IMPLEMENTED 0

// collider abstract base class
struct Collider {
  virtual bool test(const phi::Transform* t_this, const Collider* other, const phi::Transform* t_other,
                    phi::CollisionInfo* info) const = 0;
  virtual bool test(const phi::Transform* t_this, const Sphere* other, const phi::Transform* t_other,
                    phi::CollisionInfo* info) const = 0;
  virtual bool test(const phi::Transform* t_this, const Heightmap* other, const phi::Transform* t_other,
                    phi::CollisionInfo* info) const = 0;
  virtual bool test(const phi::Transform* t_this, const LandingGear* other, const phi::Transform* t_other,
                    phi::CollisionInfo* info) const = 0;
};

struct Sphere : public Collider {
  const float radius;
  Sphere(float radius_) : radius(radius_) {}
  bool test(const phi::Transform* t_this, const Collider* other, const phi::Transform* t_other,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* t_this, const Sphere* other, const phi::Transform* t_other,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* t_this, const Heightmap* other, const phi::Transform* t_other,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* t_this, const LandingGear* other, const phi::Transform* t_other,
            phi::CollisionInfo* info) const override;
};

struct Heightmap : public Collider {
  const float height;
  Heightmap(float height_) : height(height_) {}
  bool test(const phi::Transform* t_this, const Collider* other, const phi::Transform* t_other,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* t_this, const Sphere* other, const phi::Transform* t_other,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* t_this, const Heightmap* other, const phi::Transform* t_other,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* t_this, const LandingGear* other, const phi::Transform* t_other,
            phi::CollisionInfo* info) const override;
};

struct LandingGear : public Collider {
  const glm::vec3 left, right, center;  // wheel positions
  LandingGear(const glm::vec3& center_, const glm::vec3& left_, const glm::vec3& right_)
      : center(center_), left(left_), right(right_)
  {
  }
  bool test(const phi::Transform* t_this, const Collider* other, const phi::Transform* t_other,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* t_this, const Sphere* other, const phi::Transform* t_other,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* t_this, const Heightmap* other, const phi::Transform* t_other,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* t_this, const LandingGear* other, const phi::Transform* t_other,
            phi::CollisionInfo* info) const override;
};

/* ############### Sphere ############### */

inline bool Sphere::test(const phi::Transform* t_this, const Collider* other, const phi::Transform* t_other,
                         phi::CollisionInfo* info) const
{
  return other->test(t_other, this, t_this, info);
}

inline bool Sphere::test(const phi::Transform* t_this, const Sphere* other, const phi::Transform* t_other,
                         phi::CollisionInfo* info) const
{
  float sum_radius = radius + other->radius;
  float distance = glm::length(t_this->position - t_other->position);

  if (distance < sum_radius) {
    glm::vec3 vec = t_other->position - t_this->position;
    info->penetration = sum_radius - distance;
    info->normal = glm::normalize(vec);
    info->point = t_this->position + vec * 0.5f;
    return true;
  } else {
    return false;
  }
}

inline bool Sphere::test(const phi::Transform* t_this, const Heightmap* other, const phi::Transform* t_other,
                         phi::CollisionInfo* info) const
{
  float lowest_point = t_this->position.y;
  float diff = other->height - lowest_point;
  if (diff > 0) {
    info->penetration = diff;
    info->normal = phi::DOWN;  // b - a
    info->point = t_this->position - glm::vec3(0.0f, radius, 0.0f);
    return true;
  }
  return false;
}

inline bool Sphere::test(const phi::Transform* t_this, const LandingGear* other, const phi::Transform* t_other,
                         phi::CollisionInfo* info) const
{
  assert(COLLISION_NOT_IMPLEMENTED);
  return false;
}

/* ############### Heightmap ############### */

inline bool Heightmap::test(const phi::Transform* t_this, const Collider* other, const phi::Transform* t_other,
                            phi::CollisionInfo* info) const
{
  return other->test(t_other, this, t_this, info);
}

inline bool Heightmap::test(const phi::Transform* t_this, const Sphere* other, const phi::Transform* t_other,
                            phi::CollisionInfo* info) const
{
  return other->test(t_other, this, t_this, info);
}

inline bool Heightmap::test(const phi::Transform* t_this, const Heightmap* other, const phi::Transform* t_other,
                            phi::CollisionInfo* info) const
{
  assert(COLLISION_NOT_IMPLEMENTED);
  return false;
}

inline bool Heightmap::test(const phi::Transform* t_this, const LandingGear* other, const phi::Transform* t_other,
                            phi::CollisionInfo* info) const
{
  return other->test(t_other, this, t_this, info);
}

/* ############### LandingGear ############### */

inline bool LandingGear::test(const phi::Transform* t_this, const Collider* other, const phi::Transform* t_other,
                              phi::CollisionInfo* info) const
{
  return other->test(t_other, this, t_this, info);
}

inline bool LandingGear::test(const phi::Transform* t_this, const Sphere* other, const phi::Transform* t_other,
                              phi::CollisionInfo* info) const
{
  assert(COLLISION_NOT_IMPLEMENTED);
  return false;
}

inline bool LandingGear::test(const phi::Transform* t_this, const Heightmap* other, const phi::Transform* t_other,
                              phi::CollisionInfo* info) const
{
  float height = other->height;

  glm::vec3 left_wheel = t_this->transform_vector(left);
  glm::vec3 right_wheel = t_this->transform_vector(right);
  glm::vec3 center_wheel = t_this->transform_vector(center);

  glm::vec3 lowest_point;

  if (left_wheel.y < right_wheel.y && left_wheel.y < center_wheel.y) {
    lowest_point = left_wheel;
  } else if (right_wheel.y < left_wheel.y && right_wheel.y < center_wheel.y) {
    lowest_point = right_wheel;
  } else {
    lowest_point = center_wheel;
  }

  float penetration = height - lowest_point.y;

  if (penetration > 0) {
    info->penetration = penetration;
    info->normal = phi::UP;
    info->point = lowest_point;
    return true;
  }

  return false;
}

inline bool LandingGear::test(const phi::Transform* t_this, const LandingGear* other, const phi::Transform* t_other,
                              phi::CollisionInfo* info) const
{
  assert(COLLISION_NOT_IMPLEMENTED);
  return false;
}

// collision detection
bool test_collision(phi::RigidBody* a, phi::RigidBody* b, phi::CollisionInfo* info)
{
  assert((a && b) && (a->collider && b->collider));

  if (a->collider->test(a, b->collider, b, info)) {
    info->a = a, info->b = b;
    return true;
  }

  return false;
}
