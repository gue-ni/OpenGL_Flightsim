#include "phi.h"

namespace phi
{

Logger::Logger(const std::string& path)
{
  file = new std::ofstream(path, std::ios::out);
  assert(file->is_open());
  *file <<  std::format("time,x,y,z") << std::endl;
}

std::string Logger::to_csv(const glm::vec3& v) { return std::format("{}, {}, {}", v.x, v.y, v.z); }

std::string Logger::to_csv(const glm::quat& q) { return std::format("{}, {}, {}, {}", q.x, q.y, q.z, q.w); }

void Logger::log(const RigidBody* rb, float dt)
{
  timer -= dt;
  time += dt;

  if (timer < 0 && file->is_open()) {
    timer = intervall;

    *file << time << ", " << to_csv(rb->position) << ", " << to_csv(rb->rotation) << ", " << to_csv(rb->velocity)
          << ", " << to_csv(rb->angular_velocity) << ", " << std::endl;
  }
}

}  // namespace phi