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
#include <functional>

constexpr float PI = 3.14159265359f;

namespace gfx {
	const std::string phong_vert = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 lightSpaceMatrix;

void main()
{
	TexCoords = aTexCoord;

    FragPos = vec3(model * vec4(aPos, 1.0));

    Normal = mat3(transpose(inverse(model))) * aNormal;  

    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    gl_Position = proj * view * vec4(FragPos, 1.0);
}
	)";
	const std::string phong_frag = R"(
#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;
in vec4 FragPosLightSpace;
  
uniform vec3 cameraPos; 

uniform vec3 objectColor;

// phong lighting parameters
uniform float ka;
uniform float kd;
uniform float ks;
uniform float alpha;

uniform sampler2D shadowMap;

struct Light {
	int type; // POINT = 0, DIRECTIONAL = 1
	vec3 color;
	vec3 position;  
};

#define MAX_LIGHTS 4
uniform int numLights;
uniform int receiveShadow;
uniform Light lights[MAX_LIGHTS];

float calculateAttenuation(float constant, float linear, float quadratic, float distance)
{
	return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}

float calculateShadow(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r; 

    float currentDepth = projCoords.z;

	float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}

vec3 calculateDirLight(Light light)
{
	vec3 direction = light.position;
	//vec3 color = vec3(texture(shadowMap, TexCoords.xy));
	vec3 color = objectColor;

    // ambient
    vec3 ambient = ka * light.color;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(direction);
    vec3 diffuse = kd * max(dot(norm, lightDir), 0.0) * light.color;
    
    // specular
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    vec3 specular = ks * pow(max(dot(viewDir, reflectDir), 0.0), alpha) * light.color;

	float shadow = 0.0;
	if (receiveShadow == 1)
	{
		shadow = calculateShadow(FragPosLightSpace);       
	}

    return (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
}

vec3 calculatePointLight(Light light)
{
	vec3 result;
	vec3 position = light.position;

    // ambient
    vec3 ambient = ka * light.color;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(position - FragPos);
    vec3 diffuse = kd * max(dot(norm, lightDir), 0.0) * light.color;
    
    // specular
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    vec3 specular = ks * pow(max(dot(viewDir, reflectDir), 0.0), alpha) * light.color;

	// attenuation
	float constant		= 1.0;
	float linear		= 0.09;
	float quadratic		= 0.032;
	float distance		= length(position - FragPos);
	float attenuation	= calculateAttenuation(constant, linear, quadratic, distance);  
	
    result += (ambient + diffuse + specular) * attenuation * objectColor;

#if 0
	// https://ijdykeman.github.io/graphics/simple_fog_shader
	vec3 cameraDir = cameraPos - FragPos;
	vec3 cameraDir = -viewDir;
	float b = length(light.position - cameraPos);

	float h = length(cross(light.position - cameraPos, cameraDir)) / length(cameraDir);
	float dropoff = 1.0;
	float fog = (atan(b / h) / (h * dropoff));

	float density = 0.1;

	// TODO: improve this
	float theta = dot(norm, viewDir) >= 0 ? 1 : 0;

	vec3 scattered = light.color * (fog * density) * theta;
	scattered *= calculateAttenuation(constant, linear, quadratic, length(cameraDir));
	
	result += scattered;
#endif

	return result;
}

void main()
{
	float depth = texture(shadowMap, TexCoords).x;
    depth = 1.0 - (1.0 - depth) * 25.0;
    FragColor = vec4(depth);
	//return;

	vec3 result;

	for (int i = 0; i < numLights; i++)
	{
		switch (lights[i].type) {
			case 0:
				result += calculatePointLight(lights[i]);
				break;
			
			case 1:
				result += calculateDirLight(lights[i]);
				break;
		}
	}
	
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
	gl_Position = proj * view * model * vec4(aPos, 1.0f);
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


	const std::string depth_vert = R"(
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
	gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
)";
	
	const std::string depth_frag = R"(
#version 330 core
void main() {}
)";

	const std::string screen_vert = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = vec4(aPos, 1.0);
}
)";
	const std::string screen_frag = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D shadowMap;

void main()
{             
    float depthValue = texture(shadowMap, TexCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0); // orthographic
}
)";

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

	class Camera;
	class Light;

	struct Shader {
	public:
		unsigned int id;
		Shader(const std::string& path);
		Shader(const std::string& vertShader, const std::string& fragShader);
		~Shader();
		void use();
		void reset();
		void setInt(const std::string& name, int value);
		void setFloat(const std::string& name, float value);
		void setVec3(const std::string& name, const glm::vec3& value);
		void setMat4(const std::string& name, const glm::mat4& value);
	};

	struct ShadowMap {
		ShadowMap(unsigned int shadow_width, unsigned int shadow_height);
		unsigned int fbo;
		unsigned int depthMap;
		unsigned int width, height;
		Shader shader;
	};

	struct RenderContext {
		Camera* camera;
		std::vector<Light*> lights;
		ShadowMap *shadowMap;
		Light* shadowCaster;
		bool isShadowPass;
	};

	class Object3D {
	public:
		Object3D()
			: m_dirty(true),
			m_dirty_transform(false),
			receiveShadow(true),
			transform(1.0), 
			parent(nullptr),
			m_position(0.0f),
			m_rotation(0.0f),
			m_scale(1.0f)
		{}

		Object3D* parent;
		std::vector<Object3D*> children;
		glm::mat4 transform;
		bool receiveShadow;

		Object3D& add(Object3D* child);
		void drawChildren(RenderContext& context);
		virtual void draw(RenderContext& context);

		void setScale(const glm::vec3& scale);
		glm::vec3 getScale();

		// yaw, roll, pitch
		void setRotation(const glm::vec3& rot);
		glm::vec3 getRotation();

		glm::vec3 getPosition();
		void setPosition(const glm::vec3& pos);

		void overrideTransform(const glm::mat4& matrix);
		glm::vec3 getWorldPosition();
		virtual bool isLight();

		void traverse(const std::function<void(Object3D*)>& func)
		{
			func(this);
			for (const auto& child : children) child->traverse(func);
		}

	protected:
		friend class Renderer;
		bool m_dirty, m_dirty_transform;
		glm::vec3 m_rotation, m_position, m_scale; 
		void updateWorldMatrix(bool dirtyParent);
		glm::mat4 getLocalTransform();
	};

	class Camera : public Object3D  {
	public:
		Camera(float fov, float aspect, float near, float far)
			: m_projection(glm::perspective(fov, aspect, near, far)), 
			m_up(0.0f, 1.0f, 0.0f),
			m_front(0.0f, 0.0f, 1.0f)
		{}

		glm::mat4 getViewMatrix();
		glm::mat4 getProjectionMatrix();
	private:
		glm::mat4 m_projection;
		glm::vec3 m_up, m_front;
	};

	class Light : public Object3D {
	public:
		enum LightType {
			POINT			= 0,
			DIRECTIONAL		= 1,
		};

		Light(glm::vec3 color_) 
			: color(color_), type(POINT), castShadow(false), Object3D() 
		{}

		Light(LightType type_, glm::vec3 color_) 
			: color(color_), type(type_), castShadow(false), Object3D() 
		{}

		bool isLight();

		LightType type;
		bool castShadow;
		glm::vec3 color;
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
		int count;

	private:
		unsigned int m_vao;
		unsigned int m_vbo;
		static int getStride(const VertexLayout& layout);
	};

	class Material  {
	public:
		Material()
			: Material(glm::vec3(1, 0.5, 0.2), 0.1f, 1.0f, 0.5f, 10.0f) {}

		Material(const glm::vec3& color_)
			: Material(color_, 0.1f, 1.0f, 0.5f, 10.0f) {}

		Material(const glm::vec3& color_, float ka_, float kd_, float ks_, float alpha_)
			: color(color_), ka(ka_), kd(kd_), ks(ks_), alpha(alpha_) {}

		glm::vec3 color;
		float ka, kd, ks, alpha;

		virtual Shader* getShader()
		{
			return nullptr;
		}
	};

	template<class Derived>
	class MaterialX : public Material {
	public:
		MaterialX(const std::string& vert, const std::string& frag, const glm::vec3& color_, float ka_, float kd_, float ks_, float alpha_)
			: Material(color_, ka_, kd_, ks_, alpha_) 
		{
			if (shader == nullptr)
				shader = std::make_shared<Shader>(vert, frag);
		}

		MaterialX( const std::string& vert, const std::string& frag, const glm::vec3& color_)
			: MaterialX<Derived>(vert, frag, color_, 0.2f, 1.0f, 0.5f, 10.0f) {}

		MaterialX( const std::string& vert, const std::string& frag)
			: MaterialX<Derived>(vert, frag, glm::vec3(0.5f), 0.2f, 1.0f, 0.5f, 10.0f) {}

		Shader* getShader()
		{
			return shader.get();
		}
		
		static std::shared_ptr<Shader> shader;
	};

	class Phong : public MaterialX<Phong> {
	public:
		Phong(const glm::vec3& color_) : MaterialX<Phong>(phong_vert, phong_frag, color_, 0.2f, 1.0f, 0.5f, 10.0f) {}
	};

	class Basic : public MaterialX<Basic> {
	public:
		Basic(const glm::vec3& color_) : MaterialX<Basic>(basic_vert, basic_frag, color_){}
	};

	class ShaderMaterial : public MaterialX<ShaderMaterial> {
	public:
		ShaderMaterial(const std::string& vert, const std::string& frag) : MaterialX<ShaderMaterial>(vert, frag) {}
	};

	class Mesh : public Object3D {
	public:
		Mesh(std::shared_ptr<Geometry> geometry, std::shared_ptr<Material> material)
			: m_geometry(geometry), m_material(material) {}
		void draw(RenderContext& context);
	private:
		std::shared_ptr<Geometry> m_geometry;
		std::shared_ptr<Material> m_material;
	};

	class Renderer {
	public:
		Renderer(GLFWwindow* window, unsigned int width, unsigned int height) 
			: m_window(window), m_shadowMap(new ShadowMap(1024, 1024)), m_width(width), m_height(height) 
		{
			
			const std::vector<float> quad_vertices = {
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // top left
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // top right

				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // top right
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
				 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom right
			};

			auto geometry = std::make_shared<Geometry>(quad_vertices, Geometry::POS_UV);
			auto material = std::make_shared<ShaderMaterial>(screen_vert, screen_frag);
			m_quad = std::make_shared<Mesh>(geometry, material);
		}

		~Renderer()
		{
			if (m_shadowMap)
				delete m_shadowMap;
		}

		void render(Camera& camera, Object3D& scene);
	private:
		GLFWwindow* m_window;
		unsigned int m_width, m_height;
		ShadowMap* m_shadowMap;
		std::shared_ptr<Mesh> m_quad;
	};
};


