#include "line2d.h"
namespace gfx {
Line2d::Line2d()
{
  vao.bind();

  vbo.bind();

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
  glEnableVertexAttribArray(0);

  vao.unbind();
}

void Line2d::batch_line(const Line& line) { m_lines.push_back(line); }

void Line2d::batch_line(const Line& line, float angle)
{
  glm::vec2 p0 = std::get<0>(line), p1 = std::get<1>(line);

  float sin_theta = std::sin(angle);
  float cos_theta = std::cos(angle);

  m_lines.push_back({{p0.x * cos_theta - p0.y * sin_theta, p0.x * sin_theta + p0.y * cos_theta},
                     {p1.x * cos_theta - p1.y * sin_theta, p1.x * sin_theta + p1.y * cos_theta}});
}

void Line2d::batch_line(const Line& line, const glm::mat4& matrix) {

  glm::vec2 p0 = std::get<0>(line), p1 = std::get<1>(line);

  glm::vec4 t0 = matrix * glm::vec4(p0.x, p0.y, 0.0f, 1.0f);
  glm::vec4 t1 = matrix * glm::vec4(p1.x, p1.y, 0.0f, 1.0f);

  batch_line({{t0.x, t0.y}, {t1.x, t1.y}});
}

void Line2d::batch_line(float width, const glm::mat4& matrix) {}

void Line2d::batch_circle(const glm::vec2& center, float radius, int points)
{
#if 1
  float angle = (2 * 3.14f) / points;

  for (int i = 0; i < points; i++) {
    glm::vec2 p0;
    p0.x = radius * std::sin(angle * (i + 0)) + center.x;
    p0.y = radius * std::cos(angle * (i + 0)) + center.y;

    glm::vec2 p1;
    p1.x = radius * std::sin(angle * (i + 1)) + center.x;
    p1.y = radius * std::cos(angle * (i + 1)) + center.y;

    batch_line({p0, p1});
  }
#endif
}

void Line2d::batch_clear() { m_lines.clear(); }

void Line2d::draw_self(RenderContext& context)
{
  if (!context.shadow_pass && context.shaders) {
    auto camera = context.camera;
    auto shader = context.shaders->get_shader("shaders/line");
    shader->bind();

    vao.bind();

    vbo.bind();
    vbo.buffer(m_lines);

    glDrawArrays(GL_LINES, 0, m_lines.size() * 2);
    vao.unbind();
  }
}

}