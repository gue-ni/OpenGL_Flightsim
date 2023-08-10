#include "app.h"

#include <iostream>

#include "data.h"
#include "flightmodel.h"

const Airfoil NACA_0012(NACA_0012_data);
const Airfoil NACA_2412(NACA_2412_data);
const Airfoil NACA_64_206(NACA_64_206_data);

inline static float scale(int16_t value)
{
  constexpr int16_t max_value = std::numeric_limits<int16_t>::max();
  return static_cast<float>(value) / static_cast<float>(max_value);
}

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

  SDL_ShowCursor(SDL_FALSE);
  SDL_CaptureMouse(SDL_TRUE);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  if (SDL_NumJoysticks()) {
    std::cout << "found joystick\n";
    SDL_JoystickEventState(SDL_ENABLE);
    m_sdljoystick = SDL_JoystickOpen(0);
  }

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

#if 1
  const std::string skybox_path = "assets/textures/skybox/1/";
  const std::array<std::string, 6>& faces = {
      skybox_path + "right.jpg",  skybox_path + "left.jpg",  skybox_path + "top.jpg",
      skybox_path + "bottom.jpg", skybox_path + "front.jpg", skybox_path + "back.jpg",
  };

  gfx::Skybox* skybox = new gfx::Skybox(faces);
  skybox->set_scale(glm::vec3(3.0f));
  m_scene->add(skybox);
#endif
#if 1
  Clipmap* clipmap = new Clipmap();
  m_scene->add(clipmap);
#endif

#if 0
  m_terrain = new phi::RigidBody();
  m_terrain->active = false;
  m_terrain->mass = 10000.0f;
  m_terrain->set_inertia(phi::inertia::sphere(m_terrain->mass, 1000.0f));
  m_terrain->collider = new Heightmap(clipmap);
#endif

  // objects
  {
    gfx::gl::Texture::Params params = {.flip_vertically = true, .texture_mag_filter = GL_LINEAR};
    auto falcon_tex = std::make_shared<gfx::gl::Texture>("assets/textures/falcon.jpg", params);
    auto falcon_geo =
        std::make_shared<gfx::Geometry>(gfx::load_obj("assets/models/falcon.obj"), gfx::Geometry::POS_NORM_UV);
    gfx::MaterialPtr falcon_material = make_shared<gfx::Material>("shaders/pbr", falcon_tex);
    m_falcon = new gfx::Mesh(falcon_geo, falcon_material);
    m_scene->add(m_falcon);
  }

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

  m_airplane = new Airplane(mass, inertia, wings, {engine}, &landing_gear);
  m_airplane->position = glm::vec3(0, 800, 0);
  m_airplane->velocity = glm::vec3(300, 0, 0);
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
      case SDL_MOUSEMOTION: {
        event_mousemotion(static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel));
        break;
      }
      case SDL_KEYDOWN: {
        event_keydown(event.key.keysym.sym);
        break;
      }
      case SDL_JOYAXISMOTION: {
        event_joyaxis(event.jaxis.axis, event.jaxis.value);
        break;
      }
      case SDL_MOUSEWHEEL: {
        event_mousewheel(event.wheel.y);
        break;
      }
      default:
        break;
    }
  }
}

void App::event_mousewheel(int32_t value) { m_controller.radius *= (1.0 + glm::sign(value) * 0.1f); }

void App::event_keydown(SDL_Keycode key)
{
  switch (key) {
    case SDLK_ESCAPE: {
      m_quit = true;
      break;
    }
  }
}

void App::event_mousemotion(float xrel, float yrel)
{
  m_controller.move_mouse(static_cast<float>(xrel), static_cast<float>(yrel));
}

void App::event_joyaxis(uint8_t axis, int16_t value)
{
  switch (axis) {
    case 0:
      m_airplane->joystick.x = std::pow(scale(value), 3.0f);
      break;
    case 1:
      m_airplane->joystick.z = std::pow(scale(value), 3.0f);
      break;
    case 2:
      m_airplane->throttle = (scale(value) + 1.0f) / 2.0f;
      break;
    case 4:
      m_airplane->joystick.y = std::pow(scale(value), 3.0f);
      break;
    default:
      break;
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

    m_airplane->update(dt);

#if 0
    phi::CollisionInfo collision;
   if (test_collision(m_airplane, m_terrain, &collision)) {
        phi::RigidBody::impulse_collision(collision);
      }
#endif

    m_falcon->set_transform(m_airplane->position, m_airplane->rotation);

    m_controller.update(*m_camera, m_falcon->get_position(), dt);

    // render scene
    m_renderer->render(*m_camera, *m_scene);

    SDL_GL_SwapWindow(m_window);
  }
}
