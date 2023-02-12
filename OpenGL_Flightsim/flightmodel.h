#pragma once

#include "phi.h"
#include "gfx.h"
#include "data.h"

#include <cmath>
#include <vector>
#include <tuple>
#include <algorithm>

struct Airfoil
{
    float min, max;
    std::vector<ValueTuple> data;

    Airfoil(const std::vector<ValueTuple> &curve_data) : data(curve_data)
    {
        min = curve_data[0].alpha;
        max = curve_data[curve_data.size() - 1].alpha;
    }

    std::tuple<float, float> sample(float alpha) const
    {
        int index = static_cast<int>(phi::utils::scale(alpha, min, max, 0, data.size() - 1));
        index = phi::utils::clamp(index, 0, static_cast<int>(data.size() - 1U));
        if (!(0 <= index && index < data.size()))
        {
            printf("alpha = %f, index = %d, size = %d\n", alpha, index, (int)data.size());
            assert(false);
        }
        return { data[index].cl, data[index].cd };
    }
};

Airfoil NACA_0012(NACA_0012_data);
Airfoil NACA_2412(NACA_2412_data);

float air_density(float altitude, float sea_level_pressure, float temperature)
{
    return 0.0f;
}

struct Engine
{   
    float throttle = 0.5f;    
    float thrust = 10000.0f; 
    float horsepower = 1000.0f;
    float rpm = 2400.0f;
    float propellor_diameter = 1.8f;

    Engine(float thrust) : thrust(thrust) {}
    
    void apply_forces(phi::RigidBody &rigid_body)
    {
#if 1
        float force = thrust * throttle;
#else
#endif
        rigid_body.add_relative_force({ force, 0.0f, 0.0f });

        // TODO: implement torque from propeller
    }
};

struct Wing
{
    const float area{};
    const glm::vec3 position{};
    const Airfoil *airfoil;

    glm::vec3 normal;
    float lift_multiplier = 1.0f;
    float drag_multiplier = 1.0f;
    phi::Degrees deflection = 0.0f;
    
    Wing(const glm::vec3 &position, float area, const Airfoil *aero, const glm::vec3 &normal = phi::UP)
         : position(position),
          area(area),
          airfoil(aero),
          normal(normal)
    {}

    Wing(const glm::vec3& position, float wingspan, float chord, const Airfoil* aero, const glm::vec3& normal = phi::UP)
        : position(position),
        area(chord * wingspan),
        airfoil(aero),
        normal(normal)
    {}
    
    void apply_forces(phi::RigidBody &rigid_body)
    {
        glm::vec3 local_velocity = rigid_body.get_point_velocity(position);
        float speed = glm::length(local_velocity);

        if (speed <= 0.0f)
            return;

        glm::vec3 wing_normal = normal;

        if (abs(deflection) > phi::epsilon)
        {
            // set rotation of wing
            glm::mat4 rot(1.0f);
            auto axis = glm::normalize(glm::cross(phi::FORWARD, normal));
            rot = glm::rotate(rot, glm::radians(deflection), axis);
            wing_normal = glm::vec3(rot * glm::vec4(normal, 1.0f));
        }

        // drag acts in the opposite direction of velocity
        glm::vec3 drag_direction = glm::normalize(-local_velocity);

        // lift is always perpendicular to drag
        glm::vec3 lift_direction 
            = glm::normalize(glm::cross(glm::cross(drag_direction, wing_normal), drag_direction));

        // angle between wing and air flow
        float angle_of_attack = glm::degrees(std::asin(glm::dot(drag_direction, wing_normal)));

        // sample our aerodynamic data
        auto [lift_coefficient, drag_coefficient] = airfoil->sample(angle_of_attack);

        float air_density = phi::rho;

        float tmp = 0.5f * phi::sq(speed) * air_density * area;
        glm::vec3 lift = lift_direction * lift_coefficient * lift_multiplier * tmp;
        glm::vec3 drag = drag_direction * drag_coefficient * drag_multiplier * tmp;

        // apply forces
        rigid_body.add_force_at_point(lift + drag, position);
    }
};

struct Aircraft
{
    Engine engine;
    std::vector<Wing> elements;
    phi::RigidBody rigid_body;
    glm::vec3 joystick{}; // roll, yaw, pitch

    float log_timer = 1.0f;

    Aircraft(float mass, float thrust, glm::mat3 inertia, std::vector<Wing> wings)
        : elements(wings), rigid_body({ .mass = mass, .inertia = inertia }), engine(thrust)
    {}

    void update(phi::Seconds dt)
    {
#if 1
        Wing& la = elements[1];
        Wing& ra = elements[2];
        Wing& el = elements[4];
        Wing& ru = elements[5];

        float roll = joystick.x;
        float yaw = joystick.y;
        float pitch = joystick.z;
        float max_elevator_deflection = 5.0f, max_aileron_deflection = 15.0f, max_rudder_deflection = 5.0f;
        float aileron_deflection = roll * max_aileron_deflection;

        la.deflection = +aileron_deflection;
        ra.deflection = -aileron_deflection;
        el.deflection = -(pitch * max_elevator_deflection);
        ru.deflection = yaw * max_rudder_deflection;
#else
        const glm::vec3 control_torque = { 1500000.0f, 1000.0f, 1000000.0f };
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
                "%.2f km/h, thr: %.2f, alt: %.2f m\n", 
                phi::units::kilometer_per_hour(glm::length(rigid_body.velocity)),
                engine.throttle,
                rigid_body.position.y
            );
#endif
        }

        rigid_body.update(dt);
    }
};

void autopilot(Aircraft& aircraft, glm::vec3& target) {
    auto& rb = aircraft.rigid_body;
    auto dir = rb.inverse_transform_direction(target - rb.position);

} 
