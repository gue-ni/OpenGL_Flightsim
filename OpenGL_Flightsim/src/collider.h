#pragma once
#include "phi.h"

struct Collider;
struct Sphere;
struct Heightmap;
struct LandingGear;

#define COLLISION_NOT_IMPLEMENTED 0

// collider abstract base class
struct Collider {
  virtual bool test(const phi::Transform* tf, const Collider* other, const phi::Transform* other_tf,
                    phi::CollisionInfo* info) const = 0;
  virtual bool test(const phi::Transform* tf, const Sphere* other, const phi::Transform* other_tf,
                    phi::CollisionInfo* info) const = 0;
  virtual bool test(const phi::Transform* tf, const Heightmap* other, const phi::Transform* other_tf,
                    phi::CollisionInfo* info) const = 0;
  virtual bool test(const phi::Transform* tf, const LandingGear* other, const phi::Transform* other_tf,
                    phi::CollisionInfo* info) const = 0;
};

struct Sphere : public Collider {
  const float radius;
  Sphere(float radius_) : radius(radius_) {}
  bool test(const phi::Transform* tf, const Collider* other, const phi::Transform* other_tf,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* tf, const Sphere* other, const phi::Transform* other_tf,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* tf, const Heightmap* other, const phi::Transform* other_tf,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* tf, const LandingGear* other, const phi::Transform* other_tf,
            phi::CollisionInfo* info) const override;
};

struct Heightmap : public Collider {
  const float height;
  Heightmap(float height_) : height(height_) {}
  bool test(const phi::Transform* tf, const Collider* other, const phi::Transform* other_tf,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* tf, const Sphere* other, const phi::Transform* other_tf,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* tf, const Heightmap* other, const phi::Transform* other_tf,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* tf, const LandingGear* other, const phi::Transform* other_tf,
            phi::CollisionInfo* info) const override;
};

struct LandingGear : public Collider {
  const glm::vec3 center, left, right;  // wheel positions
  LandingGear(const glm::vec3& center_, const glm::vec3& left_, const glm::vec3& right_)
      : center(center_), left(left_), right(right_)
  {
  }
  bool test(const phi::Transform* tf, const Collider* other, const phi::Transform* other_tf,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* tf, const Sphere* other, const phi::Transform* other_tf,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* tf, const Heightmap* other, const phi::Transform* other_tf,
            phi::CollisionInfo* info) const override;
  bool test(const phi::Transform* tf, const LandingGear* other, const phi::Transform* other_tf,
            phi::CollisionInfo* info) const override;
};

/* ############### Sphere ############### */

inline bool Sphere::test(const phi::Transform* tf, const Collider* other, const phi::Transform* other_tf,
                         phi::CollisionInfo* info) const
{
  return other->test(other_tf, this, tf, info);
}

inline bool Sphere::test(const phi::Transform* tf, const Sphere* other, const phi::Transform* other_tf,
                         phi::CollisionInfo* info) const
{
  float sum_radius = radius + other->radius;
  float distance = glm::length(tf->position - other_tf->position);

  if (distance < sum_radius) {
    glm::vec3 vec = other_tf->position - tf->position;
    info->penetration = sum_radius - distance;
    info->normal = glm::normalize(vec);
    info->point = tf->position + vec * 0.5f;
    return true;
  } else {
    return false;
  }
}

inline bool Sphere::test(const phi::Transform* tf, const Heightmap* other, const phi::Transform* other_tf,
                         phi::CollisionInfo* info) const
{
  float lowest_point = tf->position.y;
  float diff = other->height - lowest_point;
  if (diff > 0) {
    info->penetration = diff;
    info->normal = phi::DOWN;  // b - a
    info->point = tf->position - glm::vec3(0.0f, radius, 0.0f);
    return true;
  }
  return false;
}

inline bool Sphere::test(const phi::Transform* tf, const LandingGear* other, const phi::Transform* other_tf,
                         phi::CollisionInfo* info) const
{
  assert(COLLISION_NOT_IMPLEMENTED);
  return false;
}

/* ############### Heightmap ############### */

inline bool Heightmap::test(const phi::Transform* tf, const Collider* other, const phi::Transform* other_tf,
                            phi::CollisionInfo* info) const
{
  return other->test(other_tf, this, tf, info);
}

inline bool Heightmap::test(const phi::Transform* tf, const Sphere* other, const phi::Transform* other_tf,
                            phi::CollisionInfo* info) const
{
  return other->test(other_tf, this, tf, info);
}

inline bool Heightmap::test(const phi::Transform* tf, const Heightmap* other, const phi::Transform* other_tf,
                            phi::CollisionInfo* info) const
{
  assert(COLLISION_NOT_IMPLEMENTED);
  return false;
}

inline bool Heightmap::test(const phi::Transform* tf, const LandingGear* other, const phi::Transform* other_tf,
                            phi::CollisionInfo* info) const
{
  return other->test(other_tf, this, tf, info);
}

/* ############### LandingGear ############### */

inline bool LandingGear::test(const phi::Transform* tf, const Collider* other, const phi::Transform* other_tf,
                              phi::CollisionInfo* info) const
{
  return other->test(other_tf, this, tf, info);
}

inline bool LandingGear::test(const phi::Transform* tf, const Sphere* other, const phi::Transform* other_tf,
                              phi::CollisionInfo* info) const
{
  assert(COLLISION_NOT_IMPLEMENTED);
  return false;
}

inline bool LandingGear::test(const phi::Transform* tf, const Heightmap* other, const phi::Transform* other_tf,
                              phi::CollisionInfo* info) const
{
  // no collision if landing gear is not pointing down
  if (glm::dot(phi::UP, tf->up()) < 0) return false;

  float height = other->height;

  glm::vec3 left_wheel = tf->transform_vector(left);
  glm::vec3 right_wheel = tf->transform_vector(right);
  glm::vec3 center_wheel = tf->transform_vector(center);

  glm::vec3 lowest_point;

  std::string wheel;

  if (center_wheel.y <= right_wheel.y && center_wheel.y <= left_wheel.y) {
    lowest_point = center_wheel;
    wheel = "center wheel";

  } else {
    if (std::abs(right_wheel.y - left_wheel.y) < 0.1f) {
      lowest_point = (right_wheel + left_wheel) / 2.0f;
      wheel = "between wheels";
    } else if (right_wheel.y < left_wheel.y && right_wheel.y < center_wheel.y) {
      lowest_point = right_wheel;
      wheel = "right wheel";
    } else {
      lowest_point = left_wheel;
      wheel = "left wheel";
    }
  }

  float penetration = height - lowest_point.y;

  if (penetration > 0) {
#if 0
    std::cout << "########### start collision #############\n";
    std::cout << wheel << std::endl;
    std::cout << center_wheel << std::endl;
    std::cout << left_wheel << std::endl;
    std::cout << right_wheel << std::endl;
    std::cout << "########### end collision #############\n";
#endif
    info->penetration = penetration;
    info->normal = phi::DOWN;
    info->point = lowest_point;
    info->restitution_coeff = 0.5f;
    return true;
  }

  return false;
}

inline bool LandingGear::test(const phi::Transform* tf, const LandingGear* other, const phi::Transform* other_tf,
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
