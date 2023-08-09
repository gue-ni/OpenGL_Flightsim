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
#include "collider.h"
#include "flightmodel.h"
#include "gfx/gfx.h"
#include "phi.h"
#include "pid.h"
#include "terrain.h"

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

#define CLIPMAP            1
#define SKYBOX             1
#define SMOOTH_CAMERA      1
#define NPC_AIRCRAFT       0
#define SHOW_MASS_ELEMENTS 0
#define USE_PID            1
#define PS1_RESOLUTION     0
#define DEBUG_INFO         0
#define TERRAIN_COLLISION  0

/* select flightmodel */
#define FAST_JET    0
#define CESSNA      1
#define FLIGHTMODEL FAST_JET

#if PS1_RESOLUTION
constexpr glm::ivec2 RESOLUTION{640, 480};
#else
constexpr glm::ivec2 RESOLUTION{1024, 728};
#endif

struct Joystick {
  int num_axis{0}, num_hats{0}, num_buttons{0};
  float aileron{0.0f}, elevator{0.0f}, rudder{0.0f}, throttle{0.0f}, trim{0.0f};
  bool engage_breaks;

  // scale from int16 to -1.0, 1.0
  inline static float scale(int16_t value)
  {
    constexpr int16_t max_value = std::numeric_limits<int16_t>::max();
    return static_cast<float>(value) / static_cast<float>(max_value);
  }
};

struct GameObject {
  gfx::Mesh2 mesh;
  Airplane& rigid_body;
};

void get_keyboard_state(Joystick& joystick, phi::Seconds dt);

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
    joystick.elevator = center(joystick.elevator, factor.z * 3.0f, dt);
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

  if (key_states[SDL_SCANCODE_B]) {
    joystick.engage_breaks = true;
  } else {
    joystick.engage_breaks = false;
  }
}

int main(void)
{
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
  std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
  std::cout << glGetString(GL_VENDOR) << std::endl;
  std::cout << glGetString(GL_RENDERER) << std::endl;

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

  // use pid for keyboard control
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

  gfx::Renderer2 renderer(RESOLUTION.x, RESOLUTION.y);

  gfx::Object3D scene;

#if SKYBOX
  const std::string skybox_path = "assets/textures/skybox/1/";
  const std::array<std::string, 6>& faces = {
      skybox_path + "right.jpg",  skybox_path + "left.jpg",  skybox_path + "top.jpg",
      skybox_path + "bottom.jpg", skybox_path + "front.jpg", skybox_path + "back.jpg",

  };

  gfx::Skybox skybox(faces);

  skybox.set_scale(glm::vec3(3.0f));
  scene.add(&skybox);
#endif

#if CLIPMAP
  Clipmap clipmap;
  scene.add(&clipmap);
#endif

#if (FLIGHTMODEL == CESSNA)
#error not implemented
#elif (FLIGHTMODEL == FAST_JET)

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

  const Airfoil NACA_0012(NACA_0012_data);
  const Airfoil NACA_2412(NACA_2412_data);
  const Airfoil NACA_64_206(NACA_64_206_data);

  std::array<Wing, 4> wings = {
      Wing({wing_offset, 0.0f, -2.7f}, 6.96f, 2.50f, &NACA_2412, phi::UP, 0.20f),    // left wing
      Wing({wing_offset, 0.0f, +2.7f}, 6.96f, 2.50f, &NACA_2412, phi::UP, 0.20f),    // right wing
      Wing({tail_offset, -0.1f, 0.0f}, 6.54f, 2.70f, &NACA_0012, phi::UP, 1.0f),     // elevator
      Wing({tail_offset, 0.0f, 0.0f}, 5.31f, 3.10f, &NACA_0012, phi::RIGHT, 0.15f),  // rudder
  };

  Engine* engine = new SimpleEngine(thrust);
#else
#error FLIGHTMODEL not defined
#endif

  Sphere sphere(15.0f);
  LandingGear landing_gear(glm::vec3(4.0f, -1.5f, 0.0f), glm::vec3(-1.0f, -1.5f, +2.0f),
                           glm::vec3(-1.0f, -1.5f, -2.0f));

  std::vector<Airplane> rigid_bodies = {
      Airplane(mass, inertia, wings, {engine}, &landing_gear),
  };

  gfx::gl::TextureParams params = {.flip_vertically = true, .texture_mag_filter = GL_LINEAR};

  auto falcon_tex = make_shared<gfx::gl::Texture>("assets/textures/falcon.jpg", params);
  auto falcon_obj = gfx::load_obj("assets/models/falcon.obj");
  auto falcon_geo = std::make_shared<gfx::Geometry>(falcon_obj, gfx::Geometry::POS_NORM_UV);

  gfx::Material2Ptr material = make_shared<gfx::Material2>("shaders/pbr", falcon_tex);

  std::vector<GameObject> objects = {{
      .mesh = gfx::Mesh2(falcon_geo, material),
      .rigid_body = rigid_bodies[0],
  }};

  GameObject& player = objects[0];

  phi::RigidBody terrain;
  terrain.active = false;
  terrain.mass = 10000.0f;
  terrain.set_inertia(phi::inertia::sphere(terrain.mass, 1000.0f));
#if CLIPMAP
  terrain.collider = new Heightmap(&clipmap);
#else
  terrain.collider = nullptr;
#endif

  player.rigid_body.position = glm::vec3(0.0f, 1500.0f, 0.0f);
  player.rigid_body.velocity = glm::vec3(phi::units::meter_per_second(500.0f), 0.0f, 0.0f);
  scene.add(&player.mesh);

#if 0
  auto wheel_texture = make_shared<gfx::Phong>(glm::vec3(0.0f, 0.0f, 1.0f));
  auto wheel_geometry = gfx::make_cube_geometry(0.5f);

  gfx::Mesh center_wheel(wheel_geometry, wheel_texture);
  center_wheel.set_position(landing_gear.center);
  player.mesh.add(&center_wheel);

  gfx::Mesh left_wheel(wheel_geometry, wheel_texture);
  left_wheel.set_position(landing_gear.left);
  player.mesh.add(&left_wheel);

  gfx::Mesh right_wheel(wheel_geometry, wheel_texture);
  right_wheel.set_position(landing_gear.right);
  player.mesh.add(&right_wheel);
#endif

#if SHOW_MASS_ELEMENTS
  auto red_texture = make_shared<gfx::Phong>(glm::vec3(1.0f, 0.0f, 0.0f));

  for (int i = 0; i < masses.size(); i++) {
    auto& mass = masses[i];
    gfx::Mesh* element = new gfx::Mesh(gfx::make_cube_geometry(1.0f), red_texture);
    element->wireframe = true;
    element->set_position(mass.offset);
    element->set_scale(mass.size);
    player.mesh.add(element);
  }
#endif

#if 0
  auto cross_texture = make_shared<gfx::gl::Texture>("assets/textures/sprites/cross.png");
  auto screen_geometry = gfx::make_quad_geometry();

  auto screen_material = std::make_shared<gfx::ScreenMaterial>(cross_texture);
  auto screen_quad = gfx::Mesh(screen_geometry, screen_material);
  scene.add(&screen_quad);
#endif

  gfx::Object3D camera_transform;
  camera_transform.set_position({-25.0f, 5, 0});
  camera_transform.set_rotation({0, glm::radians(-90.0f), 0.0f});
  player.mesh.add(&camera_transform);

  gfx::Camera camera(glm::radians(45.0f), (float)RESOLUTION.x / (float)RESOLUTION.y, 1.0f, 150000.0f);
#if SMOOTH_CAMERA
  camera.set_position(player.rigid_body.position);
  camera.set_rotation({0, glm::radians(-90.0f), 0.0f});
  scene.add(&camera);
#else
  camera_transform.add(&camera);
#endif

  gfx::OrbitController controller(30.0f);

  SDL_Event event;
  bool quit = false, paused = false, orbit = false;
  uint64_t last = 0, now = SDL_GetPerformanceCounter();
  phi::Seconds dt, timer = 0, log_timer = 0, flight_time = 0.0f, hud_timer = 0.0f;
  float fps = 0.0f;

  float alt{}, spd{}, ias{}, aoa{}, gee{};
  int thr{};

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

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;

    if ((hud_timer += dt) > 0.1f) {
      hud_timer = 0.0f;
      alt = player.rigid_body.get_altitude();
      spd = player.rigid_body.get_speed();
      spd = phi::units::kilometer_per_hour(player.rigid_body.get_speed());
      thr = player.rigid_body.throttle;
      gee = player.rigid_body.get_g();
      aoa = player.rigid_body.get_aoa();
    }

    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(145, 160));
    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("Flightsim", nullptr, window_flags);
    ImGui::Text("ALT:   %.1f m", alt);
    ImGui::Text("SPD:   %.1f km/h", spd);
    ImGui::Text("IAS:   %.1f km/h", ias);
    ImGui::Text("THR:   %.1f %%", player.rigid_body.throttle * 100.0f);
    ImGui::Text("G:     %.1f", gee);
    ImGui::Text("AoA:   %.2f", aoa);
    ImGui::Text("Trim:  %.2f", player.rigid_body.joystick.w);
    ImGui::Text("FPS:   %.1f", fps);
    ImGui::End();

#if DEBUG_INFO
    auto angular_velocity = glm::degrees(player.rigid_body.angular_velocity);
    auto attitude = glm::degrees(player.rigid_body.get_euler_angles());

    ImVec2 size(140, 140);
    ImGui::SetNextWindowPos(ImVec2(RESOLUTION.x - size.y - 10.0f, RESOLUTION.y - size.y - 10.0f));
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("Debug", nullptr, window_flags);
    ImGui::Text("Time:       %.1f", flight_time);
    ImGui::Text("Roll Rate:  %.1f", angular_velocity.x);
    ImGui::Text("Yaw Rate:   %.1f", angular_velocity.y);
    ImGui::Text("Pitch Rate: %.1f", angular_velocity.z);
    ImGui::Text("Roll:       %.1f", attitude.x);
    ImGui::Text("Yaw:        %.1f", attitude.y);
    ImGui::Text("Pitch:      %.1f", attitude.z);
    ImGui::End();
#endif

    // TODO: render hud to texture
    // TODO: use texture in screen quad

    get_keyboard_state(joystick, dt);

    player.rigid_body.joystick = glm::vec4(joystick.aileron, joystick.rudder, joystick.elevator, joystick.trim);
#if USE_PID
    {
      float max_av = 45.0f;  // deg/s
      float target_av = max_av * joystick.elevator;
      float current_av = glm::degrees(player.rigid_body.angular_velocity.z);
      player.rigid_body.joystick.z = pitch_control_pid.calculate(current_av, target_av, dt);
    }
#endif
    player.rigid_body.throttle = joystick.throttle;

#if NPC_AIRCRAFT
    target_marker.visible = glm::length(camera.get_world_position() - npc.airplane.position) > 500.0f;
    // fly_towards(npc.airplane, player.airplane.position);
#endif

    if (!paused) {
      phi::step_physics(rigid_bodies, dt);

      for (GameObject& obj : objects) {
        obj.mesh.set_transform(obj.rigid_body.position, obj.rigid_body.rotation);
      }

#if TERRAIN_COLLISION
      phi::CollisionInfo collision;

      if (test_collision(&player.rigid_body, &terrain, &collision)) {
        // std::cout << "contact\n";
        phi::RigidBody::impulse_collision(collision);

        // TODO: vehicle takes damage if ke is too large
        float kinetic_energy = phi::calc::kinetic_energy(player.rigid_body.mass, player.rigid_body.get_speed());
        std::cout << kinetic_energy << std::endl;

#if 0
        // auto wheel = collision.point;
        auto wheels_in_contact = landing_gear.wheels_in_contact(static_cast<Heightmap*>(terrain.collider)->height);
        for (auto& wheel : wheels_in_contact)  // maybe actually only get wheels in contact
        {
          // velocity without y axis
          glm::vec3 plane_velocity = player.rigid_body.get_point_velocity(wheel);
          // glm::vec3 plane_velocity = player.rigid_body.velocity * glm::vec3(1, 0, 1);

          if (glm::length(plane_velocity) > 0.01f) {
            glm::vec3 friction_direction = glm::normalize(-plane_velocity);

            float weight = (player.rigid_body.mass * phi::EARTH_GRAVITY) / wheels_in_contact.size();

            // float rolling_resistance_coeff = 5.0f;
            // glm::vec3 rolling_resistance = friction_direction * weight * rolling_resistance_coeff;

            float friction_coeff = 0.7f;  // rubber on dry pavement

            float sideslip_ratio =
                std::abs(1 - glm::dot(phi::FORWARD, glm::normalize(plane_velocity)));  // 1 if going sideways
            // std::cout << sideslip_ratio << std::endl;

            glm::vec3 sliding_friction = friction_direction * weight * friction_coeff * sideslip_ratio;

            glm::vec3 force = sliding_friction * 100.0f;
            std::cout << force << std::endl;

            player.rigid_body.add_force_at_point(force, wheel);
          }
        }
#else
        if (joystick.engage_breaks) {
          float breaking_force = 100000.0f;
          glm::vec3 direction = glm::normalize(-player.rigid_body.velocity * glm::vec3(1, 0, 1));
          player.rigid_body.add_relative_force(direction * breaking_force);
        }
#endif
      } else {
        // std::cout << "no contact\n";
      }
#endif
    }

    joystick.engage_breaks = false;

    if (orbit) {
      controller.update(camera, player.rigid_body.position, dt);
    } else if (!paused) {
#if SMOOTH_CAMERA
      auto& rb = player.rigid_body;
      camera.set_position(glm::mix(camera.get_position(), rb.position + rb.up() * 3.5f, dt * 0.035f * rb.get_speed()));
      camera.set_rotation_quat(
          glm::mix(camera.get_rotation_quat(), camera_transform.get_world_rotation_quat(), dt * 5.0f));
#else
      camera.set_transform(glm::vec3(0.0f), glm::vec3(0.0f));
#endif
    }
    renderer.render(camera, scene);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }
  return 0;
}
