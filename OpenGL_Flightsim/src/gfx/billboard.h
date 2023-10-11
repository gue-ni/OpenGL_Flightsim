#pragma once

#include "gfx.h"

namespace gfx
{

class Billboard : public Object3D
{
 public:
  Billboard(gl::TexturePtr sprite, glm::vec3 color = glm::vec3(1.0f));
  void draw_self(RenderContext& context) override;
  Object3D& add(Object3D* child) = delete;

 private:
  glm::vec3 color;
  gl::Shader shader;
  gl::TexturePtr texture;
  gl::VertexArrayObject vao;
  gl::VertexBuffer vbo;
  gl::ElementBuffer ebo;
};

}  // namespace gfx
