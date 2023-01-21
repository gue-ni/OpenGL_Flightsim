#pragma once

#include "gfx.h"

void push_back(std::vector<float>& vertices, const glm::vec3& v)
{
	std::cout << "vertex = " << v << std::endl;
	vertices.push_back(v.x);
	vertices.push_back(v.y);
	vertices.push_back(v.z);
}

void push_back(std::vector<unsigned int>& indices, unsigned int i)
{
	printf("index = %d\n", i);
	indices.push_back(i);
}

class Clipmap : public gfx::Object3D {
public:


	Clipmap() 
		: shader("shaders/clipmap")
	{
		// https://www.learnopengles.com/tag/vertex-buffer-object/
		// https://www.learnopengles.com/tag/degenerate-triangles/

		const float tile_height = 1.0f;
		const float tile_width = 2.0f;


#if 0
		std::vector<float> vertices = {
			0.0f, 0.0f, 0.0f, // top left
			0.0f, 0.0f, tile_width, // top right
			tile_height, 0.0f, 0.0f, // bottom left
			tile_height, 0.0,  tile_width, // bottom right 
		};

		std::vector<unsigned int> indices = {  
			1, 0, 2,
			2, 3, 1
		};
#else
		std::vector<float> vertices;
		std::vector<unsigned int> indices;

		int width = 2, height = 2; // number of tiles

		for (unsigned int i = 0; i <= height; i++)
		{
			for (unsigned int j = 0; j <= width; j++)
			{
				push_back(vertices, glm::vec3(i * tile_height, 0.0f, j * tile_width));
			}
		}

		for (unsigned int i = 0; i < height; i++)
		{
			for (unsigned int j = 0; j < width; j++)
			{
				push_back(indices, (i + 0) * width + j + 1);
				push_back(indices, (i + 0) * width + j);
				push_back(indices, (i + 1) * width + j + 1);

				push_back(indices, (i + 1) * width + j + 1);
				push_back(indices, (i + 1) * width + j + 2);
				push_back(indices, (i + 0) * width + j + 1);
			}
		}
#endif

		num_indices = indices.size();
		std::cout << "indices = " << indices.size() << std::endl;
		std::cout << "vertices = " << vertices.size() / 3 << std::endl;

		tile_vao.bind();
		tile_vbo.buffer(&vertices[0], vertices.size() * sizeof(vertices[0]));
		tile_ebo.buffer(&indices[0], indices.size() * sizeof(indices[0]));

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		tile_vbo.unbind();
		tile_vao.unbind();
	}

	void draw(gfx::RenderContext& context) override
	{
		if (!context.is_shadow_pass)
		{
			shader.bind();
			shader.uniform("u_Model",			transform);
			shader.uniform("u_View",			context.camera->get_view_matrix());
			shader.uniform("u_Projection",		context.camera->get_projection_matrix());

			tile_vao.bind();

			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElements(GL_TRIANGLE_STRIP, num_indices, GL_UNSIGNED_INT, 0);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			tile_vao.unbind();
		}
	}

private:
	gfx::Shader shader;
	//gfx::Texture heightmap;

	gfx::VertexBuffer tile_vbo;
	gfx::ElementBufferObject tile_ebo;
	gfx::VertexArrayObject tile_vao;

	unsigned int num_indices;
	const int levels = 3;
};
