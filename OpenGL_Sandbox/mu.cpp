#include "mu.h"

namespace mu {

	Shader::Shader(const std::string& vertShaderPath, const std::string& fragShaderPath)
	{
		std::cout << "loading shader " << vertShaderPath << ", " << fragShaderPath << std::endl;

		std::fstream vfile(vertShaderPath);
		std::stringstream vbuffer;
		vbuffer << vfile.rdbuf();
		std::string vsource = vbuffer.str();
		const char* vertexShaderSource = vsource.c_str();


		std::cout << vertexShaderSource << std::endl;

		std::fstream ffile(fragShaderPath);
		std::stringstream fbuffer;
		fbuffer << ffile.rdbuf();
		std::string fsource = fbuffer.str();
		const char* fragmentShaderSource = fsource.c_str();


		std::cout << fragmentShaderSource << std::endl;

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
	
	Geometry::Geometry(const std::vector<float>& vertices)
		: count(vertices.size() / (6))
	{
		glGenVertexArrays(1, &m_vao);
		glGenBuffers(1, &m_vbo);

		glBindVertexArray(m_vbo);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	Geometry::~Geometry()
	{
		glDeleteVertexArrays(1, &m_vao);
		glDeleteBuffers(1, &m_vbo);
	}

	void Geometry::use()
	{
        glBindVertexArray(m_vao); 
	}


	void Geometry::write(const std::vector<float>& vertices)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	}

	void Renderer::render(Camera& camera, Object3D& scene)
	{
		scene.draw(camera);
	}

	Camera::Camera()
		: view(1.0),
		projection(1.0)
	{}

	Object3D::Object3D()
		: dirty(true),
		transform(1.0), 
		worldTransform(1.0)
	{}

	void Object3D::draw(Camera& camera)
	{}

	void Mesh::draw(Camera& camera)
	{
		Shader& shader = m_material.shader;

		shader.use();
		shader.setMat4("view", camera.view);
		shader.setMat4("proj", camera.projection);
		shader.setMat4("model", worldTransform);

		shader.setVec3("lightPos", glm::vec3(0,10,0));
		shader.setVec3("viewPos", glm::vec3(0,0,-3));
		shader.setVec3("lightColor", glm::vec3(1.0f));
		shader.setVec3("objectColor", m_material.color);

		m_geometry.use();

		glDrawArrays(GL_TRIANGLES, 0, m_geometry.count);
	}

	Material::Material(const std::string& vertPath, const std::string& fragPath)
		: shader(vertPath, fragPath), 
		color(1.0f, 0.0f, 0.0f)
	{
	}
}
