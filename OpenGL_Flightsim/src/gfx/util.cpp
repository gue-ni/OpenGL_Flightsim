#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../lib/stb_image.h"




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

Image::Image(const std::string& path, bool flip_vertically)
{
  stbi_set_flip_vertically_on_load(flip_vertically);
  data = stbi_load(path.c_str(), &width, &height, &channels, 0);
  assert(data != nullptr);
}

Image::~Image() { stbi_image_free(data); }

glm::vec3 Image::sample(const glm::vec2 uv) const
{
  // nearest pixel
  glm::ivec2 pixel_coord = uv * glm::vec2(width, height);
  int index = (height * pixel_coord.y + pixel_coord.x) * channels;
  return gfx::rgb(data[index + 0], data[index + 1], data[index + 2]);
}


}  // namespace gfx