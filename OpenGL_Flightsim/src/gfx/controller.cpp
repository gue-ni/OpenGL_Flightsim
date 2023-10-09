
#include "controller.h"

#include "gfx.h"

namespace gfx
{
void FirstPersonController::update(Object3D& target, float dt)
{
  const auto pos = target.get_position();
  target.set_position(pos + m_velocity * dt);
  target.set_transform(glm::inverse(glm::lookAt(pos, pos + m_front, m_up)));
  m_velocity = glm::vec3(0.0f);
}

void FirstPersonController::move_mouse(float x, float y)
{
  glm::vec2 offset(x, y);

  const float sensitivity = 0.1f;
  offset *= sensitivity;

  m_yaw += offset.x;
  m_pitch -= offset.y;

  if (m_pitch > +89.9f) m_pitch = +89.0f;
  if (m_pitch < -89.9f) m_pitch = -89.0f;

  glm::vec3 front(0);
  front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
  front.y = sin(glm::radians(m_pitch));
  front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
  m_front = glm::normalize(front);
}

void FirstPersonController::move(const Direction& direction)
{
  switch (direction) {
    case FORWARD: {
      m_velocity += (m_speed * m_front);
      break;
    }
    case LEFT: {
      m_velocity -= (m_speed * glm::normalize(glm::cross(m_front, m_up)));
      break;
    }
    case BACKWARD: {
      m_velocity -= (m_speed * m_front);
      break;
    }
    case RIGHT: {
      m_velocity += (m_speed * glm::normalize(glm::cross(m_front, m_up)));
      break;
    }
    default:
      break;
  }
}

void OrbitController::update(Object3D& target, const glm::vec3& center, float dt)
{
  glm::vec3 front;
  front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
  front.y = sin(glm::radians(m_pitch));
  front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
  auto offset = glm::normalize(front) * radius;

  const auto pos = center + offset;
  target.set_position(pos);
  target.set_transform(glm::inverse(glm::lookAt(pos, pos - front, glm::vec3(0, 1, 0))));
  target.update_transform();
}

void OrbitController::move_mouse(float x, float y)
{
  glm::vec2 offset(x, y);

  const float sensitivity = 0.1f;
  offset *= sensitivity;

  m_yaw += offset.x;
  m_pitch += offset.y;

  m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
}

}  // namespace gfx