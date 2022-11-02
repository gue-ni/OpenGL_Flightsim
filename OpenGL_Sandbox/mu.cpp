#include "mu.h"


std::string read_shader(const std::string& path)
{
		std::fstream file(path);
		if (!file.is_open())
			return std::string();

		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
}


namespace mu {
	Shader::Shader(const std::string& path) : Shader(read_shader(path + ".vert"), read_shader(path + ".frag")) {}
	
	Shader::Shader(const std::string& vertShader, const std::string& fragShader)
	{
		//std::cout << "create Shader\n";
#if 0
		std::fstream vfile(vertShader);
		std::stringstream vbuffer;
		vbuffer << vfile.rdbuf();
		std::string vsource = vbuffer.str();
		const char* vertexShaderSource = vsource.c_str();

		std::fstream ffile(fragShader);
		std::stringstream fbuffer;
		fbuffer << ffile.rdbuf();
		std::string fsource = fbuffer.str();
		const char* fragmentShaderSource = fsource.c_str();
#else
		const char* vertexShaderSource = vertShader.c_str();
		const char* fragmentShaderSource = fragShader.c_str();
#endif

		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// check for shader compile errors
		int success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// fragment shader
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		// check for shader compile errors
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// link shaders
		id = glCreateProgram();
		glAttachShader(id, vertexShader);
		glAttachShader(id, fragmentShader);
		glLinkProgram(id);
		// check for linking errors
		glGetProgramiv(id, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(id, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	Shader::~Shader()
	{
		//std::cout << "destroy Shader\n";
		glDeleteProgram(id);
	}

	void Shader::use()
	{
		glUseProgram(id);
	}

	void Shader::setInt(const std::string& name, int value) 
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}

	void Shader::setFloat(const std::string& name, float value) 
	{
		glUniform1f(glGetUniformLocation(id, name.c_str()), value);
	}

	void Shader::setVec3(const std::string& name, const glm::vec3& value)
	{
		glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}

	void Shader::setMat4(const std::string& name, const glm::mat4& value)
	{
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &value[0][0]);
	}
	
	Geometry::Geometry(const std::vector<float>& vertices, const VertexLayout& layout)
		: count(static_cast<int>(vertices.size()) / (getStride(layout)))
	{
		//std::cout << "create Geometry\n";

		const int stride = getStride(layout);
		glGenVertexArrays(1, &m_vao);
		glGenBuffers(1, &m_vbo);

		glBindVertexArray(m_vbo);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

		unsigned int index = 0;
		glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
		glEnableVertexAttribArray(index);

		if (layout == POS_NORM || layout == POS_NORM_UV) // add normal
		{
			index++;
			glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(index);
		}

		if (layout == POS_UV || layout == POS_NORM_UV) // add uv
		{
			index++;
			glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(static_cast<int>(index) * 3 * sizeof(float)));
			glEnableVertexAttribArray(index);
		}

#if 0
		switch (layout)
		{

		case POS_UV:
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			break;

		case POS_NORM:
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			break;

		case POS_NORM_UV:
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(6 * sizeof(float)));
			glEnableVertexAttribArray(1);
			break;

		case POS:
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			break;


		default:
			assert(false);
			break;
		}
#endif

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	Geometry::Geometry(const Geometry& geometry)
	{
		//std::cout << "copy Geometry\n";
		count = geometry.count;
		m_vao = geometry.m_vao;
		m_vbo = geometry.m_vbo;
	}

	Geometry::~Geometry()
	{
		//std::cout << "destroy Geometry\n";
		glDeleteVertexArrays(1, &m_vao);
		glDeleteBuffers(1, &m_vbo);
	}

	void Geometry::use()
	{
        glBindVertexArray(m_vao); 
	}

	int Geometry::getStride(const VertexLayout& layout)
	{
		switch (layout)
		{
		case POS:			return 3;
		case POS_UV:		return 5;
		case POS_NORM:		return 6;
		case POS_NORM_UV:	return 8;
		}
		return 0;
	}

	void Geometry::write(const std::vector<float>& vertices)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	}

	void Renderer::render(Camera& camera, Object3D& scene)
	{
		scene.updateWorldMatrix(false);

		RenderContext context;
		context.camera = &camera;

		scene.traverse([&context](Object3D* obj) {
			if (obj->isLight())
			{
				context.lights.push_back(dynamic_cast<Light*>(obj));
			}
		});

		scene.draw(context);
	}

	glm::mat4 Camera::getViewMatrix()
	{
		return glm::inverse(transform);
	}

	glm::mat4 Camera::getProjectionMatrix()
	{
		return m_projection;
	}

	template<class Derived>
	std::shared_ptr<Shader> MaterialX<Derived>::shader = nullptr;
	
	void Object3D::draw(RenderContext& context)
	{
		for (auto child : children)
			child->draw(context);
	}

	const glm::vec3& Object3D::getPosition()
	{
		return m_position;
	}

	const glm::vec3& Object3D::getRotation()
	{
		return m_rotation;
	}

	const glm::vec3& Object3D::getScale()
	{
		return m_scale;
	}

	void Object3D::setPosition(float x, float y, float z)
	{
		m_position = glm::vec3(x,y,z); m_dirty = true;
	}

	void Object3D::setRotation(float x, float y, float z)
	{
		m_rotation = glm::vec3(x,y,z); m_dirty = true;
	}

	void Object3D::setScale(float x, float y, float z)
	{
		m_scale = glm::vec3(x,y,z); m_dirty = true;
	}

	void Object3D::setScale(const glm::vec3& scale)
	{
		m_scale = scale; m_dirty = true;
	}

	void Object3D::setPosition(const glm::vec3& pos)
	{
		m_position = pos; m_dirty = true;
	}

	void Object3D::setRotation(const glm::vec3& rot)
	{
		m_rotation = rot; m_dirty = true;
	}

	glm::mat4 Object3D::getLocalTransform()
	{
		auto S = glm::scale(glm::mat4(1.0f), m_scale);
		auto T = glm::translate(glm::mat4(1.0f), m_position);
		auto R = glm::eulerAngleYXZ(m_rotation.x, m_rotation.y, m_rotation.z);
		return T * R * S;
	}

	void Object3D::overrideTransform(const glm::mat4& matrix)
	{
		m_dirty_transform = true;
		transform = matrix;
	}

	void Object3D::updateWorldMatrix(bool dirtyParent)
	{
		bool dirty = m_dirty || dirtyParent;

		if (dirty && !m_dirty_transform)
		{
			if (parent)
				transform = parent->transform * getLocalTransform();
			else
				transform = getLocalTransform();
		}
	
		for (auto child : children)
		{
			child->updateWorldMatrix(dirty || m_dirty_transform);
		}

		m_dirty = m_dirty_transform = false;
	}
	
	Object3D& Object3D::add(Object3D* child)
	{
		child->parent = this;
		children.push_back(child);
		return (*this);
	}


	glm::vec3 Object3D::getWorldPosition()
	{
		updateWorldMatrix(true);
		auto world = transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		return glm::vec3(world.x, world.y, world.z);
	}

	bool Object3D::isLight()
	{
		return false;
	}

	bool Light::isLight()
	{
		return true;
	}

	void Mesh::draw(RenderContext& context)
	{
		Shader* shader = m_material.get()->getShader();

		shader->use();
		shader->setMat4("view", context.camera->getViewMatrix());
		shader->setMat4("proj", context.camera->getProjectionMatrix());
		shader->setMat4("model", transform);
		shader->setVec3("cameraPos", context.camera->getPosition()); // not world position, transform is applied twice


		shader->setInt("numPointLights", context.lights.size());

		for (int i = 0; i < context.lights.size(); i++)
		{
			auto localPos = context.lights[i]->getPosition();
			auto worldPos = context.lights[i]->getWorldPosition();
			//std::cout << worldPos << std::endl;
			auto index = std::to_string(i);
			shader->setVec3("pointLights[" + index + "].color", context.lights[i]->color);
			shader->setVec3("pointLights[" + index + "].position", worldPos);
		}

		// phong
		shader->setFloat("ka", m_material.get()->ka);
		shader->setFloat("kd", m_material.get()->kd);
		shader->setFloat("ks", m_material.get()->ks);
		shader->setFloat("alpha", m_material.get()->alpha);
		shader->setVec3("objectColor", m_material.get()->color);

		m_geometry.get()->use();
		glDrawArrays(GL_TRIANGLES, 0, m_geometry.get()->count);

		for (auto child : children)
		{
			child->draw(context);
		}
	}
}
