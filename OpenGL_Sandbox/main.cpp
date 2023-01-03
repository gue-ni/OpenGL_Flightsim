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

using std::shared_ptr;
using std::make_shared;

std::ostream& operator<<(std::ostream& os, const glm::vec3& v)
{
	return os << v.x << ", " << v.y << ", " << v.z;
}

constexpr auto SCREEN_WIDTH = 800;
constexpr auto SCREEN_HEIGHT = 600;

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

#if 0
	const std::vector<float> cube_vertices = {
        // position           // normals
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};
#endif

    std::vector<float> ico_vertices;
    std::vector<float> cube_vertices;

    gfx::load_obj("assets/cube.obj", cube_vertices);
    gfx::load_obj("assets/icosphere.obj", ico_vertices);

    gfx::Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);

    auto blue   = make_shared<gfx::Phong>(gfx::rgb(0, 0, 255));
    auto red    = make_shared<gfx::Phong>(gfx::rgb(255, 0, 0));
    auto cube_geo   = std::make_shared<gfx::Geometry>(cube_vertices, gfx::Geometry::POS_NORM);
    auto ico_geo    = std::make_shared<gfx::Geometry>(ico_vertices, gfx::Geometry::POS_NORM);


    gfx::Object3D scene;

    gfx::Camera camera(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
    camera.set_position(glm::vec3(0, 1, 3));
    scene.add(&camera);
  
    gfx::Mesh icosphere(ico_geo, blue);
    icosphere.set_position(glm::vec3(0, 1.0, 0));
    scene.add(&icosphere);

    gfx::Mesh cube(cube_geo, blue);
    cube.set_position(glm::vec3(0.0f, 0.0f, -3.0f));
    cube.set_scale(glm::vec3(0.25f));
    icosphere.add(&cube);
    
    gfx::Mesh ground(cube_geo, red);
    ground.set_scale(glm::vec3(10, 0.5, 10));
    ground.set_position(glm::vec3(0, -1, 0));
    ground.receive_shadow = true;
    scene.add(&ground);

    gfx::Light sun(gfx::Light::DIRECTIONAL, gfx::rgb(154, 219, 172));
    sun.set_position(glm::vec3(0.5f, 2.0f, 2.0f));
    sun.cast_shadow = true;
    scene.add(&sun);

    gfx::Controller controller(0.025f);

    SDL_Event event;
    bool quit = false;
    uint64_t last = 0, now = 0;
    float dt;

    std::cout << glGetString(GL_VERSION) << std::endl;

    while (!quit)
    {
        // delta time
        last = now;
        now = SDL_GetPerformanceCounter();
        dt = static_cast<float>((now - last) * 1000 / static_cast<float>(SDL_GetPerformanceFrequency()));
#if 0
        std::cout << "FPS: " << 1000 / dt << std::endl;
#endif

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

         // rendering
        icosphere.set_rotation(icosphere.get_rotation() + glm::vec3(1.0f, 0.0f, 1.0f) * 0.001f);
        controller.update(camera, dt);
        renderer.render(camera, scene);

        // swap window
        SDL_GL_SwapWindow(window);
    }

    return 0;
}
