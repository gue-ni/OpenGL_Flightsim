#pragma once

#include "object3d.h"
#include "gl.h"

namespace gfx
{

struct Particle {
  glm::vec3 position, velocity;
  glm::vec4 color;
  float lifetime, distance_from_camera, size;

  Particle() : position(0.0f), velocity(0.0f), color(0.0f), lifetime(0.0f), distance_from_camera(-1.0f), size(1.0f) {}

  bool operator<(Particle& that) { return this->distance_from_camera > that.distance_from_camera; }
};

// https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
class ParticleSystem : public Object3D
{
 public:
  struct Config {
    size_t count;
    std::string texture_path;
  };

  ParticleSystem(const Config& config);
  void draw_self(RenderContext& context) override;
  void update(float dt);

 private:
  int find_unused_particle();
  int m_last_used_particle;
  int m_particle_count;
  const int m_max_particle_count;
  gl::VertexArrayObject m_vao;
  gl::VertexBuffer m_quad, m_positions;
  const std::string m_shader = "shaders/particle";
  std::vector<Particle> m_particles;
  std::vector<glm::vec3> m_position_buffer;
  //gl::TexturePtr m_texture;
};
}  // namespace gfx
