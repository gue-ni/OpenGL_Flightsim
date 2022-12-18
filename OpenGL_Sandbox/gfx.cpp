#include "gfx.h"

std::string read_shader(const std::string& path)
{
		std::fstream file(path);
		if (!file.is_open())
			return std::string();

		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
}

namespace gfx {
	Shader::Shader(const std::string& path) : Shader(read_shader(path + ".vert"), read_shader(path + ".frag")) {}
	
	Shader::Shader(const std::string& vertShader, const std::string& fragShader)
	{
		//std::cout << "create Shader\n";
		const char* vertexShaderSource = vertShader.c_str();
		const char* fragmentShaderSource = fragShader.c_str();

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

	void Shader::reset()
	{
		glUseProgram(0);
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

	void Shader::setVec4(const std::string& name, const glm::vec4& value)
	{
		glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}

	void Shader::setMat4(const std::string& name, const glm::mat4& value)
	{
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &value[0][0]);
	}
	
	Geometry::Geometry(const std::vector<float>& vertices, const VertexLayout& layout)
		: count(static_cast<int>(vertices.size()) / (getStride(layout)))
	{
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

	glm::mat4 Camera::getViewMatrix()
	{
		return glm::inverse(transform);
	}

	glm::mat4 Camera::getProjectionMatrix()
	{
		return m_projection;
	}

	void Camera::lookAt(const glm::vec3& target)
	{
		overrideTransform(
			glm::inverse(
				glm::lookAt(
					m_position,
					target,
					m_up	
				)
			)
		);
	}

	template<class Derived>
	std::shared_ptr<Shader> MaterialX<Derived>::shader = nullptr;
	
	void Object3D::draw(RenderContext& context)
	{
		drawChildren(context);
	}

	void Object3D::drawChildren(RenderContext& context)
	{
		for (auto child : children) child->draw(context);
	}

	glm::vec3 Object3D::getPosition()
	{
		return m_position;
	}

	glm::vec3 Object3D::getRotation()
	{
		return m_rotation;
	}

	glm::vec3 Object3D::getScale()
	{
		return m_scale;
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
		m_dirty_transform = true; m_dirty = true;
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

	void Renderer::render(Camera& camera, Object3D& scene)
	{
		scene.updateWorldMatrix(false);

		RenderContext context;
		context.camera			= &camera;
		context.shadowMap		= m_shadowMap;
		context.shadowCaster	= nullptr;
		context.backgroundColor = background;

		scene.traverse([&context](Object3D* obj) {
			if (obj->isLight())
			{
				Light* light = dynamic_cast<Light*>(obj);
				if (light->castShadow)
				{
					context.shadowCaster = light;
				}
				context.lights.push_back(light);
			}
		});

#if 1
		if (m_shadowMap)
		{
			glViewport(0, 0, m_shadowMap->width, m_shadowMap->height);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadowMap->fbo);
			glClear(GL_DEPTH_BUFFER_BIT);

			context.isShadowPass	= true;
			scene.draw(context);
	
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_shadowMap->depthMap);
		}
#endif

		glViewport(0, 0, m_width, m_height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		context.isShadowPass = false;
#if 1


#ifdef WIREFRAME
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
		scene.draw(context);
#ifdef WIREFRAME
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

#else
		m_quad.get()->draw(context);
#endif
	}

	void Mesh::draw(RenderContext& context)
	{
		float near_plane = 0.1f, far_plane = 25.0f, m = 10.0f;

		assert(context.shadowCaster);

		auto lightPos = context.shadowCaster->getWorldPosition();

		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightProjection = glm::ortho(-m, m, -m, m, -10.0f, 20.0f);
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		if (context.isShadowPass)
		{
			Shader* shader = &context.shadowMap->shader;

			shader->use();
			shader->setMat4("model", transform);
			shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
		}
		else {
			Shader* shader = m_material.get()->getShader();

			shader->use();
			shader->setMat4("model", transform);
			shader->setMat4("view", context.camera->getViewMatrix());
			shader->setMat4("proj", context.camera->getProjectionMatrix());
			shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
			shader->setVec3("cameraPos", context.camera->getWorldPosition()); 
			shader->setInt("shadowMap", 0);
			shader->setInt("numLights", context.lights.size());
			shader->setInt("receiveShadow", receiveShadow);
			shader->setVec3("backgroundColor", context.backgroundColor);


			for (int i = 0; i < context.lights.size(); i++)
			{
				auto index = std::to_string(i);
				auto type = context.lights[i]->type;

				shader->setInt( "lights[" + index + "].type", type);
				shader->setVec3("lights[" + index + "].color", context.lights[i]->rgb);
				shader->setVec3("lights[" + index + "].position", context.lights[i]->getWorldPosition());
			}

			// phong
			shader->setFloat("ka", m_material.get()->ka);
			shader->setFloat("kd", m_material.get()->kd);
			shader->setFloat("ks", m_material.get()->ks);
			shader->setFloat("alpha", m_material.get()->alpha);
			shader->setVec3("objectColor", m_material.get()->rgb);
		}

		m_geometry.get()->use();
		glDrawArrays(GL_TRIANGLES, 0, m_geometry.get()->count);

		drawChildren(context);
	}

	ShadowMap::ShadowMap(unsigned int shadow_width, unsigned int shadow_height)
		: width(shadow_width), height(shadow_height), shader("shaders/depth")
	{
#if 1
		glGenFramebuffers(1, &fbo);

		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(
			GL_TEXTURE_2D, 
			0, 
			GL_DEPTH_COMPONENT, 
			shadow_width, 
			shadow_height, 
			0, 
			GL_DEPTH_COMPONENT, 
			GL_FLOAT, 
			NULL
		);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindBuffer(GL_FRAMEBUFFER, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "failure\n";
			exit(-1);
		}
#endif
	}
}
