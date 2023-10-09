#include "particles.h"

namespace gfx
{

ParticleSystem::ParticleSystem(const Config& config)
{
  const std::vector<float> quad = {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                   0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                                   1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};

  m_vao.bind();
  m_quad.bind();
  m_quad.buffer(quad);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  m_vao.unbind();
}

void ParticleSystem::draw_self(RenderContext& context) {}

}  // namespace gfx
