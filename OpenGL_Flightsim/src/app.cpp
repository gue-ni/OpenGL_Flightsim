#include "app.h"

#include <iostream>

App::App(int w, int h, const std::string& name) : m_width(w), m_height(h)
{
  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  m_window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL);
  m_context = SDL_GL_CreateContext(m_window);
  glewExperimental = GL_TRUE;

  glewInit();

  init_app();
}

App::~App()
{
  SDL_GL_DeleteContext(m_context);
  SDL_DestroyWindow(m_window);
  SDL_Quit();
  destroy_app();
}

void App::init_app()
{
  m_renderer = new gfx::Renderer2(m_width, m_height);

  m_camera = new gfx::Camera(glm::radians(45.0f), (float)m_width / (float)m_height, 1.0f, 150000.0f);

  m_scene = new gfx::Object3D();

  m_scene->add(m_camera);

  m_camera->set_position(glm::vec3(0, 0, 10));

  gfx::gl::Texture::Params params = {.flip_vertically = true, .texture_mag_filter = GL_LINEAR};
  auto falcon_tex = std::make_shared<gfx::gl::Texture>("assets/textures/falcon.jpg", params);
  auto falcon_geo =
      std::make_shared<gfx::Geometry>(gfx::load_obj("assets/models/falcon.obj"), gfx::Geometry::POS_NORM_UV);

  gfx::MaterialPtr falcon_material = make_shared<gfx::Material>("shaders/pbr", falcon_tex);

  auto box = new gfx::Mesh(falcon_geo, falcon_material);
  m_scene->add(box);
}

void App::destroy_app()
{
  delete m_camera;
  delete m_renderer;
  delete m_scene;
}

void App::poll_events()
{
  SDL_Event event;
  while (SDL_PollEvent(&event) != 0) {
    switch (event.type) {
      case SDL_QUIT: {
        m_quit = true;
        break;
      }

      case SDL_KEYDOWN: {
        handle_keydown(event.key.keysym.sym);
        break;
      }

      default:
        break;
    }
  }
}

void App::handle_keydown(SDL_Keycode key)
{
  switch (key) {
    case SDLK_ESCAPE: {
      m_quit = true;
      break;
    }
  }
}

float App::delta_time()
{
  last = now;
  now = SDL_GetPerformanceCounter();
  float frequency = static_cast<float>(SDL_GetPerformanceFrequency());
  return static_cast<float>((now - last) / frequency);
}

void App::execute()
{
  now = SDL_GetPerformanceCounter();

  while (m_seconds < 10.0f && !m_quit) {
    const float dt = delta_time();
    m_frames++;
    m_seconds += dt;

    // get input events
    poll_events();

    // render scene
    m_renderer->render(*m_camera, *m_scene);

    SDL_GL_SwapWindow(m_window);
  }
}
