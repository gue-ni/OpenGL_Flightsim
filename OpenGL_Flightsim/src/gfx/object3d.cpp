#include "object3d.h"

#include <glm/gtx/matrix_decompose.hpp>

#include "gfx.h"

namespace gfx
{

int Object3D::counter = 0;

void Object3D::draw(RenderContext& context)
{
  if (visible) {
    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    draw_self(context);

    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    draw_children(context);
  }
}

void Object3D::draw_self(RenderContext& context) {}

void Object3D::draw_children(RenderContext& context)
{
  for (auto child : children) child->draw(context);
}

glm::vec3 Object3D::get_position() const { return m_position; }

glm::vec3 Object3D::get_rotation() const { return glm::eulerAngles(m_rotation); }

glm::quat Object3D::get_rotation_quat() const { return m_rotation; }

glm::vec3 Object3D::get_scale() const { return m_scale; }

void Object3D::set_scale(const glm::vec3& scale)
{
  m_scale = scale;
  m_dirty = true;
}

void Object3D::set_position(const glm::vec3& pos)
{
  m_position = pos;
  m_dirty = true;
}

void Object3D::set_transform(const Object3D& transform)
{
  set_scale(transform.get_scale());
  set_position(transform.get_position());
  set_rotation(transform.get_rotation());
}

void Object3D::set_transform(const glm::vec3& position, const glm::quat& rotation)
{
  set_position(position);
  set_rotation(rotation);
}

void Object3D::set_rotation(const glm::vec3& rot)
{
  m_rotation = glm::quat(rot);
  m_dirty = true;
}

void Object3D::rotate_by(const glm::vec3& rot) { set_rotation(get_rotation() + rot); }

void Object3D::set_rotation(const glm::quat& quat)
{
  m_rotation = quat;
  m_dirty = true;
}

glm::mat4 Object3D::get_local_transform() const
{
  auto T = glm::translate(glm::mat4(1.0f), m_position);
  auto R = glm::toMat4(m_rotation);
  auto S = glm::scale(glm::mat4(1.0f), m_scale);
  return T * R * S;
}

void Object3D::traverse(const std::function<bool(Object3D*)>& func)
{
  if (func(this)) {
    for (const auto& child : children) {
      child->traverse(func);
    }
  }
}

void Object3D::set_transform(const glm::mat4& matrix)
{
  glm::vec3 scale;
  glm::quat rotation;
  glm::vec3 translation;
  glm::vec3 skew;
  glm::vec4 perspective;

  glm::decompose(matrix, scale, rotation, translation, skew, perspective);

  set_scale(scale);
  set_rotation(glm::normalize(rotation));
  set_position(translation);
}

bool Object3D::is_dirty() const { return m_dirty; }

glm::mat4 Object3D::get_transform() const
{
  // this function must only be called after updating the transform
  if (m_dirty) {
    // assert(!m_dirty);
  }
  return m_transform;
}

glm::mat3 Object3D::get_normal_transform() const { return glm::inverseTranspose(glm::mat3(get_transform())); }

void Object3D::update_transform(bool force_update)
{
  bool updated = false;

  if (m_dirty || (parent && parent->is_dirty()) || force_update) {
    if (parent) {
      m_transform = get_parent_transform() * get_local_transform();
    } else {
      m_transform = get_local_transform();
    }

    updated = true;
    m_dirty = false;
  }

  for (auto child : children) {
    child->update_transform(updated);
  }
}

glm::mat4 Object3D::get_parent_transform() const
{
  assert(parent != nullptr);

  if (transform_flags == (OBJ3D_TRANSFORM | OBJ3D_ROTATE | OBJ3D_SCALE)) {
    return parent->get_transform();
  }

  glm::mat4 parent_transform(1.0f);

  if (transform_flags & OBJ3D_TRANSFORM) {
    parent_transform *= glm::translate(glm::mat4(1.0f), parent->get_world_position());
  }
  if (transform_flags & OBJ3D_ROTATE) {
    parent_transform *= glm::toMat4(parent->get_world_rotation_quat());
  }
  if (transform_flags & OBJ3D_SCALE) {
    parent_transform *= glm::scale(glm::mat4(1.0f), parent->get_scale());
  }

  return parent_transform;
}

Object3D& Object3D::add(Object3D* child)
{
  child->parent = this;
  children.push_back(child);
  return (*this);
}

glm::quat Object3D::get_world_rotation_quat() const
{
  if (parent == nullptr)
    return get_rotation_quat();
  else
    return parent->get_world_rotation_quat() * m_rotation;
}

glm::vec3 Object3D::get_world_position() const { return glm::vec3(get_transform() * glm::vec4(glm::vec3(0.0f), 1.0f)); }

}  // namespace gfx