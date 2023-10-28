#pragma once

#include <GL/glew.h>

#include <array>
#include <fstream>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "util.h"
#include "image.h"

#if 1
#define GL_CALL(stmt)                      \
  do {                                     \
    stmt;                                  \
    CheckError(#stmt, __FILE__, __LINE__); \
  } while (0)
#else
#define GL_CALL(stmt) stmt
#endif

namespace gfx
{
namespace gl
{

void CheckError(const char* stmt, const char* fname, int line);

struct Object;
struct Texture;
using TexturePtr = std::shared_ptr<Texture>;

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

// abstract opengl object
struct Object {
  Object() = default;
  ~Object() = default;

  // copy
  Object(const Object& src) = delete;
  Object& operator=(const Object& rhs) = delete;

  // move
  Object(Object&& src) noexcept : m_id(src.m_id) { src.m_id = 0; }
  Object& operator=(Object&& rhs) noexcept
  {
    if (this != &rhs) std::swap(m_id, rhs.m_id);
    return *this;
  }

  inline operator GLuint() const noexcept { return m_id; };
  inline GLuint id() const { return m_id; }

 protected:
  GLuint m_id = 0;
};

struct Buffer : public Object {
 public:
  Buffer(GLenum target_) : target(target_) { GL_CALL(glGenBuffers(1, &m_id)); }
  ~Buffer() { GL_CALL(glDeleteBuffers(1, &m_id)); }
  void bind() const { GL_CALL(glBindBuffer(target, m_id)); }
  void unbind() const { GL_CALL(glBindBuffer(target, 0)); }

  void buffer_data(const void* data, size_t size, GLenum usage = GL_STATIC_DRAW)
  {
    bind();
    GL_CALL(glNamedBufferData(m_id, size, data, usage));
  }

  void buffer_sub_data(size_t offset, size_t size, const void* data)
  {
    GL_CALL(glNamedBufferSubData(m_id, offset, size, data));
  }

  void bind_buffer_range(GLuint index, size_t offset, size_t size)
  {
    GL_CALL(glBindBufferRange(target, index, m_id, offset, size));
  }

  void buffer(const void* data, size_t size, GLenum usage = GL_STATIC_DRAW)
  {
    bind();
    GL_CALL(glBufferData(target, size, data, usage));
  }

  template <typename T>
  void buffer(const std::vector<T>& data, GLenum usage = GL_STATIC_DRAW)
  {
    buffer_data(&data[0], sizeof(data[0]) * data.size(), usage);
  }

  template <typename T>
  void buffer_2(const std::vector<T>& data, GLenum usage = GL_STATIC_DRAW)
  {
    GL_CALL(glBufferData(target, sizeof(data[0]) * data.size(), &data[0], usage));
  }

 protected:
  const GLenum target;
};

struct VertexBuffer : public Buffer {
  VertexBuffer() : Buffer(GL_ARRAY_BUFFER) {}
};

struct ElementBuffer : public Buffer {
  ElementBuffer() : Buffer(GL_ELEMENT_ARRAY_BUFFER) {}
};

struct UniformBuffer : public Buffer {
  UniformBuffer() : Buffer(GL_UNIFORM_BUFFER) {}
};

struct FrameBuffer : public Object {
  FrameBuffer() { glGenFramebuffers(1, &m_id); }
  ~FrameBuffer() { glDeleteFramebuffers(1, &m_id); }
  void bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_id); }
  void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
  bool complete() const { return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE; }
};

struct VertexArrayObject : public Object {
  VertexArrayObject() { glGenVertexArrays(1, &m_id); }
  ~VertexArrayObject() { glDeleteVertexArrays(1, &m_id); }
  void bind() const { glBindVertexArray(m_id); }
  void unbind() const { glBindVertexArray(0); }
};

struct RenderBuffer : public Object {
  RenderBuffer() { glGenRenderbuffers(1, &m_id); }
  ~RenderBuffer() { glDeleteRenderbuffers(1, &m_id); }
  void bind() const { glBindRenderbuffer(GL_RENDERBUFFER, m_id); }
  void unbind() const { glBindRenderbuffer(GL_RENDERBUFFER, 0); }
};

struct Shader : public Object {
  Shader(const std::string& path);
  Shader(const std::string& vert_shader, const std::string& frag_shader);
  ~Shader();
  void bind() const;
  void unbind() const;
  void set_uniform(const std::string& name, GLint value) const;
  void set_uniform(const std::string& name, GLuint value) const;
  void set_uniform(const std::string& name, GLfloat value) const;
  void set_uniform(const std::string& name, const glm::vec3& value) const;
  void set_uniform(const std::string& name, const glm::vec4& value) const;
  void set_uniform(const std::string& name, const glm::mat3& value) const;
  void set_uniform(const std::string& name, const glm::mat4& value) const;
  void set_uniform_buffer(const std::string& name, GLuint binding = 0U);
};

using ShaderPtr = std::shared_ptr<Shader>;

struct Texture : public Object {
  const GLenum target;

  struct Params {
    bool flip_vertically = false;
    GLint wrap = GL_REPEAT;
    GLint min_filter = GL_LINEAR_MIPMAP_LINEAR;
    GLint mag_filter = GL_NEAREST;
  };

  Texture(GLenum target_ = GL_TEXTURE_2D) : target(target_) { glGenTextures(1, &m_id); }
  ~Texture() { glDeleteTextures(1, &m_id); }
  Texture(const std::string& path);
  Texture(const std::string& path, const Params& params);
  Texture(const Image& image, const Params& params);
  void bind() const;
  void bind(GLuint texture_unit) const;
  void unbind() const;
  void set_parameter(GLenum pname, GLint param);
  void set_parameter(GLenum pname, GLfloat param);
  void set_parameter(GLenum pname, const GLfloat* param);
  static TexturePtr load(const std::string& path, const Params& params);
  static TexturePtr load(const std::string& path);
};

struct CubemapTexture : public Texture {
  CubemapTexture(const std::array<std::string, 6>& paths, bool flip_vertically = false);
};

class TextureArray : public Object
{
 public:
  struct Params {
    Image::Format format{Image::Format::RGB};
    glm::ivec2 texture_size{1024};
    int array_size{1};
    GLint wrap_s{GL_CLAMP_TO_BORDER}, wrap_t{GL_CLAMP_TO_BORDER};
    GLint min_filter{GL_LINEAR}, mag_filter{GL_LINEAR};
  };

  TextureArray(const Params& params);
  void bind() const;
  void bind(GLuint texture_unit) const;
  void unbind() const;
  void add_image(const Image& image);
  void set_parameter(GLenum pname, GLint param);
  void set_parameter(GLenum pname, GLfloat param);
  void set_parameter(GLenum pname, const GLfloat* param);

  const GLenum target = GL_TEXTURE_2D_ARRAY;

 private:
  const Image::Format m_format;
  const glm::ivec2 m_texture_size;
  const int m_array_size;
  int m_image_index = 0;
};

}  // namespace gl
}  // namespace gfx
