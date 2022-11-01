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

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

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

    auto red = std::make_shared<mu::Phong>(mu::color(0x000000));

    auto phong2 = std::make_shared<mu::Phong>(mu::color(0x00ff00));;

    auto phong3 = std::make_shared<mu::Phong>(mu::color(0xff00ff));;

    auto cube = std::make_shared<mu::Geometry>(cube_vertices, mu::Geometry::POS_NORM);

    mu::Camera camera(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100000.0f);
    camera.setPosition(glm::vec3(0, 1, 7));

    mu::Mesh skybox(cube, red);
    skybox.setScale(glm::vec3(5000.0f));

    mu::Mesh mesh1(cube, phong1);
    mesh1.setPosition(glm::vec3(0, 1, 0));

    mu::Mesh mesh2(cube, phong2);
    mesh2.setPosition(glm::vec3(0.0f, 1.5f, 0.0f));
    mesh2.setScale(glm::vec3(0.25f));

    mu::Mesh mesh3(cube, phong3);
    mesh3.setPosition(glm::vec3(0.0f, -1.0f, 0.0f));
    mesh3.setScale(glm::vec3(15, 0.1, 15));

    mu::Mesh light(cube, basic);
    light.setPosition(glm::vec3(1.2, 1.0f, 2.0f));
    light.setScale(glm::vec3(0.25));

    mu::Object3D scene;
    scene.addChild(&skybox);
    scene.addChild(&camera);
    scene.addChild(&light);
    scene.addChild(&mesh1);
    mesh1.addChild(&mesh2);
    scene.addChild(&mesh3);

    glm::vec3 A(0.0f, 1.0f, 0.0f), B(0.0f), C(2.0f, 0.0f, 0.0f);

    float distacne = glm::length(glm::cross(A - B, C - B)) / glm::length(C - B);
    std::cout << "distance = " << distacne << std::endl;


    int frames = 0;
    double currentTime, previousTime = 0;

    while (!glfwWindowShouldClose(window))
    {
#if 0
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

        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float t = 0.0005f;
        mesh1.setRotation(mesh1.getRotation() + glm::vec3(t, 0, t));
        mesh2.setRotation(mesh2.getRotation() + glm::vec3(0, t, t));

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
