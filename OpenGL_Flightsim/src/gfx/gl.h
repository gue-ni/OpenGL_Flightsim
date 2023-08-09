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

#include "util.h"

namespace gfx
{
namespace gl
{

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

struct VertexBuffer : public Object {
  VertexBuffer() { glGenBuffers(1, &m_id); }
  ~VertexBuffer() { glDeleteBuffers(1, &m_id); }
  void bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_id); }
  void unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

  void buffer(const void* data, size_t size)
  {
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  }

  template <typename T>
  void buffer(const std::vector<T>& data)
  {
    buffer(&data[0], sizeof(data[0]) * data.size());
  }
};

struct FrameBuffer : public Object {
  FrameBuffer() { glGenFramebuffers(1, &m_id); }
  ~FrameBuffer() { glDeleteFramebuffers(1, &m_id); }
  void bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_id); }
  void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
};

struct VertexArrayObject : public Object {
  VertexArrayObject() { glGenVertexArrays(1, &m_id); }
  ~VertexArrayObject() { glDeleteVertexArrays(1, &m_id); }
  void bind() const { glBindVertexArray(m_id); }
  void unbind() const { glBindVertexArray(0); }
};

struct ElementBufferObject : public Object {
  ElementBufferObject() { glGenBuffers(1, &m_id); }
  ~ElementBufferObject() { glDeleteBuffers(1, &m_id); }
  void bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id); }
  void unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

  void buffer(const void* data, size_t size)
  {
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  }

  template <typename T>
  void buffer(const std::vector<T>& data)
  {
    buffer(&data[0], sizeof(data[0]) * data.size());
  }
};

class Shader : public Object {
 public:
  Shader(const std::string& path);
  Shader(const std::string& vert_shader, const std::string& frag_shader);
  ~Shader();
  void bind() const;
  void unbind() const;
  void set_uniform(const std::string& name, int value) const;
  void set_uniform(const std::string& name, float value) const;
  void set_uniform(const std::string& name, unsigned int value) const;
  void set_uniform(const std::string& name, const glm::vec3& value) const;
  void set_uniform(const std::string& name, const glm::vec4& value) const;
  void set_uniform(const std::string& name, const glm::mat4& value) const;
};

using ShaderPtr = std::shared_ptr<Shader>;

class Texture : public Object {
public:
  struct Params {
    bool flip_vertically = false;
    GLint texture_wrap = GL_REPEAT;
    GLint texture_min_filter = GL_LINEAR_MIPMAP_LINEAR;
    GLint texture_mag_filter = GL_NEAREST;
  };

  Texture() { glGenTextures(1, &m_id); }
  ~Texture() { glDeleteTextures(1, &m_id); }
  Texture(GLuint texture_id) { m_id = texture_id; }
  Texture(const std::string& path);
  Texture(const std::string& path, const Params& params);
  Texture(const Image& image, const Params& params);

  virtual void bind(GLuint active_texture = 0U) const;
  virtual void unbind() const;
  GLint get_format(int channels);
  void set_parameteri(GLenum target, GLenum pname, GLint param);
};

using TexturePtr = std::shared_ptr<Texture>;

class CubemapTexture : public Texture {
public:
  CubemapTexture(const std::array<std::string, 6>& paths, bool flip_vertically = false);
  void bind(GLuint texture) const override;
  void unbind() const override;
};

}  // namespace gl
}  // namespace gfx
