#include "phi.h"

namespace phi
{

void RigidBody::update(phi::Seconds dt)
{
  if (!active) return;

  glm::vec3 acceleration = m_force / mass;

  if (apply_gravity) acceleration.y -= EARTH_GRAVITY;

  velocity += acceleration * dt;
  position += velocity * dt;

  angular_velocity += inverse_inertia * (m_torque - glm::cross(angular_velocity, inertia * angular_velocity)) * dt;
  orientation += (orientation * glm::quat(0.0f, angular_velocity)) * (0.5f * dt);
  orientation = glm::normalize(orientation);

  // update collider transform
  if (collider != nullptr) {
    collider->update(this);
  }

  // reset accumulators
  m_force = glm::vec3(0.0f), m_torque = glm::vec3(0.0f);
}

#if 0
void OBB::update(const RigidBody* rb) {}
bool OBB::test_collision(const Collider* other) const { return false; }
bool OBB::test_collision(const Plane* other) const { return false; }
#endif

glm::vec3 Heightmap::sample(const glm::vec2& coord) const
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

Plane Heightmap::plane_from_points(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
  Plane p;
  p.normal = glm::normalize(glm::cross(b - a, c - a));
  p.origin = a;
  return p;
}

float Heightmap::get_height(const glm::vec2& coord) const
{
  glm::vec2 tmp = glm::clamp(coord / magnification, glm::vec2(-1.0f), glm::vec2(1.0f));
  auto uv       = phi::scale(tmp, glm::vec2(-1.0f), glm::vec2(1.0f), glm::vec2(0.0f), glm::vec2(1.0f));
  float value   = sample(uv).r;
  float height  = scale * value + shift;
  return height;
}

Plane Heightmap::get_plane(const glm::vec2& coord) const
{
  float height = get_height(coord);
  return Plane{glm::vec3(coord.x, height, coord.y), phi::UP};
}

void Heightmap::update(const RigidBody* rb)
{
  // nothing to do here
}

bool Heightmap::test_collision(const Collider* other) const { return other->test_collision(this); }

bool Heightmap::test_collision(const Sphere* other) const { return other->test_collision(other); }

bool Heightmap::test_collision(const Heightmap* other) const { return false; }

void Sphere::update(const RigidBody* rb) { origin = rb->position; }

bool Sphere::test_collision(const Collider* other) const { return other->test_collision(this); }

bool Sphere::test_collision(const Heightmap* other) const
{
  Plane p = other->get_plane({origin.x, origin.z});
  return collision_primitive(&p, this);
}

bool Sphere::test_collision(const Sphere* other) const { return false; }

bool collision_primitive(const Plane* plane, const Sphere* sphere)
{
  float dist = glm::dot(sphere->origin, plane->normal) - glm::dot(plane->normal, plane->origin);
  return glm::abs(dist) < sphere->radius;
}

bool collision_primitive(const Sphere* a, const Sphere* b) { return false; }

};  // namespace phi
