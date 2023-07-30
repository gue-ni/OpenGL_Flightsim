#include "gfx.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../lib/tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

std::string load_text_file(const std::string& path)
{
  std::fstream file(path);
  if (!file.is_open()) return std::string();

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

namespace gfx
{

namespace gl
{

Shader::Shader(const std::string& path) : Shader(load_text_file(path + ".vert"), load_text_file(path + ".frag")) {}

Shader::Shader(const std::string& vertShader, const std::string& fragShader)
{
  // std::cout << "create Shader\n";
  const char* vertexShaderSource = vertShader.c_str();
  const char* fragmentShaderSource = fragShader.c_str();

  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  // check for shader compile errors
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // fragment shader
  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  // check for shader compile errors
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  // link shaders
  id = glCreateProgram();
  glAttachShader(id, vertexShader);
  glAttachShader(id, fragmentShader);
  glLinkProgram(id);
  // check for linking errors
  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(id, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

Shader::~Shader() { glDeleteProgram(id); }

void Shader::bind() const { glUseProgram(id); }

void Shader::unbind() const { glUseProgram(0); }

void Shader::uniform(const std::string& name, int value) { glUniform1i(glGetUniformLocation(id, name.c_str()), value); }

void Shader::uniform(const std::string& name, unsigned int value)
{
  glUniform1ui(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::uniform(const std::string& name, float value)
{
  glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::uniform(const std::string& name, const glm::vec3& value)
{
  glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::uniform(const std::string& name, const glm::vec4& value)
{
  glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::uniform(const std::string& name, const glm::mat4& value)
{
  glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

Texture::Texture(const std::string& path) : Texture(path, {}) {}

unsigned char* Texture::load_image(const std::string path, int* width, int* height, int* channels, bool flip)
{
  stbi_set_flip_vertically_on_load(flip);
  return stbi_load(path.c_str(), width, height, channels, 0);
}

Texture::Texture(const std::string& path, const TextureParams& params)
{
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.texture_wrap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.texture_wrap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.texture_min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.texture_mag_filter);

  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, channels;
  // stbi_set_flip_vertically_on_load(params.flip_vertically);
  unsigned char* data = load_image(path, &width, &height, &channels, params.flip_vertically);

  // printf("channels = %d\n", channels);

  if (data) {
    auto format = get_format(channels);

    // std::cout  << path << ": format = " << format << ", channels = " <<
    // channels << std::endl;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);
}

Texture::~Texture() { glDeleteTextures(1, &id); }

void Texture::bind(GLuint texture) const
{
  glActiveTexture(GL_TEXTURE0 + texture);
  glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

GLint Texture::get_format(int channels)
{
  GLint format{};

  switch (channels) {
    case 1:
      format = GL_RED;
      break;
    case 3:
      format = GL_RGB;
      break;
    case 4:
      format = GL_RGBA;
      break;
    default:
      std::cout << "Failed to load texture, invalid format, channels = " << channels << std::endl;
      assert(false);
      break;
  }

  return format;
}

void Texture::set_parameteri(GLenum target, GLenum pname, GLint param) { glTexParameteri(target, pname, param); }

CubemapTexture::CubemapTexture(const std::array<std::string, 6>& paths, bool flip_vertically)
{
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, id);

  int width, height, channels;
  for (int i = 0; i < 6; i++) {
    stbi_set_flip_vertically_on_load(flip_vertically);
    unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &channels, 0);
    if (data) {
      auto format = get_format(channels);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      std::cout << "Cubemap tex failed to load at path: " << paths[i] << std::endl;
      stbi_image_free(data);
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void CubemapTexture::bind(GLuint texture) const
{
  glActiveTexture(GL_TEXTURE0 + texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

void CubemapTexture::unbind() const { glBindTexture(GL_TEXTURE_CUBE_MAP, 0); }

VertexBuffer::VertexBuffer() { glGenBuffers(1, &id); }

VertexBuffer::~VertexBuffer() { glDeleteBuffers(1, &id); }

void VertexBuffer::bind() const { glBindBuffer(GL_ARRAY_BUFFER, id); }

void VertexBuffer::unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

void VertexBuffer::buffer(const void* data, size_t size)
{
  bind();
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VertexArrayObject::VertexArrayObject() { glGenVertexArrays(1, &id); }

VertexArrayObject::~VertexArrayObject() { glDeleteVertexArrays(1, &id); }

void VertexArrayObject::bind() const { glBindVertexArray(id); }

void VertexArrayObject::unbind() const { glBindVertexArray(0); }

ElementBufferObject::ElementBufferObject() { glGenBuffers(1, &id); }

ElementBufferObject::~ElementBufferObject() { glDeleteBuffers(1, &id); }

void ElementBufferObject::bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id); }

void ElementBufferObject::unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

void ElementBufferObject::buffer(const void* data, size_t size)
{
  bind();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}
};  // namespace gl

Geometry::Geometry(const std::vector<float>& vertices, const VertexLayout& layout)
    : triangle_count(static_cast<int>(vertices.size()) / (get_stride(layout)))
{
  const int stride = get_stride(layout);

  vao.bind();
  vbo.buffer(vertices);

  unsigned int index = 0;
  glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
  glEnableVertexAttribArray(index);

  if (layout == POS_NORM || layout == POS_NORM_UV)  // add normal
  {
    index++;
    glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(index);
  }

  if (layout == POS_UV || layout == POS_NORM_UV)  // add uv
  {
    index++;
    glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float),
                          (void*)(static_cast<int>(index) * 3 * sizeof(float)));
    glEnableVertexAttribArray(index);
  }

  vbo.unbind();
  vao.bind();
}

Geometry::Geometry(const Geometry& geometry) { triangle_count = geometry.triangle_count; }

Geometry::~Geometry() {}

void Geometry::bind() { vao.bind(); }

void Geometry::unbind() { vao.unbind(); }

int Geometry::get_stride(const VertexLayout& layout)
{
  switch (layout) {
    case POS:
      return 3;
    case POS_UV:
      return 5;
    case POS_NORM:
      return 6;
    case POS_NORM_UV:
      return 8;
  }
  return 0;
}

Object3D::Type Camera::get_type() const { return Object3D::Type::CAMERA; }

glm::mat4 Camera::get_view_matrix() const { return glm::inverse(transform); }

glm::mat4 Camera::get_projection_matrix() const { return m_projection; }

void Camera::look_at(const glm::vec3& target)
{
  override_transform(glm::inverse(glm::lookAt(m_position, target, m_up)));
}

template <class Derived>
std::shared_ptr<gl::Shader> MaterialX<Derived>::shader = nullptr;

int Object3D::counter = 0;

void Object3D::draw(RenderContext& context)
{
  if (visible) {
    draw_self(context);
  }
  draw_children(context);
}

void Object3D::draw_self(RenderContext& context) {}

void Object3D::draw_children(RenderContext& context)
{
  for (auto child : children) child->draw(context);
}

glm::vec3 Object3D::get_position() const { return m_position; }

glm::vec3 Object3D::get_rotation() const { return glm::eulerAngles(m_rotation); }

glm::quat Object3D::get_rotation_quat() const { return m_rotation; }

glm::vec3 Object3D::get_scale() const { return m_scale; }

void Object3D::set_scale(const glm::vec3& scale)
{
  m_scale = scale;
  m_dirty_dof = true;
}

void Object3D::set_position(const glm::vec3& pos)
{
  m_position = pos;
  m_dirty_dof = true;
}

void Object3D::set_transform(const Object3D& transform)
{
  set_scale(transform.get_scale());
  set_position(transform.get_position());
  set_rotation(transform.get_rotation());
}

void Object3D::set_transform(const glm::vec3& position, const glm::quat& rotation)
{
  set_position(position);
  set_rotation_quat(rotation);
}

void Object3D::set_rotation(const glm::vec3& rot)
{
  m_rotation = glm::quat(rot);
  m_dirty_dof = true;
}

void Object3D::rotate_by(const glm::vec3& rot) { set_rotation(get_rotation() + rot); }

void Object3D::set_rotation_quat(const glm::quat& quat) { m_rotation = quat; }

glm::mat4 Object3D::get_local_transform() const
{
  auto T = glm::translate(glm::mat4(1.0f), m_position);
  auto R = glm::toMat4(m_rotation);
  auto S = glm::scale(glm::mat4(1.0f), m_scale);
  return T * R * S;
}

void Object3D::traverse(const std::function<bool(Object3D*)>& func)
{
  if (func(this)) {
    for (const auto& child : children) {
      child->traverse(func);
    }
  }
}

void Object3D::override_transform(const glm::mat4& matrix)
{
  m_dirty_transform = true;
  m_dirty_dof = true;
  transform = matrix;
  // TODO: relcalculate position, rotation etc
}

void Object3D::update_world_matrix(bool dirty_parent)
{
  bool dirty = m_dirty_dof || dirty_parent;

  if (dirty && !m_dirty_transform) {
    if (parent)
      transform = get_parent_transform() * get_local_transform();
    else
      transform = get_local_transform();
  }

  for (auto child : children) {
    child->update_world_matrix(dirty || m_dirty_transform);
  }

  m_dirty_dof = m_dirty_transform = false;
}

glm::mat4 Object3D::get_parent_transform() const
{
  assert(parent != nullptr);

  if (transform_flags == (OBJ3D_TRANSFORM | OBJ3D_ROTATE | OBJ3D_SCALE)) {
    return parent->transform;
  }

  glm::mat4 parent_transform(1.0f);

  if (transform_flags & OBJ3D_TRANSFORM) {
    parent_transform *= glm::translate(glm::mat4(1.0f), parent->get_world_position());
  }
  if (transform_flags & OBJ3D_ROTATE) {
    parent_transform *= glm::toMat4(parent->get_world_rotation_quat());
  }
  if (transform_flags & OBJ3D_SCALE) {
    parent_transform *= glm::scale(glm::mat4(1.0f), parent->get_scale());
  }

  return parent_transform;
}

Object3D& Object3D::add(Object3D* child)
{
  child->parent = this;
  children.push_back(child);
  return (*this);
}

glm::quat Object3D::get_world_rotation_quat() const
{
  if (parent == nullptr)
    return get_rotation_quat();
  else
    return parent->get_world_rotation_quat() * m_rotation;
}

glm::vec3 Object3D::get_world_position() const { return glm::vec3(transform * glm::vec4(glm::vec3(0.0f), 1.0f)); }

Object3D::Type Object3D::get_type() const { return Type::OBJECT3D; }

Object3D::Type Light::get_type() const { return Object3D::Type::LIGHT; }

glm::mat4 Light::light_space_matrix()
{
  float near_plane = 0.1f, far_plane = 10.0f, m = 10.0f;
  auto wp = get_world_position();
  glm::mat4 light_view = glm::lookAt(wp, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 light_projection = glm::ortho(-m, m, -m, m, near_plane, far_plane);
  return light_projection * light_view;
}

void Renderer::render(Camera& camera, Object3D& scene)
{
  scene.update_world_matrix(false);

  RenderContext context;
  context.camera = &camera;
  context.shadow_map = shadow_map;
  context.shadow_caster = nullptr;
  context.background_color = background;

  scene.traverse([&context](Object3D* obj) {
    if (obj->get_type() == Object3D::Type::LIGHT) {
      Light* light = dynamic_cast<Light*>(obj);
      context.lights.push_back(light);

      if (light->cast_shadow) {
        context.shadow_caster = light;
      }
    }

    return true;
  });

  if (shadow_map && context.shadow_caster) {
    context.is_shadow_pass = true;
    glViewport(0, 0, shadow_map->width, shadow_map->height);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map->fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    scene.draw(context);
  }

  context.is_shadow_pass = false;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, m_width, m_height);
  glClearColor(background.x, background.y, background.z, 1.0f);
  // glClearColor(1, 0, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 1
  scene.draw(context);
#else
  screen_quad->draw(context);
#endif
}

void Mesh::draw_self(RenderContext& context)
{
  glm::mat4 lightSpaceMatrix(1.0f);

  if (context.is_shadow_pass) {
    assert(context.shadow_caster);

    gl::Shader* shader = &context.shadow_map->shader;

    shader->bind();
    shader->uniform("u_Model", transform);
    shader->uniform("u_LightSpaceMatrix", context.shadow_caster->light_space_matrix());
  } else {
    gl::Shader* shader = m_material->get_shader();

    shader->bind();
    shader->uniform("u_Model", transform);
    shader->uniform("u_View", context.camera->get_view_matrix());
    shader->uniform("u_Projection", context.camera->get_projection_matrix());

    if (context.shadow_caster) {
      shader->uniform("u_LightSpaceMatrix", context.shadow_caster->light_space_matrix());
    }

    shader->uniform("u_BackgroundColor", context.background_color);
    shader->uniform("u_NumLights", static_cast<int>(context.lights.size()));
    shader->uniform("u_CameraPosition", context.camera->get_world_position());
    shader->uniform("u_ReceiveShadow", (receive_shadow && context.shadow_caster));

    for (int i = 0; i < context.lights.size(); i++) {
      auto index = std::to_string(i);
      auto type = context.lights[i]->type;

      shader->uniform("u_Lights[" + index + "].type", type);
      shader->uniform("u_Lights[" + index + "].color", context.lights[i]->rgb);
      shader->uniform("u_Lights[" + index + "].position", context.lights[i]->get_world_position());
    }

    context.shadow_map->depth_map.bind(0);
    shader->uniform("u_ShadowMap", 0);

    m_material->bind();
  }

  m_geometry->bind();

  glDrawArrays(GL_TRIANGLES, 0, m_geometry->triangle_count);
  m_geometry->unbind();

  draw_children(context);
}

ShadowMap::ShadowMap(unsigned int shadow_width, unsigned int shadow_height)
    : width(shadow_width), height(shadow_height), shader("shaders/depth")
{
  // glGenTextures(1, &depth_map_texture_id);
  glBindTexture(GL_TEXTURE_2D, depth_map.id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_width, shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
               NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map.id, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindBuffer(GL_FRAMEBUFFER, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "failure\n";
    exit(-1);
  }
}

void FirstPersonController::update(Object3D& target, float dt)
{
  const auto pos = target.get_position();
  target.set_position(pos + m_velocity * dt);
  target.override_transform(glm::inverse(glm::lookAt(pos, pos + m_front, m_up)));
  m_velocity = glm::vec3(0.0f);
}

void FirstPersonController::move_mouse(float x, float y)
{
  glm::vec2 offset(x, y);

  const float sensitivity = 0.1f;
  offset *= sensitivity;

  m_yaw += offset.x;
  m_pitch -= offset.y;

  if (m_pitch > +89.9f) m_pitch = +89.0f;
  if (m_pitch < -89.9f) m_pitch = -89.0f;

  glm::vec3 front(0);
  front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
  front.y = sin(glm::radians(m_pitch));
  front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
  m_front = glm::normalize(front);
}

void FirstPersonController::move(const Direction& direction)
{
  switch (direction) {
    case FORWARD: {
      m_velocity += (m_speed * m_front);
      break;
    }
    case LEFT: {
      m_velocity -= (m_speed * glm::normalize(glm::cross(m_front, m_up)));
      break;
    }
    case BACKWARD: {
      m_velocity -= (m_speed * m_front);
      break;
    }
    case RIGHT: {
      m_velocity += (m_speed * glm::normalize(glm::cross(m_front, m_up)));
      break;
    }
    default:
      break;
  }
}

void OrbitController::update(Object3D& target, const glm::vec3& center, float dt)
{
  glm::vec3 front;
  front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
  front.y = sin(glm::radians(m_pitch));
  front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
  auto offset = glm::normalize(front) * radius;

  const auto pos = center + offset;
  target.set_position(pos);
  target.override_transform(glm::inverse(glm::lookAt(pos, pos - front, glm::vec3(0, 1, 0))));
}

void OrbitController::move_mouse(float x, float y)
{
  glm::vec2 offset(x, y);

  const float sensitivity = 0.1f;
  offset *= sensitivity;

  m_yaw += offset.x;
  m_pitch += offset.y;

  m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
}

void Phong::bind()
{
  if (texture != nullptr) {
    int texture_unit = 1;
    texture->bind(texture_unit);
    shader->uniform("u_UseTexture", true);
    shader->uniform("u_Texture1", texture_unit);
  } else {
    shader->uniform("u_UseTexture", false);
    shader->uniform("u_SolidObjectColor", rgb);
  }

  gl::Shader* shader = get_shader();
  shader->bind();
  shader->uniform("ka", ka);
  shader->uniform("kd", kd);
  shader->uniform("ks", ks);
  shader->uniform("alpha", alpha);
}

void Basic::bind()
{
  gl::Shader* shader = get_shader();
  shader->uniform("ka", 0.6f);
  shader->uniform("kd", 0.8f);
  shader->uniform("ks", 0.2f);
  shader->uniform("alpha", 10.0f);
  shader->uniform("u_UseTexture", false);
  shader->uniform("u_SolidObjectColor", rgb);
}

std::vector<float> load_obj(const std::string path)
{
  std::vector<float> vertices;

  std::istringstream source(load_text_file(path));

  std::string warning, error;
  tinyobj::attrib_t attributes;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, &source)) {
    std::cout << "loadObj::Error: " << warning << error << std::endl;
    return {};
  }

#if 0
        printf("# of vertices  = %d\n", (int)(attributes.vertices.size()) / 3);
        printf("# of normals   = %d\n", (int)(attributes.normals.size()) / 3);
        printf("# of texcoords = %d\n", (int)(attributes.texcoords.size()) / 2);
        printf("# of materials = %d\n", (int)materials.size());
        printf("# of shapes    = %d\n", (int)shapes.size());
#endif

  for (size_t s = 0; s < shapes.size(); s++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      // hardcode loading to triangles
      int fv = 3;

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

        // vertex position
        tinyobj::real_t vx = attributes.vertices[3 * idx.vertex_index + 0];
        tinyobj::real_t vy = attributes.vertices[3 * idx.vertex_index + 1];
        tinyobj::real_t vz = attributes.vertices[3 * idx.vertex_index + 2];
        // vertex normal
        tinyobj::real_t nx = attributes.normals[3 * idx.normal_index + 0];
        tinyobj::real_t ny = attributes.normals[3 * idx.normal_index + 1];
        tinyobj::real_t nz = attributes.normals[3 * idx.normal_index + 2];

        tinyobj::real_t tx = attributes.texcoords[2 * idx.texcoord_index + 0];
        tinyobj::real_t ty = attributes.texcoords[2 * idx.texcoord_index + 1];

        vertices.push_back(vx);
        vertices.push_back(vy);
        vertices.push_back(vz);
        vertices.push_back(nx);
        vertices.push_back(ny);
        vertices.push_back(nz);
        vertices.push_back(tx);
        vertices.push_back(ty);
      }
      index_offset += fv;
    }
  }

  return vertices;
}

std::shared_ptr<Geometry> make_cube_geometry(float size)
{
  float s = size / 2;

  // clang-format off
  std::vector<float> vertices = {
      // left
      -s, -s, -s, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
      s, -s, -s, 0.0f, 0.0f,-1.0f,1.0f,0.0f,
      s,s,-s,0.0f,0.0f,-1.0f,1.0f,1.0f,
      s,s,-s,0.0f,0.0f,-1.0f,1.0f,1.0f,
      -s,s,-s,0.0f,0.0f,-1.0f,0.0f,1.0f,
      -s,-s,-s,0.0f,0.0f,-1.0f,0.0f,0.0f,

      // right
      -s,-s,s,0.0f,0.0f,1.0f,0.0f,0.0f,
      s,-s,s,0.0f,0.0f,1.0f,1.0f,0.0f,
      s,s,s,0.0f,0.0f,1.0f,1.0f,1.0f,
      s,s,s,0.0f,0.0f,1.0f,1.0f,1.0f,
      -s,s,s,0.0f,0.0f,1.0f,0.0f,1.0f,
      -s,-s,s,0.0f,0.0f,1.0f,0.0f,0.0f,

      // backward
      -s,s,s,-1.0f,0.0f,0.0f,0.0f,0.0f,
      -s,s,-s,-1.0f,0.0f,0.0f,1.0f,0.0f,
      -s,-s,-s,-1.0f,0.0f,0.0f,1.0f,1.0f,
      -s,-s,-s,-1.0f,0.0f,0.0f,1.0f,1.0f,
      -s,-s,s,-1.0f,0.0f,0.0f,0.0f,1.0f,
      -s,s,s,-1.0f,0.0f,0.0f,0.0f,0.0f,

      // forward
      s,s,s,1.0f,0.0f,0.0f,0.0f,0.0f,
      s,s,-s,1.0f,0.0f,0.0f,1.0f,0.0f,
      s,-s,-s,1.0f,0.0f,0.0f,1.0f,1.0f,
      s,-s,-s,1.0f,0.0f,0.0f,1.0f,1.0f,
      s,-s,s,1.0f,0.0f,0.0f,0.0f,1.0f,
      s,s,s,1.0f,0.0f,0.0f,0.0f,0.0f,

      // down
      -s,-s,-s,0.0f,-1.0f,0.0f,0.0f,0.0f,
      s,-s,-s,0.0f,-1.0f,0.0f,1.0f,0.0f,
      s,-s,s,0.0f,-1.0f,0.0f,1.0f,1.0f,
      s,-s,s,0.0f,-1.0f,0.0f,1.0f,1.0f,
      -s,-s,s,0.0f,-1.0f,0.0f,0.0f,1.0f,
      -s,-s,-s,0.0f,-1.0f,0.0f,0.0f,0.0f,

      // up
      -s,s,-s,0.0f,1.0f,0.0f,0.0f,0.0f,
      s,s,-s,0.0f,1.0f,0.0f,1.0f,0.0f,
      s,s,s,0.0f,1.0f,0.0f,1.0f,1.0f,
      s,s,s,0.0f,1.0f,0.0f,1.0f,1.0f,
      -s,s,s,0.0f,1.0f,0.0f,0.0f,1.0f,
      -s,s,-s,0.0f,1.0f,0.0f,0.0f,0.0f,

  };
  // clang-format on
  return std::make_shared<Geometry>(vertices, Geometry::POS_NORM_UV);
}

void push_back(std::vector<float>& vector, const glm::vec3& v)
{
  vector.push_back(v.x);
  vector.push_back(v.y);
  vector.push_back(v.z);
}

void push_back(std::vector<float>& vector, const glm::vec2& v)
{
  vector.push_back(v.x);
  vector.push_back(v.y);
}

void push_back(std::vector<float>& vector, const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv)
{
  push_back(vector, pos);
  push_back(vector, normal);
  push_back(vector, uv);
}

std::shared_ptr<Geometry> make_plane_geometry(int x_elements, int y_elements, float size)
{
  // TODO
  const float width = size, height = size;

  std::vector<float> vertices;

  glm::vec3 normal(0.0f, 1.0f, 0.0f);

  for (int y = 0; y < y_elements; y++) {
    for (int x = 0; x < x_elements; x++) {
      auto bottom_left = glm::vec3((x + 0) * width, 0.0f, (y + 0) * height);
      auto bottom_right = glm::vec3((x + 1) * width, 0.0f, (y + 0) * height);
      auto top_left = glm::vec3((x + 0) * width, 0.0f, (y + 1) * height);
      auto top_right = glm::vec3((x + 1) * width, 0.0f, (y + 1) * height);

      auto tex_coord = glm::vec2(static_cast<float>(x) / static_cast<float>(x_elements),
                                 static_cast<float>(y) / static_cast<float>(y_elements));

      // triangle 1
      push_back(vertices, top_right, normal, glm::vec2(1.0f, 1.0f));
      push_back(vertices, bottom_right, normal, glm::vec2(1, 0.0f));
      push_back(vertices, bottom_left, normal, glm::vec2(0.0f, 0.0f));

      // triangle 2
      push_back(vertices, bottom_left, normal, glm::vec2(0.0f, 0.0f));
      push_back(vertices, top_left, normal, glm::vec2(0.0f, 1.0f));
      push_back(vertices, top_right, normal, glm::vec2(1.0f, 1.0f));
    }
  }

  return std::make_shared<Geometry>(vertices, Geometry::POS_NORM_UV);
}

Billboard::Billboard(std::shared_ptr<gl::Texture> sprite, glm::vec3 color)
    : texture(sprite), shader("shaders/billboard"), color(color)
{
  // clang-format off
  float vertices[] = {
      0.5f,  0.5f,  0.0f, 
      0.0f, 0.0f, 0.5f,  
      -0.5f, 0.0f, 0.0f, 
      1.0f,-0.5f, -0.5f, 
      0.0f, 1.0f, 1.0f, 
      -0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
  };
  // clang-format on

  unsigned int indices[] = {
      0, 1, 3,  // first Triangle
      1, 2, 3   // second Triangle
  };

  vao.bind();
  vbo.buffer(vertices, sizeof(vertices));

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  ebo.buffer(indices, sizeof(indices));
  vao.unbind();
}

void Billboard::draw_self(RenderContext& context)
{
  if (context.is_shadow_pass) return;

  auto camera = context.camera;

  auto view = camera->get_view_matrix();
  glm::vec3 up = {view[0][1], view[1][1], view[2][1]};
  glm::vec3 right = {view[0][0], view[1][0], view[2][0]};

  shader.bind();
  shader.uniform("u_View", view);
  shader.uniform("u_Projection", camera->get_projection_matrix());
  shader.uniform("u_Texture", 5);
  shader.uniform("u_Color", color);
  shader.uniform("u_Position", get_world_position());
  shader.uniform("u_Scale", get_scale());
  shader.uniform("u_Right", right);
  shader.uniform("u_Up", up);
  texture->bind(5);

  vao.bind();

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // TODO draw
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  vao.unbind();

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
}

Skybox::Skybox(const std::array<std::string, 6>& faces)
    : Mesh(make_cube_geometry(1.0f), std::make_shared<SkyboxMaterial>(std::make_shared<gl::CubemapTexture>(faces)))
{
}

void Skybox::draw_self(RenderContext& context)
{
  if (!context.is_shadow_pass) {
    glDepthMask(GL_FALSE);

    m_material->bind();
    gl::Shader* shader = m_material->get_shader();

    auto u_View = glm::mat4(glm::mat3(context.camera->get_view_matrix()));
    shader->uniform("u_View", u_View);
    shader->uniform("u_Model", transform);
    shader->uniform("u_Projection", context.camera->get_projection_matrix());

    m_geometry->bind();
    glDrawArrays(GL_TRIANGLES, 0, m_geometry->triangle_count);

    glDepthMask(GL_TRUE);
  }
}

void SkyboxMaterial::bind()
{
  gl::Shader* shader = get_shader();
  int unit = 2;
  cubemap->bind(unit);
  shader->bind();
  shader->uniform("u_Skybox", unit);
}

void ScreenMaterial::bind()
{
  gl::Shader* shader = get_shader();
  texture->bind(0);
  shader->bind();
  shader->uniform("u_ShadowMap", 0);
}
}  // namespace gfx