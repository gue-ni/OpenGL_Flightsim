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
std::vector<float> invert_normals(const std::vector<float> vertices);
std::vector<float> generatePlane(int width, int height, float widthSegments, float heightSegments);

std::ostream& operator<<(std::ostream& os, const glm::vec3 v)
{
	return os << v.x << ", " << v.y << ", " << v.z;
}

class FPS_Controller {
public:
    
    FPS_Controller() 
        : m_initialized(false), m_last_pos(0.0f), m_yaw(-90.0f), m_pitch(0.0f), m_front(0,0,-1), m_velocity(0.0f), m_up(0.0f,1.0f,0.0f)
    {}

    void mouseCallback(GLFWwindow* window, double xpos, double ypos)
    {
		if (!m_initialized) 
		{
			m_last_pos.x = xpos;
			m_last_pos.y = ypos;
			m_initialized = true;
		}

		glm::vec2 pos(xpos, ypos);

		glm::vec2 offset(
			m_last_pos.x - pos.x, 
			m_last_pos.y - pos.y
		);

		m_last_pos = pos;

		const float sensitivity = 0.05f;

		offset      *= sensitivity;
		m_yaw     -= offset.x;
		m_pitch   += offset.y;

		glm::vec3 front_(0);
		front_.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		front_.y = sin(glm::radians(m_pitch));
		front_.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

		m_front = glm::normalize(front_);
    }

    void update(GLFWwindow* window, gfx::Object3D& target)
    {
		const float speed = 0.025;
		m_velocity = glm::vec3(0);

		if (PRESSED(GLFW_KEY_W))
		    m_velocity = (speed * m_front);

		if (PRESSED(GLFW_KEY_S))
			m_velocity = -(speed * m_front);

		if (PRESSED(GLFW_KEY_A))
			m_velocity = -(speed * glm::normalize(glm::cross(m_front, m_up)));

		if (PRESSED(GLFW_KEY_D))
			m_velocity = (speed * glm::normalize(glm::cross(m_front, m_up)));

        target.setPosition(target.getPosition() + m_velocity);
        target.overrideTransform(glm::inverse(glm::lookAt(
            target.getPosition(), 
            target.getPosition() + m_front, 
            m_up
        )));
    }
    
private:
    glm::vec3 m_front, m_velocity, m_up;
    bool m_initialized;
    glm::vec2 m_last_pos;
    float m_yaw, m_pitch;

};

FPS_Controller controller;

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

    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
        controller.mouseCallback(w, x, y);
    });

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

    const std::vector<float> plane_vertices = {
        // positions            // normals         // texcoords
     0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.5f,  0.0f, // lower right
    -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f, // 
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 0.5f,

     0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.5f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 0.5f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.5f, 0.5f
    };

    gfx::Renderer renderer(window, SCR_WIDTH, SCR_HEIGHT);

    auto basic  = make_shared<gfx::Basic>(gfx::rgb(0xffffff));
    auto red    = make_shared<gfx::Phong>(renderer.background);

    auto phong1 = make_shared<gfx::Phong>(gfx::rgb(165, 113, 100)); // bronze
    auto phong2 = make_shared<gfx::Phong>(gfx::rgb(0x00ff00));
    auto phong3 = make_shared<gfx::Phong>(gfx::rgb(0x00f0f0));

    auto cube_geometry       = make_shared<gfx::Geometry>(cube_vertices, gfx::Geometry::POS_NORM);
    auto plane_geometry      = make_shared<gfx::Geometry>(generatePlane(2,3,1,1), gfx::Geometry::POS_NORM);
    auto inv_cube            = make_shared<gfx::Geometry>(invert_normals(cube_vertices), gfx::Geometry::POS_NORM);

    gfx::Camera camera(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

    /*
    float xz = 15.0f, y = 15.0f;
    auto isometric_offset = glm::vec3(xz, y, xz);
    auto isometric_rotation = glm::vec3(glm::radians(45.0f), glm::radians(-35.264f), 0);
    camera.setPosition(isometric_offset);
    camera.setRotation(isometric_rotation);
    */
   
    auto isometric = glm::mat4(1.0f);
    isometric = glm::translate(isometric, glm::vec3(10, 10, 10));
    isometric = glm::rotate(isometric, glm::radians(-35.0f), glm::vec3(1, 0, 0));
    isometric = glm::rotate(isometric, glm::radians(45.0f), glm::vec3(0, 1, 0));
    camera.overrideTransform(isometric);


    gfx::Mesh skybox(inv_cube, red);
    skybox.setScale(glm::vec3(50.0f));

    gfx::Mesh cube(cube_geometry, phong1);
    cube.setPosition(glm::vec3(5, 1, 5));
    cube.setScale(glm::vec3(2, 0.5, 1));
    cube.receiveShadow = false;

    gfx::Mesh plane(cube_geometry, phong2);
    plane.setPosition(glm::vec3(0.0f, 2.0f, -2.0f));
    plane.setScale(glm::vec3(2.25f));

    gfx::Mesh ground(cube_geometry, phong3);
    ground.setPosition(glm::vec3(0.0f, -1.0f, 0.0f));
    ground.setScale(glm::vec3(20, 1, 20));

    gfx::Light sun(gfx::Light::DIRECTIONAL, gfx::rgb(154, 219, 172));
    sun.setPosition(glm::vec3(0.5f, 2.0f, 2.0f));
    sun.castShadow = true;
    
    gfx::Object3D scene;
    scene.add(&sun);
    scene.add(&camera);
    scene.add(&ground);
    scene.add(&cube);
    scene.add(&plane);
    //scene.add(&skybox);

    int frames = 0;
    double currentTime, previousTime = 0;

    while (!glfwWindowShouldClose(window))
    {
#if 1
        currentTime = glfwGetTime();
        frames++;
        if (currentTime - previousTime >= 1.0)
        {
            std::cout << 1000.0 / static_cast<double>(frames) << " ms/frame\n";
            frames = 0;
            previousTime = currentTime;
        }
#endif

		if (PRESSED(GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(window, true);


        glClearColor(renderer.background.r, renderer.background.g, renderer.background.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float t = 0.001f;
        cube.setRotation(cube.getRotation() + glm::vec3(t, 0, 0));
        plane.setRotation(plane.getRotation() + glm::vec3(0, t, t));

        controller.update(window, camera);
        renderer.render(camera, scene);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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

std::vector<float> generatePlane(int width, int height, float widthSegments, float heightSegments)
{
    std::vector<float> vertices;

    auto push = [&vertices](glm::vec3 v) {
        vertices.push_back(v.x);
        vertices.push_back(v.y);
        vertices.push_back(v.z);
    };

    auto normal = [](glm::vec3 a, glm::vec3 b, glm::vec3 c) -> glm::vec3 {
        glm::vec3 v0 = a - c;
        glm::vec3 v1 = b - c;
        return glm::normalize(cross(v0, v1));
    };

    for (int y = 0; y < height; y++)
    {
		for (int x = 0; x < width; x++)
		{
            glm::vec3 v0((x+0) * widthSegments, (y+0) * heightSegments, 0);     // lower left
            glm::vec3 v1((x+0) * widthSegments, (y+1) * heightSegments, 0);     // upper left
            glm::vec3 v2((x+1) * widthSegments, (y+1) * heightSegments, 0);     // upper right
            glm::vec3 v3((x+1) * widthSegments, (y+0) * heightSegments, 0);     // lower right

            auto n1 = normal(v0, v3, v2);

            push(v0);
            push(n1);
            push(v3);
            push(n1);
            push(v2);
            push(n1);
        
            auto n2 = normal(v2, v1, v0);

            push(v2);
            push(n2);
            push(v1);
            push(n2);
            push(v0);
            push(n2);
		}
    }
   
    return vertices;
}

