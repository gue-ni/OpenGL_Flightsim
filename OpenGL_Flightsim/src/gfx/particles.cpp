#include "gfx.h"
#include "particles.h"

namespace gfx
{

ParticleSystem::ParticleSystem(const Config& config, const std::string& path)
    : m_config(config),
      m_max_particle_count(static_cast<int>(config.particles_per_second * config.lifetime.max_value)),
      m_particles(config.count),
      m_position_buffer(config.count),
      m_color_buffer(config.count),
      m_last_used_particle(0),
      m_particle_count(0),
      m_texture(gl::Texture::load(path))
{
  const std::vector<float> vertex_buffer_data = {
      -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f,
  };

  m_vao.bind();

  m_quad.bind();
  m_quad.buffer(vertex_buffer_data);

  m_positions.bind();
  m_positions.buffer_data(nullptr, sizeof(m_position_buffer[0]) * m_position_buffer.size(), GL_STREAM_DRAW);

  m_colors.bind();
  m_colors.buffer_data(nullptr, sizeof(m_color_buffer[0]) * m_color_buffer.size(), GL_STREAM_DRAW);

  m_vao.unbind();
}

int ParticleSystem::find_unused_particle()
{
  for (int i = m_last_used_particle; i < m_max_particle_count; i++) {
    if (m_particles[i].lifetime <= 0.0f) {
      m_last_used_particle = i;
      return i;
    }
  }

  for (int i = 0; i < m_last_used_particle; i++) {
    if (m_particles[i].lifetime <= 0.0f) {
      m_last_used_particle = i;
      return i;
    }
  }

  return 0;
}

void ParticleSystem::update(float dt, const glm::vec3& camera_position, const glm::vec3& emitter_velocity)
{
  int new_particles = static_cast<int>(dt * m_config.particles_per_second);
  new_particles = glm::max(new_particles, 1);

  // std::cout << "new_particles " << new_particles << std::endl;

  glm::vec3 world_position = get_world_position();

  // create new particles
  for (int i = 0; i < new_particles; i++) {
    int unused_particle = find_unused_particle();

    Particle* particle = &m_particles[unused_particle];

    glm::vec4 color = glm::vec4(m_config.color.min_value, 0.0f);

    auto rotation = get_world_rotation_quat();

    float speed = m_config.speed.value();

    glm::vec3 direction = point_on_hemisphere(m_config.emitter_cone);

    auto velocity = rotation * (direction * speed);

    particle->distance_from_camera = 1.0f;
    particle->position = world_position + point_on_sphere() * m_config.emitter_radius;
    particle->velocity = emitter_velocity + velocity;  // TODO: fix this
    particle->color = color;
    particle->size = m_config.size.value();
    particle->lifetime = m_config.lifetime.value();
  }

  // update particles
  m_particle_count = 0;

  for (int i = 0; i < m_max_particle_count; i++) {
    Particle* particle = &m_particles[i];

    particle->lifetime -= dt;

    if (0 < particle->lifetime) {
      float t = particle->lifetime / m_config.lifetime.max_value;
      particle->position += particle->velocity * dt;
      particle->size *= 0.95f;
      particle->color = glm::vec4(m_config.color.value(t), glm::mix(m_config.start_alpha, 0.0f, t));
      particle->distance_from_camera = glm::length(particle->position - camera_position);
      m_particle_count++;
    } else {
      particle->distance_from_camera = -1.0f;
    }
  }

  std::sort(m_particles.begin(), m_particles.end());

  for (int i = 0; i < m_particle_count; i++) {
    m_position_buffer[i] = glm::vec4(m_particles[i].position, m_particles[i].size);
    m_color_buffer[i] = m_particles[i].color;
  }

  m_vao.bind();
  m_positions.bind();
  m_positions.buffer_sub_data(0, sizeof(m_position_buffer[0]) * m_position_buffer.size(), &m_position_buffer[0]);

  m_colors.bind();
  m_colors.buffer_sub_data(0, sizeof(m_color_buffer[0]) * m_color_buffer.size(), &m_color_buffer[0]);
  m_vao.unbind();
}

void ParticleSystem::draw_self(RenderContext& context)
{
  if (context.shadow_pass) return;

  gl::ShaderPtr shader = context.shaders->get_shader(m_shader);

  auto camera = context.camera;

  auto view = camera->get_view_matrix();
  auto projection = camera->get_projection_matrix();

  glm::vec3 up = {view[0][1], view[1][1], view[2][1]};
  glm::vec3 right = {view[0][0], view[1][0], view[2][0]};

  // get currently set blend func
  GLint blendSrc, blendDst;
  glGetIntegerv(GL_BLEND_SRC, &blendSrc);
  glGetIntegerv(GL_BLEND_DST, &blendDst);

  glEnable(GL_BLEND);
  glBlendFunc(m_config.blend_src, m_config.blend_dest);

  shader->bind();
  shader->set_uniform("u_View", view);
  shader->set_uniform("u_Projection", projection);
  shader->set_uniform("u_Right", right);
  shader->set_uniform("u_Up", up);

  if (m_texture != nullptr) {
    m_texture->bind(5);
    shader->set_uniform("u_Texture", 5);
  }

  m_vao.bind();

  // vertex positions
  glEnableVertexAttribArray(0);
  m_quad.bind();
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  // particle positions
  glEnableVertexAttribArray(1);
  m_positions.bind();
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glEnableVertexAttribArray(2);
  m_colors.bind();
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glVertexAttribDivisor(0, 0);
  glVertexAttribDivisor(1, 1);
  glVertexAttribDivisor(2, 1);

  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_particle_count);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);

  m_vao.unbind();
  shader->unbind();

  // reset blend func
  glBlendFunc(blendSrc, blendDst);
}

}  // namespace gfx
