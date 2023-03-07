#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>
#include <vector>

#include "data.h"
#include "gfx.h"
#include "phi.h"

#define DEBUG_FLIGHTMODEL 0

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

  const float a = 1.83f, b = -1.32f;
  float propellor_advance_ratio = speed / ((propellor_rpm / 60.0f) * propellor_diameter);
  float propellor_efficiency = a * propellor_advance_ratio + std::pow(b * propellor_advance_ratio, 3.0f);

  const float C = 0.12f;
  float air_density = get_air_density(rb.position.y);
  float sea_level_air_density = get_air_density(0.0f);
  float power_drop_off_factor = ((air_density / sea_level_air_density) - C) / (1 - C);

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
  std::vector<glm::vec3> data;  // alpha, cl, cd

  Airfoil(const std::vector<glm::vec3>& curve) : data(curve) {
    min_alpha = curve.front().x;
    max_alpha = curve.back().x;
  }

  // get lift coefficent and drag coefficient
  std::tuple<float, float> sample(float alpha) const {
    int max_index = static_cast<int>(data.size() - 1);
    int index = phi::utils::inverse_lerp(min_alpha, max_alpha, alpha) * max_index;
    index = glm::clamp(index, 0, max_index);
    return {data[index].y, data[index].z};
  }
};

struct Engine : public phi::ForceEffector {
  float throttle = 0.25f;
  float thrust = 10000.0f;

  Engine(float thrust) : thrust(thrust) {}

  void apply_forces(phi::RigidBody& rigid_body, phi::Seconds dt) override {
    rigid_body.add_relative_force({thrust * throttle, 0.0f, 0.0f});
  }
};

struct Wing : public phi::ForceEffector {
  const float area;
  const float wingspan;
  const float chord;
  const float aspect_ratio;
  const Airfoil* airfoil;
  const glm::vec3 normal;
  const glm::vec3 center_of_pressure;
  const float incidence;
  const float efficiency_factor;
  const bool is_control_surface = true;

  float lift_multiplier = 1.0f;
  float drag_multiplier = 1.0f;

  float deflection = 0.0f;
  float control_input = 0.0f;
  const float actuator_speed = 90.0f;
  const float min_deflection = -10.0f;
  const float max_deflection = +10.0f;

#if DEBUG_FLIGHTMODEL
  bool log = false;
  float log_timer = 0.0f;
  std::string name = "None";
#endif

  Wing(const glm::vec3& position, float wingspan, float chord, const Airfoil* airfoil,
       const glm::vec3& normal = phi::UP, float incidence = 0.0f)
      : center_of_pressure(position),
        area(chord * wingspan),
        chord(chord),
        wingspan(wingspan),
        airfoil(airfoil),
        normal(normal),
        efficiency_factor(1.0f),
        incidence(incidence),
        aspect_ratio(std::pow(wingspan, 2) / area) {}

  // controls how much the wing is deflected
  void set_control_input(float input) { control_input = glm::clamp(input, -1.0f, 1.0f); }

  // how far the wing can be deflected, degrees
  void set_deflection_limits(float min, float max) { min_deflection = min, max_deflection = max; }

  // compute and apply aerodynamic forces
  void apply_forces(phi::RigidBody& rigid_body, phi::Seconds dt) override {
    glm::vec3 local_velocity = rigid_body.get_point_velocity(center_of_pressure);
    float speed = glm::length(local_velocity);

    if (speed <= phi::EPSILON) return;

    glm::vec3 wing_normal = is_control_surface ? deflect_wing(rigid_body, dt) : normal;

    // drag acts in the opposite direction of velocity
    glm::vec3 drag_direction = glm::normalize(-local_velocity);

    // lift is always perpendicular to drag
    glm::vec3 lift_direction = glm::normalize(glm::cross(glm::cross(drag_direction, wing_normal), drag_direction));

    // angle between chord line and air flow
    float angle_of_attack = glm::degrees(std::asin(glm::dot(drag_direction, wing_normal)));

    // sample our aerodynamic data
    auto [lift_coefficient, drag_coefficient] = airfoil->sample(angle_of_attack);

    // induced drag
    float induced_drag_coefficient = std::pow(lift_coefficient, 2) / (phi::PI * aspect_ratio * efficiency_factor);

    // air density depends on altitude
    float air_density = get_air_density(rigid_body.position.y);

    float tmp = 0.5f * std::pow(speed, 2) * air_density * area;
    glm::vec3 lift = lift_direction * lift_coefficient * lift_multiplier * tmp;
    glm::vec3 drag = drag_direction * (drag_coefficient + induced_drag_coefficient) * drag_multiplier * tmp;

#if DEBUG_FLIGHTMODEL
    if (log && (log_timer -= dt) <= 0.0f) {
      log_timer = 0.2f;

      auto force = rigid_body.transform_direction(lift);
      auto torque = glm::cross(center_of_pressure, lift);

      printf("[%s] aoa = %.2f, cl = %.2f, t = %.2f, p = %.2f\n", name.c_str(), angle_of_attack, lift_coefficient,
             torque.z, rigid_body.angular_velocity.z);
    }
#endif

    // aerodynamic forces are applied at the center of pressure
    rigid_body.add_force_at_point(lift + drag, center_of_pressure);
  }

  // returns updated wing normal according to control input and deflection
  glm::vec3 deflect_wing(phi::RigidBody& rigid_body, phi::Seconds dt) {
    // TODO: actuator speed and deflection limits should depend on speed
    float target_deflection = (control_input >= 0.0f ? max_deflection : min_deflection) * std::abs(control_input);
    deflection = phi::utils::move_towards(deflection, target_deflection, actuator_speed * dt);
    auto axis = glm::normalize(glm::cross(phi::FORWARD, normal));
    auto rotation = glm::rotate(glm::mat4(1.0f), glm::radians(incidence + deflection), axis);
    return glm::vec3(rotation * glm::vec4(normal, 1.0f));
  }
};

struct Airplane {
  Engine engine;
  std::vector<Wing> elements;
  phi::RigidBody rigid_body;
  glm::vec3 joystick{};  // roll, yaw, pitch

  Airplane(float mass, float thrust, glm::mat3 inertia, std::vector<Wing> wings)
      : elements(wings), rigid_body({.mass = mass, .inertia = inertia}), engine(thrust) {
#if DEBUG_FLIGHTMODEL
    elements[0].log = false;
    elements[0].name = "lw";

    elements[1].log = false;
    elements[1].name = "la";

    elements[2].log = false;
    elements[2].name = "ra";

    elements[3].log = false;
    elements[3].name = "rw";

    elements[4].log = true;
    elements[4].name = "el";

    elements[5].log = false;
    elements[5].name = "rd";
#endif
  }

  void update(phi::Seconds dt) {
#if 1
    float aileron = joystick.x, rudder = joystick.y, elevator = joystick.z;
    elements[1].set_control_input(+aileron);
    elements[2].set_control_input(-aileron);
    elements[4].set_control_input(-elevator);
    elements[5].set_control_input(-rudder);
#else
    rigid_body.add_relative_torque(glm::vec3(400000.0f, 100000.0f, 1500000.0f) * joystick);
#endif

    for (Wing& wing : elements) {
      wing.apply_forces(rigid_body, dt);
    }

    engine.apply_forces(rigid_body, dt);

    rigid_body.update(dt);
  }
};
