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

#if 0
constexpr glm::ivec2 RESOLUTION{ 640, 480 };
#else
constexpr glm::ivec2 RESOLUTION{1024, 728};
#endif

const std::string textures = "assets/textures";

struct Joystick {
  int num_axis{0}, num_hats{0}, num_buttons{0};
  float roll{0.0f}, pitch{0.0f}, yaw{0.0f}, throttle{0.0f};

  // scale from int16 to -1.0, 1.0
  inline static float scale(int16_t value) {
    // return static_cast<float>(value) / static_cast<float>(32767);
    return static_cast<float>(value) /
           static_cast<float>(std::numeric_limits<int16_t>::max() / 2);
  }
};

struct GameObject {
  gfx::Mesh transform;
  Aircraft aircraft;

  void update(float dt) {
    aircraft.update(dt);
    transform.set_transform(aircraft.rigid_body.position,
                            aircraft.rigid_body.orientation);
  }
};

void get_keyboard_state(Joystick& joystick, phi::Seconds dt);
void solve_constraints(phi::RigidBody& rigid_body);
void apply_to_object3d(const phi::RigidBody& rigid_body, gfx::Object3D& object);

int main(void) {
#if RUN_COLLISION_UNITTESTS
  collisions::run_unit_tests();
#endif

  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_Window* window = SDL_CreateWindow("Flightsim", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, RESOLUTION.x,
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
  SDL_Joystick* sdl_joystick = nullptr;

  int num_joysticks;
  if ((num_joysticks = SDL_NumJoysticks()) < 0) {
    std::cout << "no joystick found\n";
    exit(-1);
  } else {
    std::cout << "found " << num_joysticks << " joysticks\n";
    SDL_JoystickEventState(SDL_ENABLE);
    sdl_joystick = SDL_JoystickOpen(0);
    joystick.num_buttons = SDL_JoystickNumButtons(sdl_joystick);
    joystick.num_axis = SDL_JoystickNumAxes(sdl_joystick);
    joystick.num_hats = SDL_JoystickNumHats(sdl_joystick);

    printf("found %d buttons, %d axis\n", joystick.num_buttons,
           joystick.num_axis);
  }

  auto vertices = gfx::load_obj("assets/models/falcon.obj");

  gfx::Renderer renderer(RESOLUTION.x, RESOLUTION.y);

  auto grey = make_shared<gfx::Phong>(glm::vec3(0.5f));
  auto colors = make_shared<gfx::Phong>(
      make_shared<gfx::gl::Texture>(textures + "/colorpalette.png"));

  gfx::gl::TextureParams params = {.flip_vertically = true};
  auto tex = make_shared<gfx::gl::Texture>(textures + "/f16_large.jpg", params);
  auto f16_texture = make_shared<gfx::Phong>(tex);
  auto f16_fuselage =
      make_shared<gfx::Geometry>(vertices, gfx::Geometry::POS_NORM_UV);

  gfx::Object3D scene;

#if 1
  const std::string skybox_path = textures + "/skybox/1/";
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
#if 1
  gfx::Light sun(gfx::Light::DIRECTIONAL, glm::vec3(1.0f));
  sun.set_position(glm::vec3(-2.0f, 4.0f, -1.0f));
  sun.cast_shadow = false;
  scene.add(&sun);
#endif

#define CLIPMAP 1
#if CLIPMAP
  Clipmap clipmap;
  scene.add(&clipmap);
#endif

  const float mass = 10000.0f;
  const float thrust = 50000.0f;

  std::vector<phi::inertia::Element> elements = {
      phi::inertia::cube({-0.5f, 0.0f, -2.7f}, {6.96f, 0.10f, 3.50f},
                         mass * 0.25f),  // left wing
      phi::inertia::cube({-1.0f, 0.0f, -2.0f}, {3.80f, 0.10f, 1.26f},
                         mass * 0.05f),  // left aileron
      phi::inertia::cube({-1.0f, 0.0f, 2.0f}, {3.80f, 0.10f, 1.26f},
                         mass * 0.05f),  // right aileron
      phi::inertia::cube({-0.5f, 0.0f, 2.7f}, {6.96f, 0.10f, 3.50f},
                         mass * 0.25f),  // right wing
      phi::inertia::cube({-6.6f, -0.1f, 0.0f}, {6.54f, 0.10f, 2.70f},
                         mass * 0.2f),  // elevator
      phi::inertia::cube({-6.6f, 0.0f, 0.0f}, {5.31f, 3.10f, 0.10f},
                         mass * 0.2f),  // rudder
  };

  auto inertia = phi::inertia::tensor(elements, true);

  std::vector<Wing> wings = {
      Wing({-0.5f, 0.0f, -2.7f}, 6.96f, 3.50f, &NACA_2412),  // left wing
      Wing({-1.0f, 0.0f, -2.0f}, 3.80f, 1.26f, &NACA_0012),  // left aileron
      Wing({-1.0f, 0.0f, 2.0f}, 3.80f, 1.26f, &NACA_0012),   // right aileron
      Wing({-0.5f, 0.0f, 2.7f}, 6.96f, 3.50f, &NACA_2412),   // right wing
      Wing({-6.6f, -0.1f, 0.0f}, 6.54f, 2.70f, &NACA_0012),  // elevator
      Wing({-6.6f, 0.0f, 0.0f}, 5.31f, 3.10f, &NACA_0012,
           phi::RIGHT),  // rudder
  };

  std::vector<GameObject*> objects;

  GameObject player = {.transform = gfx::Mesh(f16_fuselage, f16_texture),
                       .aircraft = Aircraft(mass, thrust, inertia, wings)};

  player.aircraft.rigid_body.position = glm::vec3(-7000.0f, 3000.0f, 0.0f);
  player.aircraft.rigid_body.velocity =
      glm::vec3(phi::units::meter_per_second(600.0f), 0.0f, 0.0f);
  scene.add(&player.transform);
  objects.push_back(&player);

#define NPC_AIRCRAFT 1
#if NPC_AIRCRAFT
  GameObject npc = {.transform = gfx::Mesh(f16_fuselage, f16_texture),
                    .aircraft = Aircraft(mass, thrust, inertia, wings)};

  npc.aircraft.rigid_body.position = glm::vec3(-6800.0f, 3020.0f, 50.0f);
  npc.aircraft.rigid_body.velocity =
      glm::vec3(phi::units::meter_per_second(600.0f), 0.0f, 0.0f);
  scene.add(&npc.transform);
  objects.push_back(&npc);
#endif

#if 1
  float size = 0.3f;
  float projection_distance = 1500.0f;
  auto green = glm::vec3(0.0f, 1.0f, 0.0f);
  gfx::Billboard cross(
      make_shared<gfx::gl::Texture>("assets/textures/sprites/cross.png"),
      green);
  cross.set_position(phi::FORWARD * projection_distance);
  cross.set_scale(glm::vec3(size));
  player.transform.add(&cross);

  gfx::Billboard fpm(
      make_shared<gfx::gl::Texture>("assets/textures/sprites/fpm.png"), green);
  fpm.set_scale(glm::vec3(size));
  player.transform.add(&fpm);
#endif

  gfx::Object3D camera_transform;
  camera_transform.set_position({-15.0f, 1, 0});
  camera_transform.set_rotation({0, glm::radians(-90.0f), 0.0f});
  player.transform.add(&camera_transform);

  gfx::Camera camera(glm::radians(45.0f),
                     (float)RESOLUTION.x / (float)RESOLUTION.y, 1.0f,
                     150000.0f);
  scene.add(&camera);
  camera.set_position(player.aircraft.rigid_body.position);
  camera.set_rotation({0, glm::radians(-90.0f), 0.0f});

  gfx::OrbitController controller(30.0f);

  SDL_Event event;
  bool quit = false, paused = false, orbit = false;
  uint64_t last = 0, now = SDL_GetPerformanceCounter();
  phi::Seconds dt, timer = 0, log_timer = 0;
  float fps = 0.0f;

  while (!quit) {
    // delta time in seconds
    last = now;
    now = SDL_GetPerformanceCounter();
    dt = static_cast<phi::Seconds>(
        (now - last) /
        static_cast<phi::Seconds>(SDL_GetPerformanceFrequency()));
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
          controller.move_mouse(static_cast<float>(event.motion.xrel),
                                static_cast<float>(event.motion.yrel));
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
                joystick.roll = std::pow(Joystick::scale(value), 3.0f);
                break;
              case 1:
                joystick.pitch = std::pow(Joystick::scale(value), 3.0f);
                break;

              case 2:
                joystick.throttle = (Joystick::scale(value) + 1.0f) / 2.0f;
                break;

              case 3:
                // ?
                break;

              case 4:
                joystick.yaw = std::pow(Joystick::scale(value), 3.0f);
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

    auto& rb = player.aircraft.rigid_body;

    float speed = phi::units::kilometer_per_hour(rb.get_speed());
    float ias = phi::units::kilometer_per_hour(get_indicated_air_speed(rb));

    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(145, 140));
    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("Flightsim", nullptr, window_flags);
    ImGui::Text("ALT:   %.2f m", rb.position.y);
    ImGui::Text("SPD:   %.2f km/h", speed);
    ImGui::Text("IAS:   %.2f km/h", ias);
    ImGui::Text("THR:   %d %%",
                static_cast<int>(player.aircraft.engine.throttle * 100.0f));
    ImGui::Text("Mach:  %.2f", get_mach_number(rb));
    ImGui::Text("G:     %.1f", get_g_force(rb));
    ImGui::Text("FPS:   %.2f", fps);
    ImGui::End();
#endif

    get_keyboard_state(joystick, dt);

    auto& player_aircraft = player.aircraft;
    player_aircraft.joystick =
        glm::vec3(joystick.roll, joystick.yaw, joystick.pitch);
    player_aircraft.engine.throttle = joystick.throttle;

    fly_towards(npc.aircraft, player.aircraft.rigid_body.position);
    // fly_towards(player.aircraft, npc.aircraft.rigid_body.position);

    if (!paused) {
      for (auto obj : objects) {
        obj->update(dt);
      }
    }

    fpm.set_position(
        glm::normalize(player_aircraft.rigid_body.get_body_velocity()) *
        (projection_distance + 1));

    if (orbit) {
      controller.update(camera, player_aircraft.rigid_body.position, dt);
      cross.visible = fpm.visible = false;
    } else if (!paused) {
      auto& rb = player_aircraft.rigid_body;
      camera.set_position(glm::mix(camera.get_position(),
                                   rb.position + rb.up() * 4.5f,
                                   dt * 0.035f * rb.get_speed()));

      camera.set_rotation_quaternion(glm::mix(
          camera.get_rotation_quaternion(),
          camera_transform.get_world_rotation_quaternion(), dt * 5.0f));

      cross.visible = fpm.visible = true;
    }
    renderer.render(camera, scene);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }
  return 0;
}

inline float move(float value, float factor, float dt) {
  return glm::clamp(value - factor * dt, -1.0f, 1.0f);
}

inline float center(float value, float factor, float dt) {
  return (value >= 0) ? glm::clamp(value - factor * dt, 0.0f, 1.0f)
                      : glm::clamp(value + factor * dt, -1.0f, 0.0f);
}

void get_keyboard_state(Joystick& joystick, phi::Seconds dt) {
  const glm::vec3 factor = {3.0f, 0.5f, 1.0f};  // roll, yaw, pitch
  const uint8_t* key_states = SDL_GetKeyboardState(NULL);

  if (key_states[SDL_SCANCODE_A] || key_states[SDL_SCANCODE_LEFT]) {
    joystick.roll = move(joystick.roll, +factor.x, dt);
  } else if (key_states[SDL_SCANCODE_D] || key_states[SDL_SCANCODE_RIGHT]) {
    joystick.roll = move(joystick.roll, -factor.x, dt);
  } else if (joystick.num_axis <= 0) {
    joystick.roll = center(joystick.roll, factor.x, dt);
  }

  if (key_states[SDL_SCANCODE_W] || key_states[SDL_SCANCODE_UP]) {
    joystick.pitch = move(joystick.pitch, +factor.z, dt);
  } else if (key_states[SDL_SCANCODE_S] || key_states[SDL_SCANCODE_DOWN]) {
    joystick.pitch = move(joystick.pitch, -factor.z, dt);
  } else if (joystick.num_axis <= 0) {
    joystick.pitch = center(joystick.pitch, factor.z, dt);
  }

  if (key_states[SDL_SCANCODE_Q]) {
    joystick.yaw = move(joystick.yaw, -factor.x, dt);
  } else if (key_states[SDL_SCANCODE_E]) {
    joystick.yaw = move(joystick.yaw, +factor.x, dt);
  } else if (joystick.num_axis <= 0) {
    joystick.yaw = center(joystick.yaw, factor.z, dt);
  }

  const float tmp = 0.002f;

  if (key_states[SDL_SCANCODE_J]) {
    joystick.throttle = glm::clamp(joystick.throttle - tmp, 0.0f, 1.0f);
  } else if (key_states[SDL_SCANCODE_K]) {
    joystick.throttle = glm::clamp(joystick.throttle + tmp, 0.0f, 1.0f);
  }
}

void apply_to_object3d(const phi::RigidBody& rigid_body,
                       gfx::Object3D& object3d) {
  object3d.set_transform(rigid_body.position, rigid_body.orientation);
}

void solve_constraints(phi::RigidBody& rigid_body) {
  if (rigid_body.position.y <= 0) {
    rigid_body.position.y = 0, rigid_body.velocity.y = 0;
  }
}
