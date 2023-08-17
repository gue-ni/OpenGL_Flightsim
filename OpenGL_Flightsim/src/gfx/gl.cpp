#include "gl.h"

#include "util.h"

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

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
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
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
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

void Shader::set_uniform(const std::string& name, GLint value) const
{
  glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::set_uniform(const std::string& name, GLuint value) const
{
  glUniform1ui(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::set_uniform(const std::string& name, GLfloat value) const
{
  glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::set_uniform(const std::string& name, const glm::vec3& value) const
{
  glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::set_uniform(const std::string& name, const glm::vec4& value) const
{
  glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::set_uniform(const std::string& name, const glm::mat4& value) const
{
  glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::set_uniform_buffer(const std::string& name, GLuint binding)
{
  GLuint index = glGetUniformBlockIndex(m_id, name.c_str());
  glUniformBlockBinding(m_id, index, binding);
}

Texture::Texture(const std::string& path) : Texture(path, {}) {}

Texture::Texture(const std::string& path, const Params& params) : Texture(Image(path, params.flip_vertically), params)
{
}

Texture::Texture(const Image& image, const Params& params) : Texture(GL_TEXTURE_2D)
{
  glBindTexture(GL_TEXTURE_2D, m_id);

  set_parameter(GL_TEXTURE_WRAP_S, params.texture_wrap);
  set_parameter(GL_TEXTURE_WRAP_T, params.texture_wrap);
  set_parameter(GL_TEXTURE_MIN_FILTER, params.texture_min_filter);
  set_parameter(GL_TEXTURE_MAG_FILTER, params.texture_mag_filter);

  glTexImage2D(target, 0, image.format(), image.width(), image.height(), 0, image.format(), GL_UNSIGNED_BYTE,
               image.data());
  glGenerateMipmap(target);
}

void Texture::bind(GLuint active_texture) const
{
  glActiveTexture(GL_TEXTURE0 + active_texture);
  glBindTexture(target, m_id);
}

void Texture::bind() const { glBindTexture(target, m_id); }

void Texture::unbind() const { glBindTexture(target, 0); }

void Texture::set_parameter(GLenum pname, GLint param) { glTexParameteri(target, pname, param); }

void Texture::set_parameter(GLenum pname, GLfloat param) { glTexParameterf(target, pname, param); }

CubemapTexture::CubemapTexture(const std::array<std::string, 6>& paths, bool flip_vertically)
    : Texture(GL_TEXTURE_CUBE_MAP)
{
  bind();

  for (int i = 0; i < 6; i++) {
    Image image(paths[i], flip_vertically);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, image.format(), image.width(), image.height(), 0,
                 image.format(), GL_UNSIGNED_BYTE, image.data());
  }

  set_parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  set_parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  set_parameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

}  // namespace gl
}  // namespace gfx
