#include "util.h"


namespace gfx
{
std::string load_text_file(const std::string& path)
{
  std::fstream file(path);
  if (!file.is_open()) return std::string();

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

float random_01() { return static_cast<float>(rand()) / static_cast<float>(RAND_MAX); }

glm::vec3 point_on_sphere()
{
  float r1 = random_01();
  float r2 = random_01();
  float phi = 2.0 * PI * r1;
  float theta = 2.0 * PI * r2;
  return glm::vec3(cos(phi) * sin(theta), cos(theta), sin(phi) * sin(theta));
}

glm::vec3 point_on_ellipsoid()
{
    return glm::vec3();
}

glm::vec3 point_on_hemisphere(float factor)
{
  float r1 = random_01();
  float r2 = random_01();
  float phi = 2.0 * PI * r1;
  float theta = std::acos(r2) * factor;
  return glm::vec3(cos(phi) * sin(theta), cos(theta), sin(phi) * sin(theta));
}



}  // namespace gfx
