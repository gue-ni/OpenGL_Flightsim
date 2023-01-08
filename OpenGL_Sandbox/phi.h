#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>


namespace phi {

    constexpr float g = 9.81f;

    constexpr glm::vec3 UP(1.0f, 0.0f, 0.0f);
    constexpr glm::vec3 RIGHT(1.0f, 0.0f, 0.0f);
    constexpr glm::vec3 FORWARD(1.0f, 0.0f, 0.0f);

    class RigidBody {
    public:
        float mass;

        bool apply_gravity = true;

        glm::vec3 position{};
        glm::quat rotation{};

        glm::vec3 velocity{};
        glm::vec3 angular_velocity{};

        glm::mat3 inertia;
        glm::mat3 inverse_inertia;

        RigidBody(const glm::vec3& pos, const glm::vec3& rot, float m, const glm::mat3& inertia_tensor) 
            : position(pos), rotation(glm::quat(rot)), mass(m), inertia(inertia_tensor), inverse_inertia(glm::inverse(inertia_tensor))
        {}

        static glm::mat3 cube_inertia_tensor(const glm::vec3& dimensions, float cube_mass)
        {
            float f = 1.0f / 12.0f;
            float d0 = f * cube_mass * (dimensions.y * dimensions.y + dimensions.z * dimensions.z);
            float d1 = f * cube_mass * (dimensions.x * dimensions.x + dimensions.z * dimensions.z);
            float d2 = f * cube_mass * (dimensions.x * dimensions.x + dimensions.y * dimensions.y);

            return glm::mat3{
                 d0, 0, 0,
                 0, d1, 0,
                 0, 0, d2
            };
        }

        inline void add_force_at_position(const glm::vec3& force, const glm::vec3& point)
        {
            m_force     += force;
            m_torque    += glm::cross(point, force);
        }

        inline void add_force(const glm::vec3& force) 
        { 
            m_force += force; 
        }
        
        inline void add_torque(const glm::vec3& torque) 
        { 
            m_torque += torque; 
        }

        inline glm::vec3 get_point_velocity(const glm::vec3& point)
        {
            return glm::vec3(0.0f); // TODO
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
            angular_velocity += (m_torque - glm::cross(angular_velocity, inertia * angular_velocity)) * inverse_inertia * dt;
            rotation += (rotation * glm::quat(0.0f, angular_velocity.x, angular_velocity.y, angular_velocity.z)) * (0.5f * dt);
            rotation = glm::normalize(rotation);

            m_force = glm::vec3(0.0f), m_torque = glm::vec3(0.0f);
        }

    private:
        glm::vec3 m_force{};
        glm::vec3 m_torque{};
    };
};

