/*
    
*/

#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace phi {

    typedef float Seconds;
    typedef float Radians;
    typedef float Degrees;

    // constants
    constexpr float g = 9.81f;          // gravity of earth, m/s^2
    constexpr float rho = 1.225f;       // air density, kg/m^3
    constexpr float epsilon = 1e-8f;

    // directions in body space
    constexpr glm::vec3 UP(0.0f, 1.0f, 0.0f);
    constexpr glm::vec3 DOWN(0.0f, -1.0f, 0.0f);
    constexpr glm::vec3 RIGHT(0.0f, 0.0f, 1.0f);
    constexpr glm::vec3 LEFT(0.0f, 0.0f, -1.0f);
    constexpr glm::vec3 FORWARD(1.0f, 0.0f, 0.0f);
    constexpr glm::vec3 BACKWARD(-1.0f, 0.0f, 0.0f);

    constexpr glm::vec3 X_AXIS(1.0f, 0.0f, 0.0f);
    constexpr glm::vec3 Y_AXIS(0.0f, 1.0f, 0.0f);
    constexpr glm::vec3 Z_AXIS(0.0f, 0.0f, 1.0f);

    template<typename T>
    constexpr inline T sq(T a)
    {
        return a * a;
    }

    namespace inertia {
        struct Element {
            float mass;
            glm::vec3 position;
            glm::vec3 inertia;
            glm::vec3 computed_offset;
        };

        constexpr glm::vec3 cube(const glm::vec3& size, float mass)
        {
            const float C = (1.0f / 12.0f) * mass;
            glm::vec3 I(0.0f);
            I.x = C * (sq(size.y) + sq(size.z));
            I.y = C * (sq(size.x) + sq(size.z));
            I.z = C * (sq(size.x) + sq(size.y));
            return I;
        }
            
        constexpr glm::vec3 cylinder(float radius, float length, float mass)
        {
            const float C = (1.0f / 12.0f) * mass;
            glm::vec3 I(0.0f);
            I.x = (0.5f) * mass * sq(radius);
            I.y = I.z = C * (3 * sq(radius) + sq(length));
            return I;
        }
        
        // inertia tensor
        constexpr glm::mat3 tensor(const glm::vec3& moment_of_inertia)
        {
            return {
                moment_of_inertia.x, 0.0f, 0.0f,
                0.0f, moment_of_inertia.y, 0.0f,
                0.0f, 0.0f, moment_of_inertia.z,
            };
        }  

        constexpr Element cube_element(const glm::vec3& position, const glm::vec3& size, float mass)
        {
            auto inertia = cube(size, mass);
            return { .mass = mass, .position = position, .inertia = inertia, .computed_offset = {} };
        }

        
        // calculate inertia tensor from list of connected masses
        constexpr glm::mat3 tensor(const std::vector<Element>& elements)
        {
            float Ixx = 0, Iyy = 0, Izz = 0;
            float Ixy = 0, Ixz = 0, Iyz = 0;

            float mass = 0;
            glm::vec3 moment(0.0f);

            for (const auto& element : elements)
            {
                mass    += element.mass;
                moment  += element.mass * element.position;
            }

            const glm::vec3 center_of_gravity = moment / mass;

            for (const auto& element : elements)
            {
                auto offset = element.position - center_of_gravity;
                Ixx += element.inertia.x + element.mass * (sq(offset.y) + sq(offset.z));
                Iyy += element.inertia.y + element.mass * (sq(offset.z) + sq(offset.x));
                Izz += element.inertia.z + element.mass * (sq(offset.x) + sq(offset.y));
                Ixy += element.mass * (offset.x * offset.y);
                Ixz += element.mass * (offset.x * offset.z);
                Iyz += element.mass * (offset.y * offset.z);
            }

            return {
                 Ixx, -Ixy, -Ixz,
                -Ixy,  Iyy, -Iyz,
                -Ixz, -Iyz,  Izz
            };
        }
    };

    namespace utils {

        constexpr inline float scale(float input, float in_min, float in_max, float out_min, float out_max)
        {
            return (input - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        }
    
        constexpr inline float lerp(float a, float b, float t)
        {
            return a + t * (b - a);
        }

        template <typename T>
        constexpr inline T max(T a, T b)
        {
            return a > b ? a : b;
        }

        template <typename T>
        constexpr inline T min(T a, T b)
        {
            return a < b ? a : b;
        }

        constexpr inline float sign(float a)
        {
            return a >= 0.0f ? 1.0f : -1.0f;
        }

        template <typename T>
        constexpr inline T clamp(T v, T lo, T hi)
        {
            return min(max(v, lo), hi);
        }
    };
    
    namespace units {
        constexpr inline float knots(float meter_per_second)
        {
            return meter_per_second * 1.94384f;
        }

        constexpr inline float meter_per_second(float kilometer_per_hour)
        {
            return kilometer_per_hour / 3.6f;
        }

        constexpr inline float kilometer_per_hour(float meter_per_second)
        {
            return meter_per_second * 3.6f;
        }
    };

    struct RigidBodyParams {
        float mass = 1.0f;
        glm::mat3 inertia{};
        glm::vec3 position{};
        glm::vec3 velocity{};
        glm::vec3 angular_velocity{};
        glm::quat orientation = glm::quat(glm::vec3(0.0f));
        bool apply_gravity = true;
    };

    class RigidBody {
    private:
        glm::vec3 m_force{};                                // force vector in world space
        glm::vec3 m_torque{};                               // torque vector in body space

    public:
        float mass;                                         // rigidbody mass in kg
        glm::vec3 position{};                               // position in world space
        glm::vec3 velocity{};                               // velocity in world space
        glm::vec3 angular_velocity{};                       // angular velocity in object space, x represents rotation around x axis
        glm::mat3 inertia{}, inverse_inertia{};             // inertia tensor
        glm::quat orientation{};                               // rotation in world space 
        bool apply_gravity = true;

#if 1
        RigidBody() : 
            RigidBody({ 
                    .mass = 1.0f, 
                    .inertia = inertia::tensor(inertia::cube(glm::vec3(1.0f), 1.0f)) 
                }) 
        {}
#endif

        RigidBody(const RigidBodyParams& params)
            : mass(params.mass),
            position(params.position),
            velocity(params.velocity),
            inertia(params.inertia),
            orientation(params.orientation),
            apply_gravity(params.apply_gravity),
            angular_velocity(params.angular_velocity),
            inverse_inertia(glm::inverse(params.inertia))
        {}

        RigidBody(float m, const glm::mat3& inertia_tensor) 
            : mass(m),
            inertia(inertia_tensor), 
            inverse_inertia(glm::inverse(inertia_tensor))
        {}

        RigidBody(const glm::vec3& pos, const glm::vec3& rot, float m, const glm::mat3& inertia_tensor) 
            : mass(m),
            position(pos), 
            orientation(glm::quat(rot)), 
            inertia(inertia_tensor), 
            inverse_inertia(glm::inverse(inertia_tensor))
        {}


        // get velocity of point in body space
        inline glm::vec3 get_point_velocity(const glm::vec3& point) const
        {
            return inverse_transform_direction(velocity) + glm::cross(angular_velocity, point);
        }

        // force and point vectors are in body space 
        inline void add_force_at_point(const glm::vec3& force, const glm::vec3& point)
        {
            m_force     += transform_direction(force);
            m_torque    += glm::cross(point, force);
        }

        // transform direction from body space to world space 
        inline glm::vec3 transform_direction(const glm::vec3& direction) const
        {
            return orientation * direction;
        }

        inline glm::vec3 get_body_velocity() const {
            return inverse_transform_direction(velocity);
        }

        // transform direction from world space to body space 
        inline glm::vec3 inverse_transform_direction(const glm::vec3& direction) const
        {
            return glm::inverse(orientation) * direction;
        }

        inline void set_inertia(const glm::mat3& inertia_tensor)
        {
            inertia = inertia_tensor, inverse_inertia = glm::inverse(inertia_tensor);
        }
        
        // force vector in world space
        inline void add_force(const glm::vec3& force) 
        { 
            m_force += force; 
        }
         
        // force vector in body space
        inline void add_relative_force(const glm::vec3& force) 
        { 
            m_force += orientation * force;
        }
        
        // torque vector in world space
        inline void add_torque(const glm::vec3& torque) 
        { 
            m_torque += inverse_transform_direction(torque);
        }

        // torque vector in body space
        inline void add_relative_torque(const glm::vec3& torque) 
        { 
            m_torque += torque; 
        }

        // get torque in body space
        inline glm::vec3 get_torque() const
        {
            return m_torque;
        }

        // get torque in world space
        inline glm::vec3 get_force() const
        {
            return m_force;
        }

        void update(Seconds dt)
        {
            glm::vec3 acceleration = m_force / mass;

            if (apply_gravity)
                acceleration.y -= g;

            velocity += acceleration * dt;
            position += velocity * dt;

            angular_velocity += inverse_inertia * 
                (m_torque - glm::cross(angular_velocity, inertia * angular_velocity)) * dt;
            orientation += (orientation * glm::quat(0.0f, angular_velocity)) * (0.5f * dt);
            orientation = glm::normalize(orientation);

            // reset accumulators
            m_force = glm::vec3(0.0f), m_torque = glm::vec3(0.0f);
        }
    };
};

