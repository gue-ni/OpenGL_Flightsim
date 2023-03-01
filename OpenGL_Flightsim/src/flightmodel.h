#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>
#include <vector>

#include "data.h"
#include "gfx.h"
#include "phi.h"

// get temperture in kelvin
inline float get_air_temperature(float altitude) {
  return 288.15f - 0.0065f * altitude;  // kelvin
}

// only accurate for altitudes < 11km
float get_air_density(float altitude) {
  assert(0.0f <= altitude && altitude <= 11000.0f);
  float temperature = get_air_temperature(altitude);  // kelvin
  float pressure = 101325.0f * std::pow(1 - 0.0065f * (altitude / 288.15f), 5.25f);
  return 0.00348f * (pressure / temperature);
}

float get_propellor_thrust(const phi::RigidBody& rb, float engine_horsepower, float propellor_rpm,
                           float propellor_diameter) {
  float speed = rb.get_speed();
  float engine_power = phi::units::watts(engine_horsepower);

#if 1
  const float a = 1.83f, b = -1.32f;
  float propellor_advance_ratio = speed / ((propellor_rpm / 60.0f) * propellor_diameter);
  float propellor_efficiency = a * propellor_advance_ratio + std::pow(b * propellor_advance_ratio, 3.0f);
#else
  float propellor_efficiency = 1.0f;
#endif

#if 1
  const float C = 0.12f;
  float air_density = get_air_density(rb.position.y);
  float sea_level_air_density = get_air_density(0.0f);
  float power_drop_off_factor = ((air_density / sea_level_air_density) - C) / (1 - C);
#else
  float power_drop_off_factor = 1.0f;
#endif

  return ((propellor_efficiency * engine_power) / speed) * power_drop_off_factor;
}

// https://aerotoolbox.com/airspeed-conversions/
float get_indicated_air_speed(const phi::RigidBody& rb) {
  const float airspeed = rb.get_speed();
  const float air_density = get_air_density(rb.position.y);
  const float sea_level_air_density = get_air_density(0.0f);
  const float dynamic_pressure = 0.5f * air_density * phi::sq(airspeed);  // bernoulli's equation
  return std::sqrt(2 * dynamic_pressure / sea_level_air_density);
}

// get g-force in pitch direction
float get_g_force(const phi::RigidBody& rb) {
  auto velocity = rb.get_body_velocity();
  auto angular_velocity = rb.angular_velocity;

  // avoid division by zero
  float turn_radius = (std::abs(angular_velocity.z) < phi::EPSILON) ? std::numeric_limits<float>::max()
                                                                    : velocity.x / angular_velocity.z;

  // centrifugal force = mass * velocity^2 / radius
  // centrifugal acceleration = force / mass
  // simplified, this results in:
  float centrifugal_acceleration = phi::sq(velocity.x) / turn_radius;

  float g_force = centrifugal_acceleration / phi::EARTH_GRAVITY;
  g_force += (rb.up().y * phi::EARTH_GRAVITY) / phi::EARTH_GRAVITY;  // add earth gravity
  return g_force;
}

float get_mach_number(const phi::RigidBody& rb) {
  float speed = rb.get_speed();
  float temperature = get_air_temperature(rb.position.y);
  float speed_of_sound = std::sqrt(1.402f * 286.f * temperature);
  return speed / speed_of_sound;
}

struct Airfoil {
  float min_alpha, max_alpha;
  std::vector<glm::vec3> data;

  Airfoil(const std::vector<glm::vec3>& curve_data) : data(curve_data) {
    min_alpha = curve_data[0].x;
    max_alpha = curve_data[curve_data.size() - 1].x;
  }

  std::tuple<float, float> sample(float alpha) const {
    int max_index = static_cast<int>(data.size() - 1);
    int index = static_cast<int>(phi::utils::scale(alpha, min_alpha, max_alpha, 0.0f, static_cast<float>(max_index)));
    assert(0 <= index && index < max_index);
    return {data[index].y, data[index].z};
  }
};

Airfoil NACA_0012(NACA_0012_data);
Airfoil NACA_2412(NACA_2412_data);

struct Engine : public phi::ForceEffector {
  float throttle = 0.25f;
  float thrust = 10000.0f;

  Engine(float thrust) : thrust(thrust) {}

  void apply_forces(phi::RigidBody& rigid_body) override {
    float force = thrust * throttle;
    rigid_body.add_relative_force({force, 0.0f, 0.0f});
  }
};

struct Wing : public phi::ForceEffector {
  const float area{};  // m^2
  const Airfoil* airfoil;
  const glm::vec3 normal;
  const glm::vec3 center_of_pressure;

  float lift_multiplier = 1.0f;
  float drag_multiplier = 1.0f;
  phi::Degrees deflection = 0.0f;

  Wing(const glm::vec3& position, float area, const Airfoil* aero, const glm::vec3& normal = phi::UP)
      : center_of_pressure(position), area(area), airfoil(aero), normal(normal) {}

  Wing(const glm::vec3& position, float wingspan, float chord, const Airfoil* aero, const glm::vec3& normal = phi::UP)
      : center_of_pressure(position), area(chord * wingspan), airfoil(aero), normal(normal) {}

  void apply_forces(phi::RigidBody& rigid_body) override {
    glm::vec3 local_velocity = rigid_body.get_point_velocity(center_of_pressure);
    float speed = glm::length(local_velocity);

    if (speed <= 0.0f) return;

    glm::vec3 wing_normal = normal;

    if (std::abs(deflection) > phi::EPSILON) {
      // rotate wing
      auto axis = glm::normalize(glm::cross(phi::FORWARD, normal));
      auto rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deflection), axis);
      wing_normal = glm::vec3(rotation * glm::vec4(normal, 1.0f));
    }

    // drag acts in the opposite direction of velocity
    glm::vec3 drag_direction = glm::normalize(-local_velocity);

    // lift is always perpendicular to drag
    glm::vec3 lift_direction = glm::normalize(glm::cross(glm::cross(drag_direction, wing_normal), drag_direction));

    // angle between chord line and air flow
    float angle_of_attack = glm::degrees(std::asin(glm::dot(drag_direction, wing_normal)));

    // sample our aerodynamic data
    auto [lift_coefficient, drag_coefficient] = airfoil->sample(angle_of_attack);

    // air density depends on altitude
    float air_density = get_air_density(rigid_body.position.y);

    float tmp = 0.5f * std::pow(speed, 2) * air_density * area;
    glm::vec3 lift = lift_direction * lift_coefficient * lift_multiplier * tmp;
    glm::vec3 drag = drag_direction * drag_coefficient * drag_multiplier * tmp;

    // aerodynamic forces are applied at the center of pressure
    rigid_body.add_force_at_point(lift + drag, center_of_pressure);
  }
};

struct Aircraft {
  Engine engine;
  std::vector<Wing> elements;
  phi::RigidBody rigid_body;
  glm::vec3 joystick{};  // roll, yaw, pitch

  Aircraft(float mass, float thrust, glm::mat3 inertia, std::vector<Wing> wings)
      : elements(wings), rigid_body({.mass = mass, .inertia = inertia}), engine(thrust) {}

  void update(phi::Seconds dt) {
    float roll = joystick.x;
    float yaw = joystick.y;
    float pitch = joystick.z;
    float max_elevator_deflection = 5.0f, max_aileron_deflection = 15.0f, max_rudder_deflection = 3.0f;
    float aileron_deflection = roll * max_aileron_deflection;

    elements[1].deflection = +aileron_deflection;
    elements[2].deflection = -aileron_deflection;
    elements[4].deflection = -(pitch * max_elevator_deflection);
    elements[5].deflection = yaw * max_rudder_deflection;

    for (Wing& wing : elements) {
      wing.apply_forces(rigid_body);
    }

    engine.apply_forces(rigid_body);

    rigid_body.update(dt);
  }
};
