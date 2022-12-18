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


#define SCR_WIDTH  640
#define SCR_HEIGHT 380

std::ostream& operator<<(std::ostream& os, const glm::vec3 v)
{
	return os << v.x << ", " << v.y << ", " << v.z;
}

int main(void)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow(
        "HELLO",
        40,
        40,
        SCR_WIDTH,
        SCR_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    glewExperimental = GL_TRUE;

    if (GLEW_OK != glewInit())
    {
        std::cout << "FAiled to init\n";
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    
    SDL_Event windowEvent;

	const std::vector<float> cube_vertices = {
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


    gfx::Renderer renderer(SCR_WIDTH, SCR_HEIGHT);


    auto phong = make_shared<gfx::Phong>(gfx::rgb(165, 113, 100)); // bronze
    auto red = make_shared<gfx::Phong>(gfx::rgb(255, 0, 0)); // bronze

    auto cube_geometry = std::make_shared<gfx::Geometry>(cube_vertices, gfx::Geometry::POS_NORM);

    gfx::Camera camera(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    camera.setPosition(glm::vec3(0, 0.5, 2));

  
    gfx::Mesh cube(cube_geometry, phong);
    
    gfx::Mesh ground(cube_geometry, red);
    ground.setScale(glm::vec3(50, 0.5, 50));
    ground.setPosition(glm::vec3(0, -1, 0));
    ground.receiveShadow = true;

    gfx::Light sun(gfx::Light::DIRECTIONAL, gfx::rgb(154, 219, 172));
    sun.setPosition(glm::vec3(0.5f, 2.0f, 2.0f));
    sun.castShadow = true;
    
    gfx::Object3D scene;
    scene.add(&sun);
    scene.add(&camera);
    scene.add(&cube);
    scene.add(&ground);




    bool close = false;
    while (!close)
    {
        if (SDL_PollEvent(&windowEvent))
        {
            if (SDL_QUIT == windowEvent.type)
                close = true;
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        camera.setPosition(camera.getPosition() + glm::vec3(0,0.00001, 0.001));
        std::cout << camera.getPosition() << std::endl;

        camera.lookAt(glm::vec3(0, 0, 0));


        // drawing
        renderer.render(camera, scene);

        SDL_GL_SwapWindow(window);
    }

    return 0;
}
