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

namespace gfx
{
namespace gl
{

struct Object;
class Texture;
using TexturePtr = std::shared_ptr<Texture>;

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

// abstract opengl object
struct Object {
 protected:
  GLuint m_id = 0;
  Object() = default;
  ~Object() = default;

#if 0
  // move semantics
  Object(Object&& src) noexcept : m_id(src.m_id) {
        src.m_id = 0;
    }

    Object& operator=(Object&& rhs) noexcept {
        if (this != &rhs) {
            std::swap(m_id, rhs.m_id);
        }
        return *this;
    }
#endif

 public:
  inline operator GLuint() const noexcept { return m_id; };
  inline GLuint id() const { return m_id; }

 private:
#if 0
     // delete copy constructor/assignment
  Object(const Object& src) = delete;
  Object& operator=(const Object& rhs) = delete;
#endif
};

struct Buffer : public Object {
 public:
  Buffer(GLenum target_) : target(target_) { glGenBuffers(1, &m_id); }
  ~Buffer() { glDeleteBuffers(1, &m_id); }
  void bind() const { glBindBuffer(target, m_id); }
  void unbind() const { glBindBuffer(target, 0); }

  void buffer_data(const void* data, size_t size, GLenum usage = GL_STATIC_DRAW)
  {
    bind();
    glNamedBufferData(m_id, size, data, usage);
  }

  void buffer_sub_data(size_t offset, size_t size, const void* data) { glNamedBufferSubData(m_id, offset, size, data); }

  void bind_buffer_range(GLuint index, size_t offset, size_t size)
  {
    glBindBufferRange(target, index, m_id, offset, size);
  }

  void buffer(const void* data, size_t size, GLenum usage = GL_STATIC_DRAW)
  {
    bind();
    glBufferData(target, size, data, usage);
  }

  template <typename T>
  void buffer(const std::vector<T>& data, GLenum usage = GL_STATIC_DRAW)
  {
    buffer_data(&data[0], sizeof(data[0]) * data.size(), usage);
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

class Shader : public Object
{
 public:
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

class Texture : public Object
{
 public:
  const GLenum target;

  struct Params {
    bool flip_vertically = false;
    GLint texture_wrap = GL_REPEAT;
    GLint texture_min_filter = GL_LINEAR_MIPMAP_LINEAR;
    GLint texture_mag_filter = GL_NEAREST;
  };

  Texture(GLenum target_ = GL_TEXTURE_2D) : target(target_) { glGenTextures(1, &m_id); }
  ~Texture() { glDeleteTextures(1, &m_id); }
  Texture(const std::string& path);
  Texture(const std::string& path, const Params& params);
  Texture(const Image& image, const Params& params);
  void bind() const;
  void bind(GLuint active_texture) const;
  void unbind() const;
  GLint get_format(int channels);
  void set_parameter(GLenum pname, GLint param);
  void set_parameter(GLenum pname, GLfloat param);
  static TexturePtr load(const std::string& path, const Params& params);
};

class CubemapTexture : public Texture
{
 public:
  CubemapTexture(const std::array<std::string, 6>& paths, bool flip_vertically = false);
};

}  // namespace gl
}  // namespace gfx
