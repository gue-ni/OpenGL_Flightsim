#pragma once

#include <GL/glew.h>

#if 1
#include <assimp/postprocess.h>  // Post processing flags
#include <assimp/scene.h>        // Output data structure
#include <assimp/Importer.hpp>   // C++ importer interface
#endif

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
std::string load_text_file(const std::string& path);

// [0, 255] -> [0, 1]
template <typename T>
constexpr glm::vec3 rgb(T r, T g, T b)
{
  return glm::vec3(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b)) / 255.0f;
}

constexpr glm::vec3 rgb(uint32_t hex)
{
  assert(hex <= 0xffffffU);
  uint32_t r = (hex & 0xff0000U) >> 16;
  uint32_t g = (hex & 0x00ff00U) >> 8;
  uint32_t b = (hex & 0x0000ffU) >> 0;
  return rgb(r, g, b);
}

class Image
{
 public:
  Image(const std::string& path, bool flip_vertically = false);
  ~Image();
  glm::vec3 sample(const glm::vec2 uv, GLint filter = GL_NEAREST) const;
  unsigned char* data() const;
  int width() const;
  int height() const;
  int channels() const;
  GLint format() const;

 private:
  unsigned char* m_data = nullptr;
  int m_width, m_height, m_channels;
};

}  // namespace gfx
