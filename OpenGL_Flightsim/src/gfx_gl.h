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
namespace gl
{
struct VertexBuffer {
  GLuint id = 0;
  VertexBuffer();
  ~VertexBuffer();
  void bind() const;
  void unbind() const;
  void buffer(const void* data, size_t size);

  template <typename T>
  void buffer(const std::vector<T>& data)
  {
    buffer(&data[0], sizeof(data[0]) * data.size());
  }
};

struct Shader {
  GLuint id;
  Shader(const std::string& path);
  Shader(const std::string& vertShader, const std::string& fragShader);
  Shader(GLuint shader_id) : id(shader_id) {}
  ~Shader();
  void bind() const;
  void unbind() const;
  void uniform(const std::string& name, int value);
  void uniform(const std::string& name, float value);
  void uniform(const std::string& name, unsigned int value);
  void uniform(const std::string& name, const glm::vec3& value);
  void uniform(const std::string& name, const glm::vec4& value);
  void uniform(const std::string& name, const glm::mat4& value);
};




}  // namespace gl
}  // namespace gfx
