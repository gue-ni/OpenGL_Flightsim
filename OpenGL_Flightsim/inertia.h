#pragma once

#include <iostream>
#include <vector>
#include <glm/vec3.hpp>

#include "gfx.h"

struct Element {
    std::string name;
    float mass = 0;
    glm::vec3 local_inertia{}, center_of_gravity{}, design_coordinates{};

    Element(const std::string& n, float m, const glm::vec3& dc, const glm::vec3& inertia) 
        : design_coordinates(dc), local_inertia(dc), name(n), mass(m)
    {}
};

static glm::mat3 calc_inertia(void)
{
    std::vector<Element> elements = {
        Element("right out", 6.56f, {14.5f, 12.0f, 2.5f}, {13.92f, 10.5f, 24.0f }),
        Element("right in",  7.31f, {14.5f,  5.5f, 2.5f}, {21.95f, 12.22f, 33.67}),
        Element("left in",   7.31f, {14.5f, -5.5f, 2.5f}, {21.95f, 12.22f, 33.67}),
        Element("left out",  6.56f, {14.5f, -12.0, 2.5f}, {13.92f, 10.5f, 24.0f}),
        Element("right elevator", 2.62f, {3.03f, 2.5f, 3.0f}, {0.837, 0.385f, 1.206f}),
        Element("left elevator", 2.62f, {3.03f, -2.5f, 3.0f}, {0.837, 0.385f, 1.206f}),
        Element("rudder", 2.93f, {2.25, 0.0, 5.0}, {1.262, 1.942, 0.718}),
        Element("fuselage", 31.8f, {15.25, 0.0, 0.0f}, {66.30, 861.9, 861.9}),
    };

    for (Element& element : elements)
    {
        std::swap(element.local_inertia.y, element.local_inertia.z);
        std::swap(element.center_of_gravity.y, element.center_of_gravity.z);
        std::swap(element.design_coordinates.y, element.design_coordinates.z);
    }

    float Ixx = 0, Iyy = 0, Izz = 0;
    float Ixy = 0, Ixz = 0, Iyz = 0;

    float mass = 0;
    for (const Element& element : elements)
    {
        mass += element.mass;
    }

    glm::vec3 moment(0.0f);
    for (const Element& element : elements)
    {
        moment += element.mass * element.design_coordinates;
    }

    glm::vec3 center_of_gravity = moment / mass;

    for (Element& element : elements)
    {
        element.center_of_gravity = element.design_coordinates - center_of_gravity;

        Ixx += element.local_inertia.x + element.mass * (element.center_of_gravity.y * element.center_of_gravity.y + element.center_of_gravity.z * element.center_of_gravity.z);
        Iyy += element.local_inertia.y + element.mass * (element.center_of_gravity.z * element.center_of_gravity.z + element.center_of_gravity.x * element.center_of_gravity.x);
        Izz += element.local_inertia.z + element.mass * (element.center_of_gravity.x * element.center_of_gravity.x + element.center_of_gravity.y * element.center_of_gravity.y);
        Ixy += element.mass * (element.center_of_gravity.x * element.center_of_gravity.y);
        Ixz += element.mass * (element.center_of_gravity.x * element.center_of_gravity.z);
        Iyz += element.mass * (element.center_of_gravity.y * element.center_of_gravity.z);
    }

    glm::mat3 inertia = {
         Ixx, -Ixy, -Ixz,
        -Ixy,  Iyy, -Iyz,
        -Ixz, -Iyz,  Izz
    };

#if 0
    printf("Ixx = %f, Iyy = %f, Izz = %f\nIxy = %f, Ixz = %f, Iyz = %f\n", Ixx, Iyy, Izz, Ixy, Ixz, Iyz);
    printf("mass = %f\n", mass);
    std::cout << inertia << std::endl;
    std::cout << "CG " << center_of_gravity << std::endl;

    for (const Element& element : elements)
    {
        std::cout << element.name << " " << element.center_of_gravity << std::endl;
    }
#endif
    return inertia;

}
