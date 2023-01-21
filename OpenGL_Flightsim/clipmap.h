#pragma once

#include "gfx.h"

class Clipmap : public gfx::Object3D {
public:


	Clipmap() 
		: shader("shaders/clipmap")
	{
		// https://www.learnopengles.com/tag/vertex-buffer-object/

#if 1
		std::vector<float> vertices = {
			1.0f, 0.0f, 0.0f, // top left
			1.0f, 0.0,  2.0f, // top right 
			0.0f, 0.0f, 0.0f, // bottom left
			0.0f, 0.0f, 2.0f, // bottom right
		};

		std::vector<unsigned int> indices = {  
			//0, 1, 3,  
			//1, 2, 3   
			1, 0, 2,
			2, 3, 1
		};
#else
		std::vector<float> vertices;
		std::vector<unsigned int> indices;

		int width = 3, height = 2;
		float tile_size = 1.0f;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
			}
		}
#endif

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

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


#if 0
			glDrawArrays(GL_TRIANGLES, 0, 3);
#else
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
#endif


			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			tile_vao.unbind();
		}
	}

private:
	gfx::Shader shader;
	//gfx::Texture heightmap;

	gfx::VertexBuffer tile_vbo;
	gfx::ElementBufferObject tile_ebo;
	gfx::VertexArrayObject tile_vao;

	const int levels = 3;
};
