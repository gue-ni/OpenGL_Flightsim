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

namespace gfx
{

constexpr float PI = 3.14159265359f;

typedef glm::vec3 RGB;
typedef glm::vec4 RGBA;

class Mesh;
class Light;
class Camera;
class Skybox;
class Geometry;
class Material;
class ShaderCache;

using GeometryPtr = std::shared_ptr<Geometry>;
using MaterialPtr = std::shared_ptr<Material>;
using MeshPtr = std::shared_ptr<Mesh>;

std::vector<float> load_obj(const std::string path);

GeometryPtr make_cube_geometry(float size);
GeometryPtr make_plane_geometry(int x_elements, int y_elements, float size);
GeometryPtr make_quad_geometry();

struct ShadowMap {
  ShadowMap(unsigned int shadow_width, unsigned int shadow_height);
  GLuint fbo;
  gl::Texture depth_map;
  gl::Shader shader;
  GLuint width, height;
};

struct RenderContext {
  Camera* camera;
  Light* shadow_caster;
  ShadowMap* shadow_map;
  ShaderCache* shader_cache = nullptr;

  gl::TexturePtr environment_map = nullptr;

  std::vector<Light*> lights;
  bool is_shadow_pass;
  glm::vec3 background_color;
};

class Camera : public Object3D
{
 public:
  Camera(float fov, float aspect, float near, float far)
      : m_projection(glm::perspective(fov, aspect, near, far)), m_up(0.0f, 1.0f, 0.0f), m_front(0.0f, 0.0f, 1.0f)
  {
  }

  Object3D::Type get_type() const override;
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
  enum LightType {
    POINT = 0,
    DIRECTIONAL = 1,
  };

  Light(glm::vec3 color_) : rgb(color_), type(POINT), cast_shadow(false), Object3D() {}

  Light(LightType type_, glm::vec3 color_) : rgb(color_), type(type_), cast_shadow(false), Object3D() {}

  Object3D::Type get_type() const override;

  glm::mat4 light_space_matrix();

  LightType type;
  bool cast_shadow;
  glm::vec3 rgb;
};

class BaseGeometry
{
 public:
  enum DrawType { DRAW_ARRAYS, DRAW_ELEMENTS, /* TODO: instanced */ };

  enum VertexLayout {
    POS,         // pos
    POS_UV,      // pos, uv
    POS_NORM,    // pos, normal
    POS_NORM_UV  // pos, normal, uv
  };

  GLsizei count;
  gl::VertexArrayObject vao;
  const DrawType draw_type = DRAW_ARRAYS;
};

class Geometry : public BaseGeometry
{
 public:
  Geometry(const std::vector<float>& vertices, const VertexLayout& layout);

 private:
  gl::VertexBuffer vbo;
  static int get_stride(const VertexLayout& layout);
};

class Material
{
 public:
  // glm::vec3 emissive, ambient, diffuse, specular;
  // float alpha, shininess;

  Material(const std::string& shader_name, const gl::TexturePtr& texture)
      : m_shader_name(shader_name), m_texture(texture)
  {
  }

  gl::TexturePtr get_texture() const { return m_texture; }
  std::string& get_shader_name() { return m_shader_name; }

 private:
  gl::TexturePtr m_texture = nullptr;
  std::string m_shader_name;
};

class Mesh : public Object3D
{
 public:
  Mesh(const GeometryPtr& geometry, const MaterialPtr& material);

  MaterialPtr get_material() { return m_material; }
  GeometryPtr get_geometry() { return m_geometry; }

 protected:
  MaterialPtr m_material;
  GeometryPtr m_geometry;
  void draw_self(RenderContext& context) override;
};

class ShaderCache
{
 public:
  void add_shader(const std::string& path);
  gl::ShaderPtr get_shader(const std::string& path);

 private:
  std::unordered_map<std::string, gl::ShaderPtr> m_cache;
};

class Billboard : public Object3D
{
 public:
  Billboard(gl::TexturePtr sprite, glm::vec3 color = glm::vec3(1.0f));
  void draw_self(RenderContext& context) override;
  Object3D& add(Object3D* child) = delete;

 private:
  glm::vec3 color;
  gl::Shader shader;
  gl::TexturePtr texture;
  gl::VertexArrayObject vao;
  gl::VertexBuffer vbo;
  gl::ElementBufferObject ebo;
};

class Skybox : public Mesh
{
 public:
  Skybox(const std::array<std::string, 6>& faces);
  Object3D& add(Object3D* child) = delete;
};

// TODO
class RenderTarget
{
};

#if 0
class Renderer
{
 public:
  Renderer(unsigned int width, unsigned int height)
      : shadow_map(new ShadowMap(1024, 1024)), m_width(width), m_height(height), background(rgb(222, 253, 255))
  {
    const std::vector<float> quad_vertices = {
        -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  // top left
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom left
        1.0f,  1.0f,  0.0f, 1.0f, 1.0f,  // top right

        1.0f,  1.0f,  0.0f, 1.0f, 1.0f,  // top right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom left
        1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
    };

#if 0
    auto geometry = std::make_shared<Geometry>(quad_vertices, Geometry::POS_UV);
    auto texture = std::make_shared<gfx::gl::Texture>(shadow_map->depth_map);
    auto material = std::make_shared<ScreenMaterial>(texture);
    screen_quad = std::make_shared<Mesh>(geometry, material);
#endif
  }

  ~Renderer()
  {
    if (shadow_map) delete shadow_map;
  }

  void render(Camera& camera, Object3D& scene);
  glm::vec3 background;

 private:
  unsigned int m_width, m_height;
  ShadowMap* shadow_map = nullptr;
  std::shared_ptr<Mesh> screen_quad;
};
#endif

class Renderer2
{
 private:
  GLsizei m_width, m_height;
  ShaderCache m_shaders;
  gfx::MeshPtr m_skybox;

  // For the future:
  // MeshPtr m_screenquad;
  // ShadowMap m_shadowmap;
  // std::optional<gl::FrameBuffer> m_framebuffer;

 public:

  Renderer2(GLsizei width, GLsizei height);
  ~Renderer2();

  void render_skybox(RenderContext& context);

  // void render_shadow_pass(Camera& camera, Object3D& scene);

  // render scene
  void render(Camera& camera, Object3D& scene);
  void render(Camera& camera, Object3D& scene, RenderTarget& render_target);
};

};  // namespace gfx
