#pragma once

#include "gfx.h"

constexpr unsigned int primitive_restart = 0xFFFFU;
const float MAX_TILE_SIZE = 50708.0f * 4;

#define DATA_SRC 0

#if (DATA_SRC == 1)
const std::string PATH = "assets/textures/terrain/data/9/268/178/";
const int ZOOM_FACTOR = 1;
#elif (DATA_SRC == 2)
// vienna
const std::string PATH = "assets/textures/terrain/data/10/557/354/";
const int ZOOM_FACTOR = 2;
#else
// vorarlberg
const std::string PATH = "assets/textures/terrain/data/10/536/356/";
const int ZOOM_FACTOR = 2;
#endif

const gfx::gl::TextureParams params = {.texture_wrap = GL_REPEAT, .texture_mag_filter = GL_LINEAR};

struct Seam {
  gfx::gl::VertexBuffer vbo;
  gfx::gl::VertexArrayObject vao;
  unsigned int index_count;

  Seam(int columns, float size)
  {
    int rows = 1;
    index_count = columns;

    std::vector<glm::vec3> vertices;

    int x;
    for (x = 0; x < columns; x++) {
      vertices.push_back({x * size, 0.0f, 0 * size});
      vertices.push_back({x * size + size / 2.0f, size, 0 * size});
      vertices.push_back({x * size + size, 0.0f, 0 * size});
    }

    vao.bind();
    vbo.buffer(&vertices[0], vertices.size() * sizeof(vertices[0]));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    vao.unbind();
  }

  void bind() { vao.bind(); }

  void unbind() { vao.unbind(); }

  void draw()
  {
    bind();
    // glDrawElements(GL_TRIANGLE_STRIP, index_count, GL_UNSIGNED_INT, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3 * index_count);
    unbind();
  }
};

struct Block {
  gfx::gl::VertexBuffer vbo;
  gfx::gl::ElementBufferObject ebo;
  gfx::gl::VertexArrayObject vao;
  unsigned int index_count;

  Block(int width, int height, float segment_size)
  {
#if 1
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

    for (int y = 0; y <= width; y++) {
      for (int x = 0; x <= height; x++) {
        vertices.push_back({x * segment_size, 0.0f, y * segment_size});
      }
    }

    for (int r = 0; r < width; r++) {
      for (int c = 0; c < height + 1; c++) {
        auto i0 = (r + 0) * (height + 1) + c;
        indices.push_back(i0);

        auto i1 = (r + 1) * (height + 1) + c;
        indices.push_back(i1);
      }
      indices.push_back(primitive_restart);  // restart primitive
    }

    index_count = indices.size();

    assert(indices.size() > 0 && vertices.size() > 0);

    vao.bind();

    vbo.buffer(&vertices[0], vertices.size() * sizeof(vertices[0]));
    ebo.buffer(&indices[0], indices.size() * sizeof(indices[0]));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    vao.unbind();
#endif
  }

  void bind() { vao.bind(); }

  void unbind() { vao.unbind(); }

  void draw()
  {
    bind();
    glDrawElements(GL_TRIANGLE_STRIP, index_count, GL_UNSIGNED_INT, 0);
    unbind();
  }
};

class Clipmap : public gfx::Object3D
{
 public:
  bool wireframe = false;

  Clipmap(int levels = 16, int segments = 32, float segment_size = 2.0f)
      : shader("shaders/terrain"),
        heightmap(PATH + "heightmap.png", params),
        normalmap(PATH + "normalmap.png", params),
        terrain(PATH + "texture.png", params),
        levels(levels),
        segments(segments),
        segment_size(segment_size),
        tile(segments, segments, segment_size),
        col_fixup(2, segments, segment_size),
        row_fixup(segments, 2, segment_size),
        horizontal(2 * segments + 2, 1, segment_size),
        vertical(1, 2 * segments + 2, segment_size),
        center(2 * segments + 2, 2 * segments + 2, segment_size),
        seam(2 * segments + 2, segment_size * 2),
        terrain_size(MAX_TILE_SIZE / ZOOM_FACTOR)
  {
  }

  float get_terrain_height(glm::vec2 coords) { return 0.0f; }

  void draw_self(gfx::RenderContext& context) override
  {
#if 1
    if (!context.is_shadow_pass) {
      auto camera_pos = context.camera->get_world_position();
      float height = camera_pos.y;
      auto camera_pos_xy = glm::vec2(camera_pos.x, camera_pos.z);

      heightmap.bind(2);
      normalmap.bind(3);
      terrain.bind(4);

      shader.bind();
      shader.uniform("u_Heightmap", 2);
      shader.uniform("u_Normalmap", 3);
      shader.uniform("u_Texture", 4);
      shader.uniform("u_Background", context.background_color);
      shader.uniform("u_View", context.camera->get_view_matrix());
      shader.uniform("u_CameraPos", context.camera->get_world_position());
      shader.uniform("u_Projection", context.camera->get_projection_matrix());
      shader.uniform("u_TerrainSize", terrain_size);

      glEnable(GL_CULL_FACE);
      glEnable(GL_PRIMITIVE_RESTART);
      glPrimitiveRestartIndex(primitive_restart);
      if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      int min_level = 1;  // should depend on camera height

      for (int l = min_level; l <= levels; l++) {
        const int rows = 5, cols = 5;
        // float border = 0.0f;
        float scale = std::pow(2.0f, l);
        float next_scale = std::pow(2.0f, l + 2);
        float scaled_segment_size = segment_size * scale;
        float tile_size = segments * scaled_segment_size;
        glm::vec2 snapped = glm::floor(camera_pos_xy / next_scale) * next_scale;
        auto base = calc_base(l, camera_pos_xy);

        shader.uniform("u_Scale", scale);
        shader.uniform("u_SegmentSize", scaled_segment_size);
        shader.uniform("u_Level", static_cast<float>(l) / levels);

#if 1
        // don't render lots of detail if we are very high up
        if (tile_size * 5 < height * 2.5) {
          min_level = l + 1;
          continue;
        }
#endif
#if 1
        if (l == min_level) {
          shader.uniform("u_Model", transform_matrix(base + glm::vec2(tile_size, tile_size), scale));
          center.draw();
        } else {
          auto prev_base = calc_base(l - 1, camera_pos_xy);
          auto diff = glm::abs(base - prev_base);

          auto l_offset = glm::vec2(tile_size, tile_size);
          if (diff.x == tile_size) {
            l_offset.x += (2 * segments + 1) * scaled_segment_size;
          }
          shader.uniform("u_Model", transform_matrix(base + l_offset, scale));
          horizontal.draw();

          auto v_offset = glm::vec2(tile_size, tile_size);
          if (diff.y == tile_size) {
            v_offset.y += (2 * segments + 1) * scaled_segment_size;
          }

          shader.uniform("u_Model", transform_matrix(base + v_offset, scale));
          vertical.draw();
        }
#endif

        glm::vec2 offset(0.0f);
        for (int r = 0; r < rows; r++) {
          offset.y = 0;
          for (int c = 0; c < cols; c++) {
            if (r == 0 || r == rows - 1 || c == 0 || c == cols - 1) {
              auto tile_pos = base + offset;
              shader.uniform("u_Model", transform_matrix(tile_pos, scale));

              if ((c != 2) && (r != 2)) {
                if (c == 0 && r == 0)  // east
                {
                  shader.uniform("u_Model", transform_matrix(tile_pos, scale));
                  seam.draw();
                } else if (c == cols - 1 && r == rows - 1)  // west
                {
                  shader.uniform("u_Model", transform_matrix(tile_pos + glm::vec2(tile_size), scale, 180.0f));
                  seam.draw();
                } else if (c == cols - 1 && r == 0)  // south
                {
                  shader.uniform("u_Model", transform_matrix(tile_pos + glm::vec2(0, tile_size), scale, 90.0f));
                  seam.draw();
                } else if (c == 0 && r == rows - 1)  // north
                {
                  shader.uniform("u_Model", transform_matrix(tile_pos + glm::vec2(tile_size, 0), scale, -90.0f));
                  seam.draw();
                }

                shader.uniform("u_Model", transform_matrix(tile_pos, scale));
                tile.draw();
              } else if (c == 2) {
                col_fixup.draw();
              } else if (r == 2) {
                row_fixup.draw();
              }
            }

            if (c == 2) {
              offset.y += 2 * scaled_segment_size;
            } else {
              offset.y += tile_size;
            }
          }

          if (r == 2) {
            offset.x += 2 * scaled_segment_size;
          } else {
            offset.x += tile_size;
          }
        }
      }

      if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glDisable(GL_CULL_FACE);

      shader.unbind();
    }
#endif
  }

 private:
  gfx::gl::Shader shader;
  gfx::gl::Texture heightmap;
  gfx::gl::Texture normalmap;
  gfx::gl::Texture terrain;

  Block tile;
  Block center;
  Block col_fixup;
  Block row_fixup;
  Block horizontal;
  Block vertical;
  Seam seam;

  unsigned index_count = 0;
  const int levels;
  const int segments;
  const float terrain_size;  // width and length of the terrain represented by the heightmap
  const float segment_size;

  glm::vec2 calc_base(int level, glm::vec2 camera_pos)
  {
    float scale = std::pow(2.0f, level);
    float next_scale = std::pow(2.0f, level + 2);
    float tile_size = segments * segment_size * scale;
    glm::vec2 snapped = glm::floor(camera_pos / next_scale) * next_scale;
    glm::vec2 base = snapped - tile_size * 2.0f;
    return base;
  }

  glm::mat4 transform_matrix(const glm::vec2& position, float scale, float angle = 0)
  {
    auto S = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
    auto T = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, 0.0f, position.y));
    auto R = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    return T * R * S;
  }
};