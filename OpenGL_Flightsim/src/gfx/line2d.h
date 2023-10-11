#pragma once

#include "gfx.h"
#include <tuple>

namespace gfx {

using Line = std::tuple<glm::vec2, glm::vec2>;

// context for drawing 2d lines 
class Line2d : public Object3D 
{
public:
  Line2d();
  void draw_self(RenderContext& context) override;
  void batch_line(const Line& line);
  void batch_line(const Line& line, float angle);
  void batch_line(const Line& line, const glm::mat4& matrix);
  void batch_line(float width, const glm::mat4& matrix);
  void batch_circle(const glm::vec2& center, float radius, int points = 16);
  void batch_clear();
private:
  std::vector<Line> m_lines;
  gl::VertexArrayObject vao;
  gl::VertexBuffer vbo;
};


}
