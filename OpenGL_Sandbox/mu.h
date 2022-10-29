#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// glm
#include <vec3.hpp>
#include <mat4x4.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/euler_angles.hpp >

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

namespace mu {

	struct Shader {
	public:
		unsigned int id;
		Shader(const std::string& vertShaderPath, const std::string& fragShaderPath);
		~Shader();
		void use();
		void setInt(const std::string& name, int value);
		void setFloat(const std::string& name, float value);
		void setVec3(const std::string& name, const glm::vec3& value);
		void setMat4(const std::string& name, const glm::mat4& value);
	};

	struct Camera {
		Camera(float fov, float aspect, float near, float far);
		glm::mat4 view;
		glm::mat4 projection;
	};

	class Object3D {
	public:
		Object3D();

		glm::mat4 transform;
		glm::mat4 worldTransform;
		glm::mat4 localTransform;

		Object3D* parent;
		std::vector<Object3D*> children;

		void addChild(Object3D* child);
		virtual void draw(Camera& camera);

		const glm::vec3& getPosition();
		const glm::vec3& getRotation();

		void setPosition(glm::vec3& pos);
		void setPosition(float x, float y, float z);
		void setRotation(glm::vec3& rot);
		void setRotation(float x, float y, float z);
		void setScale(float x, float y, float z);

	private:
		friend class Renderer;
		bool m_dirty;
		glm::vec3 m_rotation, m_position, m_scale;
		void updateWorldMatrix(bool dirtyParent);
		glm::mat4 getLocalTransform();
	};

	class Geometry {
	public:
		/*
			Vertices must be of the structure:
			{ ...,
			pos.x, pos.y, pos.z, normal.x, normal.y, normal.z,
			..., }
		*/
		Geometry(const std::vector<float>& vertices);
		~Geometry();
		void write(const std::vector<float>& vertices);
		void use();
		int count;

	private:
		unsigned int m_vao;
		unsigned int m_vbo;
	};

	struct Material  {
		Material(const std::string& vertPath, const std::string& fragPath);
		Shader shader;
		glm::vec3 color;
	};

	class Mesh : public Object3D {
	public:
		Mesh(Geometry& geometry, Material& material)
			: m_geometry(geometry), m_material(material)
		{}

		void draw(Camera& camera);

	private:
		Geometry m_geometry;
		Material m_material;
	};

	class Renderer {
	public:
		Renderer(GLFWwindow* window) : m_window(window) {}
		void render(Camera& camera, Object3D& scene);
	private:
		GLFWwindow* m_window;
	};
};


