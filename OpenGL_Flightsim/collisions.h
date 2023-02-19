#pragma once

#include <optional>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace collisions {

    constexpr float EPSILON = 1e-8f;

    // axis aligned bounding box
    struct AABB {
        glm::vec3 center, size;

        glm::vec3 min() const
        { 
            return center - size / 2.0f;
        }

        glm::vec3 max() const
        {
            return center + size / 2.0f;
        }
    };

    // bounding sphere
    struct Sphere {
        glm::vec3 center;
        float radius;
    };

    // test collision of two axis aligned bounding boxes
    constexpr bool test_collision(const AABB& a, const AABB& b)
    {
        auto a_min = a.min(), a_max = a.max();
        auto b_min = b.min(), b_max = b.max();
        return (
            (a_max.x < b_min.x || a_min.x > b_max.x) &&
            (a_max.y < b_min.y || a_min.y > b_max.y) &&
            (a_max.z < b_min.z || a_min.z > b_max.z)
        );
    }

    // test collision of two spheres
    constexpr bool test_collision(const Sphere& a, const Sphere& b)
    {
        return glm::length(a.center - a.center) < (a.radius + b.radius);
    }

    // test collision of two moving spheres
    constexpr bool test_moving_collision(const Sphere& a, const glm::vec3& a_velocity, const Sphere& b, const glm::vec3& b_velocity, float *t)
    { 
        // Christer_Ericson-Real-Time_Collision_Detection.pdf#page=264
        if (test_collision(a, b))
            return true;

        auto distance = a.center - b.center;
        auto relative_velocity = b_velocity - a_velocity;

        float tmp = glm::dot(relative_velocity, relative_velocity);
        if (tmp < EPSILON)
            return false;

        float tmp2 = glm::dot(relative_velocity, distance);
        if (tmp2 >= 0.0f)
            return false;

        //float d = tmp2 * tmp2 - tmp * 
       

        return false;
    }

    constexpr AABB update_with_orientation(const AABB& aabb, glm::quat orientation)
    {
        return {};
    }

    constexpr void run_tests()
    {
        AABB aabb_0(glm::vec3(0.0f), glm::vec3(1.0f));
        AABB aabb_1(glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(1.0f));
        assert(test_collision(aabb_0, aabb_1));
    }
};
