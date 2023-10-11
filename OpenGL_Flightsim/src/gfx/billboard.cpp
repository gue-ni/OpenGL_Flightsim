#include "billboard.h"
namespace gfx
{
Billboard::Billboard(gl::TexturePtr sprite, glm::vec3 color)
    : texture(sprite), shader("shaders/billboard"), color(color)
{
  // clang-format off
  float vertices[] = {
      0.5f,  0.5f,  0.0f, 
      0.0f, 0.0f, 0.5f,  
      -0.5f, 0.0f, 0.0f, 
      1.0f,-0.5f, -0.5f, 
      0.0f, 1.0f, 1.0f, 
      -0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
  };
  // clang-format on

  unsigned int indices[] = {
      0, 1, 3,  // first Triangle
      1, 2, 3   // second Triangle
  };

  vao.bind();
  vbo.buffer(vertices, sizeof(vertices));

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  ebo.buffer(indices, sizeof(indices));
  vao.unbind();
}

void Billboard::draw_self(RenderContext& context)
{
  if (context.shadow_pass) return;

  auto camera = context.camera;

#if 1
  auto view = camera->get_view_matrix();
#else
  glm::mat4 tmp_transform;
  auto r = get_rotation();
  camera->rotate_by(glm::vec3(+r.x, 0, 0));
  // auto view = camera->get_view_matrix();
  auto view = glm::inverse(camera->get_transform());
  camera->rotate_by(glm::vec3(-r.x, 0, 0));
#endif

  glm::vec3 up = {view[0][1], view[1][1], view[2][1]};
  glm::vec3 right = {view[0][0], view[1][0], view[2][0]};

  shader.bind();
  shader.set_uniform("u_View", view);
  shader.set_uniform("u_Projection", camera->get_projection_matrix());
  shader.set_uniform("u_Texture_01", 5);
  shader.set_uniform("u_Color", color);
  shader.set_uniform("u_Position", get_world_position());
  shader.set_uniform("u_Scale", get_scale());
  shader.set_uniform("u_Right", right);
  shader.set_uniform("u_Up", up);
  texture->bind(5);

  vao.bind();

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // TODO draw
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  vao.unbind();

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
}

}  // namespace gfx