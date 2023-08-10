#include "app.h"

#include <iostream>

#include "flightmodel.h"
#include "collider.h"
#include "data.h"

  const Airfoil NACA_0012(NACA_0012_data);
  const Airfoil NACA_2412(NACA_2412_data);
  const Airfoil NACA_64_206(NACA_64_206_data);


App::App(int w, int h, const std::string& name) : m_width(w), m_height(h), m_controller(30.0f)
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

  m_scene = new gfx::Object3D();

  m_camera = new gfx::Camera(glm::radians(45.0f), (float)m_width / (float)m_height, 1.0f, 150000.0f);


  m_camera->set_position(glm::vec3(0, 0, 20));
  m_scene->add(m_camera);



  // objects
  gfx::gl::Texture::Params params = {.flip_vertically = true, .texture_mag_filter = GL_LINEAR};
  auto falcon_tex = std::make_shared<gfx::gl::Texture>("assets/textures/falcon.jpg", params);
  auto falcon_geo =
      std::make_shared<gfx::Geometry>(gfx::load_obj("assets/models/falcon.obj"), gfx::Geometry::POS_NORM_UV);

  gfx::MaterialPtr falcon_material = make_shared<gfx::Material>("shaders/pbr", falcon_tex);

  m_falcon = new gfx::Mesh(falcon_geo, falcon_material);
  m_scene->add(m_falcon);

  //
  const float mass = 10000.0f;
  const float thrust = 75000.0f;

  const float wing_offset = -1.0f;
  const float tail_offset = -6.6f;

  std::vector<phi::inertia::Element> masses = {
      phi::inertia::cube({wing_offset, 0.0f, -2.7f}, {6.96f, 0.10f, 3.50f}, mass * 0.25f),  // left wing
      phi::inertia::cube({wing_offset, 0.0f, +2.7f}, {6.96f, 0.10f, 3.50f}, mass * 0.25f),  // right wing
      phi::inertia::cube({tail_offset, -0.1f, 0.0f}, {6.54f, 0.10f, 2.70f}, mass * 0.1f),   // elevator
      phi::inertia::cube({tail_offset, 0.0f, 0.0f}, {5.31f, 3.10f, 0.10f}, mass * 0.1f),    // rudder
      phi::inertia::cube({0.0f, 0.0f, 0.0f}, {8.0f, 2.0f, 2.0f}, mass * 0.5f),              // fuselage
  };

  const auto inertia = phi::inertia::tensor(masses, true);


  std::array<Wing, 4> wings = {
      Wing({wing_offset, 0.0f, -2.7f}, 6.96f, 2.50f, &NACA_2412, phi::UP, 0.20f),    // left wing
      Wing({wing_offset, 0.0f, +2.7f}, 6.96f, 2.50f, &NACA_2412, phi::UP, 0.20f),    // right wing
      Wing({tail_offset, -0.1f, 0.0f}, 6.54f, 2.70f, &NACA_0012, phi::UP, 1.0f),     // elevator
      Wing({tail_offset, 0.0f, 0.0f}, 5.31f, 3.10f, &NACA_0012, phi::RIGHT, 0.15f),  // rudder
  };

  Engine* engine = new SimpleEngine(thrust);

  Sphere sphere(15.0f);
  LandingGear landing_gear(glm::vec3(4.0f, -1.5f, 0.0f), glm::vec3(-1.0f, -1.5f, +2.0f),
                           glm::vec3(-1.0f, -1.5f, -2.0f));

  m_rigidbody = new Airplane(mass, inertia, wings, {engine}, &landing_gear);
  m_rigidbody->position = glm::vec3(0,1000,0);
  m_rigidbody->velocity = glm::vec3(200,0,0);

  //m_camera->set_position({-25.0f, 5, 0});
  //m_camera->set_rotation({0, glm::radians(-90.0f), 0.0f});



  //m_scene->add(m_camera);

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

  while (!m_quit) {
    const float dt = delta_time();
    m_frames++;
    m_seconds += dt;

    // get input events
    poll_events();

    //m_rigidbody->update(dt);
    //m_falcon->set_transform(m_rigidbody->position, m_rigidbody->rotation);


     //m_controller.update(*m_camera, m_rigidbody->position, dt);

    // render scene
    m_renderer->render(*m_camera, *m_scene);

    SDL_GL_SwapWindow(m_window);
  }
}
