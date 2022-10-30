#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vec3.hpp>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "mu.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

constexpr glm::vec3 color(int r, int g, int b)
{
    return glm::vec3(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b)) / 255.0f;
}

constexpr glm::vec3 color(uint32_t hex)
{
    assert(hex <= 0xffffffU);
    return glm::vec3(
        static_cast<float>((hex & 0xff0000U) >> 16) / 255.0f, 
        static_cast<float>((hex & 0x00ff00U) >>  8) / 255.0f, 
        static_cast<float>((hex & 0x0000ffU) >>  0) / 255.0f
    );
}

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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    glEnable(GL_DEPTH_TEST);

	const std::vector<float> cube = {
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

    std::shared_ptr<mu::Phong> phong1
        = std::make_shared<mu::Phong>(color(165, 113, 100));

    std::shared_ptr<mu::Basic> basic
        = std::make_shared<mu::Basic>(color(0x00ff00));

    std::shared_ptr<mu::Phong> phong2 
        = std::make_shared<mu::Phong>(color(0x0000ff));;

    std::shared_ptr<mu::Phong> phong3
        = std::make_shared<mu::Phong>(color(0xff00ff));;

    std::shared_ptr<mu::Geometry> geometry 
        = std::make_shared<mu::Geometry>(cube, mu::Geometry::POS_NORM);

    mu::Camera camera(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    camera.setPosition(glm::vec3(0, 1, 7));

    mu::Mesh mesh1(geometry, phong1);
    mesh1.setPosition(glm::vec3(0, 1, 0));

    mu::Mesh mesh2(geometry, phong2);
    mesh2.setPosition(glm::vec3(0.0f, 1.5f, 0.0f));
    mesh2.setScale(glm::vec3(0.25f));

    mu::Mesh mesh3(geometry, phong3);
    mesh3.setPosition(glm::vec3(0.0f, -1.0f, 0.0f));
    mesh3.setScale(glm::vec3(15, 0.1, 15));

    mu::Mesh light(geometry, basic);
    light.setPosition(glm::vec3(1.2, 1.0f, 2.0f));
    light.setScale(glm::vec3(0.25));

    mu::Object3D scene;
    scene.addChild(&camera);
    scene.addChild(&light);
    scene.addChild(&mesh1);
    mesh1.addChild(&mesh2);
    scene.addChild(&mesh3);

    int frames = 0;
    double currentTime, previousTime = 0;

    while (!glfwWindowShouldClose(window))
    {
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

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float t = 0.0005f;
        mesh1.setRotation(mesh1.getRotation() + glm::vec3(t, 0, t));
        mesh2.setRotation(mesh2.getRotation() + glm::vec3(0, t, t));

        renderer.render(camera, scene);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}