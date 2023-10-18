#pragma once

#include "gfx/gfx.h"

constexpr unsigned int primitive_restart = 0xFFFFU;
constexpr float MAX_TILE_SIZE = 50708.0f * 4;

#define DATA_SRC 5

#if (DATA_SRC == 1)
const std::string PATH = "assets/textures/terrain/data/9/268/178/";
const int ZOOM_FACTOR = 1;
#elif (DATA_SRC == 2)
const std::string PATH = "assets/textures/terrain/data/10/536/356/";
/// const std::string PATH = "assets/textures/terrain/data/10/536/360/";
// const std::string PATH = "assets/textures/terrain/debug/";
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

constexpr gfx::gl::Texture::Params params = {.wrap = GL_REPEAT, .mag_filter = GL_LINEAR};

// pixel value in range [0, 1]
inline float height_from_pixel(const glm::vec3& rgb)
{
  glm::vec3 pixel = rgb * 255.0f;
  return (pixel.r * 256.0f + pixel.g + pixel.b / 256.0f) - 32768.0f;
}

inline glm::vec3 pixel_from_height(float height)
{
  // TODO
  const float c = 32768.0f;
  glm::vec3 pixel(0.0f);

  double decodedHeight = height + 32768;
  int redDec = static_cast<int>(decodedHeight / 256);
  int greenDec = static_cast<int>(decodedHeight) % 256;
  // int blueDec = static_cast<int>((decodedHeight * 256 - greenDec - redDec * 256) / 65536);
  int blueDec = static_cast<int>(((decodedHeight * 256) - (greenDec * 256) - (redDec * 65536)) / 256);

  return pixel / 256.0f;
}

inline float scale(float input_val, float in_min, float in_max, float out_min, float out_max)
{
  return (input_val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

  // [-1, 1] -> [0, 1]
  // (input_val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  // (input_val - -1) * (1 - 0) / (1 - -1) + 0
  // (input_val + 1) / 2
inline float sample_heightmap(const gfx::Image& heightmap, const glm::vec2& pos, float terrain_size)
{
  glm::vec2 coord = pos / (terrain_size / 2.0f);
  coord.x = scale(coord.x, -1.0, 1.0, 0.0, 1.0);
  coord.y = scale(coord.y, -1.0, 1.0, 0.0, 1.0);

  auto pixel = heightmap.sample(coord);
  return height_from_pixel(glm::vec3(pixel));
}

class Seam {
public:
  gfx::gl::VertexBuffer vbo;
  gfx::gl::VertexArrayObject vao;
  size_t index_count;

  Seam(int columns, float size);
  void bind();
  void unbind();
  void draw();
};

class Block {
public:
  gfx::gl::VertexBuffer vbo;
  gfx::gl::ElementBuffer ebo;
  gfx::gl::VertexArrayObject vao;
  size_t index_count;

  Block(int width, int height, float segment_size);
  void bind();
  void unbind();
  void draw();
};

// A TextureClipmap is used to represent a Texture of arbitrary 'virtual' size
// Level 0 is the largest, with the highest resolution
// https://www-f9.ijs.si/~matevz/docs/007-2392-003/sgi_html/ch08.html#LE62092-PARENT
class TextureClipmap
{
 public:
  TextureClipmap(int clipsize, int levels);

  // update center in texel coordinate space
  void update(const glm::vec2& center);

  // load grid of 4x4 tiles centered on 'center_tile'
  void load_tiles(int level, const glm::ivec2& center_tile);

  void bind(GLuint texture_unit);
  void unbind();

  gfx::gl::TextureArray texture;

 private:
   const int m_levels;
   const int m_tilesize;
   const glm::ivec2 m_clipsize;
   const glm::ivec2 m_virtual_size;
   std::vector<glm::ivec2> m_centers;



   static int pow2(int n);
   // manhattan distance
   static int manhattan(const glm::ivec2& a, const glm::ivec2& b);
};

// https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-2-terrain-rendering-using-gpu-based-geometry
// https://mikejsavage.co.uk/blog/geometry-clipmaps.html
class GeometryClipmap : public gfx::Object3D
{
 public:
  GeometryClipmap(int levels_ = 12, int segments_ = 16);

  float get_terrain_height(const glm::vec2 pos) const { return sample_heightmap(heightmap_image, pos, terrain_size); }

  float get_terrain_size() const { return terrain_size; }

  void draw_self(gfx::RenderContext& context) override;

 private:
  const float segment_size;
  const int levels;
  const int segments;
  const float terrain_size;  // width and length of the terrain represented by the heightmap

  gfx::gl::Shader shader;
  gfx::Image heightmap_image;
  gfx::gl::Texture heightmap, normalmap, terrain, terrain_0;

  TextureClipmap m_texture_clipmap;

  Block tile, center, col_fixup, row_fixup, horizontal, vertical;
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
