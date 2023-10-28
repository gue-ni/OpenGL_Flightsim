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

#include "controller.h"
#include "gl.h"
#include "object3d.h"
#include "util.h"
#include "particles.h"
#include "material.h"
#include "geometry.h"
#include "line2d.h"
#include "billboard.h"
#include "mesh.h"
#include "image.h"

namespace gfx
{

typedef glm::vec3 RGB;
typedef glm::vec4 RGBA;

class Mesh;
class Light;
class Camera;
class Skybox;
class Geometry;
class BaseGeometry;
class Material;
class ShaderCache;

using MeshPtr = std::shared_ptr<Mesh>;

struct RenderContext {
  bool shadow_pass;
  glm::vec3 fog_color;
  Camera* camera;
  Light* light;
  ShaderCache* shaders = nullptr;
  gl::TexturePtr env_map = nullptr;
  gl::TexturePtr depth_map = nullptr;
  glm::mat4 light_space_matrix;
};

class Camera : public Object3D
{
 public:
  Camera(float fov, float aspect, float near, float far)
      : m_projection(glm::perspective(fov, aspect, near, far)),
        m_up(0.0f, 1.0f, 0.0f),
        m_front(0.0f, 0.0f, 1.0f),
        Object3D(CAMERA)
  {
  }

  glm::mat4 get_view_matrix() const;
  glm::mat4 get_projection_matrix() const;
  void look_at(const glm::vec3& target);

 private:
  glm::mat4 m_projection;
  glm::vec3 m_up, m_front;
};

class Light : public Object3D
{
 public:
  glm::vec3 color;
  glm::vec3 direction;
  bool cast_shadow;
  Light(const glm::vec3& color_) : color(color_), cast_shadow(false), Object3D(LIGHT) {}
  glm::mat4 light_space_matrix();
};

class ShaderCache
{
 public:
  void add_shader(const std::string& path);
  gl::ShaderPtr get_shader(const std::string& path);

 private:
  std::unordered_map<std::string, gl::ShaderPtr> m_cache;
};

struct RenderTargetBase {
  RenderTargetBase(int w, int h) : width(w), height(h) {}
  gl::FrameBuffer framebuffer;
  GLuint width, height;
};

struct RenderTarget : public RenderTargetBase {
  RenderTarget(int w, int h);
  gl::RenderBuffer depthbuffer;
  gl::TexturePtr texture = nullptr;
};

struct ShadowMap : public RenderTargetBase {
  ShadowMap(int w, int h);
  gl::TexturePtr texture = nullptr;
};

class Renderer
{
 private:
  GLsizei m_width, m_height;
  glm::vec3 m_fog_color;
  ShaderCache m_shaders;
  MeshPtr m_skybox;
  MeshPtr m_screenquad;
  std::shared_ptr<RenderTarget> m_rendertarget;
  std::shared_ptr<ShadowMap> m_shadowmap;
  Light* m_light;

 public:
  Renderer(GLsizei width, GLsizei height);

  void render_skybox(RenderContext& context);
  void render_shadows(RenderContext& context);
  void render_screenquad(RenderContext& context);

  // render scene
  void render(Camera* camera, Object3D* scene);
  void render(Camera* camera, Object3D* scene, RenderTarget* target);
};

};  // namespace gfx
