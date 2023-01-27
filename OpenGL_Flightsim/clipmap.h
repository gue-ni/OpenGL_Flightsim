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


struct Block {
	gfx::VertexBuffer vbo;
	gfx::ElementBufferObject ebo;
	gfx::VertexArrayObject vao;
	unsigned int index_count;

	Block(int width, int height, float segment_size)
	{
		printf("w = %d, h = %d, s = %f\n", width, height, segment_size);
		std::vector<glm::vec3> vertices;
		std::vector<unsigned int> indices;

		generate_mesh(vertices, indices, width, height, segment_size);

		index_count = indices.size();
		std::cout << "indices = " << indices.size() << std::endl;
		std::cout << "vertices = " << vertices.size() / 3 << std::endl;

		assert(indices.size() > 0 && vertices.size() > 0);

		vao.bind();
		vbo.buffer(&vertices[0], vertices.size() * sizeof(vertices[0]));
		ebo.buffer(&indices[0], indices.size() * sizeof(indices[0]));

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		vbo.unbind();
		vao.unbind();


	}

	void bind()
	{
		vao.bind();
	}

	void draw()
	{
		bind();
		glDrawElements(GL_TRIANGLE_STRIP, index_count, GL_UNSIGNED_INT, 0);
	}

};


class Clipmap : public gfx::Object3D {
public:
	Clipmap() 
		: shader("shaders/clipmap"),
		segments(3),
		segment_size(2.0f),
		block(3, 3, 2.0f),
		fix_up(2, 3, 2.0f)
	{
#if 0
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
#endif
	}

	glm::mat4 transform_matrix(const glm::vec2& position, float scale, float angle = 0)
	{
		const auto one = glm::mat4(1.0f);
		auto S = glm::scale(one, glm::vec3(scale));
		auto T = glm::translate(one, glm::vec3(position.x, 0.0f, position.y));
		auto R = glm::rotate(one, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		return T * R * S;
	}

	void draw(gfx::RenderContext& context) override
	{
		if (!context.is_shadow_pass)
		{

			auto camera_pos = context.camera->get_world_position();
			glm::vec2 camera_pos_xy = glm::vec2(camera_pos.x, camera_pos.z);

			shader.bind();
			shader.uniform("u_View", context.camera->get_view_matrix());
			shader.uniform("u_Projection", context.camera->get_projection_matrix());

			bool wireframe = true;

			//glFrontFace(GL_CW);
			glEnable(GL_PRIMITIVE_RESTART);
			glPrimitiveRestartIndex(primitive_restart);
			if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			for (int l = 0; l <= levels; l++)
			{
				int rows = 5, cols = 5;
				float border = 0.0f;

				float scale = pow(2.0f, l);
				float next_scale = pow(2.0f, l+2);
				float scaled_segment_size = segment_size * scale;
				float tile_size = segments * scaled_segment_size;
				glm::vec2 offset = { tile_size + border, tile_size + border };


				glm::vec2 snapped = glm::floor(camera_pos_xy / next_scale) * next_scale;
				glm::vec2 start = snapped -  offset * 2.0f;

				glm::vec3 tmp_offset(0.0f);

				for (int r = 0; r < rows; r++)
				{
					//float y_offset = 0.0f;
					tmp_offset.y = 0;

					for (int c = 0; c < cols; c++)
					{
						if 
						(
							((r == 0 || r == rows - 1) || 
							(c == 0 || c == cols - 1)) 
							//&& (c != 2) && (r != 2)
						)
						{

							auto tile_pos = start + glm::vec2(tmp_offset.x, tmp_offset.y);
							shader.uniform("u_Model", transform_matrix(tile_pos, scale));
							shader.uniform("u_Level", static_cast<float>(l) / levels);

							if ((c != 2) && (r != 2))
							{
								block.draw();
							}
							else if(c == 2)
							{
								fix_up.draw();
							}
							else
							{
								//auto pos = tile_pos;
								auto pos = tile_pos + glm::vec2(2 * scaled_segment_size, 0);
								shader.uniform("u_Model", transform_matrix(pos, scale, -gfx::PI / 2));
								fix_up.draw();
							}
						}

#if 1
						if (c == 2)
						{
							tmp_offset.y += 2 * scaled_segment_size;
						}
						else
						{
							tmp_offset.y += tile_size;
						}
#else
						tmp_offset.x += tile_size;
						tmp_offset.y += tile_size;
#endif
					}

					if (r == 2)
					{
						tmp_offset.x += 2 * scaled_segment_size;
					}
					else
					{
						tmp_offset.x += tile_size;
					}
				}
			}

			if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			//glFrontFace(GL_CCW);
		}
	}

private:
	gfx::Shader shader;
	//gfx::Texture heightmap;

#if 0
	gfx::VertexBuffer tile_vbo;
	gfx::ElementBufferObject tile_ebo;
	gfx::VertexArrayObject tile_vao;
#else
	Block block; 
	Block fix_up; 
#endif

	unsigned int index_count = 0;
	const int levels = 3;
	const int segments = 3;
	const float segment_size = 2.0f;
};
