#include "terrain.h"

TextureClipmap::TextureClipmap(int clipsize, int levels)
    : m_levels{levels},
      m_tilesize{256},
      m_clipsize{clipsize},
      m_center{0},
      m_virtual_size{m_clipsize * pow2(levels - 1)},
      texture({.texture_size{clipsize, clipsize}, .array_size{levels}})
{
  m_center = m_virtual_size / 2;
  std::cout << "clipmap:" << std::endl;
  std::cout << "levels = " << m_levels << std::endl;
  std::cout << "clipsize = " << m_clipsize << std::endl;
  std::cout << "virtual size = " << m_virtual_size << std::endl;
  std::cout << "inital center = " << m_center << std::endl;
}

int TextureClipmap::pow2(int n) { return 1 << n; }

int TextureClipmap::manhattan(const glm::ivec2& a, const glm::ivec2& b)
{
  return glm::abs(a.x - b.x) + glm::abs(a.y - b.y);
}

// center in uv coordinate [0, 1] of virtual texture size
void TextureClipmap::update(const glm::vec2& center)
{
  glm::ivec2 new_center = center * glm::vec2(m_virtual_size);

  // start at 1, as level 0 is never updated
  for (int level = 1; level < m_levels; level++) {
    // TODO: possibly load new tiles, update center

    auto size = m_clipsize * pow2(level);

    // the 256x256 tile we are currently in
    glm::ivec2 tile_offset = new_center / m_tilesize;

    // TODO: check if manhatten distance from old center (snapped to grid) is larger
    // than some value, if so, move center and load tiles

    glm::ivec2 clipped_center = tile_offset * m_tilesize;

    // shift tiles if difference between new_center and center is greater than one
    // tile in any direction

    glm::ivec2 difference = new_center - m_center;

    auto shift = glm::greaterThan(glm::abs(difference), glm::ivec2(m_tilesize));

    if (glm::any(shift)) {
      m_center += m_tilesize * (glm::sign(difference) * glm::ivec2(shift));

      // TODO:
    }
  }
}

void TextureClipmap::bind(GLuint texture_unit) { texture.bind(texture_unit); }

void TextureClipmap::unbind() { texture.unbind(); }

Seam::Seam(int columns, float size)
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

void Seam::bind() { vao.bind(); }

void Seam::unbind() { vao.unbind(); }

void Seam::draw()
{
  bind();
  glDrawArrays(GL_TRIANGLES, 0, 3 * index_count);
  unbind();
}

Block::Block(int width, int height, float segment_size)
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

void Block::bind() { vao.bind(); }

void Block::unbind() { vao.unbind(); }

void Block::draw()
{
  bind();
  glDrawElements(GL_TRIANGLE_STRIP, index_count, GL_UNSIGNED_INT, 0);
  unbind();
}

GeometryClipmap::GeometryClipmap(int levels_, int segments_)
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
      seam(2 * segments + 2, segment_size * 2),
      m_texture_clipmap(1024, 2)
{
  std::cout << "terrain_size = " << terrain_size << " m" << std::endl;

  auto fog = gfx::rgb(0x5e5e6e);
  auto color = glm::vec4(fog, 1.0f);

  auto p = pixel_from_height(2561.0f);

  terrain.bind();
  terrain.set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  terrain.set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  terrain.set_parameter(GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));

  m_texture_clipmap.texture.bind();
  m_texture_clipmap.texture.set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  m_texture_clipmap.texture.set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  m_texture_clipmap.texture.set_parameter(GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
  m_texture_clipmap.texture.add_image(gfx::Image(PATH + "texture.png"));

  heightmap.bind();
  heightmap.set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  heightmap.set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  heightmap.set_parameter(GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(pixel_from_height(0.0f), 1.0f)));
}

void GeometryClipmap::draw_self(gfx::RenderContext& context)
{
  if (context.shadow_pass) {
    return;
  }
  auto camera_pos = context.camera->get_world_position();
  float height = camera_pos.y;
  auto camera_pos_xy = glm::vec2(camera_pos.x, camera_pos.z);

  // TODO: select proper texture LOD
  glm::vec2 origin = camera_pos_xy - glm::vec2(terrain_size / 2);

  // coordinate in range [-1, 1]
  glm::vec2 relative_position = camera_pos_xy / glm::vec2(terrain_size / 2);

  // std::cout << "relpos = " << relative_position << std::endl;

#if 0
  assert(
    glm::all(glm::greaterThanEqual(glm::vec2(-1.0), relative_position))
    && glm::all(glm::lessThanEqual(relative_position, glm::vec2(1.0)))
  );
#endif

  // remap to [0, 1]
  glm::vec2 clipmap_center = (relative_position + glm::vec2(1.0f)) / 2.0f;

  // std::cout << "cc = " << clipmap_center << std::endl;

  m_texture_clipmap.update(clipmap_center);

  shader.bind();
  shader.set_uniform("u_Heightmap", 2);
  shader.set_uniform("u_Normalmap", 3);
  shader.set_uniform("u_Texture_01", 4);
  shader.set_uniform("u_TextureArray", 8);
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
    m_texture_clipmap.bind(8);
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
