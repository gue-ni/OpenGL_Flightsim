#pragma once

#include "phi.h"
#include "gfx.h"
#include "data.h"

#include <cmath>
#include <vector>
#include <tuple>
#include <algorithm>


struct Aerodynamics
{
    float min, max;
    std::vector<ValueTuple> data;

    Aerodynamics(const std::vector<ValueTuple> &curve_data) : data(curve_data)
    {
        min = curve_data[0].alpha;
        max = curve_data[curve_data.size() - 1].alpha;
    }

    std::tuple<float, float> sample(float alpha) const
    {
        int index = static_cast<int>(phi::utils::scale(alpha, min, max, 0, data.size()));
        index = phi::utils::clamp(index, 0, static_cast<int>(data.size() - 1U));
        if (!(0 <= index && index < data.size()))
        {
            printf("alpha = %f, index = %d, size = %d\n", alpha, index, (int)data.size());
            assert(false);
        }
        return { data[index].cl, data[index].cd };
    }
};

Aerodynamics naca0012(NACA_0012);
Aerodynamics naca2412(NACA_2412);

struct Engine
{   
    float throttle = 0.5f;    
    float thrust = 10000.0f; 

    Engine(float engine_thrust) : thrust(engine_thrust) {}
    
    void apply_forces(phi::RigidBody &rigid_body)
    {
        rigid_body.add_relative_force({ thrust * throttle, 0.0f, 0.0f });
        // TODO: implement torque from propeller
    }
};

struct Wing
{
    const float area{};
    const glm::vec3 position{};
    const Aerodynamics *aerodynamics;

    glm::vec3 normal;
    float lift_multiplier = 1.0f;
    float drag_multiplier = 1.0f;
    
    Wing(const glm::vec3 &offset, float wing_area, const Aerodynamics *aero, const glm::vec3 &wing_normal = phi::UP)
         : position(offset),
          area(wing_area),
          aerodynamics(aero),
          normal(wing_normal)
    {}

    Wing(const glm::vec3& offset, float wingspan, float chord, const Aerodynamics* aero, const glm::vec3& wing_normal = phi::UP)
        : position(offset),
        area(chord * wingspan),
        aerodynamics(aero),
        normal(wing_normal)
    {}
    
    static glm::vec3 calculate_normal(float incidence_angle_degrees)
    {
        float theta = glm::radians(incidence_angle_degrees + 90.0f);
        float x = cos(theta), y = sin(theta);
        return glm::normalize(glm::vec3(x, y, 0.0f));
    }
    
    glm::vec3 deflect(float angle, const glm::vec3& axis)
    {
        glm::mat4 rot(1.0f);
        rot = glm::rotate(rot, angle, axis);
        return rot * normal

    void set_incidence(float incidence)
    {
        normal = calculate_normal(incidence);
    }

    void apply_forces(phi::RigidBody &rigid_body)
    {
        auto local_velocity = rigid_body.get_point_velocity(position);
        auto speed = glm::length(local_velocity);

        if (speed <= 0.0f || area <= 0.0f)
            return;

        auto drag_direction = glm::normalize(-local_velocity);
        auto lift_direction = glm::normalize(glm::cross(glm::cross(drag_direction, normal), drag_direction));

        auto angle_of_attack = glm::degrees(std::asin(glm::dot(drag_direction, normal)));

        auto [lift_coefficient, drag_coefficient] = aerodynamics->sample(angle_of_attack);

        float tmp = phi::sq(speed) * phi::rho * area * 0.5f;
        auto lift = lift_direction * lift_coefficient * lift_multiplier * tmp;
        auto drag = drag_direction * drag_coefficient * drag_multiplier * tmp;

        rigid_body.add_force_at_point(lift + drag, position);
    }
};

glm::mat3 inertia = phi::inertia::tensor({100000.0f, 50000.0f, 500000.0f}); 

struct Aircraft
{
    Engine engine;
    std::vector<Wing> elements;
    phi::RigidBody rigid_body;
    glm::vec3 joystick{}; // roll, yaw, pitch

    float log_timer = 1.0f;
    const glm::vec3 control_torque = { 1500000.0f, 1000.0f, 1000000.0f };

#if 0
    Aircraft()
        : elements({
          Wing({-0.5f,   0.0f, -2.73f},      24.36f, &naca2412, phi::UP),       // left wing
          Wing({ 0.0f,   0.0f, -2.0f},        8.79f, &naca0012, phi::UP),       // left aileron
          Wing({ 0.0f,   0.0f,  2.0f},        8.79f, &naca0012, phi::UP),       // right aileron
          Wing({-0.5f,   0.0f,  2.73f},      24.36f, &naca2412, phi::UP),       // right wing
          Wing({-6.64f, -0.12f, 0.0f},       17.66f, &naca0012, phi::UP),       // elevator
          Wing({-6.64f,  0.0f,  0.0f},       16.46f, &naca0012, phi::RIGHT),    // rudder
        }),
        rigid_body({ .mass = 16000.0f, .inertia = inertia })
    {}
#endif

    Aircraft(float mass, float thrust, glm::mat3 inertia, std::vector<Wing> wings)
        : elements(wings), rigid_body({ .mass = mass, .inertia = inertia }), engine(thrust)
    {}

    static void init_flightmodel(Aircraft& aircraft)
    {
#if 0
        aircraft.rigid_body.mass = 16000.0f;
        aircraft.rigid_body.set_inertia(inertia);
        aircraft.elements = {
          Wing({-0.5f,   0.0f, -2.73f},      24.36f, &naca2412, phi::UP), // left wing
          Wing({ 0.0f,   0.0f, -2.0f},        8.79f, &naca0012, phi::UP), // left aileron
          Wing({ 0.0f,   0.0f,  2.0f},        8.79f, &naca0012, phi::UP), // right aieleron
          Wing({-0.5f,   0.0f,  2.73f},      24.36f, &naca2412, phi::UP), // right wing
          Wing({-6.64f, -0.12f, 0.0f},       17.66f, &naca0012, phi::UP), // elevator
          Wing({-6.64f,  0.0f,  0.0f},       16.46f, &naca0012, phi::RIGHT), // rudder
        };
        aircraft.engine.thrust = 20000.0f;
#endif
    }

    void update(phi::Seconds dt)
    {
        Wing& lw = elements[0];
        Wing& la = elements[1];
        Wing& ra = elements[2];
        Wing& rw = elements[3];
        Wing& el = elements[4];
        Wing& ru = elements[5];

#if 1
        float roll = joystick.x;
        float yaw = joystick.y;
        float pitch = joystick.z;
        float max_elevator_deflection = 5.0f, max_aileron_deflection = 15.0f;
        float aileron_deflection = roll * max_aileron_deflection;
        la.set_incidence(+aileron_deflection);
        ra.set_incidence(-aileron_deflection);
        el.set_incidence(-pitch * max_elevator_deflection);
#else
        float control_authority = phi::utils::clamp(glm::length(rigid_body.velocity) / 150.0f, 0.0f, 1.0f);
        rigid_body.add_relative_torque((joystick * control_torque) * control_authority);
#endif

        for (Wing& wing : elements)
        {
            wing.apply_forces(rigid_body);
        }    

        engine.apply_forces(rigid_body);

        if ((log_timer += dt) > 0.5f)
        {
            log_timer = 0;
#if 1
            printf(
                "%.2f km/h, thrtl: %.2f, alt: %.2f m\n", 
                phi::units::kilometer_per_hour(glm::length(rigid_body.velocity)),
                engine.throttle,
                rigid_body.position.y
            );
#endif
        }

        rigid_body.update(dt);
    }
};
