#include "gfx.h"

#include "util.h"

#if 0
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#endif

namespace gfx
{

glm::mat4 Camera::get_view_matrix() const { return glm::inverse(m_transform); }

glm::mat4 Camera::get_projection_matrix() const { return m_projection; }

void Camera::look_at(const glm::vec3& target) { set_transform(glm::inverse(glm::lookAt(m_position, target, m_up))); }

glm::mat4 Light::light_space_matrix()
{
  float near_plane = 0.1f, far_plane = 10.0f, m = 10.0f;
  auto wp = get_world_position();
  glm::mat4 light_view = glm::lookAt(wp, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 light_projection = glm::ortho(-m, m, -m, m, near_plane, far_plane);
  return light_projection * light_view;
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

RenderTarget::RenderTarget(int w, int h) : RenderTargetBase(w, h)
{
#if 1
  // setup texture
  texture = std::make_shared<gl::Texture>();
  texture->bind();
  glTexImage2D(texture->target, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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

ShadowMap::ShadowMap(int w, int h) : RenderTargetBase(w, h)
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

Renderer::Renderer(GLsizei width, GLsizei height) : m_width(width), m_height(height)
{
#if 0
  // init renderer
  std::cout << "========== OpenGL ==========\n";
  std::cout << glGetString(GL_VERSION) << std::endl;
  std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
  std::cout << glGetString(GL_VENDOR) << std::endl;
  std::cout << glGetString(GL_RENDERER) << std::endl;
  std::cout << "============================\n";
#endif

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
  glViewport(0, 0, m_width, m_height);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  bool shadowmap = true;

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

  if (shadowmap) {
    m_shadowmap = std::make_shared<ShadowMap>(2048, 2048);
  }

#if 1
  m_rendertarget = std::make_shared<gfx::RenderTarget>(m_width, m_height);

  auto mat1 = std::make_shared<gfx::Material>("shaders/screen", m_rendertarget->texture);
  // auto mat2 = std::make_shared<gfx::Material>("shaders/shadow_debug", m_shadowmap->texture);

  m_screenquad = std::make_shared<gfx::Mesh>(gfx::Geometry::quad(), mat1);
#else
  m_rendertarget = m_screenquad = nullptr;
#endif
}

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
  glm::mat4 light_space_matrix;

  // find the light
  scene->traverse([&center](Object3D* obj) {
    Light* light = dynamic_cast<Light*>(obj);
    if (light != NULL) {
      center = light->get_world_position();
    }
    return true;
  });

  float m = 50.0f;
  float near_plane = 1.0f, far_plane = 5000.0f;
  glm::vec3 light_pos = center + glm::vec3(-2.0f, 18.0f, -1.0f);
  glm::mat4 light_proj = glm::ortho(-m, m, -m, m, near_plane, far_plane);
  glm::mat4 light_view = glm::lookAt(light_pos, center, glm::vec3(0.0f, 1.0f, 0.0f));
  light_space_matrix = light_proj * light_view;

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
    // glClear(GL_DEPTH_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene->traverse([&context](Object3D* obj) {
      Mesh* mesh = dynamic_cast<Mesh*>(obj);

      if (mesh != NULL && mesh->cast_shadow == true) {
        gl::ShaderPtr shader = context.shaders->get_shader("shaders/mesh");
        shader->bind();
        shader->set_uniform("u_Model", mesh->get_transform());
        shader->set_uniform("u_ShadowPass", true);
        shader->set_uniform("u_LightSpaceMatrix", context.light_space_matrix);

        auto geo = mesh->get_geometry();

        // TODO: also handle indexed geometry
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

void Renderer::render(Camera* camera, Object3D* scene, RenderTarget* target)
{
  RenderContext context;
  context.camera = camera;
  context.shadow_pass = false;
  context.shaders = &m_shaders;
  context.env_map = m_skybox->get_material()->get_texture();
  context.depth_map = m_shadowmap->texture;

  glViewport(0, 0, target->width, target->height);
  target->framebuffer.bind();
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  scene->draw(context);
  target->framebuffer.unbind();
}

}  // namespace gfx
