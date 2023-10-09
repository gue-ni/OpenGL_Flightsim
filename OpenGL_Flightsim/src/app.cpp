#include "app.h"

#include <iostream>

#include "data.h"
#include "flightmodel.h"
#include "terrain.h"

#define DRAW_HUD 0

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
  m_hud_renderer = new gfx::Renderer(m_width, m_height);

  auto tex = gfx::gl::Texture::load("assets/textures/container.jpg", {});
  auto mat = std::make_shared<gfx::Material>("shaders/mesh", tex);

  m_scene = new gfx::Object3D();
#if 1
  m_hud = new gfx::Line2d();

#else
  m_hud = new gfx::Mesh(gfx::make_cube_geometry(1.0f), mat);
  m_hud->set_position({0, 0, -5});
  m_hud->set_rotation({phi::PI / 2, 0, 0});
#endif
  m_hud->update_transform();
  m_hud_target = new gfx::RenderTarget(m_width, m_height);

  auto mat1 = std::make_shared<gfx::Material>("shaders/screen", m_hud_target->texture);
  auto mat2 = std::make_shared<gfx::Material>("shaders/screen", tex);
  m_screen = new gfx::Mesh(gfx::make_quad_geometry(), mat1);
#if 1
  m_screen->set_scale(glm::vec3(15.0f));
  m_screen->set_position({15, 0, 0});
  m_screen->set_rotation({0, phi::PI / 2.0, 0});
#endif
  m_screen->cast_shadow = false;
  m_screen->disable_depth_test = true;

  // m_scene->add(m_screen);

  float aspect_ratio = (float)m_width / (float)m_height, near = 0.1f, far = 150000.0f;

  // orbit camera
  m_cameras[0] = new gfx::Camera(glm::radians(45.0f), aspect_ratio, near, far);
  m_scene->add(m_cameras[0]);

  // cockpit camera
  m_cameras[1] = new gfx::Camera(glm::radians(75.0f), aspect_ratio, 2.0f, far);

  // following camera
  m_cameras[2] = new gfx::Camera(glm::radians(45.0f), aspect_ratio, near, far);
  m_scene->add(m_cameras[2]);

  m_cameratype = 0;

#if 1
#endif
#if 1
  m_clipmap = new Clipmap();
  m_clipmap->visible = true;
  m_scene->add(m_clipmap);
#endif

#if 1
  m_terrain = new phi::RigidBody();
  m_terrain->active = false;
  m_terrain->mass = 10000.0f;
  m_terrain->collider = new Heightmap(m_clipmap);
  m_terrain->set_inertia(phi::inertia::sphere(m_terrain->mass, 1000.0f));
#endif

  float height = m_clipmap->get_terrain_height(glm::vec2(0));

#if 1
  // m_runway = gfx::Mesh::load("assets/models/falcon.obj", "assets/textures/falcon.jpg");
  m_runway = gfx::Mesh::load("assets/models/runway2.obj", "assets/textures/runway.jpg");
  m_runway->set_position(glm::vec3(0, height + 0.05f, 0));
  m_scene->add(m_runway);
#endif

  init_airplane();

  glm::vec3 look_forward = glm::vec3(0, glm::radians(-90.0f), 0.0f);

  // smooth follow camera
  m_camera_attachment = new gfx::Object3D();
  glm::vec3 offset(-20.0f, 4.5f, 0.0f);
  m_camera_attachment->set_position(offset);
  m_camera_attachment->set_rotation({0, glm::radians(-90.0f), 0.0f});
  m_falcon->add(m_camera_attachment);

  // cockpit camera
  m_cameras[1]->set_position({6.5f, 0.8f, 0.0f});
  m_cameras[1]->set_rotation(look_forward);
  m_falcon->add(m_cameras[1]);

#if 0
  m_airplane->position = glm::vec3(0, height + 10, 0);
  m_airplane->velocity = glm::vec3(0, 0, 0);
  m_airplane->rotation = glm::quat(glm::vec3(0.1, 0, 0.1));
  // m_airplane->set_speed_and_attitude(150, glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(35.0f)));
#else
  m_airplane->position = glm::vec3(-1500.0f, height + 100, 0);
  m_airplane->set_speed_and_attitude(phi::units::meter_per_second(350.0f), glm::radians(glm::vec3(0, 0, 0)));
#endif

  m_cameras[2]->set_transform(m_airplane->position - offset, glm::quat(look_forward));

  gfx::Light* light = new gfx::Light(glm::vec3(1.0f));
  light->transform_flags = OBJ3D_TRANSFORM;
  light->set_position(glm::vec3(2, 8, 2));
  m_falcon->add(light);

  m_falcon->add(m_screen);

  // setup all transforms
  m_scene->update_transform();
}

void App::init_airplane()
{
  const std::string obj = "assets/models/falcon.obj";
  const std::string glb = "assets/models/Falcon.glb";
  const std::string jpg = "assets/textures/falcon.jpg";

  const gfx::gl::Texture::Params params = {.flip_vertically = true, .texture_mag_filter = GL_LINEAR};
  const auto texture = gfx::gl::Texture::load(jpg, params);
  const auto geometry = gfx::Geometry::load(obj);
  const auto cube = gfx::make_cube_geometry(1.0f);
  const auto material = make_shared<gfx::Material>("shaders/mesh", texture);

  // m_falcon = new gfx::Mesh(tmp2, material);
  // m_falcon = gfx::Mesh::load_mesh(glb);
  // m_falcon = new gfx::Mesh(geometry, material);
  m_falcon = gfx::Mesh::load(obj, jpg);
  auto c = static_cast<gfx::Mesh*>(m_falcon->children[3]);
  c->get_material()->shininess = 1.0f;
  c->get_material()->opacity = 0.25f;
  m_falcon->receive_shadow = false;
  m_falcon->traverse([](gfx::Object3D* obj) {
    obj->receive_shadow = false;
    return true;
  });
  m_scene->add(m_falcon);

  const auto particles = new gfx::ParticleSystem({});
  m_falcon->add(particles);

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

  float wheelbase = 2.5f;
  LandingGear* collider = new LandingGear(glm::vec3(4.0f, -1.8f, 0.0f), glm::vec3(-1.0f, -1.8f, +wheelbase),
                                          glm::vec3(-1.0f, -1.8f, -wheelbase));

  m_airplane = new Airplane(mass, inertia, wings, {Engine(thrust)}, collider);

#if 1
  gfx::Object3D* landing_gear = new gfx::Object3D();
  m_falcon->add(landing_gear);

  auto wheel_texture = std::make_shared<gfx::gl::Texture>("assets/textures/container.jpg");
  auto wheel_material = std::make_shared<gfx::Material>("shaders/mesh", wheel_texture);
  auto wheel_geometry = gfx::Geometry::load("assets/models/wheel.obj");
  // auto wheel_geometry = gfx::make_cube_geometry(0.5f);

  for (auto wheel : collider->wheels()) {
    gfx::Object3D* obj = new gfx::Object3D();
    obj->set_position(wheel);
    gfx::Mesh* wheel_mesh = new gfx::Mesh(wheel_geometry, wheel_material);
    wheel_mesh->visible = true;
    obj->add(wheel_mesh);
    m_falcon->add(obj);
  }
#endif
}

void App::init_flightmodel() {}

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

  const uint8_t* state = SDL_GetKeyboardState(NULL);

  if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT]) {
  } else if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT]) {
  } else {
  }

  if (state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP]) {
  } else if (state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN]) {
  } else {
  }

  if (state[SDL_SCANCODE_B]) {
    m_breaks = true;
  } else {
    m_breaks = false;
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
  ImGui::Text("SPD:   %.0f km/h", phi::units::kilometer_per_hour(m_airplane->get_speed()));
  ImGui::Text("IAS:   %.0f km/h", phi::units::kilometer_per_hour(m_airplane->get_ias()));
  ImGui::Text("THR:   %.0f %%", std::abs(m_airplane->throttle * 100.0f));
  ImGui::Text("G:     %.1f", m_airplane->get_g());
  ImGui::Text("AoA:   %.2f", m_airplane->get_aoa());
  ImGui::Text("FPS:   %.1f", 1.0f / dt);
  ImGui::End();

  auto rotation1 = glm::degrees(m_airplane->get_attitude());
  auto rotation2 = glm::degrees(m_airplane->get_euler_angles());

  ImGui::SetNextWindowPos(ImVec2(180, 10));
  ImGui::SetNextWindowSize(ImVec2(280, 50));
  ImGui::SetNextWindowBgAlpha(0.35f);
  ImGui::Begin("Debug", nullptr, window_flags);
  ImGui::Text("bank: %.1f, yaw: %.1f, pitch: %.1f", rotation1.x, rotation1.y, rotation1.z);
  ImGui::Text("bank: %.1f, yaw: %.1f, pitch: %.1f", rotation2.x, rotation2.y, rotation2.z);
  ImGui::End();
}

void App::draw_hud()
{
  glm::vec3 v = glm::normalize(m_airplane->get_body_velocity());

  if (m_airplane->get_speed() < 10.0f) {
    v = glm::vec3(0);
  }

  float s, r;

  // velocity vector
  glm::vec2 o = glm::vec2(v.z, v.y);

  // flightpath marker
#if 1
  s = 0.03f;
  r = 0.01f;
  m_hud->batch_line({{o.x, o.y + r}, {o.x, o.y + s}});
  m_hud->batch_line({{o.x - s, o.y}, {o.x - r, o.y}});
  m_hud->batch_line({{o.x + s, o.y}, {o.x + r, o.y}});
  m_hud->batch_circle(o, r);
#endif

  // forward
  s = 0.02f;
  r = 0.01f;
  m_hud->batch_line({{-s, 0}, {-r, 0}});
  m_hud->batch_line({{+s, 0}, {+r, 0}});
  m_hud->batch_line({{0, +r}, {0, +s}});
  m_hud->batch_line({{0, -r}, {0, -s}});

  auto rotation = m_airplane->get_attitude();
  float roll = rotation.x, yaw = rotation.y, pitch = rotation.z;

  float w1 = 0.20f;
  float w2 = 0.05f;
  float h = 0.25f;
  float scale = -2.4f;
  float pitch_offset = -pitch * scale;

  float side_offset = v.y * std::sin(glm::radians(90.0f) - roll);
  // std::cout << side_offset << std::endl;
  // float side_offset = 0.0f;

  glm::mat4 mat(1.0f);
  mat = glm::rotate(mat, -roll, glm::vec3(0, 0, 1));
  mat = glm::translate(mat, glm::vec3(0, pitch_offset, 0));

#if 1
  // pitch ladder
  int n = 15;
  for (int i = -n; i < n; i++) {
    float offset = h * i;

    m_hud->batch_line({glm::vec2{-w1, offset}, glm::vec2{-w2, offset}}, mat);
    m_hud->batch_line({glm::vec2{-w1, offset}, glm::vec2{-w1, offset + 0.03f * glm::sign(i)}}, mat);

    m_hud->batch_line({glm::vec2{+w2, offset}, glm::vec2{+w1, offset}}, mat);
    m_hud->batch_line({glm::vec2{+w1, offset}, glm::vec2{+w1, offset + 0.03f * glm::sign(i)}}, mat);

    // m_hud->batch_line({glm::vec2{0, offset + 0.1f}, glm::vec2{0, offset - 0.1f}}, mat);
  }
#else
  // m_hud->batch_line({glm::vec2{o.x - w1, pitch_offset}, glm::vec2{o.x + w1, pitch_offset}}, mat);
#endif
}

void App::game_loop(float dt)
{
  if (!m_paused) {
    m_airplane->update(dt);

#if 1
    phi::Collision collision;
    if (test_collision(m_airplane, m_terrain, &collision)) {
#if 0
      if (!m_breaks)
        collision.kinetic_friction_coeff = 0;
#endif
      phi::RigidBody::impulse_collision(collision);
    }
#endif

    // airplane model
    m_falcon->set_transform(m_airplane->position, m_airplane->rotation);

#if 0
    // control surfaces
    glm::vec3 r;
    r = m_falcon->children[2]->get_rotation();
   m_falcon->children[2]->set_rotation(glm::vec3(r.x, r.y, phi::PI + m_airplane->joystick.z * 0.1f));

    r = m_falcon->children[6]->get_rotation();
    //m_falcon->children[6]->set_rotation(glm::vec3(r.x, r.y, -m_airplane->joystick.x * 0.5f));
    //m_falcon->children[5]->set_rotation(glm::vec3(r.x, r.y, +m_airplane->joystick.x * 0.5f));
#endif

    // smooth following camera
    const float speed = 15.0f * dt;
    m_cameras[2]->set_transform(
        glm::mix(m_cameras[2]->get_world_position(), m_camera_attachment->get_world_position(), speed),
        glm::mix(m_cameras[2]->get_world_rotation_quat(), m_camera_attachment->get_world_rotation_quat(), speed));

#if DRAW_HUD
    draw_hud();
    gfx::Camera c(glm::radians(45.0f), 1.0, 0.1, 1000);
    m_hud_renderer->render(&c, m_hud, m_hud_target);
#endif

    m_screen->visible = (m_cameratype != 0);

    m_hud->batch_clear();
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
