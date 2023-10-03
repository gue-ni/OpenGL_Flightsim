#include "object3d.h"
#include "gl.h"


constexpr int MAX_NUM_PARTICLES = 1000;

namespace gfx
{

struct Particle {
  glm::vec3 position, velocity;
  glm::vec4 color;
  float lifetime;
  float alpha;

  Particle() :
    position(0.0f),
    velocity(0.0f),
    lifetime(0.0f),
    alpha(0.0f)
  {}
};

class ParticleSystem : public Object3D
{
 public:

   struct Config {
     float lifetime;
   };

  ParticleSystem(const Config& config);
  void draw_self(RenderContext& context) override;
  void update(float dt);

 private:
  gl::VertexArrayObject m_vao;
  gl::VertexBuffer m_vbo;

  const std::string m_shader = "shaders/particle";

  int m_num_particles = 0;

  Particle m_particles[MAX_NUM_PARTICLES];
};
}  // namespace gfx
