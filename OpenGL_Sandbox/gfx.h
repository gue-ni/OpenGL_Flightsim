#pragma once

#include <GL/glew.h>

// glm
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>





constexpr float PI = 3.14159265359f;

typedef glm::vec3 RGB;
typedef glm::vec4 RGBA;



namespace gfx {




	std::ostream& operator<<(std::ostream& os, const glm::vec3& v);

	typedef glm::vec3 RGB;
	typedef glm::vec4 RGBA;

	class Light;
	class Camera;

	constexpr RGB rgb(int r, int g, int b)
	{
		return RGB( static_cast<float>(r), static_cast<float>(g), static_cast<float>(b) ) / 255.0f;
	}

	constexpr RGB rgb(uint32_t hex)
	{
		assert(hex <= 0xffffffU);
		return RGB{
			static_cast<float>((hex & 0xff0000U) >> 16) / 255.0f,
			static_cast<float>((hex & 0x00ff00U) >> 8)  / 255.0f, 
			static_cast<float>((hex & 0x0000ffU) >> 0)  / 255.0f
		};
	}

	void load_obj(const std::string path, std::vector<float>& vertices);

	struct Shader {
		GLuint id;
		Shader(const std::string& path);
		Shader(const std::string& vertShader, const std::string& fragShader);
		~Shader();
		void upload_uniforms() const;
		void unbind() const;
		void set_int(const std::string& name, int value);
		void set_float(const std::string& name, float value);
		void set_vec3(const std::string& name, const glm::vec3& value);
		void set_vec4(const std::string& name, const glm::vec4& value);
		void set_mat4(const std::string& name, const glm::mat4& value);
	};

	struct VertexBuffer {
		GLuint id;
		VertexBuffer(const void* data, size_t size);
		~VertexBuffer();
		void upload_uniforms() const;
		void unbind() const;
	};

#if 0
	struct VertexArray {
		unsigned int id;
		VertexArray();
		~VertexArray();
		void bind() const;
		void unbind() const;
	};
#endif

	struct ShadowMap {
		ShadowMap(unsigned int shadow_width, unsigned int shadow_height);
		unsigned int fbo;
		unsigned int depth_map;
		unsigned int width, height;
		Shader shader;
	};

	struct RenderContext {
		Camera* camera;
		Light* shadow_caster;
		ShadowMap *shadow_map;

		std::vector<Light*> lights;
		bool is_shadow_pass;
		glm::vec3 background_color;
	};

	class Object3D {
	public:

		enum Type {
			OBJECT3D,
			LIGHT,
			CAMERA
		};

		Object3D()
			: parent(nullptr),
			transform(1.0), 
			m_position(0.0f),
			m_rotation(0.0f),
			m_scale(1.0f)
		{}

		Object3D* parent;
		std::vector<Object3D*> children;
		glm::mat4 transform;
		bool receive_shadow = true;

		Object3D& add(Object3D* child);
		
		virtual void draw(RenderContext& context);
		void draw_children(RenderContext& context);

		void set_scale(const glm::vec3& scale); // yaw, roll, pitch
		void set_rotation(const glm::vec3& rot);
		void set_position(const glm::vec3& pos);

		glm::vec3 get_scale() const;
		glm::vec3 get_rotation() const;
		glm::vec3 get_position() const;

		virtual Object3D::Type get_type() const;

		glm::vec3 get_world_position() const;
		void override_transform(const glm::mat4& matrix);

		void traverse(const std::function<bool(Object3D*)>& func)
		{
			if (func(this))
			{
				for (const auto& child : children)
				{
					child->traverse(func);
				}
			}
		}

	protected:
		friend class Renderer;
		bool m_dirty_dof = false, m_dirty_transform = false;
		glm::vec3 m_rotation, m_position, m_scale; 
		void update_world_matrix(bool dirtyParent);
		glm::mat4 get_local_transform() const;
	};

	class Camera : public Object3D  {
	public:
		Camera(float fov, float aspect, float near, float far)
			: m_projection(glm::perspective(fov, aspect, near, far)), 
			m_up(0.0f, 1.0f, 0.0f),
			m_front(0.0f, 0.0f, 1.0f)
		{}

		Object3D::Type get_type() const override;
		glm::mat4 get_view_matrix() const;
		glm::mat4 get_projection_matrix() const;
		void look_at(const glm::vec3& target);

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
			: rgb(color_), type(POINT), cast_shadow(false), Object3D() 
		{}

		Light(LightType type_, glm::vec3 color_) 
			: rgb(color_), type(type_), cast_shadow(false), Object3D() 
		{}

		Object3D::Type get_type() const override;

		glm::mat4 light_space_matrix();

		LightType type;
		bool cast_shadow;
		glm::vec3 rgb;
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
		Geometry(const void* data, size_t size, const VertexLayout& layout);
		Geometry(const Geometry& geometry);
		~Geometry();
		void upload_uniforms();
		int count;

	private:
		unsigned int m_vao, m_vbo;
		static int get_stride(const VertexLayout& layout);
	};

	class Material  {
	public:
		virtual Shader* get_shader()
		{
			return nullptr;
		}

		virtual void upload_uniforms() {}
	};

	template<class Derived>
	class MaterialX : public Material {
	public:
		MaterialX(const std::string& path)
		{
			if (shader == nullptr)
				shader = std::make_shared<Shader>(path);
		}

		Shader* get_shader() { return shader.get(); }
		static std::shared_ptr<Shader> shader;
	};

	class Phong : public MaterialX<Phong> {
	public:
		glm::vec3 rgb;
		float ka, kd, ks, alpha;

		Phong(const glm::vec3& color_, float ka_, float kd_, float ks_, float alpha_) 
			: MaterialX<Phong>("shaders/phong"), rgb(color_), ka(ka_), kd(kd_), ks(ks_), alpha(alpha_) 
		{}

		Phong(const glm::vec3& color_)
			: MaterialX<Phong>("shaders/phong"), rgb(color_), ka(0.1f), kd(1.0f), ks(0.5f), alpha(10.0f) 
		{}

		void upload_uniforms();
	};

	class Basic : public MaterialX<Basic> {
	public:
		glm::vec3 rgb;
		Basic(const glm::vec3& color_) : MaterialX<Basic>("shaders/basic"), rgb(color_) {}
		void upload_uniforms();
	};

	class ShaderMaterial : public MaterialX<ShaderMaterial> {
	public:
		ShaderMaterial(const std::string& path) : MaterialX<ShaderMaterial>(path) {}
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
		Renderer(unsigned int width, unsigned int height) 
			: m_shadowMap(new ShadowMap(1024, 1024)),  m_width(width), m_height(height), background(rgb(18, 100, 132))
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
			auto material = std::make_shared<ShaderMaterial>("shaders/screen");
			m_quad = std::make_shared<Mesh>(geometry, material);
		}

		~Renderer()
		{
			if (m_shadowMap)
				delete m_shadowMap;
		}

		void render(Camera& camera, Object3D& scene);
		glm::vec3 background;
	private:
		unsigned int m_width, m_height;
		ShadowMap* m_shadowMap = nullptr;
		std::shared_ptr<Mesh> m_quad;
	};

	class Controller 
	{
	public:
		enum Direction {
			FORWARD,
			RIGHT,
			BACKWARD,
			LEFT,
		};

		Controller(float speed) 
			: m_speed(speed), 
			m_yaw(-90.0f), 
			m_pitch(0.0f), 
			m_front(0.0f, 0.0f, -1.0f), 
			m_up(0.0f, 1.0f, 0.0f),
			m_velocity(0.0f),
			m_direction(0.0f)
		{}

		void update(Object3D& object, float dt);
		void move_mouse(float x, float y);
		void move(const Direction& direction);
	
	private:
		bool m_initialized{ false };

		float m_speed, m_yaw, m_pitch;
		glm::vec2 m_last_pos{};
		glm::vec3 m_front, m_up, m_velocity, m_direction;
	};
};


