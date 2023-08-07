#pragma once

#include <GL/glew.h>

#include <array>
#include <fstream>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace gfx
{
std::string load_text_file(const std::string& path);

// value [0, 255]
template <typename T>
constexpr glm::vec3 rgb(T r, T g, T b)
{
  return glm::vec3(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b)) / 255.0f;
}

constexpr glm::vec3 rgb(uint32_t hex)
{
  assert(hex <= 0xffffffU);
  return glm::vec3(static_cast<float>((hex & 0xff0000U) >> 16) / 255.0f, static_cast<float>((hex & 0x00ff00U) >> 8) / 255.0f,
             static_cast<float>((hex & 0x0000ffU) >> 0) / 255.0f);
}

}
