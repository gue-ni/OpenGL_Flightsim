#include "gfx.h"

#include "util.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../../lib/tiny_obj_loader.h"

namespace gfx
{

Geometry::Geometry(const std::vector<float>& vertices, const VertexLayout& layout)
{
  const int stride = static_cast<int>(layout);
  count = static_cast<int>(vertices.size()) / (stride);

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

Object3D::Type Camera::get_type() const { return Object3D::Type::CAMERA; }

glm::mat4 Camera::get_view_matrix() const { return glm::inverse(m_transform); }

glm::mat4 Camera::get_projection_matrix() const { return m_projection; }

void Camera::look_at(const glm::vec3& target) { set_transform(glm::inverse(glm::lookAt(m_position, target, m_up))); }

Object3D::Type Light::get_type() const { return Object3D::Type::LIGHT; }

glm::mat4 Light::light_space_matrix()
{
  float near_plane = 0.1f, far_plane = 10.0f, m = 10.0f;
  auto wp = get_world_position();
  glm::mat4 light_view = glm::lookAt(wp, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 light_projection = glm::ortho(-m, m, -m, m, near_plane, far_plane);
  return light_projection * light_view;
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

GeometryPtr make_cube_geometry(float size)
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

GeometryPtr make_plane_geometry(int x_elements, int y_elements, float size)
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

GeometryPtr make_quad_geometry()
{
  const std::vector<float> quad_vertices = {
      -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  // top left
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom left
      1.0f,  1.0f,  0.0f, 1.0f, 1.0f,  // top right

      1.0f,  1.0f,  0.0f, 1.0f, 1.0f,  // top right
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom left
      1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
  };

  return std::make_shared<Geometry>(quad_vertices, Geometry::POS_UV);
}

Billboard::Billboard(gl::TexturePtr sprite, glm::vec3 color)
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
  if (context.shadow_pass) return;

  auto camera = context.camera;

#if 1
  auto view = camera->get_view_matrix();
#else
  glm::mat4 tmp_transform;
  auto r = get_rotation();
  camera->rotate_by(glm::vec3(+r.x, 0, 0));
  // auto view = camera->get_view_matrix();
  auto view = glm::inverse(camera->get_transform());
  camera->rotate_by(glm::vec3(-r.x, 0, 0));
#endif

  glm::vec3 up = {view[0][1], view[1][1], view[2][1]};
  glm::vec3 right = {view[0][0], view[1][0], view[2][0]};

  shader.bind();
  shader.set_uniform("u_View", view);
  shader.set_uniform("u_Projection", camera->get_projection_matrix());
  shader.set_uniform("u_Texture_01", 5);
  shader.set_uniform("u_Color", color);
  shader.set_uniform("u_Position", get_world_position());
  shader.set_uniform("u_Scale", get_scale());
  shader.set_uniform("u_Right", right);
  shader.set_uniform("u_Up", up);
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
    : Mesh(make_cube_geometry(1.0f),
           std::make_shared<Material>("shaders/skybox", std::make_shared<gl::CubemapTexture>(faces)))
{
}

void ShaderCache::add_shader(const std::string& path)
{
  if (!m_cache.contains(path)) {
    m_cache.insert(std::make_pair(path, std::make_shared<gl::Shader>(path)));
  }
}

gl::ShaderPtr ShaderCache::get_shader(const std::string& path)
{
  if (!m_cache.contains(path)) {
    add_shader(path);
  }
  return m_cache.at(path);
}

RenderTarget::RenderTarget(int w, int h) : width(w), height(h)
{
#if 1
  // setup texture
  texture = std::make_shared<gl::Texture>();
  texture->bind();
  glTexImage2D(texture->target, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  texture->set_parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  texture->set_parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  texture->unbind();

  // setup depthmap
  depthbuffer.bind();
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  depthbuffer.unbind();

  framebuffer.bind();

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->target, texture->id(), 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthbuffer.id());

  if (!framebuffer.complete()) {
    std::cout << "Framebuffer is not complete!" << std::endl;
  }

  framebuffer.unbind();
#endif
}

ShadowMap::ShadowMap(int w, int h) : width(w), height(h)
{
  texture = std::make_shared<gl::Texture>();
  texture->bind();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(1.0f)));
  texture->unbind();

  framebuffer.bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->target, texture->id(), 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  if (!framebuffer.complete()) {
    std::cout << "ShadowMap framebuffer is not complete\n";
  }

  framebuffer.unbind();
}

void RenderTarget::bind() {}

void RenderTarget::unbind() {}

Mesh::Mesh(const GeometryPtr& geometry, const MaterialPtr& material) : m_material(material), m_geometry(geometry) {}

void Mesh::draw_self(RenderContext& context)
{
  if (!context.shadow_pass && context.shaders) {
    auto camera = context.camera;

    // get shader from cache
    std::string shader_name = m_material->get_shader_name();
    gl::ShaderPtr shader = context.shaders->get_shader(shader_name);

    shader->bind();

    // transform
    shader->set_uniform("u_Model", get_transform());

    // camera
    shader->set_uniform("u_View", camera->get_view_matrix());
    shader->set_uniform("u_Projection", camera->get_projection_matrix());
    shader->set_uniform("u_CameraPos", camera->get_world_position());

    // lights
    glm::vec3 light_dir = glm::vec3(0, 1, 0);
    glm::vec3 light_color = glm::vec3(1, 1, 1);
    shader->set_uniform("u_LightDir", light_dir);
    shader->set_uniform("u_LightColor", light_color);

    shader->set_uniform("u_UseTexture", true);

    // material texture
    m_material->get_texture()->bind(5);
    shader->set_uniform("u_Texture_01", 5);

    // environment map
    context.env_map->bind(6);
    shader->set_uniform("u_EnvironmentMap", 6);

    glm::vec3 rgb = glm::vec3(1.0f, 0.0f, 0.0f);
    shader->set_uniform("u_SolidObjectColor", rgb);

    // shader->set_uniform("ka", 0.5f);
    // shader->set_uniform("kd", 1.0f);
    // shader->set_uniform("ks", 0.4f);
    // shader->set_uniform("alpha", 20.0f);

    m_geometry->vao.bind();

    switch (m_geometry->draw_type) {
      case BaseGeometry::DRAW_ARRAYS:
        glDrawArrays(GL_TRIANGLES, 0, m_geometry->count);
        break;
      case BaseGeometry::DRAW_ELEMENTS:
        glDrawElements(GL_TRIANGLES, m_geometry->count, GL_UNSIGNED_INT, 0);
        break;
      default:
        assert(false);
        break;
    }
    m_geometry->vao.unbind();

    shader->unbind();
  }
}

Renderer::Renderer(GLsizei width, GLsizei height) : m_width(width), m_height(height)
{
  // init renderer
  std::cout << "========== OpenGL ==========\n";
  std::cout << glGetString(GL_VERSION) << std::endl;
  std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
  std::cout << glGetString(GL_VENDOR) << std::endl;
  std::cout << glGetString(GL_RENDERER) << std::endl;
  std::cout << "============================\n";

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
  glViewport(0, 0, m_width, m_height);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  // init skybox
#if 1
  const std::string skybox_path = "assets/textures/skybox/2/";
  const std::array<std::string, 6>& faces = {
      skybox_path + "right.png",  skybox_path + "left.png",  skybox_path + "top.png",
      skybox_path + "bottom.png", skybox_path + "front.png", skybox_path + "back.png",
  };
  m_fog_color = gfx::rgb(0x5e5e6e);
#else
  const std::string skybox_path = "assets/textures/skybox/1/";
  const std::array<std::string, 6>& faces = {
      skybox_path + "right.jpg",  skybox_path + "left.jpg",  skybox_path + "top.jpg",
      skybox_path + "bottom.jpg", skybox_path + "front.jpg", skybox_path + "back.jpg",
  };
  m_fog_color = gfx::rgb(222, 253, 255);
#endif

  m_skybox = std::make_shared<gfx::Skybox>(faces);
  m_skybox->set_scale(glm::vec3(10.0f));
  m_skybox->update_transform();

#if 1
  m_shadowmap = std::make_shared<ShadowMap>(2048, 2048);
#endif

#if 1
  m_rendertarget = std::make_shared<gfx::RenderTarget>(m_width, m_height);

  auto mat1 = std::make_shared<gfx::Material>("shaders/screen", m_rendertarget->texture);
  auto mat2 = std::make_shared<gfx::Material>("shaders/shadow_debug", m_shadowmap->texture);

  m_screenquad = std::make_shared<gfx::Mesh>(gfx::make_quad_geometry(), mat1);
#else
  m_rendertarget = m_screenquad = nullptr;
#endif
}

void ShadowMap::bind()
{
  glViewport(0, 0, width, height);
  framebuffer.bind();
  glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::unbind() { framebuffer.unbind(); }

void Renderer::render_shadows(RenderContext& context)
{
  // TODO
}

void Renderer::render_skybox(RenderContext& context)
{
  glDepthMask(GL_FALSE);
  m_skybox->draw(context);
  glDepthMask(GL_TRUE);
}

void Renderer::render_screenquad(RenderContext& context) { m_screenquad->draw(context); }

void Renderer::render(Camera* camera, Object3D* scene)
{
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // glEnable(GL_DEPTH_TEST);

  RenderContext context;
  context.camera = camera;
  context.shadow_pass = false;
  context.shaders = &m_shaders;
  context.env_map = m_skybox->get_material()->get_texture();
  context.depth_map = m_shadowmap->texture;

  glm::vec3 center;

  // find the light
  scene->traverse([&center](Object3D* obj) {
    Light* light = dynamic_cast<Light*>(obj);
    if (light != NULL) {
      center = light->get_world_position();
    }
    return true;
  });

  float m = 150.0f;
  float near_plane = 1.0f, far_plane = 5000.0f;
  glm::vec3 light_pos = center + glm::vec3(-2.0f, 18.0f, -1.0f);
  glm::mat4 light_proj = glm::ortho(-m, m, -m, m, near_plane, far_plane);
  glm::mat4 light_view = glm::lookAt(light_pos, center, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 light_space_matrix = light_proj * light_view;

  context.light_space_matrix = light_space_matrix;

  // depends on skybox
  context.fog_color = m_fog_color;

  // update transforms
  scene->update_transform();

  // render to shadowmap
#if 1
  {
    glViewport(0, 0, m_shadowmap->width, m_shadowmap->height);
    m_shadowmap->framebuffer.bind();
     glClear(GL_DEPTH_BUFFER_BIT);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene->traverse([&context](Object3D* obj) {
      Mesh* mesh = dynamic_cast<Mesh*>(obj);

      if (mesh != NULL) {
        gl::ShaderPtr shader = context.shaders->get_shader("shaders/depth");

        shader->bind();
        shader->set_uniform("u_Model", mesh->get_transform());
        shader->set_uniform("u_LightSpaceMatrix", context.light_space_matrix);

        auto geo = mesh->get_geometry();

        geo->vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, geo->count);
        geo->vao.unbind();

        shader->unbind();
      }

      return true;
    });

    m_shadowmap->framebuffer.unbind();
  }
#endif

  // render skybox

  if (m_rendertarget && m_screenquad) {
    glViewport(0, 0, m_rendertarget->width, m_rendertarget->height);
    m_rendertarget->framebuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_skybox(context);

    scene->draw(context);

    m_rendertarget->framebuffer.unbind();

    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_screenquad(context);

  } else {  // do not render to texture
    render_skybox(context);
    scene->draw(context);
  }
}

}  // namespace gfx
