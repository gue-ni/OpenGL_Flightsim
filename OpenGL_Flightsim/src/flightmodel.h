#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>
#include <vector>

#include "data.h"
#include "gfx.h"
#include "phi.h"
#include "pid.h"

#define PITCH_RATE_CONTROL 0

// get temperture in kelvin
inline float get_air_temperature(float altitude) { return 288.15f - 0.0065f * altitude; }

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

  const float a = 1.83f, b = -1.32f;  // curve coefficients
  float propellor_advance_ratio = speed / ((propellor_rpm / 60.0f) * propellor_diameter);
  float propellor_efficiency = a * propellor_advance_ratio + std::pow(b * propellor_advance_ratio, 3.0f);

  const float C = 0.12f;
  float air_density = get_air_density(rb.position.y);
  float sea_level_air_density = get_air_density(0.0f);
  float power_drop_off_factor = ((air_density / sea_level_air_density) - C) / (1 - C);

  return ((propellor_efficiency * engine_power) / speed) * power_drop_off_factor;
}

float get_indicated_air_speed(float airspeed, float air_density, float sea_level_air_density) {
  // https://aerotoolbox.com/airspeed-conversions/
  float dynamic_pressure = 0.5f * air_density * phi::sq(airspeed);  // bernoulli's equation
  return std::sqrt(2 * dynamic_pressure / sea_level_air_density);
}

float get_indicated_air_speed(const phi::RigidBody& rb) {
  float airspeed = rb.get_speed();
  float air_density = get_air_density(rb.position.y);
  float sea_level_air_density = get_air_density(0.0f);
  return get_indicated_air_speed(airspeed, air_density, sea_level_air_density);
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
    assert(curve_data.size() > 0);
    min_alpha = curve_data.front().x;
    max_alpha = curve_data.back().x;
  }

  std::tuple<float, float> sample(float alpha) const {
    int max_index = data.size() - 1;
    int index = static_cast<int>(phi::utils::scale(alpha, min_alpha, max_alpha, 0.0f, static_cast<float>(max_index)));
    index = glm::clamp(index, 0, max_index);
    return {data[index].y, data[index].z};
  }
};

enum EngineType { PROPELLOR, JET };

struct Engine : public phi::ForceEffector {
  float thrust;
  float throttle = 0.25f;
  const EngineType engine_type = EngineType::JET;

  Engine(float thrust) : thrust(thrust), engine_type(EngineType::JET) {}
  Engine() : thrust(0.0f), engine_type(EngineType::PROPELLOR) {}

  void apply_forces(phi::RigidBody& rigid_body, phi::Seconds dt) override {
    if (engine_type == EngineType::PROPELLOR) {
      thrust = get_propellor_thrust(rigid_body, 1000.0f, 3000.0f, 3.0f);
    }
    rigid_body.add_relative_force({thrust * throttle, 0.0f, 0.0f});
  }
};

enum WingType { CONTROL_SURFACE, WING };

class Wing : public phi::ForceEffector {
 public:
  const float area{};  // m^2
  const Airfoil* airfoil;
  const glm::vec3 normal;
  const glm::vec3 center_of_pressure;
  const WingType type = WING;

  float lift_multiplier = 2.0f;
  float drag_multiplier = 1.0f;
  float actuator_speed = 90.0f;  // degrees per second

  Wing(const glm::vec3& position, float area, const Airfoil* aero, const glm::vec3& normal = phi::UP,
       const WingType type = WING)
      : center_of_pressure(position), area(area), airfoil(aero), normal(normal), wing_normal(normal), type(type) {}

  Wing(const glm::vec3& position, float wingspan, float chord, const Airfoil* aero, const glm::vec3& normal = phi::UP,
       const WingType type = WING)
      : Wing(position, wingspan * chord, aero, normal, type) {}

  // how far a control surface can be deflected (degrees)
  void set_deflection_limits(float min, float max) {
    assert(type == CONTROL_SURFACE);
    min_deflection = min, max_deflection = max;
  }

  // input controls wing deflection
  void set_control_input(float input) {
    assert(type == CONTROL_SURFACE);
    control_input = glm::clamp(input, -1.0f, 1.0f);
  }

  void apply_forces(phi::RigidBody& rigid_body, phi::Seconds dt) override {
    glm::vec3 local_velocity = rigid_body.get_point_velocity(center_of_pressure);
    float speed = glm::length(local_velocity);

    if (type == CONTROL_SURFACE) {
      // rotate wing according to control input
      apply_deflection(rigid_body, dt);
    }

    // drag acts in the opposite direction of velocity
    glm::vec3 drag_direction = glm::normalize(-local_velocity);

    // lift is always perpendicular to drag
    glm::vec3 lift_direction = glm::normalize(glm::cross(glm::cross(drag_direction, wing_normal), drag_direction));

    // angle between chord line and air flow
    float angle_of_attack = glm::degrees(std::atan(glm::dot(drag_direction, wing_normal)));

    // sample our aerodynamic data
    auto [lift_coefficient, drag_coefficient] = airfoil->sample(angle_of_attack);

    // air density depends on altitude
    float air_density = get_air_density(rigid_body.position.y);

    float tmp = std::pow(speed, 2) * air_density * area;
    glm::vec3 lift = lift_direction * lift_coefficient * lift_multiplier * tmp;
    glm::vec3 drag = drag_direction * drag_coefficient * drag_multiplier * tmp;

    // aerodynamic forces are applied at the center of pressure
    rigid_body.add_force_at_point(lift + drag, center_of_pressure);
  }

 private:
  float deflection = 0.0f;  // degrees
  float min_deflection = -10.0f;
  float max_deflection = 10.0f;
  float control_input = 0.0f;  // range [-1.0f, 1.0f]
  glm::vec3 wing_normal;       // is computed depending on deflection

  void apply_deflection(phi::RigidBody rigid_body, phi::Seconds dt) {
    float target_deflection = (control_input >= 0.0f ? max_deflection : min_deflection) * std::abs(control_input);

    // TODO: max deflection and actuator speed should depend on air speed
    deflection = phi::utils::move_towards(deflection, target_deflection, actuator_speed * dt);
    deflection = glm::clamp(deflection, min_deflection, max_deflection);

    auto axis = glm::normalize(glm::cross(phi::FORWARD, normal));
    auto rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deflection), axis);
    wing_normal = glm::vec3(rotation * glm::vec4(normal, 1.0f));
  }
};

struct Airplane {
  Engine engine;
  std::vector<Wing> elements;
  phi::RigidBody rigid_body;
  glm::vec3 joystick{};  // roll, yaw, pitch
  PID pid;

  Airplane(float mass, float thrust, glm::mat3 inertia, std::vector<Wing> wings)
      : elements(wings), rigid_body({.mass = mass, .inertia = inertia}), engine(thrust), pid(15.0f, 0.1f, 0.0f, false) {
    assert(elements.size() >= 6U);
    elements[1].set_deflection_limits(-15.0f, 15.0f);  // aileron
    elements[2].set_deflection_limits(-15.0f, 15.0f);  // aileron
    elements[4].set_deflection_limits(-10.0f, 5.0f);   // elevator
    elements[5].set_deflection_limits(-3.0f, 3.0f);    // rudder
  }

  void update(phi::Seconds dt) {
    // control input
    float aileron = joystick.x, rudder = joystick.y, elevator = joystick.z;

    elements[1].set_control_input(+aileron);
    elements[2].set_control_input(-aileron);
    elements[5].set_control_input(-rudder);

#if PITCH_RATE_CONTROL
    // pitch input controls rate of pitch (radians/second)
    constexpr float max_pitch_rate = glm::radians(30.0f);
    float target_pitch_rate = max_pitch_rate * pitch;
    float current_pitch_rate = rigid_body.angular_velocity.z;
    pitch = pid.calculate(current_pitch_rate, target_pitch_rate, dt);
    // printf("pitch %.2f, c = %.2f, t = %.2f,  i = %.2f\n", pitch, current_pitch_rate, target_pitch_rate, input);
#endif

    elements[4].set_control_input(-elevator);

    for (Wing& wing : elements) {
      wing.apply_forces(rigid_body, dt);
    }

    engine.apply_forces(rigid_body, dt);

    rigid_body.update(dt);
  }
};
