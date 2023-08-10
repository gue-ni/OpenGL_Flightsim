#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../lib/stb_image.h"

namespace gfx
{
std::string load_text_file(const std::string& path)
{
  std::fstream file(path);
  if (!file.is_open()) return std::string();

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

Image::Image(const std::string& path, bool flip_vertically)
{
  stbi_set_flip_vertically_on_load(flip_vertically);
  m_data = stbi_load(path.c_str(), &m_width, &m_height, &m_channels, 0);
  assert(m_data != nullptr);
}

Image::~Image() { stbi_image_free(m_data); }

glm::vec3 Image::sample(const glm::vec2 uv) const
{
  // nearest pixel
  glm::ivec2 pixel_coord = uv * glm::vec2(m_width, m_height);
  int index = (m_height * pixel_coord.y + pixel_coord.x) * m_channels;
  return gfx::rgb(m_data[index + 0], m_data[index + 1], m_data[index + 2]);
}

unsigned char* Image::data() const { return m_data; }

int Image::width() const { return m_width; }

int Image::height() const { return m_height; }

int Image::channels() const { return m_channels; }

GLint Image::format() const {
  assert(1 <= m_channels && m_channels <= 4);
  static GLint formats[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
  return formats[m_channels - 1];
} 


}  // namespace gfx
