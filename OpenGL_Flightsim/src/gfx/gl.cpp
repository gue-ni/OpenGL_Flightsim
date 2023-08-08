#include "gl.h"

#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../lib/stb_image.h"

namespace gfx
{
namespace gl
{

Shader::Shader(const std::string& path) : Shader(load_text_file(path + ".vert"), load_text_file(path + ".frag")) {}

Shader::Shader(const std::string& vertShader, const std::string& fragShader)
{
  // std::cout << "create Shader\n";
  const char* vertexShaderSource = vertShader.c_str();
  const char* fragmentShaderSource = fragShader.c_str();

  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  // check for shader compile errors
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // fragment shader
  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  // check for shader compile errors
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  // link shaders
  m_id = glCreateProgram();
  glAttachShader(m_id, vertexShader);
  glAttachShader(m_id, fragmentShader);
  glLinkProgram(m_id);
  // check for linking errors
  glGetProgramiv(m_id, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(m_id, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

Shader::~Shader() { glDeleteProgram(m_id); }

void Shader::bind() const { glUseProgram(m_id); }

void Shader::unbind() const { glUseProgram(0); }

void Shader::uniform(const std::string& name, int value)
{
  glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::uniform(const std::string& name, unsigned int value)
{
  glUniform1ui(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::uniform(const std::string& name, float value)
{
  glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::uniform(const std::string& name, const glm::vec3& value)
{
  glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
}

void Shader::uniform(const std::string& name, const glm::vec4& value)
{
  glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
}

void Shader::uniform(const std::string& name, const glm::mat4& value)
{
  glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &value[0][0]);
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

Texture::Texture(const std::string& path) : Texture(path, {}) {}

Texture::Texture(const std::string& path, const TextureParams& params)
    : Texture(gl::Image(path, params.flip_vertically), params)
{
}

Texture::Texture(const Image& image, const TextureParams& params) : Texture()
{
  glBindTexture(GL_TEXTURE_2D, m_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.texture_wrap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.texture_wrap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.texture_min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.texture_mag_filter);

  auto format = get_format(image.channels);
  glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, image.data);
  glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::~Texture() { glDeleteTextures(1, &m_id); }

void Texture::bind(GLuint active_texture) const
{
  glActiveTexture(GL_TEXTURE0 + active_texture);
  glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

GLint Texture::get_format(int channels)
{
  GLint format{};

  switch (channels) {
    case 1:
      format = GL_RED;
      break;
    case 3:
      format = GL_RGB;
      break;
    case 4:
      format = GL_RGBA;
      break;
    default:
      std::cout << "Failed to load texture, invalid format, channels = " << channels << std::endl;
      assert(false);
      break;
  }

  return format;
}

void Texture::set_parameteri(GLenum target, GLenum pname, GLint param) { glTexParameteri(target, pname, param); }

CubemapTexture::CubemapTexture(const std::array<std::string, 6>& paths, bool flip_vertically)
{
  glGenTextures(1, &m_id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

  for (int i = 0; i < 6; i++) {
    Image image(paths[i], flip_vertically);

    if (image.data) {
      auto format = get_format(image.channels);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, image.width, image.height, 0, format,
                   GL_UNSIGNED_BYTE, image.data);
    } else {
      std::cout << "Cubemap tex failed to load at path: " << paths[i] << std::endl;
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void CubemapTexture::bind(GLuint texture) const
{
  glActiveTexture(GL_TEXTURE0 + texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
}

void CubemapTexture::unbind() const { glBindTexture(GL_TEXTURE_CUBE_MAP, 0); }

}  // namespace gl
}  // namespace gfx