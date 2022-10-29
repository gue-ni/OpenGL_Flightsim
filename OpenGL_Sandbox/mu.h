#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// glm
#include <vec3.hpp>
#include <mat4x4.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

namespace mu {

	class Shader {
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

	class Camera {
	public:
		Camera();
		glm::mat4 view;
		glm::mat4 projection;
	};

	class Object3D {
	public:
		Object3D();

		bool dirty;
		glm::mat4 transform;
		glm::mat4 worldTransform;

	private:
		friend class Renderer;
		virtual void draw(Camera& camera);
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

	class Material  {
	public:
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


