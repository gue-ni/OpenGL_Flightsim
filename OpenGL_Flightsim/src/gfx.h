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

constexpr float PI = 3.14159265359f;

typedef glm::vec3 RGB;
typedef glm::vec4 RGBA;

class Mesh;
class Light;
class Camera;
class Skybox;
class Material;
class Geometry;

constexpr RGB rgb(int r, int g, int b)
{
  return RGB(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b)) / 255.0f;
}

constexpr RGB rgb(uint32_t hex)
{
  assert(hex <= 0xffffffU);
  return RGB{static_cast<float>((hex & 0xff0000U) >> 16) / 255.0f, static_cast<float>((hex & 0x00ff00U) >> 8) / 255.0f,
             static_cast<float>((hex & 0x0000ffU) >> 0) / 255.0f};
}

std::vector<float> load_obj(const std::string path);
std::shared_ptr<Geometry> make_cube_geometry(float size);
std::shared_ptr<Geometry> make_plane_geometry(int x_elements, int y_elements, float size);

// gl primitives
namespace gl
{

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
  ~Texture();

  virtual void bind(GLuint texture = 0U) const;
  virtual void unbind() const;
  GLint get_format(int channels);
  void set_parameteri(GLenum target, GLenum pname, GLint param);

  static unsigned char* load_image(const std::string path, int* width, int* height, int* channels, bool flip);
};

struct CubemapTexture : public Texture {
  CubemapTexture(const std::array<std::string, 6>& paths, bool flip_vertically = false);
  void bind(GLuint texture) const override;
  void unbind() const override;
};
};  // namespace gl

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

 private:
  unsigned int m_vao, m_vbo;
  gl::VertexBuffer vbo;
  gl::VertexArrayObject vao;
  static int get_stride(const VertexLayout& layout);
};

class Material
{
 public:
  virtual gl::Shader* get_shader() { return nullptr; }
  virtual void bind() {}
};

template <class Derived>
class MaterialX : public Material
{
 public:
  MaterialX(const std::string& path)
  {
    if (shader == nullptr) shader = std::make_shared<gl::Shader>(path);
  }
  gl::Shader* get_shader() { return shader.get(); }
  static std::shared_ptr<gl::Shader> shader;
};

class Phong : public MaterialX<Phong>
{
 public:
  RGB rgb{};
  float ka, kd, ks, alpha;
  std::shared_ptr<gl::Texture> texture = nullptr;

  Phong(const glm::vec3& color_, float ka_, float kd_, float ks_, float alpha_)
      : MaterialX<Phong>("shaders/phong"), rgb(color_), ka(ka_), kd(kd_), ks(ks_), alpha(alpha_)
  {
  }

  Phong(const glm::vec3& color_)
      : MaterialX<Phong>("shaders/phong"), rgb(color_), ka(0.3f), kd(1.0f), ks(0.5f), alpha(10.0f)
  {
  }

  Phong(std::shared_ptr<gl::Texture> tex)
      : MaterialX<Phong>("shaders/phong"),
        texture(tex),
        rgb(0.0f, 1.0f, 0.0f),
        ka(0.3f),
        kd(1.0f),
        ks(0.5f),
        alpha(20.0f)
  {
  }

  void bind() override;
};

class Basic : public MaterialX<Basic>
{
 public:
  glm::vec3 rgb;
  Basic(const glm::vec3& color_) : MaterialX<Basic>("shaders/basic"), rgb(color_) {}
  void bind();
};

class ShaderMaterial : public MaterialX<ShaderMaterial>
{
 public:
  ShaderMaterial(const std::string& path) : MaterialX<ShaderMaterial>(path) {}
};

class ScreenMaterial : public MaterialX<ScreenMaterial>
{
 private:
  std::shared_ptr<gl::Texture> texture = nullptr;

 public:
  ScreenMaterial(std::shared_ptr<gl::Texture> t) : MaterialX<ScreenMaterial>("shaders/screen"), texture(t) {}
  void bind() override;
};

class SkyboxMaterial : public MaterialX<SkyboxMaterial>
{
 public:
  std::shared_ptr<gl::CubemapTexture> cubemap = nullptr;
  SkyboxMaterial(std::shared_ptr<gl::CubemapTexture> map) : MaterialX<SkyboxMaterial>("shaders/skybox"), cubemap(map) {}

  void bind() override;
};

class Mesh : public Object3D
{
 public:
  Mesh(std::shared_ptr<Geometry> geometry, std::shared_ptr<Material> material)
      : m_geometry(geometry), m_material(material)
  {
  }
  void draw_self(RenderContext& context) override;

 protected:
  std::shared_ptr<Geometry> m_geometry;
  std::shared_ptr<Material> m_material;
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

class Skybox : public Mesh
{
 public:
  Skybox(const std::array<std::string, 6>& faces);
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

    auto geometry = std::make_shared<Geometry>(quad_vertices, Geometry::POS_UV);
    auto texture = std::make_shared<gfx::gl::Texture>(shadow_map->depth_map.id);
    auto material = std::make_shared<ScreenMaterial>(texture);
    screen_quad = std::make_shared<Mesh>(geometry, material);
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
