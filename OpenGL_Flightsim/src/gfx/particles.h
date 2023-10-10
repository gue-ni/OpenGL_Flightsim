#include "object3d.h"
#include "gl.h"


constexpr int MAX_NUM_PARTICLES = 1000;

namespace gfx
{

struct Particle {
  glm::vec3 position, velocity, color;
  float lifetime;
  float alpha;

  Particle() :
    position(0.0f),
    velocity(0.0f),
    lifetime(0.0f),
    alpha(0.0f),
    color(glm::vec3(1.0f, 0.0f, 0.0f))
  {}
};

class ParticleSystem : public Object3D
{
 public:

   struct Config {
      size_t count;
      float lifetime;
   };

  ParticleSystem(const Config& config);
  void draw_self(RenderContext& context) override;
  void update(float dt);

 private:
  gl::VertexArrayObject m_vao;
  gl::VertexBuffer m_quad;
  gl::VertexBuffer m_positions_vbo;

  const std::string m_shader = "shaders/particle";

  const size_t particle_count;

  std::vector<Particle> m_particles;
  std::vector<glm::vec3> m_positions;
};
}  // namespace gfx
