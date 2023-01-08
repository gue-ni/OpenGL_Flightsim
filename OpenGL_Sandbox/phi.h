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
        
        glm::mat4 inertia;
        glm::mat4 inverse_inertia;

        glm::vec3 m_force;
        glm::vec3 m_moment;

        void add_force_at_position(glm::vec3 pos, glm::vec3 force)
        {

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
            angular_velocity += inverse_inertia + (m_moment - glm::cross(angular_velocity, inertia * angular_velocity)) * dt;
            rotation += (rotation * angular_velocity) * (0.5 * dt);

            m_force = glm::vec3(0.0f);
            m_torque = glm::vec3(0.0f);
        }
    };
};

