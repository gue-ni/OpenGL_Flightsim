#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "flightmodel.h"

template <typename T>
struct PID {
  T p{}, i{}, d{};
  const T Kp, Ki, Kd;

  PID(T kp, T ki, T kd) : Kp(kp), Ki(ki), Kd(kd) {}

  T calculate(T error, float dt) {
    float previous_error = p;
    p = error;
    i += p * dt;
    d = (p - previous_error) / dt;
    return p * Kp + i * Ki + d * Kd;
  }
};

glm::vec3 get_intercept_point(const glm::vec3& position, const glm::vec3& velocity, const glm::vec3& target_position,
                              const glm::vec3& target_velocity) {
  auto velocity_delta = target_velocity - velocity;
  auto position_delta = target_position - position;
  auto time_to_intercept = glm::length(position_delta) / glm::length(velocity_delta);
  return target_position + target_velocity * time_to_intercept;
}

void fly_towards(Aircraft& aircraft, const glm::vec3& target) {
  auto& rb = aircraft.rigid_body;
  auto& joystick = aircraft.joystick;
  auto position = rb.position;
  auto direction = glm::normalize(rb.inverse_transform_direction(target - rb.position));
  auto angle = glm::angle(phi::FORWARD, direction);

  float yaw = direction.z;
  float pitch = direction.y * 5.0f;

  float m = M_PI / 4.0f;
  float agressive_roll = direction.z;
  float wings_level_roll = rb.right().y;
  float wings_level_influence = phi::utils::inverse_lerp(0.0f, m, glm::clamp(angle, -m, m));
  float roll = phi::utils::lerp(wings_level_roll, agressive_roll, wings_level_influence);

  joystick = glm::clamp(glm::vec3(roll, yaw, pitch), glm::vec3(-1.0f), glm::vec3(1.0f));
}

#if 1
void fly_towards(Aircraft& aircraft, const Aircraft& target) {
  auto point = get_intercept_point(aircraft.rigid_body.position, aircraft.rigid_body.velocity,
                                   target.rigid_body.position, target.rigid_body.velocity);

  fly_towards(aircraft, point);
}
#endif
