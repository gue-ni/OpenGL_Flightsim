#pragma once

#include "object3d.h"
#include "gl.h"
#include "util.h"

namespace gfx
{

template <typename T>
struct Range {
  T min_value;
  T max_value;
  Range(T min, T max) : min_value(min), max_value(max) {}
  T random_in_range() { return glm::mix(min_value, max_value, random_01()); }
};

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
    float emitter_radius = 1.0f;
    float emitter_cone = 0.8f;
    float speed = 30.0f;
    float size = 0.2f;
    float lifetime = 1.0f;
  };

  ParticleSystem(const Config& config);
  void draw_self(RenderContext& context) override;
  void update(float dt);

 private:
  int find_unused_particle();

  Config m_config;

  int m_last_used_particle;
  int m_particle_count;
  const int m_max_particle_count;
  gl::VertexArrayObject m_vao;
  gl::VertexBuffer m_quad, m_positions, m_colors;
  const std::string m_shader = "shaders/particle";
  std::vector<Particle> m_particles;
  std::vector<glm::vec4> m_position_buffer;  // x,y,z,size
  std::vector<glm::vec4> m_color_buffer;     // rgba
  // gl::TexturePtr m_texture;
};
}  // namespace gfx
