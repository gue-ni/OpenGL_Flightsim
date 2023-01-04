#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace phi {
    struct RigidBody {

        float mass;

        glm::vec3 position;
        glm::vec3 rotation;

        glm::vec3 velocity;
        glm::vec3 angular_velocity;

        glm::vec3 m_force;
        glm::vec3 m_torque;

        void add_force_at_position(glm::vec3 pos, glm::vec3 force)
        {

        }

        void add_torque(glm::vec3 t)
        {
            m_torque += t;
        }

        void add_force(glm::vec3 f)
        {
            m_force += f;
        }

        void update_euler(float dt)
        {
            // position
            glm::vec3 acceleration = m_force / mass;
            velocity += acceleration * dt;
            position += velocity * dt;

            // rotation


            m_force = glm::vec3(0.0f);
            m_torque = glm::vec3(0.0f);
        }
    };
};

