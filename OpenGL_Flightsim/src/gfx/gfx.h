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

#include "gl.h"
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
class ShaderCache;

using GeometryPtr = std::shared_ptr<Geometry>;

std::vector<float> load_obj(const std::string path);
std::shared_ptr<Geometry> make_cube_geometry(float size);
std::shared_ptr<Geometry> make_plane_geometry(int x_elements, int y_elements, float size);
std::shared_ptr<Geometry> make_quad_geometry();

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

  std::vector<Light*> lights;
  bool is_shadow_pass;
  glm::vec3 background_color;
};

#define OBJ3D_TRANSFORM 1U << 0U
#define OBJ3D_ROTATE    1U << 1U
#define OBJ3D_SCALE     1U << 2U

class Object3D
{
 public:
  enum Type { OBJECT3D, LIGHT, CAMERA };

  Object3D()
      : id(counter++), parent(nullptr), transform(1.0), m_position(0.0f), m_rotation(glm::vec3(0.0f)), m_scale(1.0f)
  {
  }

  const int id;
  static int counter;
  // which transform to inherit from parent
  unsigned transform_flags = OBJ3D_TRANSFORM | OBJ3D_ROTATE | OBJ3D_SCALE;

  Object3D* parent;
  std::vector<Object3D*> children;
  glm::mat4 transform;
  bool receive_shadow = true;
  bool visible = true;
  bool wireframe = false;

  Object3D& add(Object3D* child);

  void draw(RenderContext& context);
  void draw_children(RenderContext& context);
  virtual void draw_self(RenderContext& context);

  void set_scale(const glm::vec3& scale);
  void set_rotation(const glm::vec3& rotation);
  void rotate_by(const glm::vec3& rotation);
  void set_rotation_quat(const glm::quat& rotation);
  void set_position(const glm::vec3& position);
  void set_transform(const Object3D& transform);
  void set_transform(const glm::vec3& position, const glm::quat& rotation);

  glm::vec3 get_scale() const;
  glm::vec3 get_rotation() const;
  glm::quat get_rotation_quat() const;
  glm::vec3 get_position() const;
  glm::quat get_world_rotation_quat() const;
  glm::vec3 get_world_position() const;

  virtual Object3D::Type get_type() const;

  void override_transform(const glm::mat4& matrix);
  void update_world_matrix(bool dirty_parent);
  glm::mat4 get_local_transform() const;
  glm::mat4 get_parent_transform() const;
  glm::mat4 get_transform() const;
  void traverse(const std::function<bool(Object3D*)>& func);

 protected:
  bool m_dirty_dof = false;
  bool m_dirty_transform = false;

  glm::vec3 m_position;
  glm::vec3 m_scale;
  glm::quat m_rotation;
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

class Geometry
{
 public:
  enum VertexLayout {
    POS,         // pos
    POS_UV,      // pos, uv
    POS_NORM,    // pos, normal
    POS_NORM_UV  // pos, normal, uv
  };

  Geometry(const std::vector<float>& vertices, const VertexLayout& layout);
  Geometry(const Geometry& geometry);
  ~Geometry();
  void bind();
  void unbind();
  int triangle_count;

  gl::VertexArrayObject vao;

 private:
  gl::VertexBuffer vbo;
  static int get_stride(const VertexLayout& layout);
};

class Material2
{
 public:
  Material2(const std::string& shader_name, const gl::TexturePtr& texture)
      : m_shader_name(shader_name), m_texture(texture)
  {
  }

  std::string& get_shader_name() { return m_shader_name; }

  gl::TexturePtr get_texture() const { return m_texture; }

 private:
  gl::TexturePtr m_texture = nullptr;
  std::string m_shader_name;
};

using Material2Ptr = std::shared_ptr<Material2>;

class Mesh2 : public Object3D
{
 public:
  Mesh2(const GeometryPtr& geometry, const Material2Ptr& material);

 protected:
  Material2Ptr m_material;
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
  Billboard(std::shared_ptr<gl::Texture> sprite, glm::vec3 color = glm::vec3(1.0f));
  void draw_self(RenderContext& context) override;
  Object3D& add(Object3D* child) = delete;

 private:
  glm::vec3 color;
  gl::Shader shader;
  std::shared_ptr<gl::Texture> texture;
  gl::VertexArrayObject vao;
  gl::VertexBuffer vbo;
  gl::ElementBufferObject ebo;
};

class Skybox : public Mesh2
{
 public:
  Skybox(const std::array<std::string, 6>& faces);
  // Skybox(const Material2Ptr& material);
  void draw_self(RenderContext& context) override;
  Object3D& add(Object3D* child) = delete;
};

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

class Renderer2
{
 private:
  GLsizei m_width, m_height;
  ShaderCache m_shaders;
  // std::optional<gl::FrameBuffer> m_framebuffer;

 public:
  Renderer2(GLsizei width, GLsizei height);
  ~Renderer2();
  void render(Camera& camera, Object3D& scene);
};

class FirstPersonController
{
 public:
  enum Direction {
    FORWARD,
    RIGHT,
    BACKWARD,
    LEFT,
  };

  FirstPersonController(float speed)
      : m_speed(speed),
        m_yaw(-90.0f),
        m_pitch(0.0f),
        m_front(0.0f, 0.0f, -1.0f),
        m_up(0.0f, 1.0f, 0.0f),
        m_velocity(0.0f),
        m_direction(0.0f)
  {
    move_mouse(0.0f, 0.0f);
  }

  void update(Object3D& object, float dt);
  void move_mouse(float x, float y);
  void move(const Direction& direction);
  inline glm::vec3 get_front() const { return m_front; }

 private:
  float m_speed, m_yaw, m_pitch;
  glm::vec3 m_front, m_up, m_velocity, m_direction;
};

class OrbitController
{
 public:
  OrbitController(float radius) : radius(radius) {}

  float radius;
  void update(Object3D& object, const glm::vec3& center, float dt);
  void move_mouse(float x, float y);

 private:
  float m_yaw = 0.0f;
  float m_pitch = 0.0f;
};
};  // namespace gfx
