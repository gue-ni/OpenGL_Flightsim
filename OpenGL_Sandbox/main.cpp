#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vec3.hpp>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

#include "gfx.h"

using std::shared_ptr;
using std::make_shared;

#define PRESSED(key) (glfwGetKey(window, key) == GLFW_PRESS)

constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 720;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
std::vector<float> invert_normals(const std::vector<float> vertices);

std::ostream& operator<<(std::ostream& os, const glm::vec3 v)
{
	return os << v.x << ", " << v.y << ", " << v.z;
}

struct FPS_Controller {
    bool initialized = false;
    glm::vec2 last_pos;
    float yaw, pitch;
    glm::vec3 front, velocity, up;

    FPS_Controller() 
        : initialized(false), last_pos(0.0f), yaw(-90.0f), pitch(0.0f), front(0,0,-1), velocity(0.0f), up(0.0f,1.0f,0.0f)
    {}
};

FPS_Controller fps;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Sandbox", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    //if (!gladLoadGLLoader(static_cast<GLADloadproc>(glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    glEnable(GL_DEPTH_TEST);

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

    const std::vector<float> plane_vertices = {
        // positions            // normals         // texcoords
     0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.5f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 0.5f,

     0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.5f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 0.5f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.5f, 0.5f
    };

    gfx::Renderer renderer(window, SCR_WIDTH, SCR_HEIGHT);

    auto basic  = make_shared<gfx::Basic>(gfx::color(0xffffff));
    auto red    = make_shared<gfx::Phong>(gfx::color(0xff00ff));

    auto phong1 = make_shared<gfx::Phong>(gfx::color(165, 113, 100)); // bronze
    auto phong2 = make_shared<gfx::Phong>(gfx::color(0x00ff00));
    auto phong3 = make_shared<gfx::Phong>(gfx::color(0x00f0f0));

    auto cube       = make_shared<gfx::Geometry>(cube_vertices, gfx::Geometry::POS_NORM);
    auto plane      = make_shared<gfx::Geometry>(cube_vertices, gfx::Geometry::POS_NORM);
    auto inv_cube   = make_shared<gfx::Geometry>(invert_normals(cube_vertices), gfx::Geometry::POS_NORM);

    gfx::Camera camera(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    camera.setPosition(glm::vec3(0, 1, 7));

    gfx::Mesh skybox(inv_cube, red);
    skybox.setScale(glm::vec3(50.0f));

    gfx::Mesh big_cube(cube, phong1);
    big_cube.setPosition(glm::vec3(0, 1, 0));

    gfx::Mesh plane_mesh(plane, phong2);
    plane_mesh.setPosition(glm::vec3(5.0f, 2.5f, 0.0f));
    plane_mesh.setScale(glm::vec3(2.25f));

    gfx::Mesh ground(cube, phong3);
    ground.setPosition(glm::vec3(0.0f, -1.0f, 0.0f));
    ground.setScale(glm::vec3(50, 0.1, 50));

    gfx::Mesh light_cube(cube, basic);
    light_cube.setScale(glm::vec3(0.25));

    glm::vec3 lightPos(-2, 4, -1);
    gfx::Light sun(gfx::Light::DIRECTIONAL, gfx::color(154, 219, 172), glm::normalize(glm::vec3(0) - lightPos));
    
    gfx::Object3D scene;
    scene.add(&sun);
    scene.add(&camera);
    scene.add(&ground);
    scene.add(&big_cube);
    scene.add(&plane_mesh);

    int frames = 0;
    double currentTime, previousTime = 0;

    while (!glfwWindowShouldClose(window))
    {
#if 1
        currentTime = glfwGetTime();
        frames++;
        if (currentTime - previousTime >= 1.0)
        {
            std::cout
                << 1000.0 / static_cast<double>(frames)
                << " ms/frame\n";

            frames = 0;
            previousTime = currentTime;
        }
#endif

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float t = 0.001f;
        big_cube.setRotation(big_cube.getRotation() + glm::vec3(t, 0, t));
        plane_mesh.setRotation(plane_mesh.getRotation() + glm::vec3(0, t, t));

        camera.setPosition(camera.getPosition() + fps.velocity);
        camera.overrideTransform(glm::inverse(glm::lookAt(camera.getPosition(), camera.getPosition() + fps.front, fps.up)));

        renderer.render(camera, scene);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    const float speed = 0.05;
    fps.velocity = glm::vec3(0);

    if (PRESSED(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    if (PRESSED(GLFW_KEY_W))
        fps.velocity = (speed * fps.front);

    if (PRESSED(GLFW_KEY_S))
        fps.velocity = -(speed * fps.front);

    if (PRESSED(GLFW_KEY_A))
        fps.velocity = -(speed * glm::normalize(glm::cross(fps.front, fps.up)));

    if (PRESSED(GLFW_KEY_D))
        fps.velocity = (speed * glm::normalize(glm::cross(fps.front, fps.up)));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!fps.initialized) 
    {
        fps.last_pos.x = xpos;
        fps.last_pos.y = ypos;
        fps.initialized = true;
    }

    glm::vec2 pos(xpos, ypos);

    glm::vec2 offset(
        fps.last_pos.x - pos.x, 
        fps.last_pos.y - pos.y
    );

    fps.last_pos = pos;

    const float sensitivity = 0.05f;

    offset      *= sensitivity;
    fps.yaw     -= offset.x;
    fps.pitch   += offset.y;

    glm::vec3 front(0);
    front.x = cos(glm::radians(fps.yaw)) * cos(glm::radians(fps.pitch));
    front.y = sin(glm::radians(fps.pitch));
    front.z = sin(glm::radians(fps.yaw)) * cos(glm::radians(fps.pitch));

    fps.front = glm::normalize(front);
}

std::vector<float> invert_normals(const std::vector<float> vertices)
{
    std::vector<float> inverted;

    std::copy(vertices.begin(), vertices.end(), std::back_inserter(inverted));

    int stride = 6;
    for (int i = 0; i < vertices.size(); i += stride)
    {
        for (int j = 3; j < 6; j++)
        {
            inverted[i + j] = -inverted[i + j];
        }
    }

    return inverted;
}


