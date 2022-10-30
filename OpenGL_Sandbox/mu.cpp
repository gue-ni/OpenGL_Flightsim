#include "mu.h"


namespace mu {

	Shader::Shader(const std::string& vertShaderPath, const std::string& fragShaderPath)
	{
		std::cout << "create Shader\n";

		std::fstream vfile(vertShaderPath);
		std::stringstream vbuffer;
		vbuffer << vfile.rdbuf();
		std::string vsource = vbuffer.str();
		const char* vertexShaderSource = vsource.c_str();

		std::fstream ffile(fragShaderPath);
		std::stringstream fbuffer;
		fbuffer << ffile.rdbuf();
		std::string fsource = fbuffer.str();
		const char* fragmentShaderSource = fsource.c_str();
#if 0
		std::cout << vertexShaderSource << std::endl;
		std::cout << fragmentShaderSource << std::endl;
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
		std::cout << "destroy Shader\n";
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
		std::cout << "create Geometry\n";

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
		std::cout << "copy Geometry\n";
		count = geometry.count;
		m_vao = geometry.m_vao;
		m_vbo = geometry.m_vbo;
	}

	Geometry::~Geometry()
	{
		std::cout << "destroy Geometry\n";
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
		scene.draw(camera);
	}

	Camera::Camera(float fov, float aspect, float near, float far)
		: projection(glm::perspective(fov, aspect, near, far))
	{}

	Object3D::Object3D()
		: m_dirty(true),
		transform(1.0), 
		parent(nullptr),
		m_position(0.0f),
		m_rotation(0.0f),
		m_scale(1.0f)
	{}

	void Object3D::draw(Camera& camera)
	{
		for (auto child : children)
			child->draw(camera);
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

	void Object3D::updateWorldMatrix(bool dirtyParent)
	{
		bool dirty = m_dirty || dirtyParent;

		if (dirty)
		{
			if (parent)
				transform = parent->transform * getLocalTransform();
			else
				transform = getLocalTransform();
		}
	
		for (auto child : children)
		{
			child->updateWorldMatrix(dirty);
		}

		m_dirty = false;
	}
	
	void Object3D::addChild(Object3D* child)
	{
		child->parent = this;
		children.push_back(child);
	}

	void Mesh::draw(Camera& camera)
	{
		Shader& shader = m_material.get()->shader;

		shader.use();
		shader.setMat4("view", camera.transform);
		shader.setMat4("proj", camera.projection);
		shader.setMat4("model", transform);

		shader.setVec3("cameraPos", camera.getPosition());

		shader.setVec3("lightPos", glm::vec3(2.5f));
		shader.setVec3("lightColor", glm::vec3(1.0f));

		shader.setVec3("objectColor", m_material.get()->color);

		m_geometry.get()->use();

		glDrawArrays(GL_TRIANGLES, 0, m_geometry.get()->count);

		for (auto child : children)
		{
			child->draw(camera);
		}
	}

	Material::Material(const std::string& vertPath, const std::string& fragPath)
		: shader(vertPath, fragPath), 
		color(1.0f, 0.5f, 0.2f)
	{
	}
}
