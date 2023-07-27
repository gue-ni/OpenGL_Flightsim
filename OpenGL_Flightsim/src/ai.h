#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "flightmodel.h"

glm::vec3 get_intercept_point(const glm::vec3& position, const glm::vec3& velocity, const glm::vec3& target_position,
                              const glm::vec3& target_velocity)
{
  auto velocity_delta = target_velocity - velocity;
  auto position_delta = target_position - position;
  auto time_to_intercept = glm::length(position_delta) / glm::length(velocity_delta);
  return target_position + target_velocity * time_to_intercept;
}

void fly_towards(Airplane& airplane, const glm::vec3& target)
{
  auto& rb = airplane;
  auto& joystick = airplane.joystick;
  auto position = rb.position;
  auto direction = glm::normalize(rb.inverse_transform_direction(target - rb.position));
  auto angle = glm::angle(phi::FORWARD, direction);

  float rudder = direction.z;
  float elevator = direction.y * 5.0f;

  float m = phi::PI / 4.0f;
  float agressive_roll = direction.z;
  float wings_level_roll = rb.right().y;
  float wings_level_influence = phi::inverse_lerp(0.0f, m, glm::clamp(angle, -m, m));
  float aileron = phi::lerp(wings_level_roll, agressive_roll, wings_level_influence);

  joystick = glm::clamp(glm::vec4(aileron, rudder, elevator, 0.0f), glm::vec4(-1.0f), glm::vec4(1.0f));
}

#if 1
void fly_towards(Airplane& airplane, const Airplane& target)
{
  auto point = get_intercept_point(airplane.position, airplane.velocity, target.position, target.velocity);

  fly_towards(airplane, point);
}
#endif
