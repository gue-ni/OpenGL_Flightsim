#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace phi {

    constexpr float g = 9.81;

    class RigidBody3D {
    public:

        float mass{};

        bool apply_gravity = true;

        glm::vec3 position{};
        glm::quat rotation{};

        glm::vec3 velocity{};
        glm::vec3 angular_velocity{};

        RigidBody3D(const glm::vec3& pos, const glm::vec3& rot, float m) 
            : position(pos), rotation(glm::quat(rot)), mass(m)
        {}

        inline void add_force_at_position(const glm::vec3& cg_to_point, const glm::vec3& force)
        {

        }

        inline void add_torque(const glm::vec3& t)
        {
            m_torque += t;
        }

        inline void add_force(const glm::vec3& f)
        {
            m_force += f;
        }

        void update(float dt)
        {
            if (apply_gravity)
            {
                m_force.y -= g * mass;
            }

            // position
            glm::vec3 acceleration = m_force / mass;
            velocity += acceleration * dt;
            position += velocity * dt;

            // rotation
            // TODO

            // reset forces
            m_force = glm::vec3(0.0f), m_torque = glm::vec3(0.0f);
        }

    private:
        glm::vec3 m_force{};
        glm::vec3 m_torque{};
    };
};

