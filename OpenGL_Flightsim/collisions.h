#pragma once

#include <optional>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace collisions {

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

    struct Sphere {
        glm::vec3 center;
        float radius;
    };

    constexpr bool collision(const AABB& a, const AABB& b)
    {
        auto a_min = a.min(), a_max = a.max();
        auto b_min = b.min(), b_max = b.max();
        return (
            (a_max.x < b_min.x || a_min.x > b_max.x) &&
            (a_max.y < b_min.y || a_min.y > b_max.y) &&
            (a_max.z < b_min.z || a_min.z > b_max.z)
        );
    }

    constexpr bool collision(const Sphere& a, const Sphere& b)
    {
        return (a.radius + b.radius) >= glm::length(a.center - a.center);
    }

    constexpr AABB update_with_orientation(const AABB& a, glm::quat orientation)
    {
        return {};
    }

    constexpr void test_collisions()
    {
        AABB aabb_0(glm::vec3(0.0f), glm::vec3(1.0f));
        AABB aabb_1(glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(1.0f));
        assert(collision(aabb_0, aabb_1));
    }
};
