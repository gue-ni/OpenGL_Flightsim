#pragma once

#include "gl.h"

namespace gfx
{

class Material
{
 public:
  float shininess = 0.0f;
  float opacity = 1.0f;

  Material(const std::string& shader_name, const gl::TexturePtr& texture)
      : m_shader_name(shader_name), m_texture(texture)
  {
  }

  Material(const std::string& shader_name, const std::string& texture)
      : m_shader_name(shader_name), m_texture(gl::Texture::load(texture, {.flip_vertically = true}))
  {
  }

  gl::TexturePtr get_texture() const { return m_texture; }
  std::string& get_shader_name() { return m_shader_name; }

 private:
  gl::TexturePtr m_texture = nullptr;
  std::string m_shader_name;
};

using MaterialPtr = std::shared_ptr<Material>;

}  // namespace gfx
