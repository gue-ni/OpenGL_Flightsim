#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>

#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

#include "gfx.h"
#include "phi.h"
#include "clipmap.h"
#include "flightmodel.h"

using std::shared_ptr;
using std::make_shared;
using std::cout;
using std::endl;

#if 0
constexpr glm::ivec2 SCREEN{ 640, 480 };
#else
constexpr glm::ivec2 SCREEN{ 1024, 728 };
#endif

struct Joystick {
    int num_axis{0}, num_hats{0}, num_buttons{0};
    float roll{ 0.0f }, pitch{ 0.0f }, yaw{ 0.0f }, throttle{ 0.0f };

    // scale from int16 to -1.0, 1.0
    inline static float scale(int16_t value)
    {
        return static_cast<float>(value) / static_cast<float>(32767);
    }
};

void get_keyboard_state(Joystick& joystick, phi::Seconds dt);
void solve_constraints(phi::RigidBody& rigid_body);
void apply_to_object3d(const phi::RigidBody& rigid_body, gfx::Object3D& object);

int main(void)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow(
        "Flightsim", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        SCREEN.x, 
        SCREEN.y, 
        SDL_WINDOW_OPENGL
    );

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    glewExperimental = GL_TRUE;

    if (GLEW_OK != glewInit())
        return -1;

    glViewport(0, 0, SCREEN.x, SCREEN.y);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_CULL_FACE);

    // SDL options
    SDL_ShowCursor(SDL_FALSE);
    //SDL_CaptureMouse(SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    Joystick joystick;
    SDL_Joystick* sdl_joystick = nullptr;

    int num_joysticks;
    if ((num_joysticks = SDL_NumJoysticks()) < 0)
    {
        std::cout << "no joystick found\n";
        exit(-1);
    }
    else
    {
        std::cout << "found " << num_joysticks << " joysticks\n";
        SDL_JoystickEventState(SDL_ENABLE);
        sdl_joystick = SDL_JoystickOpen(0);
        joystick.num_buttons = SDL_JoystickNumButtons(sdl_joystick);
        joystick.num_axis = SDL_JoystickNumAxes(sdl_joystick);
        joystick.num_hats = SDL_JoystickNumHats(sdl_joystick);

        printf("found %d buttons, %d axis\n", joystick.num_buttons, joystick.num_axis);
    }

#if 1
    std::vector<float> cube_vertices_2 = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
    };

    std::vector<float> triangle_vertices = {
        // positions          // normals           // texture coords
        0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,   // bottom right
       -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    };
#endif

    std::vector<float> ico_vertices;
    std::vector<float> cube_vertices;
    std::vector<float> fuselage_vertices;
    std::vector<float> prop_vertices;

    gfx::load_obj("assets/models/cube.obj", cube_vertices);
    gfx::load_obj("assets/models/icosphere.obj", ico_vertices);
    gfx::load_obj("assets/models/cessna/fuselage.obj", fuselage_vertices);
    gfx::load_obj("assets/models/cessna/Cessna_172_prop.obj", prop_vertices);

    gfx::Renderer renderer(SCREEN.x, SCREEN.y);

    auto blue = make_shared<gfx::Phong>(gfx::rgb(0, 0, 255));
    auto grey = make_shared<gfx::Phong>(glm::vec3(0.5f));
    auto green = make_shared<gfx::Phong>(gfx::rgb(0, 255, 0));
    auto colors = make_shared<gfx::Phong>(make_shared<gfx::Texture>("assets/textures/colors.png"));
    auto grass = make_shared<gfx::Phong>(make_shared<gfx::Texture>("assets/textures/grass.jpg"));
    auto container = make_shared<gfx::Phong>(make_shared<gfx::Texture>("assets/textures/container.jpg"));
    auto cube_geo = std::make_shared<gfx::Geometry>(cube_vertices_2, gfx::Geometry::POS_NORM_UV);
    auto ico_geo = std::make_shared<gfx::Geometry>(ico_vertices, gfx::Geometry::POS_NORM_UV);
    auto triangle = std::make_shared<gfx::Geometry>(triangle_vertices, gfx::Geometry::POS_NORM_UV);

    gfx::Object3D scene;

#if 0
    gfx::Skybox skybox({
        "assets/textures/skybox2/right.jpg",
        "assets/textures/skybox2/left.jpg",
        "assets/textures/skybox2/top.jpg",
        "assets/textures/skybox2/bottom.jpg",
        "assets/textures/skybox2/front.jpg",
        "assets/textures/skybox2/back.jpg",
    });
    scene.add(&skybox);
#endif
#if 1
    gfx::Light sun(gfx::Light::DIRECTIONAL, gfx::rgb(154, 219, 172));
    sun.set_position(glm::vec3(-2.0f, 4.0f, -1.0f));
    sun.cast_shadow = false;
    scene.add(&sun);
#endif

#if 1
    Clipmap clipmap;
    scene.add(&clipmap);
#endif
   
#if 1
    gfx::Object3D aircraft_transform;
    scene.add(&aircraft_transform);
    auto fg = std::make_shared<gfx::Geometry>(fuselage_vertices, gfx::Geometry::POS_NORM_UV);
    auto pg = std::make_shared<gfx::Geometry>(prop_vertices, gfx::Geometry::POS_NORM_UV);
    gfx::Mesh fuselage(fg, colors);
    gfx::Mesh prop(pg, grey);
    aircraft_transform.add(&fuselage);
    aircraft_transform.add(&prop);
#endif

#if 1
    gfx::Object3D npc_aircraft_transform;
    scene.add(&npc_aircraft_transform);
    gfx::Mesh fuselage2(fg, colors);
    gfx::Mesh prop2(pg, grey);
    npc_aircraft_transform.add(&fuselage2);
    npc_aircraft_transform.add(&prop2);
#endif

#if 0
    gfx::Mesh cube(cube_geo, container);
    transform.add(&cube);
#endif

#if 1
    float projection_distance = 500.0f;
    float size = 0.3f;
    gfx::Billboard cross(make_shared<gfx::Texture>("assets/textures/sprites/cross.png"));
    cross.set_position(phi::FORWARD * projection_distance);
    cross.set_scale(glm::vec3(size));
    aircraft_transform.add(&cross);

    // flight path marker
    gfx::Billboard fpm(make_shared<gfx::Texture>("assets/textures/sprites/fpm.png"));
    fpm.set_scale(glm::vec3(size));
    aircraft_transform.add(&fpm);
#endif

    const float mass = 10000.0f;
    const float thrust = 20000.0f;

    std::vector<phi::inertia::Element> elements = {
      phi::inertia::cube_element({-0.5f,  0.0f, -2.7f}, {6.96f, 0.10f, 3.50f}, mass * 0.25f),               // left wing
      phi::inertia::cube_element({-1.0f,  0.0f, -2.0f}, {3.80f, 0.10f, 1.26f}, mass * 0.05f),               // left aileron
      phi::inertia::cube_element({-1.0f,  0.0f,  2.0f}, {3.80f, 0.10f, 1.26f}, mass * 0.05f),               // right aileron
      phi::inertia::cube_element({-0.5f,  0.0f,  2.7f}, {6.96f, 0.10f, 3.50f}, mass * 0.25f),               // right wing
      phi::inertia::cube_element({-6.6f, -0.1f,  0.0f}, {6.54f, 0.10f, 2.70f}, mass * 0.2f),                // elevator
      phi::inertia::cube_element({-6.6f,  0.0f,  0.0f}, {5.31f, 3.10f, 0.10f}, mass * 0.2f),                // rudder
    };

#if 0
    auto inertia = phi::inertia::tensor({100000.0f, 400000.0f, 500000.0f}); 
#else
    auto inertia = phi::inertia::tensor(elements, true);
#endif

    std::vector<Wing> wings = {
      Wing({-0.5f,   0.0f, -2.7f}, 6.96f, 3.50f, &NACA_2412),               // left wing
      Wing({-1.0f,   0.0f, -2.0f},  3.80f, 1.26f, &NACA_0012),              // left aileron
      Wing({-1.0f,   0.0f,  2.0f},  3.80f, 1.26f, &NACA_0012),              // right aileron
      Wing({-0.5f,   0.0f,  2.7f}, 6.96f, 3.50f, &NACA_2412),               // right wing
      Wing({-6.6f, -0.1f, 0.0f},  6.54f, 2.70f, &NACA_0012),                // elevator
      Wing({-6.6f,  0.0f,  0.0f},  5.31f, 3.10f, &NACA_0012, phi::RIGHT),   // rudder
    };

    Aircraft player_aircraft(mass, thrust, inertia, wings);
    player_aircraft.rigid_body.position = glm::vec3(0.0f, 2000.0f, 0.0f);
    player_aircraft.rigid_body.velocity = glm::vec3(phi::units::meter_per_second(600.0f), 0.0f, 0.0f);

    Aircraft npc_aircraft(mass, thrust, inertia, wings);
    npc_aircraft.rigid_body.position = glm::vec3(100.0f, 2050.0f, 10.0f);
    npc_aircraft.rigid_body.velocity = glm::vec3(phi::units::meter_per_second(600.0f), 0.0f, 0.0f);

    ai::Pilot ai;

    gfx::Object3D camera_transform;
    camera_transform.set_position({-15.0f, 1, 0});
    camera_transform.set_rotation({0, glm::radians(-90.0f), 0.0f});
    aircraft_transform.add(&camera_transform);

    gfx::Camera camera(glm::radians(45.0f), (float)SCREEN.x / (float)SCREEN.y, 1.0f, 50000.0f);
    scene.add(&camera);
    camera.set_position(player_aircraft.rigid_body.position);
    camera.set_rotation({0, glm::radians(-90.0f), 0.0f});

    gfx::OrbitController controller(20.0f);

    SDL_Event event;
    bool quit = false, paused = false, orbit = false;
    uint64_t last = 0, now = SDL_GetPerformanceCounter();
    phi::Seconds dt, timer = 0, log_timer = 0;

    std::cout << glGetString(GL_VERSION) << std::endl;

    while (!quit)
    {
        // delta time in seconds
        last = now;
        now = SDL_GetPerformanceCounter();
        dt = static_cast<phi::Seconds>((now - last) / static_cast<phi::Seconds>(SDL_GetPerformanceFrequency()));
        dt = std::min(dt, 0.02f);

        if ((timer += dt) >= 1.0f)
        {
            timer = 0.0f;
        }

        while (SDL_PollEvent(&event) != 0)
        {
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
                    clipmap.wireframe = !clipmap.wireframe;
                    break;

                default:
                    break;
                }
                break;
            }
            case SDL_JOYAXISMOTION: {
                if ((event.jaxis.value < -3200) || (event.jaxis.value > 3200))
                {
                    uint8_t axis = event.jaxis.axis;
                    int16_t value = event.jaxis.value;
                    switch (axis)
                    {
                    case 0:
                        joystick.roll = Joystick::scale(value);
                        break;
                    case 1:
                        joystick.pitch = Joystick::scale(value);
                        break;

                    case 2:
                        joystick.throttle = (Joystick::scale(value) + 1.0f) / 2.0f;
                        break;

                    case 3:
                        // ?
                        break;

                    case 4:
                        joystick.yaw = Joystick::scale(value);
                        break;
                    default:
                        break;
                    }
                }
                break;
            }
            }
        }

        get_keyboard_state(joystick, dt);

        //player_aircraft.joystick = glm::vec3(joystick.roll, joystick.yaw, joystick.pitch);
        player_aircraft.joystick = glm::vec3(joystick.roll, 0.0f, joystick.pitch);
        player_aircraft.engine.throttle = joystick.throttle;
        
        if (!paused)
        {
#if 0
            auto point = ai::intercept_point(
                player_aircraft.rigid_body.position, player_aircraft.rigid_body.velocity, 
                npc_aircraft.rigid_body.position, npc_aircraft.rigid_body.velocity
            );

            ai.fly_towards(player_aircraft, point, dt);
#else
            //ai.fly_towards(player_aircraft, npc_aircraft.rigid_body.position, dt);
#endif
            player_aircraft.update(dt);
            npc_aircraft.update(dt);

            // solve_constraints(aircraft.rigid_body);
            apply_to_object3d(player_aircraft.rigid_body, aircraft_transform);
            apply_to_object3d(npc_aircraft.rigid_body, npc_aircraft_transform);
        }

        prop.rotate_by({0.1f, 0.0f, 0.0f});
        fpm.set_position(glm::normalize(player_aircraft.rigid_body.get_body_velocity()) * (projection_distance + 1));
       
        if (orbit)
        {
            controller.update(camera, player_aircraft.rigid_body.position, dt);
            cross.visible = fpm.visible = false;
        }
        else
        {
            auto& rb = player_aircraft.rigid_body;
            camera.set_position(glm::mix(camera.get_position(), rb.position + rb.up() * 1.0f, dt * 5.0f));
#if 1
            camera.set_rotation_quaternion(glm::mix(camera.get_rotation_quaternion(), camera_transform.get_world_rotation_quaternion(), dt * 5.0f));
#else
            camera.look_at(player_aircraft.rigid_body.position);
#endif
            cross.visible = fpm.visible = true;
        }
        renderer.render(camera, scene);

        SDL_GL_SwapWindow(window);
    }
    return 0;
}

inline float move(float value, float factor, float dt)
{
    return glm::clamp(value - factor * dt, -1.0f, 1.0f);
}

inline float center(float value, float factor, float dt)
{
    return (value >= 0) 
        ? glm::clamp(value - factor * dt,  0.0f, 1.0f) 
        : glm::clamp(value + factor * dt, -1.0f, 0.0f);
}

void get_keyboard_state(Joystick& joystick, phi::Seconds dt)
{
    const glm::vec3 factor = {3.0f, 3.0f, 1.0f}; // roll, yaw, pitch
    const uint8_t* key_states = SDL_GetKeyboardState(NULL);

    if (key_states[SDL_SCANCODE_A])
    {
        joystick.roll = move(joystick.roll, +factor.x, dt);
    }
    else if (key_states[SDL_SCANCODE_D])
    {
        joystick.roll = move(joystick.roll, -factor.x, dt);
    }
    else
    {
        joystick.roll = center(joystick.roll, factor.x, dt);
    }

    if (key_states[SDL_SCANCODE_W])
    {
        joystick.pitch = move(joystick.pitch, +factor.z, dt);
    }
    else if (key_states[SDL_SCANCODE_S])
    {
        joystick.pitch = move(joystick.pitch, -factor.z, dt);
    }
    else
    {
        joystick.pitch = center(joystick.pitch, factor.z, dt);
    }

    if (key_states[SDL_SCANCODE_Q])
    {
        joystick.yaw = move(joystick.yaw, -factor.x, dt);
    }
    else if (key_states[SDL_SCANCODE_E])
    {
        joystick.yaw = move(joystick.yaw, +factor.x, dt);
    }
    else
    {
        joystick.yaw = center(joystick.yaw, factor.z, dt);
    }
    
    if (key_states[SDL_SCANCODE_J])
    {
        joystick.throttle = glm::clamp(joystick.throttle - 0.01f, 0.0f, 1.0f);
    }
    else if (key_states[SDL_SCANCODE_K])
    {
        joystick.throttle = glm::clamp(joystick.throttle + 0.01f, 0.0f, 1.0f);
    }
}

void apply_to_object3d(const phi::RigidBody& rigid_body, gfx::Object3D& object3d)
{
    object3d.set_position(rigid_body.position);
    object3d.set_rotation_quaternion(rigid_body.orientation);
}

void solve_constraints(phi::RigidBody& rigid_body)
{
    if (rigid_body.position.y <= 0)
    {
        rigid_body.position.y = 0, rigid_body.velocity.y = 0;
    }
}

