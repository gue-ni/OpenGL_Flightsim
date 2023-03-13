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
#define LOG_FLIGHT 1

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
float get_indicated_air_speed(float speed, float altitude) {
  const float air_density = get_air_density(altitude);
  const float sea_level_air_density = get_air_density(0.0f);
  const float dynamic_pressure = 0.5f * air_density * phi::sq(speed);  // bernoulli's equation
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

float get_mach_number(float speed, float altitude) {
  float temperature = get_air_temperature(altitude);
  float speed_of_sound = std::sqrt(1.402f * 286.f * temperature);
  return speed / speed_of_sound;
}

struct Airfoil {
  float min_alpha, max_alpha;
  std::vector<glm::vec4> data;  // alpha, cl, cd

  Airfoil(const std::vector<glm::vec4>& curve) : data(curve) {
    min_alpha = curve.front().x, max_alpha = curve.back().x;
  }

  // get lift coefficent and drag coefficient
  std::tuple<float, float, float> sample(float alpha) const {
    int max_index = data.size() - 1;
    float t = phi::inverse_lerp(min_alpha, max_alpha, alpha) * max_index;
    float integer = std::floor(t);
    float fractional = t - integer;
    int index = static_cast<int>(integer);
    auto value = (index < max_index) ? phi::lerp(data[index], data[index + 1], fractional) : data[max_index];
    return {value.y, value.z, value.w};
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
  const float efficiency_factor;
  float incidence;

  float lift_multiplier = 1.0f;
  float drag_multiplier = 1.0f;

  float deflection = 0.0f;
  float control_input = 0.0f;
  float min_deflection = -10.0f;
  float max_deflection = +10.0f;
  float max_actuator_speed = 90.0f;
  float max_actuator_torque = 6000.0f;
  bool is_control_surface = true;

#if DEBUG_FLIGHTMODEL
  bool log = false;
  float log_timer = 0.0f;
  std::string name = "None";
#endif

  Wing(const Airfoil* airfoil, const glm::vec3& position, float area, float span, const glm::vec3& normal = phi::UP)
      : airfoil(airfoil),
        center_of_pressure(position),
        area(area),
        chord(area / span),
        wingspan(span),
        normal(normal),
        incidence(0.0f),
        efficiency_factor(1.0f),
        aspect_ratio(std::pow(span, 2) / area) {}

  // controls how much the wing is deflected
  void set_control_input(float input) { control_input = glm::clamp(input, -1.0f, 1.0f); }

  // how far the wing can be deflected, degrees
  void set_deflection_limits(float min, float max) { min_deflection = min, max_deflection = max; }

  // compute and apply aerodynamic forces
  void apply_forces(phi::RigidBody& rigid_body, phi::Seconds dt) override {
    glm::vec3 local_velocity = rigid_body.get_point_velocity(center_of_pressure);
    float speed = glm::length(local_velocity);

    if (speed <= phi::EPSILON) return;

    // control surfaces can be rotated
    glm::vec3 wing_normal = is_control_surface ? deflect_wing(rigid_body, dt) : normal;

    // drag acts in the opposite direction of velocity
    glm::vec3 drag_direction = glm::normalize(-local_velocity);

    // lift is always perpendicular to drag
    glm::vec3 lift_direction = glm::normalize(glm::cross(glm::cross(drag_direction, wing_normal), drag_direction));

    // angle between chord line and air flow
    float angle_of_attack = glm::degrees(std::asin(glm::dot(drag_direction, wing_normal)));

    // sample our aerodynamic data
    auto [lift_coefficient, drag_coefficient, moment_coefficient] = airfoil->sample(angle_of_attack);

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
      // printf("[%s] aoa = %.2f, cl = %.2f, t = %.2f, p = %.2f\n", name.c_str(), angle_of_attack, lift_coefficient,
      // torque.z, rigid_body.angular_velocity.z); printf("[%s] aoa = %.2f, cl = %.2f p = %.2f\n", name.c_str(),
      // angle_of_attack, lift_coefficient, rigid_body.angular_velocity.z);
      printf("[%s] aoa = %.2f, cl = %.2f cd = %.2f, cdi = %2f\n", name.c_str(), angle_of_attack, lift_coefficient,
             drag_coefficient, induced_drag_coefficient);
    }
#endif

    // aerodynamic forces are applied at the center of pressure
    rigid_body.add_force_at_point(lift + drag, center_of_pressure);
  }

  // returns updated wing normal according to control input and deflection
  glm::vec3 deflect_wing(phi::RigidBody& rigid_body, phi::Seconds dt) {
#if 0
    // with increased speed control surfaces become harder to move
    float torque = std::pow(rigid_body.get_speed(), 2) * area;
    float compression = glm::degrees(std::asin(max_actuator_torque / torque));
    compression = glm::clamp(compression, 0.0f, 1.0f);

    float target_deflection =
        (control_input >= 0.0f ? max_deflection : min_deflection) * compression * std::abs(control_input);
    deflection = phi::move_towards(deflection, target_deflection, max_actuator_speed * compression * dt);
#else
    deflection = (control_input >= 0.0f ? max_deflection : min_deflection) * std::abs(control_input);
#endif

    auto axis = glm::normalize(glm::cross(phi::FORWARD, normal));
    auto rotation = glm::rotate(glm::mat4(1.0f), glm::radians(incidence + deflection), axis);
    return glm::vec3(rotation * glm::vec4(normal, 1.0f));
  }
};

struct Airplane {
  Engine engine;
  std::vector<Wing> wings;
  phi::RigidBody rigid_body;
  glm::vec3 joystick{};  // roll, yaw, pitch
  float trim = 0.0f;

#if LOG_FLIGHT
  float log_timer = 0.0f;
  float log_intervall = 0.1f;
  float flight_time = 0.0f;
  std::ofstream log_file;
#endif

  Airplane(float mass, float thrust, glm::mat3 inertia, std::vector<Wing> elements)
      : wings(elements), rigid_body({.mass = mass, .inertia = inertia}), engine(thrust) {
#if DEBUG_FLIGHTMODEL
    wings[0].log = true;
    wings[0].name = "lw";

    wings[1].log = false;
    wings[1].name = "la";

    wings[2].log = false;
    wings[2].name = "ra";

    wings[3].log = false;
    wings[3].name = "rw";

    wings[4].log = true;
    wings[4].name = "el";

    wings[5].log = false;
    wings[5].name = "rd";
#endif
#if LOG_FLIGHT
    std::time_t now = std::time(nullptr);
    log_file.open("log/flight_log_" + std::to_string(now) + ".csv");
    log_file
        << "flight_time,altitude,speed,ias,aoa,roll_rate,yaw_rate,pitch_rate,roll,yaw,pitch,aileron,rudder,elevator,\n";
#endif

    // main wings
    wings[0].is_control_surface = false;
    wings[3].is_control_surface = false;
  }

  void update(phi::Seconds dt) {
    float aileron = joystick.x, rudder = joystick.y, elevator = joystick.z;

    wings[1].set_control_input(+aileron);
    wings[2].set_control_input(-aileron);
    wings[4].set_control_input(-elevator);
    wings[5].set_control_input(-rudder);
    wings[4].incidence = trim * 10.0f;

#if LOG_FLIGHT
    flight_time += dt;
    if ((log_timer -= dt) <= 0.0f) {
      log_timer = log_intervall;
      auto av = glm::degrees(rigid_body.angular_velocity);
      auto euler = glm::degrees(glm::eulerAngles(glm::normalize(rigid_body.orientation)));

      log_file 
          << flight_time << "," 
          << get_altitude() << "," 
          << get_speed() << "," 
          << get_ias() << "," 
          << get_aoa() << "," 
          << av.x << "," // deg/s
          << av.y << "," 
          << av.z << "," 
          << euler.x << "," // deg
          << euler.y << "," 
          << euler.z << ","
          << wings[1].incidence << "," // aileron
          << wings[5].incidence << "," // rudder
          << wings[4].incidence << "," // elevator
          << std::endl;
    }
#endif




    for (Wing& wing : wings) {
      wing.apply_forces(rigid_body, dt);
    }

    engine.apply_forces(rigid_body, dt);

    rigid_body.update(dt);
  }

  float get_altitude() const { return rigid_body.position.y; }
  float get_speed() const { return rigid_body.get_speed(); }
  float get_ias() const { return get_indicated_air_speed(get_speed(), get_altitude()); }
  float get_g() const { return get_g_force(rigid_body); }
  float get_mach() const { return get_mach_number(get_speed(), get_altitude()); }
  float get_aoa() const {
    auto drag_direction = -rigid_body.get_body_velocity();
    return glm::degrees(std::asin(glm::dot(glm::normalize(drag_direction), phi::UP)));
  }
};
