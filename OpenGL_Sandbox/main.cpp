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

using std::shared_ptr;
using std::make_shared;

std::ostream& operator<<(std::ostream& os, const glm::vec3& v)
{
	return os << v.x << ", " << v.y << ", " << v.z;
}

constexpr auto SCREEN_WIDTH = 800;
constexpr auto SCREEN_HEIGHT = 600;

void apply_physics(const phi::RigidBody3D& rigid_body, gfx::Object3D& object3d)
{
    object3d.set_position(rigid_body.position);
}

void apply_constraints(phi::RigidBody3D& rigid_body)
{
	if (rigid_body.position.y <= 0)
	{
		rigid_body.apply_gravity = false;
		rigid_body.position.y = 0;
		rigid_body.velocity = glm::vec3(0.0f);
	}
}

int main(void)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow(
        "OpenGL/SDL Sandbox", 
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

    // SDL options
    SDL_ShowCursor(SDL_FALSE);
    SDL_CaptureMouse(SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

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

    gfx::load_obj("assets/cube.obj", cube_vertices);
    gfx::load_obj("assets/icosphere.obj", ico_vertices);

    gfx::Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);

    auto blue   = make_shared<gfx::Phong>(gfx::rgb(0, 0, 255));
    auto red    = make_shared<gfx::Phong>(gfx::rgb(255, 0, 0));
    auto green    = make_shared<gfx::Phong>(gfx::rgb(0, 255, 0));
    auto test_texture = make_shared<gfx::Phong>(make_shared<gfx::Texture>("assets/uv-test.png"));
    auto container = make_shared<gfx::Phong>(make_shared<gfx::Texture>("assets/container.jpg"));
    auto cube_geo   = std::make_shared<gfx::Geometry>(cube_vertices_2, gfx::Geometry::POS_NORM_UV);
    auto ico_geo    = std::make_shared<gfx::Geometry>(ico_vertices, gfx::Geometry::POS_NORM_UV);
    auto triangle    = std::make_shared<gfx::Geometry>(triangle_vertices, gfx::Geometry::POS_NORM_UV);

    gfx::Object3D scene;

    gfx::Camera camera(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
    camera.set_position(glm::vec3(0, 1, 7));
    scene.add(&camera);


#if 1
    gfx::Skybox skybox({
        "assets/skybox/hd/right.jpg",
        "assets/skybox/hd/left.jpg",
        "assets/skybox/hd/top.jpg",
        "assets/skybox/hd/bottom.jpg",
        "assets/skybox/hd/front.jpg",
        "assets/skybox/hd/back.jpg",
    });
    scene.add(&skybox);
#endif
  
    gfx::Mesh icosphere(ico_geo, blue);
    icosphere.set_position(glm::vec3(0, 1.0, 0));
    scene.add(&icosphere);

    gfx::Mesh cube(cube_geo, container);
    cube.set_position(glm::vec3(0.0f, 50.0f, -5.0f));
    cube.set_scale(glm::vec3(1.0f));
    scene.add(&cube);
    
    gfx::Mesh ground(cube_geo, test_texture);
    ground.set_scale(glm::vec3(20, 0.5, 20));
    ground.set_position(glm::vec3(0, -1, 0));
    ground.receive_shadow = true;
    scene.add(&ground);

    gfx::Light pointlight(gfx::Light::POINT, gfx::RGB(1.0f));
    //pointlight.set_position(glm::vec3(-4, 2, 5));
    icosphere.add(&pointlight);

#if 1
    gfx::Light sun(gfx::Light::DIRECTIONAL, gfx::rgb(154, 219, 172));
    sun.set_position(glm::vec3(0.5f, 2.0f, 2.0f));
    sun.cast_shadow = true;
    scene.add(&sun);
#endif

    gfx::Controller controller(25.0f);

    SDL_Event event;
    bool quit = false;
    uint64_t last = 0, now = SDL_GetPerformanceCounter();
    gfx::Seconds dt, timer = 0;

    std::cout << glGetString(GL_VERSION) << std::endl;

    phi::RigidBody3D rigid_body(cube.get_position(), cube.get_rotation(), 20);

    while (!quit)
    {
        // delta time in seconds
        last = now;
        now = SDL_GetPerformanceCounter();
        dt = static_cast<gfx::Seconds>((now - last) / static_cast<gfx::Seconds>(SDL_GetPerformanceFrequency()));

        if ((timer += dt) >= 1.0f)
        {
            printf("dt = %f, fps = %f\n", dt, 1 / dt);
            timer = 0.0f;
        }

        // user input
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
                }
                default:
                    break;
                }
                break;
            }
            }
        }

        const uint8_t* key_states = SDL_GetKeyboardState(NULL);
        if (key_states[SDL_SCANCODE_W]) controller.move(gfx::Controller::FORWARD);
        if (key_states[SDL_SCANCODE_A]) controller.move(gfx::Controller::LEFT);
        if (key_states[SDL_SCANCODE_S]) controller.move(gfx::Controller::BACKWARD);
        if (key_states[SDL_SCANCODE_D]) controller.move(gfx::Controller::RIGHT);


        rigid_body.update(dt);

        apply_constraints(rigid_body);
        apply_physics(rigid_body, cube);


         // rendering
        icosphere.set_rotation(icosphere.get_rotation() + glm::vec3(1.0f, 0.0f, 1.0f) * 0.001f);
        controller.update(camera, dt);
        renderer.render(camera, scene);

        // swap window
        SDL_GL_SwapWindow(window);
    }

    return 0;
}
