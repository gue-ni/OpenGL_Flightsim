#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

#include "util.h"

namespace gfx
{
class Image
{
 public:
  enum Format : GLint { RED = GL_RED, RG = GL_RG, RGB = GL_RGB, RGBA = GL_RGBA };

  Image(const std::string& path, bool flip_vertically = false);
  ~Image();
  glm::vec3 sample(const glm::vec2 uv, GLint filter = GL_NEAREST) const;
  unsigned char* data() const;
  int width() const;
  int height() const;
  int channels() const;
  Format format() const;

 private:
  unsigned char* m_data = nullptr;
  int m_width, m_height, m_channels;
};

}  // namespace gfx