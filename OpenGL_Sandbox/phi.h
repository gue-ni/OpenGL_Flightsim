#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>


namespace phi {

    constexpr float g = 9.81f;

    constexpr glm::vec3 UP(    0.0f, 1.0f, 0.0f);
    constexpr glm::vec3 DOWN(  0.0f, -1.0f, 0.0f);
    constexpr glm::vec3 RIGHT( 0.0f, 0.0f, 1.0f);
    constexpr glm::vec3 LEFT(  0.0f, 0.0f, -1.0f);
    constexpr glm::vec3 FORWARD(1.0f, 0.0f, 0.0f);
    constexpr glm::vec3 BACKWARD(-1.0f, 0.0f, 0.0f);

    class RigidBody {
    private:
        // force vector in world space
        glm::vec3 m_force{};

        // torque vector in object space
        glm::vec3 m_torque{};

    public:
        float mass;

        bool apply_gravity = true;

        // position in world space
        glm::vec3 position = glm::vec3(0.0f); 

        // rotation in world space
        glm::quat rotation = glm::quat(glm::vec3(0.0f));

        // velocity in world space
        glm::vec3 velocity          = glm::vec3(0.0f);

        // angular velocity in object space
        glm::vec3 angular_velocity  = glm::vec3(0.0f);

        glm::mat3 inertia{};
        glm::mat3 inverse_inertia{};

        RigidBody(float body_mass) 
            : mass(body_mass), 
            inertia(cube_inertia_tensor(glm::vec3(1.0f), body_mass)), 
            inverse_inertia(glm::inverse(inertia))
        {}

        RigidBody(float m, const glm::mat3& inertia_tensor) 
            : mass(m),
            inertia(inertia_tensor), 
            inverse_inertia(glm::inverse(inertia_tensor))
        {}

        RigidBody(const glm::vec3& pos, const glm::vec3& rot, float m, const glm::mat3& inertia_tensor) 
            : mass(m),
            position(pos), 
            rotation(glm::quat(rot)), 
            inertia(inertia_tensor), 
            inverse_inertia(glm::inverse(inertia_tensor))
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

        // TODO: fix
        inline void add_force_at_point(const glm::vec3& force, const glm::vec3& point)
        {
            m_force     += force;
            m_torque    += glm::cross(point, force);
        }

        // transform from direction in world space to local space 
        inline glm::vec3 inverse_transform_direction(const glm::vec3& direction)
        {
            return direction * glm::inverse(rotation);
        }

        // force vector in world coordinates.
        inline void add_force(const glm::vec3& force) 
        { 
            m_force += force; 
        }
         
        // force vector in local coordinates.
        inline void add_relative_force(const glm::vec3& force) 
        { 
            m_force += rotation * force;
        }
        
        // torque vector in world coordinates.
        inline void add_torque(const glm::vec3& torque) 
        { 
            m_torque += inverse_transform_direction(torque); // TODO: not sure if this really works
        }

        // torque vector in local coordinates.
        inline void add_relative_torque(const glm::vec3& torque) 
        { 
            m_torque += torque; 
        }

        inline glm::vec3 get_torque() const
        {
            return m_torque;
        }

        inline glm::vec3 get_force() const
        {
            return m_force;
        }

        inline glm::vec3 get_point_velocity(const glm::vec3& point)
        {
            return inverse_transform_direction(velocity) + glm::cross(angular_velocity, point);
        }

        void update(float dt)
        {
            /*
            if (apply_gravity)
            {
                m_force.y -= g * mass;
            }
            */

            glm::vec3 acceleration = m_force / mass;
            velocity += acceleration * dt;
            position += velocity * dt;

            angular_velocity += (m_torque - glm::cross(angular_velocity, inertia * angular_velocity)) * inverse_inertia * dt;
            rotation += (rotation * glm::quat(0.0f, angular_velocity.x, angular_velocity.y, angular_velocity.z)) * (0.5f * dt);
            rotation = glm::normalize(rotation);

            // reset accumulators
            m_force = glm::vec3(0.0f), m_torque = glm::vec3(0.0f);
        }
    };
};

