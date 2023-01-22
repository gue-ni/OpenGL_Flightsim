#pragma once

#include "gfx.h"

void push_back(std::vector<float>& vertices, const glm::vec3& v)
{
	//std::cout << "vertex = " << v << std::endl;
	vertices.push_back(v.x);
	vertices.push_back(v.y);
	vertices.push_back(v.z);
}

void push_back(std::vector<unsigned int>& indices, unsigned int i)
{
	//printf("index = %d\n", i);
	indices.push_back(i);
}


void create_plane(std::vector<glm::vec3>& vertices, std::vector<unsigned int>& indices)
{
	float tile_height = 1.0f, tile_width = 2.0f;

	int rows = 1, columns = 2;

	indices.clear();

	for (int y = 0; y <= rows; y++)
	{
		for (int x = 0; x <= columns; x++)
		{
			vertices.push_back(glm::vec3(x * tile_height, 0.0f, y * tile_width));
		}
	}

	// Create Indices
	indices.clear();

	// Create Indices with degenerate triangles ( stitching )
	// Notice < iRows, <= iCols
	for (int r = 0; r < rows; r++)
	{
		// Degenerate first triangle in this column
		indices.push_back((r + 0) * (columns + 1) + 0);

		for (int c = 0; c < columns + 1; c++)
		{
			// Vertex in actual row
			indices.push_back((r + 0) * (columns + 1) + c);

			// Vertex row below
			indices.push_back((r + 1) * (columns + 1) + c);
		}

		// Degenerate last triangle in this column
		indices.push_back((r + 0) * (columns + 1) + columns + 1);
	}
}

class Clipmap : public gfx::Object3D {
public:


	Clipmap() 
		: shader("shaders/clipmap")
	{
		// https://www.learnopengles.com/tag/vertex-buffer-object/
		// https://www.learnopengles.com/tag/degenerate-triangles/
		// https://www.learnopengles.com/android-lesson-eight-an-introduction-to-index-buffer-objects-ibos/

		const float tile_height = 1.0f;
		const float tile_width = 1.5f;

		std::vector<glm::vec3> vertices;
		std::vector<unsigned int> indices;

#if 0
		vertices = {
			0.0f, 0.0f, 0.0f,				// top left
			0.0f, 0.0f, tile_width,			// top right
			tile_height, 0.0f, 0.0f,		// bottom left
			tile_height, 0.0,  tile_width, // bottom right 
		};

		indices = {
			0, 2, 1,
			2, 3, 1
		};
#else
		create_plane(vertices, indices);
		/*
		for (int i = 0; )
		{
		}
		*/
#endif

		index_count = indices.size();
		std::cout << "indices = " << indices.size() << std::endl;
		std::cout << "vertices = " << vertices.size() / 3 << std::endl;

		tile_vao.bind();
		tile_vbo.buffer(&vertices[0], vertices.size() * sizeof(vertices[0]));
		tile_ebo.buffer(&indices[0], indices.size() * sizeof(indices[0]));

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		//tile_ebo.unbind();
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

			bool wireframe = true;
			glFrontFace(GL_CW);

			if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElements(GL_TRIANGLE_STRIP, index_count, GL_UNSIGNED_INT, 0);
			if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			tile_vao.unbind();


			glFrontFace(GL_CCW);
		}
	}

private:
	gfx::Shader shader;
	//gfx::Texture heightmap;

	gfx::VertexBuffer tile_vbo;
	gfx::ElementBufferObject tile_ebo;
	gfx::VertexArrayObject tile_vao;

	unsigned int index_count;
	const int levels = 3;
};
