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
#if 1
		std::vector<glm::vec3> vertices;
		std::vector<unsigned int> indices;

		generate_mesh(vertices, indices, width, height, segment_size);

		index_count = indices.size();
		//std::cout << "indices = " << indices.size() << std::endl;
		//std::cout << "vertices = " << vertices.size() / 3 << std::endl;

		assert(indices.size() > 0 && vertices.size() > 0);

		vao.generate();
		vbo.generate();
		ebo.generate();

		vao.bind();

		vbo.buffer(&vertices[0], vertices.size() * sizeof(vertices[0]));
		ebo.buffer(&indices[0], indices.size() * sizeof(indices[0]));

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		//vbo.bind();
		vao.unbind();
#endif
	}

	void bind()
	{
		vao.bind();
	}

	void unbind()
	{
		vao.unbind();
	}

	void draw()
	{
		bind();
		glDrawElements(GL_TRIANGLE_STRIP, index_count, GL_UNSIGNED_INT, 0);
		unbind();
	}

};


class Clipmap : public gfx::Object3D {
public:
	Clipmap(int l = 16, int segs = 16, float s = 2.0f) 
		: shader("shaders/clipmap"),
		heightmap("assets/textures/terrain-s.png"),
		levels(l),
		segments(segs),
		segment_size(s)
#if 1
		, tile(segs, segs, s),
		col_fixup(2, segs, s),
		row_fixup(segs, 2, s),
		horizontal(2 * segs + 2, 1, s),
		vertical(1, 2 * segs + 2, s),
		center(2 * segs + 2, 2 * segs + 2, s)
#endif
	{}

	glm::mat4 transform_matrix(const glm::vec2& position, float scale, float angle = 0)
	{
		const auto one = glm::mat4(1.0f);
		auto S = glm::scale(one, glm::vec3(scale));
		auto T = glm::translate(one, glm::vec3(position.x, 0.0f, position.y));
		auto R = glm::rotate(one, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		return T * R * S;
	}

	glm::vec2 calc_base(int level, glm::vec2 camera_pos)
	{
		float scale = pow(2.0f, level);
		float next_scale = pow(2.0f, level+2);
		float tile_size = segments * segment_size * scale;
		glm::vec2 snapped = glm::floor(camera_pos / next_scale) * next_scale;
		glm::vec2 base = snapped - tile_size * 2.0f;
		return base;
	}

	void draw(gfx::RenderContext& context) override
	{
#if 1
		if (!context.is_shadow_pass)
		{
			auto camera_pos = context.camera->get_world_position();
			float height = camera_pos.y;
			//printf("height = %f\n", height);
			glm::vec2 camera_pos_xy = glm::vec2(camera_pos.x, camera_pos.z);


			int unit = 2;
			heightmap.bind(unit);

			shader.bind();
			shader.uniform("u_CameraPos", context.camera->get_world_position());
			shader.uniform("u_View", context.camera->get_view_matrix());
			shader.uniform("u_Projection", context.camera->get_projection_matrix());
			shader.uniform("u_Heightmap", unit);

			bool wireframe = true;

			glEnable(GL_PRIMITIVE_RESTART);
			glPrimitiveRestartIndex(primitive_restart);
			if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			int min_level = 1; // should depend on camera height

			for (int l = min_level; l <= levels; l++)
			{
				int rows = 5, cols = 5;
				float border = 0.0f;
				float scale = pow(2.0f, l);
				float next_scale = pow(2.0f, l+2);
				float bigger_scale = pow(2.0f, l + 1 + 2);
				float scaled_segment_size = segment_size * scale;
				float tile_size = segments * scaled_segment_size;
				glm::vec2 snapped = glm::floor(camera_pos_xy / next_scale) * next_scale;
				auto base = calc_base(l, camera_pos_xy);

				shader.uniform("u_Level", static_cast<float>(l) / levels);
				shader.uniform("u_Scale", scale);

				if (tile_size * 5 < height * 2.5)
				{
					min_level = l + 1;
					continue;
				}

#if 1
				if (l == min_level)
				{
					shader.uniform("u_Model", transform_matrix(base + glm::vec2(tile_size, tile_size), scale));
					center.draw();

				}
				else
				{
					auto prev_base = calc_base(l - 1, camera_pos_xy);
					auto diff = glm::abs(base - prev_base);

					auto l_offset = glm::vec2(tile_size, tile_size);
					if (diff.x == tile_size)
					{
						l_offset.x += (2 * segments + 1) * scaled_segment_size;
					}
					shader.uniform("u_Model", transform_matrix(base + l_offset, scale));
					horizontal.draw();

					auto v_offset = glm::vec2(tile_size, tile_size);
					if (diff.y == tile_size)
					{
						v_offset.y += (2 * segments + 1) * scaled_segment_size;
					}
					shader.uniform("u_Model", transform_matrix(base + v_offset, scale));
					vertical.draw();
				}
#endif

				glm::vec2 offset(0.0f);
				for (int r = 0; r < rows; r++)
				{
					offset.y = 0;
					for (int c = 0; c < cols; c++)
					{
						if 
						(
							((r == 0 || r == rows - 1) || 
							(c == 0 || c == cols - 1)) 
						)
						{
							auto tile_pos = base + offset;
							shader.uniform("u_Model", transform_matrix(tile_pos, scale));

							if ((c != 2) && (r != 2))
							{
								tile.draw();
							}
							else if(c == 2)
							{
								col_fixup.draw();
							}
							else if(r == 2)
							{
								row_fixup.draw();
							}
						}

						if (c == 2)
						{
							offset.y += 2 * scaled_segment_size;
						}
						else
						{
							offset.y += tile_size;
						}
					}

					if (r == 2)
					{
						offset.x += 2 * scaled_segment_size;
					}
					else
					{
						offset.x += tile_size;
					}
				}
			}

			if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			shader.unbind();
		}
#endif
	}

private:
	gfx::Shader shader;
	gfx::Texture heightmap;

#if 1
	Block tile; 
	Block center; 
	Block col_fixup; 
	Block row_fixup; 
	Block horizontal;
	Block vertical;
#endif

	unsigned int index_count = 0;
	const int levels;
	const int segments;
	const float segment_size;
};
