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

struct RenderContext;

#define OBJ3D_TRANSFORM 1U << 0U
#define OBJ3D_ROTATE    1U << 1U
#define OBJ3D_SCALE     1U << 2U

class Object3D
{
 public:
  enum Type { OBJECT3D, LIGHT, CAMERA };

  Object3D()
      : id(counter++), parent(nullptr), transform(1.0), m_position(0.0f), m_rotation(glm::vec3(0.0f)), m_scale(1.0f)
  {
  }

  const int id;
  static int counter;
  // which transform to inherit from parent
  unsigned transform_flags = OBJ3D_TRANSFORM | OBJ3D_ROTATE | OBJ3D_SCALE;

  Object3D* parent;
  std::vector<Object3D*> children;
  glm::mat4 transform;
  bool receive_shadow = true;
  bool visible = true;
  bool wireframe = false;

  Object3D& add(Object3D* child);

  void draw(RenderContext& context);
  void draw_children(RenderContext& context);
  virtual void draw_self(RenderContext& context);

  void set_scale(const glm::vec3& scale);
  void set_rotation(const glm::vec3& rotation);
  void rotate_by(const glm::vec3& rotation);
  void set_rotation_quat(const glm::quat& rotation);
  void set_position(const glm::vec3& position);
  void set_transform(const Object3D& transform);
  void set_transform(const glm::vec3& position, const glm::quat& rotation);

  glm::vec3 get_scale() const;
  glm::vec3 get_rotation() const;
  glm::quat get_rotation_quat() const;
  glm::vec3 get_position() const;
  glm::quat get_world_rotation_quat() const;
  glm::vec3 get_world_position() const;

  virtual Object3D::Type get_type() const;

  void override_transform(const glm::mat4& matrix);
  void update_world_matrix(bool dirty_parent);
  glm::mat4 get_local_transform() const;
  glm::mat4 get_parent_transform() const;
  glm::mat4 get_transform() const;
  void traverse(const std::function<bool(Object3D*)>& func);

 protected:
  bool m_dirty_dof = false;
  bool m_dirty_transform = false;

  glm::vec3 m_position;
  glm::vec3 m_scale;
  glm::quat m_rotation;
};

}  // namespace gfx
