#pragma once

#include "flightmodel.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>

template<typename T>
struct PID {
    T p{}, i{}, d{};
    const T Kp, Ki, Kd;

    PID(T kp, T ki, T kd)
        : Kp(kp), Ki(ki), Kd(kd)
    {}

    T calculate(T error, float dt)
    {
        float previous_error = p;
        p = error;
        i += p * dt;
        d = (p - previous_error) / dt;
        return p * Kp + i * Ki + d * Kd;
    }
};

void fly_towards(Aircraft& aircraft, const glm::vec3& target, float dt)
{
    auto& rb = aircraft.rigid_body;
    auto& joystick = aircraft.joystick;
    auto position = rb.position;
    auto direction = glm::normalize(rb.inverse_transform_direction(target - rb.position));

    joystick.x = direction.z;
    joystick.y = direction.z;
    joystick.z = direction.y;

    auto factor = glm::vec3(1.0f, 1.0f, 5.0f);
    auto tmp = joystick * factor;
    joystick = glm::clamp(tmp, glm::vec3(-1.0f), glm::vec3(1.0f));
}

glm::vec3 intercept_point(
    const glm::vec3& position, const glm::vec3& velocity,
    const glm::vec3& target_position, const glm::vec3& target_velocity)
{
    auto velocity_delta = target_velocity - velocity;
    auto position_delta = target_position - position;
    auto time_to_intercept = glm::length(position_delta) / glm::length(velocity_delta);
    return target_position + target_velocity * time_to_intercept;
}

