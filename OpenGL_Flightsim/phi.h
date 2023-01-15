#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace phi {

    constexpr float g = 9.81f;

    constexpr glm::vec3 UP(0.0f, 1.0f, 0.0f);
    constexpr glm::vec3 DOWN(0.0f, -1.0f, 0.0f);
    constexpr glm::vec3 RIGHT(0.0f, 0.0f, 1.0f);
    constexpr glm::vec3 LEFT(0.0f, 0.0f, -1.0f);
    constexpr glm::vec3 FORWARD(1.0f, 0.0f, 0.0f);
    constexpr glm::vec3 BACKWARD(-1.0f, 0.0f, 0.0f);

    constexpr glm::vec3 X_AXIS(1.0f, 0.0f, 0.0f);
    constexpr glm::vec3 Y_AXIS(0.0f, 1.0f, 0.0f);
    constexpr glm::vec3 Z_AXIS(0.0f, 0.0f, 1.0f);

    struct RigidBodyParams {
        float mass = 10.0f;
        glm::mat3 inertia;
        bool apply_gravity = true;
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 velocity = glm::vec3(0.0f);
        glm::vec3 angular_velocity = glm::vec3(0.0f);
        glm::quat rotation = glm::quat(glm::vec3(0.0f));
    };

    /*
        The rigidbody has  
    */
    class RigidBody {
    private:
        // force vector in world space
        glm::vec3 m_force{};

        // torque vector in object space
        glm::vec3 m_torque{};

    public:
        // rigidbody mass in kg
        float mass; 

        bool apply_gravity = true;

        // position in world space
        glm::vec3 position = glm::vec3(0.0f); 

        // rotation in world space
        glm::quat rotation = glm::quat(glm::vec3(0.0f));

        // velocity in world space
        glm::vec3 velocity = glm::vec3(0.0f);

        // angular velocity in object space
        glm::vec3 angular_velocity = glm::vec3(0.0f);

        // inertia tensor
        glm::mat3 inertia{};
        glm::mat3 inverse_inertia{};

        RigidBody(const RigidBodyParams& params)
            : mass(params.mass),
            position(params.position),
            velocity(params.velocity),
            inertia(params.inertia),
            rotation(params.rotation),
            apply_gravity(params.apply_gravity),
            angular_velocity(params.angular_velocity),
            inverse_inertia(glm::inverse(params.inertia))
        {}

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

            printf("d0 = %f, d1 = %f, d2 = %f\n", d0, d1, d2);

            return glm::mat3{
                 d0, 0, 0,
                 0, d1, 0,
                 0, 0, d2
            };
        }

        // get velocity of point in body space
        inline glm::vec3 get_point_velocity(const glm::vec3& point) const
        {
            return inverse_transform_direction(velocity) + glm::cross(angular_velocity, point);
        }

        // force and point vectors are in body coordinates 
        inline void add_force_at_point(const glm::vec3& force, const glm::vec3& point)
        {
            m_force     += transform_direction(force);
            m_torque    += glm::cross(point, force);
        }

        // transform direction from local space to world space 
        inline glm::vec3 transform_direction(const glm::vec3& direction) const
        {
            return rotation * direction;
        }

        // transform direction from world space to local space 
        inline glm::vec3 inverse_transform_direction(const glm::vec3& direction) const
        {
            return glm::inverse(rotation) * direction;
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
            m_torque += inverse_transform_direction(torque);
        }

        // torque vector in local coordinates.
        inline void add_relative_torque(const glm::vec3& torque) 
        { 
            m_torque += torque; 
        }

        // get torque in object space
        inline glm::vec3 get_torque() const
        {
            return m_torque;
        }

        // get torque in world space
        inline glm::vec3 get_force() const
        {
            return m_force;
        }

        void update(float dt)
        {
            if (apply_gravity)
            {
                m_force.y -= g * mass;
            }

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

    namespace utils {
        inline float scale(float input, float in_min, float in_max, float out_min, float out_max)
        {
            return (input - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        }

	    inline float lerp(float a, float b, float t)
		{
			return a + t * (b - a);
		}

		inline float max(float a, float b)
		{
			return a > b ? a : b;
		}

		inline float min(float a, float b)
		{
			return a < b ? a : b;
		}

		inline float sign(float a)
		{
			return a >= 0.0f ? 1.0f : -1.0f;
		}

		inline float clamp(float v, float lo, float hi)
		{
			return min(max(v, lo), hi);
		}

		inline float knots(float meter_per_second)
		{
			return meter_per_second * 1.94384f;
		}

		inline float meter_per_second(float kilometer_per_hour)
		{
			return kilometer_per_hour / 3.6f;
		}

		inline float kilometer_per_hour(float meter_per_second)
		{
			return meter_per_second * 3.6f;
		}
    };
};

