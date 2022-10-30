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
	const std::string phong_vert = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    gl_Position = proj * view * vec4(FragPos, 1.0);
}
	)";
	const std::string phong_frag = R"(
#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
  
uniform vec3 camera; 

uniform vec3 lightPos; 
uniform vec3 lightColor;

uniform vec3 objectColor;

// phong parameters
uniform float ka;
uniform float kd;
uniform float ks;
uniform float alpha;

void main()
{
    // ambient
    vec3 ambient = ka * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 diffuse = kd * max(dot(norm, lightDir), 0.0) * lightColor;
    
    // specular
    vec3 viewDir = normalize(camera - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    vec3 specular = ks * pow(max(dot(viewDir, reflectDir), 0.0), alpha) * lightColor;
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
	)";

	const std::string basic_vert = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	gl_Position = proj* view * model * vec4(aPos, 1.0f);
}
	)";
	const std::string basic_frag = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 objectColor;

void main()
{
	FragColor = vec4(objectColor.rgb, 1.0f);
}
)";

	class Camera;

	struct Shader {
	public:
		unsigned int id;
		Shader(const std::string& path);
		Shader(const std::string& vertShader, const std::string& fragShader);
		~Shader();
		void use();
		void setInt(const std::string& name, int value);
		void setFloat(const std::string& name, float value);
		void setVec3(const std::string& name, const glm::vec3& value);
		void setMat4(const std::string& name, const glm::mat4& value);
	};

	struct ShadowMap {};

	class Object3D {
	public:
		Object3D()
			: m_dirty(true),
			transform(1.0), 
			parent(nullptr),
			m_position(0.0f),
			m_rotation(0.0f),
			m_scale(1.0f)
		{}

		glm::mat4 transform;

		Object3D* parent;
		std::vector<Object3D*> children;

		void addChild(Object3D* child);
		virtual void draw(Camera& camera);

		const glm::vec3& getPosition();
		const glm::vec3& getRotation();
		const glm::vec3& getScale();

		void setPosition(const glm::vec3& pos);
		void setPosition(float x, float y, float z);
		void setRotation(const glm::vec3& rot);
		void setRotation(float x, float y, float z);
		void setScale(float x, float y, float z);
		void setScale(const glm::vec3& scale);

	protected:
		friend class Renderer;
		bool m_dirty;
		glm::vec3 m_rotation, m_position, m_scale; // relative to parent
		void updateWorldMatrix(bool dirtyParent);
		glm::mat4 getLocalTransform();
	};

	class Camera : public Object3D  {
	public:
		Camera(float fov, float aspect, float near, float far)
			: m_projection(glm::perspective(fov, aspect, near, far)), 
			m_up(0.0f, 1.0f, 0.0f),
			m_front(0,0,1)
		{}

		glm::mat4 getViewMatrix();
		glm::mat4 getProjectionMatrix();
	private:
		glm::mat4 m_projection;
		glm::vec3 m_up, m_front;
	};

	class Geometry {
	public:
		enum VertexLayout {
			POS,			// pos
			POS_UV,			// pos, uv
			POS_NORM,		// pos, normal
			POS_NORM_UV		// pos, normal, uv
		};

		Geometry(const std::vector<float>& vertices, const VertexLayout& layout);
		Geometry(const Geometry& geometry);
		~Geometry();
		void use();
		void write(const std::vector<float>& vertices);
		int count;

	private:
		unsigned int m_vao;
		unsigned int m_vbo;
		static int getStride(const VertexLayout& layout);
	};

	class Material  {
	public:
		Material(const std::string& vert, const std::string& frag)
			: Material(vert, frag, glm::vec3(1, 0.5, 0.2), 0.1f, 1.0f, 0.5f, 10.0f) {}

		Material(const std::string& vert, const std::string& frag, const glm::vec3& color_)
			: Material(vert, frag, color_, 0.1f, 1.0f, 0.5f, 10.0f) {}

		Material(const std::string& vert, const std::string& frag,
				const glm::vec3& color_, float ka_, float kd_, float ks_, float alpha_)
			: shader(vert, frag), color(color_), ka(ka_), kd(kd_), ks(ks_), alpha(alpha_) {}

		Shader shader;
		glm::vec3 color;
		float ka, kd, ks, alpha;
	};

	template<class Derived>
	class MaterialX : public Material {
	public:
		MaterialX(const glm::vec3& color_, float ka_, float kd_, float ks_, float alpha_)
			: Material(phong_vert, phong_frag, color_, ka_, kd_, ks_, alpha_) 
		{
			if (staticShader == nullptr)
			{
				std::cout << "create static shader\n";
				staticShader = std::make_shared<Shader>(phong_vert, phong_frag);
			}
		}

		MaterialX(const glm::vec3& color_)
			: MaterialX<Derived>(color_, 0.2f, 1.0f, 0.5f, 10.0f) {}
		
		static std::shared_ptr<Shader> staticShader;
	};

	class Phong : public MaterialX<Phong> {
	public:
		Phong(const glm::vec3& color_) : MaterialX<Phong>(color_, 0.2f, 1.0f, 0.5f, 10.0f) {}
	};

	class Basic : public MaterialX<Basic> {
	public:
		Basic(const glm::vec3& color_) : MaterialX<Basic>(color_){}
	};

	class Mesh : public Object3D {
	public:
		Mesh(std::shared_ptr<Geometry> geometry, std::shared_ptr<Material> material)
			: m_geometry(geometry), m_material(material) {}
		void draw(Camera& camera);
	private:
		std::shared_ptr<Geometry> m_geometry;
		std::shared_ptr<Material> m_material;
	};

	class Renderer {
	public:
		Renderer(GLFWwindow* window) : m_window(window) {}
		void render(Camera& camera, Object3D& scene);
	private:
		GLFWwindow* m_window;
	};
};


