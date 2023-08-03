#pragma once
#include "phi.h"

struct Collider;
struct Sphere;
struct Heightmap;
struct LandingGear;

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
  if (glm::length(t_this->position - t_other->position) < (radius + other->radius)) {
    // TODO
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
    // TODO:
    info->penetration = diff;
    return true;
  }
  return false;
}

inline bool Sphere::test(const phi::Transform* t_this, const LandingGear* other, const phi::Transform* t_other,
                         phi::CollisionInfo* info) const
{
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
  return false;
}

inline bool Heightmap::test(const phi::Transform* t_this, const LandingGear* other, const phi::Transform* t_other,
                            phi::CollisionInfo* info) const
{
  return false;
}

/* ############### LandingGear ############### */

inline bool LandingGear::test(const phi::Transform* t_this, const Collider* other, const phi::Transform* t_other,
                              phi::CollisionInfo* info) const
{
  return false;
}

inline bool LandingGear::test(const phi::Transform* t_this, const Sphere* other, const phi::Transform* t_other,
                              phi::CollisionInfo* info) const
{
  return false;
}

inline bool LandingGear::test(const phi::Transform* t_this, const Heightmap* other, const phi::Transform* t_other,
                              phi::CollisionInfo* info) const
{
  return false;
}

inline bool LandingGear::test(const phi::Transform* t_this, const LandingGear* other, const phi::Transform* t_other,
                              phi::CollisionInfo* info) const
{
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
