#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>
#include <vector>

#include "data.h"
#include "gfx.h"
#include "phi.h"

#define LOG_FLIGHT 0

//  International Standard Atmosphere (ISA)
namespace isa
{
// get temperture in kelvin
inline float get_air_temperature(float altitude)
{
  assert(0.0f <= altitude && altitude <= 11000.0f);
  return 288.15f - 0.0065f * altitude;
}

// only accurate for altitudes < 11km
float get_air_density(float altitude)
{
  assert(0.0f <= altitude && altitude <= 11000.0f);
  float temperature = get_air_temperature(altitude);
  float pressure = 101325.0f * std::pow(1 - 0.0065f * (altitude / 288.15f), 5.25f);
  return 0.00348f * (pressure / temperature);
}

const float sea_level_air_density = get_air_density(0.0f);
};  // namespace isa

// World Geodetic System (WGS 84)
// TODO: fix calculations
namespace wgs84
{
constexpr float EARTH_RADIUS = 6378.0f;

glm::vec2 coordinate_diff_to_meters(const glm::vec2& diff, float latitude)
{
  float km_per_latitude = (phi::PI / 180.0f) * EARTH_RADIUS;
  float km_per_longitude = (phi::PI / 180.0f) * EARTH_RADIUS * cos(latitude * phi::PI / 180.0f);
  return glm::vec2(km_per_latitude, km_per_longitude) / 1000.0f;
}

// origin is lat/lon, offset in meters
glm::vec2 lat_lon_from_offset(const glm::vec2& origin, const glm::vec2& offset)
{
  float latitude = origin.x, longitude = origin.y;
  float new_latitude = latitude + (offset.y / EARTH_RADIUS) * (180.0f / phi::PI);
  float new_longitude = longitude + (offset.x / EARTH_RADIUS) * (180.0f / phi::PI) / cos(latitude * phi::PI / 180.0f);
  return glm::vec2(new_latitude, new_longitude);
}
}  // namespace wgs84

// AoA, Cl, Cd
using AeroData = glm::vec3;

// aerodynamic data sampler
struct Airfoil {
  float min_alpha, max_alpha;
  float cl_max;
  int max_index;
  std::vector<AeroData> data;

  Airfoil(const std::vector<AeroData>& curve) : data(curve)
  {
    min_alpha = curve.front().x, max_alpha = curve.back().x;
    max_index = static_cast<int>(data.size() - 1);

    cl_max = 0.0f;

    for (auto& val : curve) {
      if (val.y > cl_max) cl_max = val.y;
    }
  }

  // lift_coeff, drag_coeff
  std::tuple<float, float> sample(float alpha) const
  {
    float t = phi::inverse_lerp(min_alpha, max_alpha, alpha) * max_index;
    float integer = std::floor(t);
    float fractional = t - integer;
    int index = static_cast<int>(integer);
    auto value = (index < max_index) ? phi::lerp(data[index], data[index + 1], fractional) : data[max_index];
    return {value.y, value.z};
  }
};

// base engine
struct Engine {
  float throttle = 0.25f;
  glm::vec3 relative_position = glm::vec3(0);  // position relative to cg
  virtual void apply_forces(phi::RigidBody* rigid_body, phi::Seconds dt) = 0;
};

// simple jet-like engine
struct SimpleEngine : public Engine {
  const float thrust;
  SimpleEngine(float thrust) : thrust(thrust) {}

  void apply_forces(phi::RigidBody* rigid_body, phi::Seconds dt) override
  {
    rigid_body->add_force_at_point({throttle * thrust, 0.0f, 0.0f}, relative_position);
  }
};

// does not yet implement engine torque
struct PropellerEngine : public Engine {
  float horsepower, rpm, propellor_diameter;

  PropellerEngine(float horsepower, float rpm, float diameter)
      : horsepower(horsepower), rpm(rpm), propellor_diameter(diameter)
  {
  }

  void apply_forces(phi::RigidBody* rigid_body, phi::Seconds dt) override
  {
    float speed = rigid_body->get_speed();
    float altitude = rigid_body->position.y;
    float engine_power = phi::units::watts(horsepower) * throttle;

    const float a = 1.83f, b = -1.32f;  // efficiency curve fit coefficients
    float turnover_rate = rpm / 60.0f;
    float propellor_advance_ratio = speed / (turnover_rate * propellor_diameter);
    float propellor_efficiency = a * propellor_advance_ratio + b * phi::cb(propellor_advance_ratio);
    assert(0.0f <= propellor_efficiency && propellor_efficiency <= 1.0f);

    const float c = 0.12f;  // mechanical power loss factor
    float air_density = isa::get_air_density(altitude);
    float power_drop_off_factor = ((air_density / isa::sea_level_air_density) - c) / (1 - c);
    assert(0.0f <= power_drop_off_factor && power_drop_off_factor <= 1.0f);

    float thrust = ((propellor_efficiency * engine_power) / speed) * power_drop_off_factor;
    assert(0.0f < thrust);
    rigid_body->add_force_at_point({thrust, 0.0f, 0.0f}, relative_position);
  }
};

// wing element
struct Wing {
  const float area;
  const float wingspan;
  const float chord;
  const float aspect_ratio;
  const Airfoil* airfoil;
  const glm::vec3 normal;
  const glm::vec3 center_of_pressure;
  const float flap_ratio;  // percentage of wing that is part of the flap
  const float efficiency_factor = 1.0f;

  float control_input = 0.0f;

  // relative position of leading edge to cg
  Wing(const Airfoil* airfoil, const glm::vec3& relative_position, float area, float span, const glm::vec3& normal,
       float flap_ratio = 0.25f)
      : airfoil(airfoil),
        center_of_pressure(relative_position),
        area(area),
        chord(area / span),
        wingspan(span),
        normal(normal),
        aspect_ratio(phi::sq(span) / area),
        flap_ratio(flap_ratio)
  {
  }

  Wing(const glm::vec3& position, float span, float chord, const Airfoil* airfoil, const glm::vec3& normal,
       float flap_ratio = 0.25f)
      : airfoil(airfoil),
        center_of_pressure(position),
        area(span * chord),
        chord(chord),
        wingspan(span),
        normal(normal),
        aspect_ratio(phi::sq(span) / area),
        flap_ratio(flap_ratio)
  {
  }

  // controls how much the wing is deflected
  void set_control_input(float input) { control_input = glm::clamp(input, -1.0f, 1.0f); }

  // compute and apply aerodynamic forces
  void apply_forces(phi::RigidBody* rigid_body, phi::Seconds dt)
  {
    glm::vec3 local_velocity = rigid_body->get_point_velocity(center_of_pressure);
    float speed = glm::length(local_velocity);

    if (speed <= phi::EPSILON) return;

    // drag acts in the opposite direction of velocity
    glm::vec3 drag_direction = glm::normalize(-local_velocity);

    // lift is always perpendicular to drag
    glm::vec3 lift_direction = glm::normalize(glm::cross(glm::cross(drag_direction, normal), drag_direction));

    // angle between chord line and air flow
    float angle_of_attack = glm::degrees(std::asin(glm::dot(drag_direction, normal)));

    // sample aerodynamic coefficients
    auto [lift_coeff, drag_coeff] = airfoil->sample(angle_of_attack);

    if (flap_ratio > 0.0f) {
      // lift coefficient changes based on flap deflection ie control input
      float delta_lift_coeff = sqrt(flap_ratio) * airfoil->cl_max * control_input;
      lift_coeff += delta_lift_coeff;
    }

    // induced drag, increases with lift
    float induced_drag_coeff = phi::sq(lift_coeff) / (phi::PI * aspect_ratio * efficiency_factor);
    drag_coeff += induced_drag_coeff;

    // air density depends on altitude
    float air_density = isa::get_air_density(rigid_body->position.y);

    float dynamic_pressure = 0.5f * phi::sq(speed) * air_density * area;

    glm::vec3 lift = lift_direction * lift_coeff * dynamic_pressure;
    glm::vec3 drag = drag_direction * drag_coeff * dynamic_pressure;

    // aerodynamic forces are applied at the center of pressure
    rigid_body->add_force_at_point(lift + drag, center_of_pressure);
  }

  // TODO: consider dihedral as well
  static glm::vec3 calc_wing_normal(const glm::vec3& normal, float incidence)
  {
    auto axis = glm::normalize(glm::cross(phi::FORWARD, normal));
    auto rotation = glm::rotate(glm::mat4(1.0f), glm::radians(incidence), axis);
    return glm::vec3(rotation * glm::vec4(normal, 1.0f));
  }
};

// simple flightmodel
struct Airplane : public phi::RigidBody {
  glm::vec4 joystick{};  // roll, yaw, pitch, elevator trim
  float throttle = 0.25f;
  std::vector<Engine*> engines;
  std::vector<Wing> wings;
  bool is_landed = false;

#if LOG_FLIGHT
  float log_timer = 0.0f;
  float log_intervall = 0.1f;
  float flight_time = 0.0f;
  std::ofstream log_file;
#endif

  // wings are in the order { left_wing, right_wing, elevator, rudder }
  Airplane(float mass_, const glm::mat3& inertia_, std::vector<Wing> wings_, std::vector<Engine*> engines_,
           phi::Collider* collider_)
      : phi::RigidBody({.mass = mass_, .inertia = inertia_, .collider = collider_}), wings(wings_), engines(engines_)
  {
#if LOG_FLIGHT
    std::time_t now = std::time(nullptr);
    log_file.open("log/flight_log_" + std::to_string(now) + ".csv");
    log_file
        << "flight_time,altitude,speed,ias,aoa,roll_rate,yaw_rate,pitch_rate,roll,yaw,pitch,aileron,rudder,elevator,\n";
#endif
    assert(wings.size() == 4);
  }

  void update(phi::Seconds dt) override
  {
    float aileron = joystick.x, rudder = joystick.y, elevator = joystick.z, trim = joystick.w;

    if (wings.size() > 0) {
      wings[0].set_control_input(+aileron);
      wings[1].set_control_input(-aileron);
      wings[2].set_control_input(-elevator);
      wings[3].set_control_input(-rudder);
    }

#if LOG_FLIGHT
    flight_time += dt;
    if ((log_timer -= dt) <= 0.0f) {
      log_timer = log_intervall;
      auto av = glm::degrees(rigid_body.angular_velocity);
      auto euler = glm::degrees(glm::eulerAngles(glm::normalize(rigid_body.orientation)));

      /* clang-format off */
      log_file 
          << flight_time << "," 
          << get_altitude() << "," 
          << get_speed() << "," 
          << get_ias() << ","
          << get_aoa() << "," 
          << av.x << "," 
          << av.y << "," 
          << av.z << "," 
          << euler.x << "," 
          << euler.y << "," 
          << euler.z << ","
          << aileron << "," 
          << rudder << "," 
          << elevator << "," 
          << std::endl;
      /* clang-format on */
    }
#endif

    for (auto& wing : wings) {
      wing.apply_forces(this, dt);
    }

    for (auto engine : engines) {
      engine->throttle = throttle;
      engine->apply_forces(this, dt);
    }

    phi::RigidBody::update(dt);
  }

  // aircraft altitude
  float get_altitude() const { return position.y; }

  // pitch g force
  float get_g() const
  {
    auto velocity = get_body_velocity();

    // avoid division by zero
    float turn_radius = (std::abs(angular_velocity.z) < phi::EPSILON) ? std::numeric_limits<float>::max()
                                                                      : velocity.x / angular_velocity.z;

    // centrifugal force = mass * velocity^2 / radius
    // centrifugal acceleration = force / mass
    // simplified, this results in:
    float centrifugal_acceleration = phi::sq(velocity.x) / turn_radius;

    float g_force = centrifugal_acceleration / phi::EARTH_GRAVITY;
    g_force += (up().y * phi::EARTH_GRAVITY) / phi::EARTH_GRAVITY;  // add earth gravity
    return g_force;
  }

  // mach number
  float get_mach() const
  {
    float temperature = isa::get_air_temperature(get_altitude());
    float speed_of_sound = std::sqrt(1.402f * 286.f * temperature);
    return get_speed() / speed_of_sound;
  }

  // angle of attack
  float get_aoa() const
  {
    auto velocity = get_body_velocity();
    return glm::degrees(std::asin(glm::dot(glm::normalize(-velocity), phi::UP)));
  }

  // indicated air speed
  float get_ias() const
  {
    // See: https://aerotoolbox.com/airspeed-conversions/
    float air_density = isa::get_air_density(get_altitude());
    float dynamic_pressure = 0.5f * phi::sq(get_speed()) * air_density;  // bernoulli's equation
    return std::sqrt(2 * dynamic_pressure / isa::sea_level_air_density);
  }
};
