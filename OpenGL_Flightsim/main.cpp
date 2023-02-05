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

constexpr int SCREEN_WIDTH = 1000;
constexpr int SCREEN_HEIGHT = 800;

void apply_to_object3d(const phi::RigidBody& rigid_body, gfx::Object3D& object3d)
{
    object3d.set_position(rigid_body.position);
    object3d.set_rotation_quaternion(rigid_body.rotation);
}

void solve_constraints(phi::RigidBody& rigid_body)
{
    if (rigid_body.position.y <= 0)
    {
        rigid_body.position.y = 0, rigid_body.velocity.y = 0;
    }
}

struct Joystick {
    int num_axis{0}, num_hats{0}, num_buttons{0};
    float roll{ 0.0f }, pitch{ 0.0f }, yaw{ 0.0f }, throttle{ 0.0f };

    // scale from int16 to -1.0, 1.0
    inline static float scale(int16_t value)
    {
        return static_cast<float>(value) / static_cast<float>(32767);
    }
};

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
        SCREEN_WIDTH, 
        SCREEN_HEIGHT, 
        SDL_WINDOW_OPENGL
    );

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    glewExperimental = GL_TRUE;

    if (GLEW_OK != glewInit())
        return -1;

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
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

    std::vector<float> ico_vertices;
    std::vector<float> cube_vertices;
    std::vector<float> fuselage_vertices;
    std::vector<float> prop_vertices;

    gfx::load_obj("assets/models/cube.obj", cube_vertices);
    gfx::load_obj("assets/models/icosphere.obj", ico_vertices);
    gfx::load_obj("assets/models/cessna/fuselage.obj", fuselage_vertices);
    gfx::load_obj("assets/models/cessna/Cessna_172_prop.obj", prop_vertices);

    gfx::Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);

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

#if 1
    gfx::Skybox skybox({
        "assets/textures/skybox/right.jpg",
        "assets/textures/skybox/left.jpg",
        "assets/textures/skybox/top.jpg",
        "assets/textures/skybox/bottom.jpg",
        "assets/textures/skybox/front.jpg",
        "assets/textures/skybox/back.jpg",
    });
    scene.add(&skybox);
#endif
#if 1
    gfx::Light sun(gfx::Light::DIRECTIONAL, gfx::rgb(154, 219, 172));
    sun.set_position(glm::vec3(-2.0f, 4.0f, -1.0f));
    sun.cast_shadow = false;
    scene.add(&sun);
#endif
  
    auto position = glm::vec3(0.0f, 1000.0f, 0.0f);
    auto velocity = glm::vec3(120.0f, 0.0f, 0.0f);

#if 1
    Clipmap clipmap;
    scene.add(&clipmap);
#endif
   
#if 1
    gfx::Object3D transform;
    transform.set_position(position);
    scene.add(&transform);

    gfx::Mesh fuselage(std::make_shared<gfx::Geometry>(fuselage_vertices, gfx::Geometry::POS_NORM_UV), colors);
    gfx::Mesh prop(std::make_shared<gfx::Geometry>(prop_vertices, gfx::Geometry::POS_NORM_UV), grey);
    transform.add(&fuselage);
    //transform.add(&prop);
#endif

#if 0
    gfx::Mesh cube(cube_geo, container);
    transform.add(&cube);
#endif

#if 1
    float projection_distance = 500.0f;
    gfx::Billboard cross(make_shared<gfx::Texture>("assets/textures/sprites/cross.png"));
    cross.set_position(phi::FORWARD * projection_distance);
    cross.set_scale(glm::vec3(0.25f));
    transform.add(&cross);

    gfx::Billboard fpm(make_shared<gfx::Texture>("assets/textures/sprites/fpm.png"));
    fpm.set_scale(glm::vec3(0.25f));
    transform.add(&fpm);
#endif

    Aircraft aircraft(position, velocity);

    gfx::Camera camera(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 50000.0f);
    camera.set_position(glm::vec3(0, 1, 0));
    camera.set_rotation(glm::vec3(0, glm::radians(-90.0f), 0.0f));
    transform.add(&camera);

    //gfx::FirstPersonController controller(50.0f);
    gfx::OrbitController controller(20.0f);

    SDL_Event event;
    bool quit = false, paused = false;
    uint64_t last = 0, now = SDL_GetPerformanceCounter();
    phi::Seconds dt, timer = 0, log_timer = 0;
    bool orbit = false;

    std::cout << glGetString(GL_VERSION) << std::endl;

    while (!quit)
    {
        // delta time in seconds
        last = now;
        now = SDL_GetPerformanceCounter();
        dt = static_cast<phi::Seconds>((now - last) / static_cast<phi::Seconds>(SDL_GetPerformanceFrequency()));
        dt = phi::utils::min(dt, 0.02f);

        if ((timer += dt) >= 1.0f)
        {
            //printf("dt = %f, fps = %f\n", dt, 1 / dt);
            //std::cout << controller.get_front() << std::endl;
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

        const uint8_t* key_states = SDL_GetKeyboardState(NULL);

        if (num_joysticks == 0)
        {
            joystick.pitch = joystick.roll = joystick.yaw = 0;
        }

        if (key_states[SDL_SCANCODE_A])
        {
            joystick.roll = -1.0f;
        }
        else if (key_states[SDL_SCANCODE_D])
        {
            joystick.roll = 1.0f;
        }

        if (key_states[SDL_SCANCODE_W])
        {
            joystick.pitch = -1.0f;
        }
        else if (key_states[SDL_SCANCODE_S])
        {
            joystick.pitch = 1.0f;
        }
        
        if (key_states[SDL_SCANCODE_J])
        {
            joystick.throttle -= 0.01f;
            joystick.throttle = phi::utils::clamp(joystick.throttle, 0.0f, 1.0f);
        }
        else if (key_states[SDL_SCANCODE_K])
        {
            joystick.throttle += 0.01f;
            joystick.throttle = phi::utils::clamp(joystick.throttle, 0.0f, 1.0f);
        }

        aircraft.controls.set(joystick.roll, joystick.pitch, joystick.yaw);
        aircraft.engine.throttle = joystick.throttle;
        
        if (!paused)
        {
            aircraft.update(dt);
            // solve_constraints(aircraft.rigid_body);
            apply_to_object3d(aircraft.rigid_body, transform);
        }

        prop.set_rotation(prop.get_rotation() + glm::vec3(0.1f, 0.0f, 0.0f));

        fpm.set_position(glm::normalize(aircraft.rigid_body.get_body_velocity()) * (projection_distance + 1));
       
        if (orbit)
        {
            controller.update(camera, aircraft.rigid_body.position, dt);
            cross.visible = fpm.visible = false;
        }
        else
        {
            //camera.set_position({ -15.0f, 3.0f, 0.0f });
            camera.set_position({ -15.0f, 3.0f + aircraft.rigid_body.angular_velocity.z * 1.0f, 0.0f });
            cross.visible = fpm.visible = true;
        }
        renderer.render(camera, scene);

        SDL_GL_SwapWindow(window);
    }
    return 0;
}
