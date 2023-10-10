#include "particles.h"

namespace gfx
{

ParticleSystem::ParticleSystem(const Config& config) 
  : particle_count(config.count)
{

  m_particles.reserve(particle_count);
  m_positions.reserve(particle_count);

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

  m_positions_vbo.bind();
  m_positions_vbo.buffer(m_positions, GL_DYNAMIC_DRAW);

  m_vao.unbind();
}

void ParticleSystem::update(float dt) {
  for (size_t i = 0; i < m_particles.size(); i++) {
    // TODO: update
    auto& particle = m_particles[i];

    particle.lifetime -= dt;

    if (particle.lifetime <= 0) {
      // reset
    }

    particle.position += particle.velocity * dt;
  }

  m_
}

void ParticleSystem::draw_self(RenderContext& context) {
#if 0
  m_vao.bind();
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particle_count);
#endif
}

}  // namespace gfx
