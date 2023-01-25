#pragma once

#include "gfx.h"

constexpr unsigned int primitive_restart = 0xFFFFU;

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

void generate_mesh(std::vector<glm::vec3>& vertices, std::vector<unsigned int>& indices, int rows = 3, int columns = 2, float size = 1.0f)
{
	vertices.clear();

	for (int y = 0; y <= rows; y++)
	{
		for (int x = 0; x <= columns; x++)
		{
			vertices.push_back({x * size, 0.0f, y * size});
		}
	}

	indices.clear();

	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < columns + 1; c++)
		{
			auto i0 = (r + 0) * (columns + 1) + c;
			indices.push_back(i0);

			auto i1 = (r + 1) * (columns + 1) + c;
			indices.push_back(i1);
		}
		indices.push_back(primitive_restart); // restart primitive
	}
}

void create_plane(std::vector<glm::vec3>& vertices, std::vector<unsigned int>& indices)
{
	float tile_height = 1.0f, tile_width = 2.0f;
	int rows = 2, columns = 5;

	vertices.clear();

	for (int r = 0; r <= rows; r++)
	{
		for (int c = 0; c <= columns; c++)
		{
			vertices.push_back({c * tile_height, 0.0f, r * tile_width});
		}
	}

	// Create Indices
	indices.clear();

	// Create Indices with degenerate triangles ( stitching )
	// Notice < iRows, <= iCols
	for (int r = 0; r < rows; r++)
	{
		// Degenerate first triangle in this column
		auto d0 = (r + 0) * (columns + 1) + 0;
		indices.push_back(d0);
		std::cout << "d0 = " << d0 << std::endl;

		for (int c = 0; c < columns + 1; c++)
		{
			auto i0 = (r + 0) * (columns + 1) + c;
			// Vertex in actual row
			indices.push_back(i0);
			std::cout << "i0 = " << i0 << std::endl;

			auto i1 = (r + 1) * (columns + 1) + c;
			// Vertex row below
			indices.push_back(i1);
			std::cout << "i1 = " << i1 << std::endl;
		}

		// Degenerate last triangle in this column
		auto d1 = (r + 0) * (columns + 1) + columns + 1;
		indices.push_back(d1);
		std::cout << "d1 = " << d1 << std::endl;
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

		generate_mesh(vertices, indices, segments, segments, segment_size);

		index_count = indices.size();
		std::cout << "indices = " << indices.size() << std::endl;
		std::cout << "vertices = " << vertices.size() / 3 << std::endl;

		assert(indices.size() > 0 && vertices.size() > 0);

		tile_vao.bind();
		tile_vbo.buffer(&vertices[0], vertices.size() * sizeof(vertices[0]));
		tile_ebo.buffer(&indices[0], indices.size() * sizeof(indices[0]));

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		tile_vbo.unbind();
		tile_vao.unbind();
	}

	glm::mat4 transform_matrix(const glm::vec3& position, float scale)
	{
		auto S = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
		auto T = glm::translate(glm::mat4(1.0f), position);
		return T * S;

	}

	void draw(gfx::RenderContext& context) override
	{
		if (!context.is_shadow_pass)
		{
			shader.bind();
			shader.uniform("u_View",			context.camera->get_view_matrix());
			shader.uniform("u_Projection",		context.camera->get_projection_matrix());

			tile_vao.bind();

			bool wireframe = true;
			//glFrontFace(GL_CW);
			glEnable(GL_PRIMITIVE_RESTART);
			glPrimitiveRestartIndex(primitive_restart);

			if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			for (int l = 0; l < levels; l++)
			{
				int rows = 4, columns = 4;
				float border = 0.0f;

				float scale = pow(2.0f, l + 1);
				float offset = segments * (segment_size * scale) + border;
				glm::vec3 start(-offset * 2, 0.0f, -offset * 2);


				for (int r = 0; r < 4; r++)
				{
					for (int c = 0; c < 4; c++)
					{
						if (((r == 0 || r == 3) || (c == 0 || c == 3)) || l == 0 )
						{
							auto tile_pos = start + glm::vec3(r * offset, 0.0f, c * offset);
							shader.uniform("u_Model", transform_matrix(tile_pos, scale));
							shader.uniform("u_Level", scale / levels);
							glDrawElements(GL_TRIANGLE_STRIP, index_count, GL_UNSIGNED_INT, 0);

						}
										}
				}

			}

		





			if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			tile_vao.unbind();


			//glFrontFace(GL_CCW);
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
	const int segments = 5;
	const float segment_size = 2.0f;
};
