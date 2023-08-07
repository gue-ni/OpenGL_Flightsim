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

struct FrameBuffer {
  GLuint id = 0;
  FrameBuffer() : id(0) { glGenFramebuffers(1, &id); }
  FrameBuffer(GLuint id_) : id(id_) {}
  ~FrameBuffer() { glDeleteFramebuffers(1, &id); };
  void bind() const;
  void unbind() const;
};

struct VertexArrayObject {
  GLuint id = 0;
  VertexArrayObject();
  ~VertexArrayObject();
  void bind() const;
  void unbind() const;
};

struct ElementBufferObject {
  GLuint id = 0;
  ElementBufferObject();
  ~ElementBufferObject();
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

struct Image {
  unsigned char* data = nullptr;
  int width, height, channels;
  Image(const std::string& path, bool flip_vertically = false);
  ~Image();
  glm::vec3 sample(const glm::vec2 uv) const;
};

struct TextureParams {
  bool flip_vertically = false;
  GLint texture_wrap = GL_REPEAT;
  GLint texture_min_filter = GL_LINEAR_MIPMAP_LINEAR;
  GLint texture_mag_filter = GL_NEAREST;
};

struct Texture {
  GLuint id = 0;
  Texture() : id(0) { glGenTextures(1, &id); }
  Texture(GLuint texture_id) : id(texture_id) {}
  Texture(const std::string& path);
  Texture(const std::string& path, const TextureParams& params);
  Texture(const Image& image, const TextureParams& params);
  ~Texture();

  virtual void bind(GLuint active_texture = 0U) const;
  virtual void unbind() const;
  GLint get_format(int channels);
  void set_parameteri(GLenum target, GLenum pname, GLint param);
};

struct CubemapTexture : public Texture {
  CubemapTexture(const std::array<std::string, 6>& paths, bool flip_vertically = false);
  void bind(GLuint texture) const override;
  void unbind() const override;
};

}  // namespace gl
}  // namespace gfx
