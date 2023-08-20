#pragma once

#include "gfx/gfx.h"

constexpr unsigned int primitive_restart = 0xFFFFU;
constexpr float MAX_TILE_SIZE = 50708.0f * 4;

#define DATA_SRC 2

#if (DATA_SRC == 1)
const std::string PATH = "assets/textures/terrain/data/9/268/178/";
const int ZOOM_FACTOR = 1;
#elif (DATA_SRC == 2)
//const std::string PATH = "assets/textures/terrain/data/10/536/356/";
const std::string PATH = "assets/textures/terrain/data/10/536/360/";
//const std::string PATH = "assets/textures/terrain/debug/";
const int ZOOM_FACTOR = 2;
#elif (DATA_SRC == 3)
const std::string PATH = "assets/textures/terrain/data/11/1072/712/";
const int ZOOM_FACTOR = 3;
#elif (DATA_SRC == 4)
const std::string PATH = "assets/textures/terrain/data/12/2144/1424/";
const int ZOOM_FACTOR = 4;
#elif (DATA_SRC == 5)
const std::string PATH = "assets/textures/terrain/data/13/4288/2848/";
const int ZOOM_FACTOR = 5;
#elif (DATA_SRC == 6)
const std::string PATH = "assets/textures/terrain/data/14/8636/5752/";
const int ZOOM_FACTOR = 5;
#else
#error Unknown DATA_SRC
#endif

constexpr gfx::gl::Texture::Params params = {.texture_wrap = GL_REPEAT, .texture_mag_filter = GL_LINEAR};

// pixel value in range [0, 1]
inline float height_from_pixel(const glm::vec3& rgb)
{
  glm::vec3 pixel = rgb * 255.0f;
  return (pixel.r * 256.0f + pixel.g + pixel.b / 256.0f) - 32768.0f;
}

inline float scale(float input_val, float in_min, float in_max, float out_min, float out_max)
{
  return (input_val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline float sample_heightmap(const gfx::Image& heightmap, const glm::vec2& pos, float terrain_size)
{
  glm::vec2 coord = pos / (terrain_size / 2.0f);
  coord.x = scale(coord.x, -1.0, 1.0, 0.0, 1.0);
  coord.y = scale(coord.y, -1.0, 1.0, 0.0, 1.0);

  auto pixel = heightmap.sample(coord);
  return height_from_pixel(glm::vec3(pixel));
}

struct Seam {
  gfx::gl::VertexBuffer vbo;
  gfx::gl::VertexArrayObject vao;
  size_t index_count;

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
    glDrawArrays(GL_TRIANGLES, 0, 3 * index_count);
    unbind();
  }
};

struct Block {
  gfx::gl::VertexBuffer vbo;
  gfx::gl::ElementBuffer ebo;
  gfx::gl::VertexArrayObject vao;
  size_t index_count;

  Block(int width, int height, float segment_size)
  {
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

// Geometry Clipmap
// https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-2-terrain-rendering-using-gpu-based-geometry
// https://mikejsavage.co.uk/blog/geometry-clipmaps.html
class Clipmap : public gfx::Object3D
{
 public:
  Clipmap(int levels_ = 16, int segments_ = 16)
      : segment_size(2.0f),
        levels(levels_),
        segments(segments_),
        terrain_size(MAX_TILE_SIZE / ZOOM_FACTOR),
        shader("shaders/terrain"),
        heightmap_image(PATH + "height.png"),
        heightmap(heightmap_image, params),
        normalmap(PATH + "normal.png", params),
        terrain(PATH + "texture.png", params),
        terrain_0(),
        tile(segments, segments, segment_size),
        col_fixup(2, segments, segment_size),
        row_fixup(segments, 2, segment_size),
        horizontal(2 * segments + 2, 1, segment_size),
        vertical(1, 2 * segments + 2, segment_size),
        center(2 * segments + 2, 2 * segments + 2, segment_size),
        seam(2 * segments + 2, segment_size * 2)
  {
  }

  float get_terrain_height(const glm::vec2 pos) const { return sample_heightmap(heightmap_image, pos, terrain_size); }

  float get_terrain_size() const { return terrain_size; }

  void draw_self(gfx::RenderContext& context) override
  {
    if (context.shadow_pass) {
      return;
    }
    auto camera_pos = context.camera->get_world_position();
    float height = camera_pos.y;
    auto camera_pos_xy = glm::vec2(camera_pos.x, camera_pos.z);

    // TODO: select proper texture LOD
    glm::vec2 origin = camera_pos_xy - glm::vec2(terrain_size / 2);

    shader.bind();
    shader.set_uniform("u_Heightmap", 2);
    shader.set_uniform("u_Normalmap", 3);
    shader.set_uniform("u_Texture_01", 4);
    shader.set_uniform("u_FogColor", context.fog_color);
    shader.set_uniform("u_View", context.camera->get_view_matrix());
    shader.set_uniform("u_CameraPos", context.camera->get_world_position());
    shader.set_uniform("u_Projection", context.camera->get_projection_matrix());
    shader.set_uniform("u_TerrainSize", terrain_size);
    shader.set_uniform("u_Shadowmap", 5);
    shader.set_uniform("u_LightSpaceMatrix", context.light_space_matrix);


    glEnable(GL_CULL_FACE);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(primitive_restart);

    int min_level = 1;  // depends on camera height

    for (int level = min_level; level <= levels; level++) {
      const int rows = 5, columns = 5;
      float scale = std::pow(2.0f, level);
      float next_scale = std::pow(2.0f, level + 2);
      float scaled_segment_size = segment_size * scale;
      float tile_size = segments * scaled_segment_size;
      glm::vec2 snapped = glm::floor(camera_pos_xy / next_scale) * next_scale;
      auto base = calc_base(level, camera_pos_xy);

      shader.set_uniform("u_Scale", scale);
      shader.set_uniform("u_SegmentSize", scaled_segment_size);
      shader.set_uniform("u_Level", static_cast<float>(level) / levels);

#if 1
      // don't render lots of detail if we are very high up
      if (tile_size * 5 < height * 2.5) {
        min_level = level + 1;
        continue;
      }
#endif

      heightmap.bind(2);
      normalmap.bind(3);
      terrain.bind(4);
      context.depth_map->bind(5);

#if 1
      if (level == min_level) {
        shader.set_uniform("u_Model", transform_matrix(base + glm::vec2(tile_size, tile_size), scale));
        center.draw();
      } else {  // not at base level
        auto prev_base = calc_base(level - 1, camera_pos_xy);
        auto diff = glm::abs(base - prev_base);

        auto l_offset = glm::vec2(tile_size, tile_size);
        if (diff.x == tile_size) {
          l_offset.x += (2 * segments + 1) * scaled_segment_size;
        }
        shader.set_uniform("u_Model", transform_matrix(base + l_offset, scale));
        horizontal.draw();

        auto v_offset = glm::vec2(tile_size, tile_size);
        if (diff.y == tile_size) {
          v_offset.y += (2 * segments + 1) * scaled_segment_size;
        }

        shader.set_uniform("u_Model", transform_matrix(base + v_offset, scale));
        vertical.draw();
      }
#endif
#if 1
      glm::vec2 offset(0.0f);
      for (int row = 0; row < rows; row++) {
        offset.y = 0;
        for (int column = 0; column < columns; column++) {
          if (row == 0 || row == rows - 1 || column == 0 || column == columns - 1) {
            auto tile_pos = base + offset;
            shader.set_uniform("u_Model", transform_matrix(tile_pos, scale));

            if ((column != 2) && (row != 2)) {
              if (column == 0 && row == 0)  // east
              {
                shader.set_uniform("u_Model", transform_matrix(tile_pos, scale));
                seam.draw();
              } else if (column == columns - 1 && row == rows - 1)  // west
              {
                shader.set_uniform("u_Model", transform_matrix(tile_pos + glm::vec2(tile_size), scale, 180.0f));
                seam.draw();
              } else if (column == columns - 1 && row == 0)  // south
              {
                shader.set_uniform("u_Model", transform_matrix(tile_pos + glm::vec2(0, tile_size), scale, 90.0f));
                seam.draw();
              } else if (column == 0 && row == rows - 1)  // north
              {
                shader.set_uniform("u_Model", transform_matrix(tile_pos + glm::vec2(tile_size, 0), scale, -90.0f));
                seam.draw();
              }

              shader.set_uniform("u_Model", transform_matrix(tile_pos, scale));
              tile.draw();
            } else if (column == 2) {
              col_fixup.draw();
            } else if (row == 2) {
              row_fixup.draw();
            }
          }

          if (column == 2) {
            offset.y += 2 * scaled_segment_size;
          } else {
            offset.y += tile_size;
          }
        }

        if (row == 2) {
          offset.x += 2 * scaled_segment_size;
        } else {
          offset.x += tile_size;
        }
      }
#endif

      heightmap.unbind();
      normalmap.unbind();
      terrain.unbind();
    }

    glDisable(GL_CULL_FACE);

    shader.unbind();
  }

 private:
  const float segment_size;
  const int levels;
  const int segments;
  const float terrain_size;  // width and length of the terrain represented by the heightmap

  gfx::gl::Shader shader;
  gfx::Image heightmap_image;
  gfx::gl::Texture heightmap, normalmap, terrain, terrain_0;

  Block tile;
  Block center;
  Block col_fixup;
  Block row_fixup;
  Block horizontal;
  Block vertical;
  Seam seam;

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
