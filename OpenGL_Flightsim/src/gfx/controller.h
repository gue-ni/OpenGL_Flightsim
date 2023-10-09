#pragma once

#include <GL/glew.h>

#include <array>
#include <fstream>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace gfx
{

class Object3D;

class FirstPersonController
{
 public:
  enum Direction {
    FORWARD,
    RIGHT,
    BACKWARD,
    LEFT,
  };

  FirstPersonController(float speed)
      : m_speed(speed),
        m_yaw(-90.0f),
        m_pitch(0.0f),
        m_front(0.0f, 0.0f, -1.0f),
        m_up(0.0f, 1.0f, 0.0f),
        m_velocity(0.0f),
        m_direction(0.0f)
  {
    move_mouse(0.0f, 0.0f);
  }

  void update(Object3D& object, float dt);
  void move_mouse(float x, float y);
  void move(const Direction& direction);
  inline glm::vec3 get_front() const { return m_front; }

 private:
  float m_speed, m_yaw, m_pitch;
  glm::vec3 m_front, m_up, m_velocity, m_direction;
};

class OrbitController
{
 public:
  OrbitController(float radius) : radius(radius), m_pitch(25.0f), m_yaw(120.0f) {}

  float radius;
  void update(Object3D& object, const glm::vec3& center, float dt);
  void move_mouse(float x, float y);

 private:
  float m_yaw, m_pitch;
};

}  // namespace gfx
