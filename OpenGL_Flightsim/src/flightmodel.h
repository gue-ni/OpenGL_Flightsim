#pragma once
#ifndef FLIGHTMODEL_H
#define FLIGHTMODEL_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>
#include <vector>

#include "phi/phi.h"
#include "collider.h"

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
inline float get_air_density(float altitude)
{
  assert(0.0f <= altitude && altitude <= 11000.0f);
  float temperature = get_air_temperature(altitude);
  float pressure = 101325.0f * std::pow(1 - 0.0065f * (altitude / 288.15f), 5.25f);
  return 0.00348f * (pressure / temperature);
}

const float sea_level_air_density = get_air_density(0.0f);
};  // namespace isa

// World Geodetic System (WGS 84)
namespace wgs84
{
// origin is degrees lat/lon, offset in meters
inline glm::vec2 coordinates(const glm::vec2& origin, const glm::vec2& offset)
{
  float latitude = origin.x, longitude = origin.y;
  float new_latitude = latitude + glm::degrees(offset.y / phi::EARTH_RADIUS);
  float new_longitude = longitude + glm::degrees(offset.x / phi::EARTH_RADIUS) / cos(glm::radians(latitude));
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
    alpha = glm::clamp(alpha, min_alpha, max_alpha - 1);
    float t = phi::inverse_lerp(min_alpha, max_alpha, alpha) * max_index;
    float integer = std::floor(t);
    float fractional = t - integer;
    int index = static_cast<int>(integer);
    auto value = (index < max_index) ? phi::lerp(data[index], data[index + 1], fractional) : data[max_index];
    return {value.y, value.z};
  }
};



// simple jet-like engine
struct Engine  {
  float throttle;
  const float thrust;
  glm::vec3 relative_position = glm::vec3(0);  // position relative to cg

  Engine(float thrust) : thrust(thrust), throttle(0.25f) {}

  void apply_forces(phi::RigidBody* rigid_body, phi::Seconds dt) 
  {
    rigid_body->add_force_at_point({throttle * thrust, 0.0f, 0.0f}, relative_position);
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

    if (speed <= 1.0f) return;

    // drag acts in the opposite direction of velocity
    glm::vec3 drag_direction = glm::normalize(-local_velocity);

    // lift is always perpendicular to drag
    glm::vec3 lift_direction = glm::normalize(glm::cross(glm::cross(drag_direction, normal), drag_direction));

    // angle between chord line and air flow
    float angle_of_attack = glm::degrees(std::asin(glm::dot(drag_direction, normal)));

    // sample aerodynamic coefficients
    auto [lift_coeff, drag_coeff] = airfoil->sample(angle_of_attack);

    if (flap_ratio > 0.0f) {
      // at high speed there is lower max deflection

#if 0
      float deflection_ratio = std::tanh(control_input) * (1.0f / (speed / 200));
#else
      float deflection_ratio = control_input;
#endif
      // lift coefficient changes based on flap deflection
      float delta_lift_coeff = sqrt(flap_ratio) * airfoil->cl_max * deflection_ratio;
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
  std::vector<Engine> engines;
  std::array<Wing, 4> wings;
  bool is_landed = false;

  // wings are in the order { left_wing, right_wing, elevator, rudder }
  Airplane(float mass_, const glm::mat3& inertia_, std::array<Wing, 4> wings_, std::vector<Engine> engines_,
           Collider* collider_)
      : phi::RigidBody({.mass = mass_, .inertia = inertia_, .collider = collider_}, "flightlog.csv"), wings(wings_), engines(engines_)
  {
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

    for (auto& wing : wings) {
      wing.apply_forces(this, dt);
    }

    for (auto& engine : engines) {
      engine.throttle = throttle;
      engine.apply_forces(this, dt);
    }

    phi::RigidBody::update(dt);
  }

  // aircraft altitude
  inline float get_altitude() const { return position.y; }

  // pitch g force
  inline float get_g() const
  {
    glm::vec3 velocity = get_body_velocity();

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
  inline float get_mach() const
  {
    float temperature = isa::get_air_temperature(get_altitude());
    float speed_of_sound = std::sqrt(1.402f * 286.f * temperature);
    return get_speed() / speed_of_sound;
  }

  // angle of attack
  inline float get_aoa() const
  {
    auto velocity = get_body_velocity();
    return glm::degrees(std::asin(glm::dot(glm::normalize(-velocity), phi::UP)));
  }

  // indicated air speed
  inline float get_ias() const
  {
    // See: https://aerotoolbox.com/airspeed-conversions/
    float air_density = isa::get_air_density(get_altitude());
    float dynamic_pressure = 0.5f * phi::sq(get_speed()) * air_density;  // bernoulli's equation
    return std::sqrt(2 * dynamic_pressure / isa::sea_level_air_density);
  }

  // positive yaw   -> nose goes right
  // positive roll  -> lift left wing
  // positive pitch -> nose goes up
  glm::vec3 get_attitude() const
  {
    // https://math.stackexchange.com/questions/3564608/calculate-yaw-pitch-roll-from-up-right-forward
    glm::vec3 forward = this->forward(), up = this->up();

    float yaw = std::atan2(forward.z, forward.x);
    float pitch = -std::asin(forward.y);

    float roll = 0.0f;
    roll = std::asin(up.x * std::sin(yaw) + up.z * -std::cos(yaw));
    //roll = ((0.0f <= roll) - (roll < 0.0f)) * 3.14f - roll;

    return {roll, yaw, pitch};
  }

  void inline set_speed_and_attitude(float speed, const glm::vec3& attitude)
  {
    glm::vec3 velocity = glm::vec3(speed, 0, 0);
    this->rotation = glm::quat(attitude);
    auto v = velocity * this->rotation;
    this->velocity = v;
  }
};

#endif  // ! FLIGHTMODEL_H
