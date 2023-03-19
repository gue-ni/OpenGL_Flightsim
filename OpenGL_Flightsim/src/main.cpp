#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "../lib/imgui/imgui.h"
#include "../lib/imgui/imgui_impl_opengl3.h"
#include "../lib/imgui/imgui_impl_sdl2.h"
#include "ai.h"
#include "clipmap.h"
#include "collisions.h"
#include "flightmodel.h"
#include "gfx.h"
#include "phi.h"
#include "pid.h"

using std::cout;
using std::endl;
using std::make_shared;
using std::shared_ptr;

std::string USAGE = R"(
Usage: 

P       pause game
O       toggle camera
I       toggle wireframe terrain
WASD    control pitch and roll
EQ      control yaw
JK      control thrust
)";

#define CLIPMAP 1
#define SKYBOX 1
#define SMOOTH_CAMERA 1
#define NPC_AIRCRAFT 1

#if 0
constexpr glm::ivec2 RESOLUTION{640, 480};
#else
constexpr glm::ivec2 RESOLUTION{1024, 728};
#endif

struct Joystick {
  int num_axis{0}, num_hats{0}, num_buttons{0};
  float aileron{0.0f}, elevator{0.0f}, rudder{0.0f}, throttle{0.0f}, trim{0.0f};

  // scale from int16 to -1.0, 1.0
  inline static float scale(int16_t value) { return static_cast<float>(value) / static_cast<float>(32767); }
};

struct GameObject {
  gfx::Mesh transform;
  Airplane airplane;
  col::Sphere collider;

  void update(float dt)
  {
    airplane.update(dt);
    transform.set_transform(airplane.rigid_body.position, airplane.rigid_body.orientation);
    collider.center = airplane.rigid_body.position;
  }
};

void get_keyboard_state(Joystick& joystick, phi::Seconds dt);
void solve_constraints(phi::RigidBody& rigid_body);
void apply_to_object3d(const phi::RigidBody& rigid_body, gfx::Object3D& object);

int main(void)
{
#if RUN_COLLISION_UNITTESTS
  collisions::run_unit_tests();
#endif

  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_Window* window = SDL_CreateWindow("Flightsim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, RESOLUTION.x,
                                        RESOLUTION.y, SDL_WINDOW_OPENGL);

  SDL_GLContext context = SDL_GL_CreateContext(window);
  glewExperimental = GL_TRUE;

  if (GLEW_OK != glewInit()) return -1;

  std::cout << glGetString(GL_VERSION) << std::endl;
  std::cout << USAGE << std::endl;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  glViewport(0, 0, RESOLUTION.x, RESOLUTION.y);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  // SDL options
  SDL_ShowCursor(SDL_FALSE);
  SDL_CaptureMouse(SDL_TRUE);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  ImGui_ImplSDL2_InitForOpenGL(window, context);
  ImGui_ImplOpenGL3_Init();

  Joystick joystick;

  PID pitch_control_pid(1.0f, 0.0f, 0.0f);

  int num_joysticks = SDL_NumJoysticks();
  bool joystick_control = num_joysticks > 0;
  if (joystick_control) {
    std::cout << "found " << num_joysticks << " joysticks\n";
    SDL_JoystickEventState(SDL_ENABLE);
    SDL_Joystick* sdl_joystick = SDL_JoystickOpen(0);
    joystick.num_axis = SDL_JoystickNumAxes(sdl_joystick);
    joystick.num_hats = SDL_JoystickNumHats(sdl_joystick);
    joystick.num_buttons = SDL_JoystickNumButtons(sdl_joystick);
    printf("found %d buttons, %d axis\n", joystick.num_buttons, joystick.num_axis);
  }

  gfx::Renderer renderer(RESOLUTION.x, RESOLUTION.y);

  gfx::gl::TextureParams params = {.flip_vertically = true};
  auto tex = make_shared<gfx::gl::Texture>("assets/textures/f16_large.jpg", params);
  auto texture = make_shared<gfx::Phong>(tex);
  auto obj = gfx::load_obj("assets/models/cessna/fuselage.obj");
  auto model = std::make_shared<gfx::Geometry>(obj, gfx::Geometry::POS_NORM_UV);

  gfx::Object3D scene;

  gfx::Light sun(gfx::Light::DIRECTIONAL, glm::vec3(1.0f));
  sun.set_position(glm::vec3(-2.0f, 4.0f, -1.0f));
  sun.cast_shadow = false;
  scene.add(&sun);

#if SKYBOX
  const std::string skybox_path = "assets/textures/skybox/1/";
  gfx::Skybox skybox({
      skybox_path + "right.jpg",
      skybox_path + "left.jpg",
      skybox_path + "top.jpg",
      skybox_path + "bottom.jpg",
      skybox_path + "front.jpg",
      skybox_path + "back.jpg",
  });
  skybox.set_scale(glm::vec3(3.0f));
  scene.add(&skybox);
#endif

#if CLIPMAP
  Clipmap clipmap;
  scene.add(&clipmap);
#endif
  constexpr float speed = phi::units::meter_per_second(200.0f);
  const float altitude = 3000.0f;

  const float mass = 1000.0f;

  const float rpm = 2400.0f;
  const float horsepower = 160.0f;
  const float prop_diameter = 1.9f;

  const float main_wing_span = 11.00f;
  const float main_wing_area = 16.17f;
  const float main_wing_chord = main_wing_area / main_wing_span;
  const float aileron_area = 1.70f;
  const float h_tail_area = 2.0f;
  const float h_tail_span = 2.0f;
  const float elevator_area = 1.35f;
  const float v_tail_area = 5.04f;  // modified
  const float v_tail_span = 2.04f;

  const float wing_offset = -0.2f;
  const float tail_offset = -4.6f;

  std::vector<phi::inertia::Element> masses = {
      phi::inertia::cube({wing_offset, 0.5f, -2.7f}, {main_wing_chord, 0.10f, main_wing_span / 2},
                         mass * 0.25f),  // left wing
      phi::inertia::cube({wing_offset, 0.5f, +2.7f}, {main_wing_chord, 0.10f, main_wing_area / 2},
                         mass * 0.25f),  // right wing
      phi::inertia::cube({tail_offset, -0.1f, 0.0f}, {h_tail_area / h_tail_span, 0.10f, h_tail_span},
                         mass * 0.1f),  // elevator
      phi::inertia::cube({tail_offset, 0.0f, 0.0f}, {v_tail_area / v_tail_span, v_tail_span, 0.10f},
                         mass * 0.1f),                                          // rudder
      phi::inertia::cube({0.0f, 0.0f, 0.0f}, {8.0f, 2.0f, 1.0f}, mass * 0.5f),  // fuselage
  };

  // assume uniform density
  float total_volume = 0.0f;
  for (const auto& element : masses) {
    total_volume += element.volume();
  }

  for (auto& element : masses) {
    element.mass = (element.volume() / total_volume) * mass;
  }

  glm::vec3 center_of_gravity;
  auto inertia = phi::inertia::tensor(masses, false, &center_of_gravity);

  std::cout << "cg = " << center_of_gravity << std::endl;

  const Airfoil NACA_0012(NACA_0012_data_2);
  const Airfoil NACA_2412(NACA_2412_data_2);

  std::vector<Wing> surfaces = {
      Wing(&NACA_2412, {wing_offset, 0.0f, -2.7f}, main_wing_area / 2, main_wing_span / 2),
      Wing(&NACA_0012, {wing_offset - 1.5f, 0.0f, -5.0f}, aileron_area, 1.26f),  // left aileron
      Wing(&NACA_0012, {wing_offset - 1.5f, 0.0f, +5.0f}, aileron_area, 1.26f),  // right aileron
      Wing(&NACA_2412, {wing_offset, 0.0f, +2.7f}, main_wing_area / 2, main_wing_span / 2),
      Wing(&NACA_0012, {tail_offset, -0.1f, 0.0f}, h_tail_area + elevator_area, h_tail_span),  // elevator
      Wing(&NACA_0012, {tail_offset, 0.0f, 0.0f}, v_tail_area, v_tail_span, phi::RIGHT),       // rudder
  };

  std::vector<GameObject*> objects;

  GameObject player = {.transform = gfx::Mesh(model, texture),
                       .airplane = Airplane(mass, Engine(horsepower, rpm, prop_diameter), inertia, surfaces),
                       .collider = col::Sphere({0.0f, 0.0f, 0.0f}, 5.0f)};

  player.airplane.rigid_body.position = glm::vec3(-7000.0f, altitude, 0.0f);
  player.airplane.rigid_body.velocity = glm::vec3(speed, 0.0f, 0.0f);
  scene.add(&player.transform);
  objects.push_back(&player);

#if 1
  auto red_texture = make_shared<gfx::Phong>(glm::vec3(1.0f, 0.0f, 0.0f));

  for (int i = 0; i < masses.size() && i <= 2; i++) {
    auto& mass = masses[i];
    auto element = new gfx::Mesh(gfx::make_cube_geometry(1.0f), red_texture);
    element->set_position(mass.offset);
    element->set_scale(mass.size);
    player.transform.add(element);
  }

#endif

#if NPC_AIRCRAFT
  GameObject npc = {.transform = gfx::Mesh(model, texture),
                    .airplane = Airplane(mass, Engine(horsepower, rpm, prop_diameter), inertia, surfaces),
                    .collider = col::Sphere({0.0f, 0.0f, 0.0f}, 5.0f)};

  npc.airplane.rigid_body.position = glm::vec3(-6990.0f, altitude, 10.0f);
  npc.airplane.rigid_body.velocity = glm::vec3(speed, 0.0f, 0.0f);
  scene.add(&npc.transform);
  objects.push_back(&npc);

  auto red = glm::vec3(1.0f, 0.0f, 0.0f);
  gfx::Billboard target_marker(make_shared<gfx::gl::Texture>("assets/textures/sprites/triangle.png"), red);
  target_marker.set_scale(glm::vec3(0.05f));
  target_marker.set_position({0.0f, 10.0f, 0.0f});
  target_marker.transform_flags = OBJ3D_TRANSFORM | OBJ3D_SCALE;
  npc.transform.add(&target_marker);
#endif

#if 1
  float size = 0.1f;
  float projection_distance = 150.0f;
  glm::vec3 green(0.0f, 1.0f, 0.0f);
  gfx::Billboard cross(make_shared<gfx::gl::Texture>("assets/textures/sprites/cross.png"), green);
  cross.set_position(phi::FORWARD * projection_distance);
  cross.set_scale(glm::vec3(size));
  player.transform.add(&cross);

  gfx::Billboard fpm(make_shared<gfx::gl::Texture>("assets/textures/sprites/fpm.png"), green);
  fpm.set_scale(glm::vec3(size));
  player.transform.add(&fpm);
#endif

  gfx::Object3D camera_transform;
  camera_transform.set_position({-25.0f, 5, 0});
  camera_transform.set_rotation({0, glm::radians(-90.0f), 0.0f});
  player.transform.add(&camera_transform);

  gfx::Camera camera(glm::radians(45.0f), (float)RESOLUTION.x / (float)RESOLUTION.y, 1.0f, 150000.0f);
#if SMOOTH_CAMERA
  camera.set_position(player.airplane.rigid_body.position);
  camera.set_rotation({0, glm::radians(-90.0f), 0.0f});
  scene.add(&camera);
#else
  camera_transform.add(&camera);
#endif

  gfx::OrbitController controller(30.0f);

  SDL_Event event;
  bool quit = false, paused = false, orbit = false;
  uint64_t last = 0, now = SDL_GetPerformanceCounter();
  phi::Seconds dt, timer = 0, log_timer = 0, flight_time = 0.0f;
  float fps = 0.0f;

  while (!quit) {
    // delta time in seconds
    last = now;
    now = SDL_GetPerformanceCounter();
    dt = static_cast<phi::Seconds>((now - last) / static_cast<phi::Seconds>(SDL_GetPerformanceFrequency()));
    flight_time += dt;
    dt = std::min(dt, 0.02f);

    if ((timer += dt) >= 1.0f) {
      timer = 0.0f;
      fps = 1.0f / dt;
    }

    while (SDL_PollEvent(&event) != 0) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      switch (event.type) {
        case SDL_QUIT: {
          quit = true;
          break;
        }
        case SDL_MOUSEMOTION: {
          controller.move_mouse(static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel));
          break;
        }
        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: {
              quit = true;
              break;
            }
            case SDLK_p:
              paused = !paused;
              break;

            case SDLK_o:
              orbit = !orbit;
              break;

            case SDLK_i:
#if CLIPMAP
              clipmap.wireframe = !clipmap.wireframe;
#endif
              break;

            default:
              break;
          }
          break;
        }
        case SDL_JOYAXISMOTION: {
          if ((event.jaxis.value < -3200) || (event.jaxis.value > 3200)) {
            uint8_t axis = event.jaxis.axis;
            int16_t value = event.jaxis.value;
            switch (axis) {
              case 0:
                joystick.aileron = std::pow(Joystick::scale(value), 3.0f);
                break;
              case 1:
                joystick.elevator = std::pow(Joystick::scale(value), 3.0f);
                break;

              case 2:
                joystick.throttle = (Joystick::scale(value) + 1.0f) / 2.0f;
                break;

              case 3:
                // ?
                break;

              case 4:
                joystick.rudder = std::pow(Joystick::scale(value), 3.0f);
                break;

              default:
                break;
            }
          }
          break;
        }
        case SDL_MOUSEWHEEL: {
          if (event.wheel.y > 0) {
            controller.radius *= 1.1f;
          } else if (event.wheel.y < 0) {
            controller.radius *= 0.9f;
          }
          break;
        }
      }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
#if 0
    ImGui::ShowDemoWindow();
#else
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(145, 160));
    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("Flightsim", nullptr, window_flags);
    ImGui::Text("ALT:   %.1f m", player.airplane.get_altitude());
#if 1
    ImGui::Text("SPD:   %.1f km/h", phi::units::kilometer_per_hour(player.airplane.get_speed()));
    ImGui::Text("IAS:   %.1f km/h", phi::units::kilometer_per_hour(player.airplane.get_ias()));
#else
    ImGui::Text("SPD:   %.1f m/s", player.airplane.get_speed());
    ImGui::Text("IAS:   %.1f m/s", player.airplane.get_ias());
#endif
    ImGui::Text("THR:   %d %%", static_cast<int>(player.airplane.engine.throttle * 100.0f));
    ImGui::Text("G:     %.1f", player.airplane.get_g());
    ImGui::Text("AoA:   %.2f", player.airplane.get_aoa());
    ImGui::Text("Trim:  %.2f", player.airplane.trim);
    ImGui::Text("FPS:   %.1f", fps);
    ImGui::End();
#endif

#if 0
    auto angular_velocity = glm::degrees(player.airplane.rigid_body.angular_velocity);
    auto orientation = glm::degrees(glm::eulerAngles(glm::normalize(player.airplane.rigid_body.orientation)));

    ImVec2 size(140, 140);
    ImGui::SetNextWindowPos(ImVec2(RESOLUTION.x - size.y - 10.0f, RESOLUTION.y - size.y - 10.0f));
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("Debug", nullptr, window_flags);
    ImGui::Text("Time:       %.1f", flight_time);
    ImGui::Text("Roll Rate:  %.1f", angular_velocity.x);
    ImGui::Text("Yaw Rate:   %.1f", angular_velocity.y);
    ImGui::Text("Pitch Rate: %.1f", angular_velocity.z);
    ImGui::Text("Roll:       %.1f", orientation.x);
    ImGui::Text("Yaw:        %.1f", orientation.y);
    ImGui::Text("Pitch:      %.1f", orientation.z);
    ImGui::End();
#endif

    get_keyboard_state(joystick, dt);

    player.airplane.joystick = glm::vec3(joystick.aileron, joystick.rudder, joystick.elevator);
    if (!joystick_control) {
      float max_av = 45.0f;  // deg/s
      float target_av = max_av * joystick.elevator;
      float current_av = glm::degrees(player.airplane.rigid_body.angular_velocity.z);
      player.airplane.joystick.z = pitch_control_pid.calculate(current_av, target_av, dt);
    }
    player.airplane.engine.throttle = joystick.throttle;
    player.airplane.trim = joystick.trim;

#if NPC_AIRCRAFT
    target_marker.visible = glm::length(camera.get_world_position() - npc.airplane.rigid_body.position) > 500.0f;
    fly_towards(npc.airplane, player.airplane.rigid_body.position);
#endif

    if (!paused) {
      for (auto obj : objects) {
        obj->update(dt);
      }

      // naive, but works for small numbers of objects
      for (int i = 0; i < objects.size(); i++) {
        for (int j = i + 1; j < objects.size(); j++) {
          auto& a = objects[i];
          auto& b = objects[j];
          if (col::test_collision(a->collider, b->collider)) {
            std::cout << "collision!\n";
          }

#if 0
          if (col::test_moving_collision(a->collider, a->airplane.rigid_body.velocity, b->collider,
                                         b->airplane.rigid_body.velocity)) {
            std::cout << "moving collision!\n";
          }
#endif
        }
      }
    }

    fpm.set_position(glm::normalize(player.airplane.rigid_body.get_body_velocity()) * projection_distance);

    if (orbit) {
      controller.update(camera, player.airplane.rigid_body.position, dt);
      cross.visible = fpm.visible = false;
    } else if (!paused) {
#if SMOOTH_CAMERA
      auto& rb = player.airplane.rigid_body;
      camera.set_position(glm::mix(camera.get_position(), rb.position + rb.up() * 4.5f, dt * 0.035f * rb.get_speed()));
      camera.set_rotation_quat(
          glm::mix(camera.get_rotation_quat(), camera_transform.get_world_rotation_quat(), dt * 5.0f));
#else
      camera.set_transform(glm::vec3(0.0f), glm::vec3(0.0f));
#endif
      cross.visible = fpm.visible = true;
    }
    renderer.render(camera, scene);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }
  return 0;
}

inline float move(float value, float factor, float dt) { return glm::clamp(value - factor * dt, -1.0f, 1.0f); }

inline float center(float value, float factor, float dt)
{
  return (value >= 0) ? glm::clamp(value - factor * dt, 0.0f, 1.0f) : glm::clamp(value + factor * dt, -1.0f, 0.0f);
}

void get_keyboard_state(Joystick& joystick, phi::Seconds dt)
{
  const glm::vec3 factor = {3.0f, 0.5f, 1.0f};  // roll, yaw, pitch
  const uint8_t* key_states = SDL_GetKeyboardState(NULL);

  if (key_states[SDL_SCANCODE_A] || key_states[SDL_SCANCODE_LEFT]) {
    joystick.aileron = move(joystick.aileron, +factor.x, dt);
  } else if (key_states[SDL_SCANCODE_D] || key_states[SDL_SCANCODE_RIGHT]) {
    joystick.aileron = move(joystick.aileron, -factor.x, dt);
  } else if (joystick.num_axis <= 0) {
    joystick.aileron = center(joystick.aileron, factor.x, dt);
  }

  if (key_states[SDL_SCANCODE_W] || key_states[SDL_SCANCODE_UP]) {
    joystick.elevator = move(joystick.elevator, +factor.z, dt);
  } else if (key_states[SDL_SCANCODE_S] || key_states[SDL_SCANCODE_DOWN]) {
    joystick.elevator = move(joystick.elevator, -factor.z, dt);
  } else if (joystick.num_axis <= 0) {
    joystick.elevator = center(joystick.elevator, factor.z, dt);
  }

  if (key_states[SDL_SCANCODE_E]) {
    joystick.rudder = move(joystick.rudder, -factor.x, dt);
  } else if (key_states[SDL_SCANCODE_Q]) {
    joystick.rudder = move(joystick.rudder, +factor.x, dt);
  } else if (joystick.num_axis <= 0) {
    joystick.rudder = center(joystick.rudder, factor.z, dt);
  }

  const float throttle_speed = 0.002f;

  if (key_states[SDL_SCANCODE_J]) {
    joystick.throttle = glm::clamp(joystick.throttle - throttle_speed, 0.0f, 1.0f);
  } else if (key_states[SDL_SCANCODE_K]) {
    joystick.throttle = glm::clamp(joystick.throttle + throttle_speed, 0.0f, 1.0f);
  }

  const float trim_speed = 0.002f;
  if (key_states[SDL_SCANCODE_N]) {
    joystick.trim = glm::clamp(joystick.trim - trim_speed, -1.0f, 1.0f);
  } else if (key_states[SDL_SCANCODE_M]) {
    joystick.trim = glm::clamp(joystick.trim + trim_speed, -1.0f, 1.0f);
  }
}

void apply_to_object3d(const phi::RigidBody& rigid_body, gfx::Object3D& object3d)
{
  object3d.set_transform(rigid_body.position, rigid_body.orientation);
}

void solve_constraints(phi::RigidBody& rigid_body)
{
  if (rigid_body.position.y <= 0) {
    rigid_body.position.y = 0, rigid_body.velocity.y = 0;
  }
}
