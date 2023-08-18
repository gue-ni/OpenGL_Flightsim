#include "app.h"

#include <iostream>

#include "data.h"
#include "flightmodel.h"
#include "terrain.h"

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

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplSDL2_InitForOpenGL(m_window, m_context);
  ImGui_ImplOpenGL3_Init();

  SDL_ShowCursor(SDL_FALSE);
  SDL_CaptureMouse(SDL_TRUE);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  if (SDL_NumJoysticks() > 0) {
    SDL_JoystickEventState(SDL_ENABLE);
    m_sdljoystick = SDL_JoystickOpen(0);
  }

  init();
}

App::~App()
{
  destroy();
  SDL_GL_DeleteContext(m_context);
  SDL_DestroyWindow(m_window);
  SDL_Quit();
}

void App::init()
{
  m_renderer = new gfx::Renderer(m_width, m_height);

  m_scene = new gfx::Object3D();

  constexpr float fov = glm::radians(45.0f);
  float aspect_ratio = (float)m_width / (float)m_height;
  float near = 0.1f, far = 150000.0f;

  m_cameras[0] = new gfx::Camera(fov, aspect_ratio, near, far);
  m_scene->add(m_cameras[0]);

  m_cameras[1] = new gfx::Camera(fov, aspect_ratio, near, far);

  m_cameras[2] = new gfx::Camera(fov, aspect_ratio, near, far);
  m_scene->add(m_cameras[2]);

#if 1
#endif
#if 1
  m_clipmap = new Clipmap();
  m_scene->add(m_clipmap);
#endif

#if 1
  m_terrain = new phi::RigidBody();
  m_terrain->active = false;
  m_terrain->mass = 10000.0f;
  m_terrain->collider = new Heightmap(m_clipmap);
  m_terrain->set_inertia(phi::inertia::sphere(m_terrain->mass, 1000.0f));
#endif

  init_airplane();

  glm::vec3 look_forward = glm::vec3(0, glm::radians(-90.0f), 0.0f);

  // smooth follow camera
  m_camera_attachment = new gfx::Object3D();
  glm::vec3 offset(-20.0f, 4.5f, 0.0f);
  m_camera_attachment->set_position(offset);
  m_camera_attachment->set_rotation({0, glm::radians(-90.0f), 0.0f});
  m_falcon->add(m_camera_attachment);

  // attached camera
  m_cameras[1]->set_position({-10.0f, 1.5f, 3.0f});
  m_cameras[1]->set_rotation(look_forward);
  m_falcon->add(m_cameras[1]);

  float height = m_clipmap->get_terrain_height(glm::vec2(0));
  m_airplane->position = glm::vec3(0, height + 3500.0f, 0);
  m_airplane->velocity = glm::vec3(150, 0, 0);
  m_airplane->rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));

  m_cameras[2]->set_transform(m_airplane->position - offset, glm::quat(look_forward));

  gfx::Light* light = new gfx::Light(glm::vec3(1.0f));
  light->transform_flags = OBJ3D_TRANSFORM;
  light->set_position(glm::vec3(2, 8, 2));
  m_falcon->add(light);

  // setup all transforms
  m_scene->update_transform();
}

void App::init_airplane()
{
  const std::string obj = "assets/models/falcon.obj";
  const std::string jpg = "assets/textures/falcon.jpg";

  const gfx::gl::Texture::Params params = {.flip_vertically = true, .texture_mag_filter = GL_LINEAR};
  const auto texture = gfx::gl::Texture::load(jpg, params);
  const auto geometry = gfx::Geometry::load(obj);
  const auto material = make_shared<gfx::Material>("shaders/mesh", texture);

  // m_falcon = new gfx::Mesh(geometry, material);
  m_falcon = gfx::Mesh::load(obj);
  m_scene->add(m_falcon);

  // m_falcon->children[4]->set_rotation(glm::vec3(0.0f, 0.0f, 0.5f));

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
      Wing({wing_offset, 0.0f, -2.7f}, 6.96f, 2.50f, &NACA_2412, phi::UP, 0.10f),    // left wing
      Wing({wing_offset, 0.0f, +2.7f}, 6.96f, 2.50f, &NACA_2412, phi::UP, 0.10f),    // right wing
      Wing({tail_offset, -0.1f, 0.0f}, 6.54f, 2.70f, &NACA_0012, phi::UP, 1.0f),     // elevator
      Wing({tail_offset, 0.0f, 0.0f}, 5.31f, 3.10f, &NACA_0012, phi::RIGHT, 0.15f),  // rudder
  };

  Engine* engine = new SimpleEngine(thrust);

  LandingGear* collider =
      new LandingGear(glm::vec3(4.0f, -1.8f, 0.0f), glm::vec3(-1.0f, -1.8f, +2.0f), glm::vec3(-1.0f, -1.8f, -2.0f));

  m_airplane = new Airplane(mass, inertia, wings, {engine}, collider);

#if 0
  gfx::Object3D* landing_gear = new gfx::Object3D();
  m_falcon->add(landing_gear);

  auto wheel_texture = std::make_shared<gfx::gl::Texture>("assets/textures/container.jpg");
  auto wheel_material = std::make_shared<gfx::Material>("shaders/mesh", wheel_texture);
  auto wheel_geometry = gfx::make_cube_geometry(0.5f);

  for (auto wheel : collider->wheels()) {
    gfx::Mesh* wheel_mesh = new gfx::Mesh(wheel_geometry, wheel_material);
    wheel_mesh->set_position(wheel);
    landing_gear->add(wheel_mesh);
  }
#endif
}

void App::destroy()
{
  delete m_cameras[0];
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
        event_mousemotion(event.motion.xrel, event.motion.yrel);
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

void App::event_mousewheel(float value) { m_controller.radius *= (1.0 + glm::sign(value) * 0.1f); }

void App::event_keydown(SDL_Keycode key)
{
  switch (key) {
    case SDLK_ESCAPE:
      m_quit = true;
      break;
    case SDLK_o:
      m_cameratype = (m_cameratype + 1) % m_cameras.size();
      break;
    case SDLK_i:
      m_clipmap->wireframe = !m_clipmap->wireframe;
      break;
    case SDLK_p:
      m_paused = !m_paused;
      break;
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
  return static_cast<float>(now - last) / frequency;
}

void App::draw_imgui(float dt)
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  window_flags |= ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoResize;

  ImGui::SetNextWindowPos(ImVec2(10, 10));
  ImGui::SetNextWindowSize(ImVec2(145, 135));
  ImGui::SetNextWindowBgAlpha(0.35f);
  ImGui::Begin("Flightsim", nullptr, window_flags);
  ImGui::Text("ALT:   %.0f m", m_airplane->position.y);
  ImGui::Text("SPD:   %.0f km/h", m_airplane->get_speed());
  ImGui::Text("IAS:   %.0f km/h", m_airplane->get_ias());
  ImGui::Text("THR:   %.0f %%", std::abs(m_airplane->throttle * 100.0f));
  ImGui::Text("G:     %.1f", m_airplane->get_g());
  ImGui::Text("AoA:   %.2f", m_airplane->get_aoa());
  ImGui::Text("FPS:   %.1f", 1.0f / dt);
  ImGui::End();
}

void App::game_loop(float dt)
{
  if (!m_paused) {
    m_airplane->update(dt);

#if 1
    phi::CollisionInfo collision;
    if (test_collision(m_airplane, m_terrain, &collision)) {
      phi::RigidBody::impulse_collision(collision);
    }
#endif

    // airplane model
    m_falcon->set_transform(m_airplane->position, m_airplane->rotation);

    // control surfaces
    m_falcon->children[2]->set_rotation(glm::vec3(0.0f, 0.0f, m_airplane->joystick.z));
    m_falcon->children[6]->set_rotation(glm::vec3(0.0f, 0.0f, -m_airplane->joystick.x * 0.1f));
    m_falcon->children[5]->set_rotation(glm::vec3(0.0f, 0.0f, +m_airplane->joystick.x * 0.1f));

    // smooth following camera
    const float speed = 15.0f * dt;
    m_cameras[2]->set_transform(
        glm::mix(m_cameras[2]->get_world_position(), m_camera_attachment->get_world_position(), speed),
        glm::mix(m_cameras[2]->get_world_rotation_quat(), m_camera_attachment->get_world_rotation_quat(), speed));
  }

  m_controller.update(*m_cameras[0], m_falcon->get_position(), dt);

  m_renderer->render(m_cameras[m_cameratype], m_scene);
}

int App::run()
{
  now = SDL_GetPerformanceCounter();

  while (!m_quit) {
    float dt = delta_time();
    m_frames++;
    m_seconds += dt;

    draw_imgui(dt);

    poll_events();

    game_loop(dt);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(m_window);
  }

  return 0;
}
