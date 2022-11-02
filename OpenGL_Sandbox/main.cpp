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

#include "mu.h"

#define PRESSED(key) (glfwGetKey(window, key) == GLFW_PRESS)

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 720;

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

    mu::Renderer renderer(window);

    auto phong1 = std::make_shared<mu::Phong>(mu::color(165, 113, 100)); // bronze

    auto basic = std::make_shared<mu::Basic>(mu::color(0xffffff));

    auto red = std::make_shared<mu::Phong>(mu::color(0xff00ff));

    auto phong2 = std::make_shared<mu::Phong>(mu::color(0x00ff00));

    auto phong3 = std::make_shared<mu::Phong>(mu::color(0x00f0f0));

    auto cube = std::make_shared<mu::Geometry>(cube_vertices, mu::Geometry::POS_NORM);

    auto inv_cube = std::make_shared<mu::Geometry>(invert_normals(cube_vertices), mu::Geometry::POS_NORM);

    mu::Camera camera(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100000.0f);
    camera.setPosition(glm::vec3(0, 1, 7));

    mu::Mesh skybox(inv_cube, red);
    skybox.setScale(glm::vec3(50.0f));

    mu::Mesh big_cube(cube, phong1);
    big_cube.setPosition(glm::vec3(0, 1, 0));

    mu::Mesh small_cube(cube, phong2);
    small_cube.setPosition(glm::vec3(5.0f, 2.5f, 0.0f));
    small_cube.setScale(glm::vec3(2.25f));

    mu::Mesh ground(cube, phong3);
    ground.setPosition(glm::vec3(0.0f, -1.0f, 0.0f));
    ground.setScale(glm::vec3(50, 0.1, 50));

    mu::Mesh light_cube(cube, basic);
    light_cube.setScale(glm::vec3(0.25));

    mu::Light light(mu::color(154, 219, 172));
    light.setPosition(glm::vec3(0.0, 1.6f, 0.0f));

    mu::Light sun(mu::Light::DIRECTIONAL, mu::color(154, 219, 172), glm::normalize(glm::vec3(1,-1,1)));

    
    mu::Object3D scene;
    scene.add(&skybox);
    scene.add(&camera);
    scene.add(&sun);
    scene.add(&ground);
    scene.add(&big_cube);
    scene.add(&small_cube);


    //big_cube.add(&light);
    //light.add(&light_cube);

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


            std::cout << "local: " << light.getPosition() << std::endl;
            std::cout << "world: " << light.getWorldPosition() << std::endl;

        }
#endif

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        const float t = 0.001f;
        big_cube.setRotation(big_cube.getRotation() + glm::vec3(t, 0, t));
        small_cube.setRotation(small_cube.getRotation() + glm::vec3(0, t, t));

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
    const float speed = 0.01;
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
