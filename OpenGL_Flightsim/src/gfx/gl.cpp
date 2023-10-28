#include "gl.h"

#include "util.h"

namespace gfx
{
namespace gl
{

void CheckError(const char* stmt, const char* fname, int line)
{
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("OpenGL error %d, at %s:%i - for %s\n", err, fname, line, stmt);
    // abort();
  }
}

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
    exit(1);
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
    exit(1);
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
    exit(1);
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

void Shader::set_uniform(const std::string& name, const glm::mat3& value) const
{
  glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
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
  glBindTexture(target, m_id);

  set_parameter(GL_TEXTURE_WRAP_S, params.wrap);
  set_parameter(GL_TEXTURE_WRAP_T, params.wrap);
  set_parameter(GL_TEXTURE_MIN_FILTER, params.min_filter);
  set_parameter(GL_TEXTURE_MAG_FILTER, params.mag_filter);

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

void Texture::set_parameter(GLenum pname, const GLfloat* param) { glTexParameterfv(target, pname, param); }

TexturePtr Texture::load(const std::string& path, const Params& params)
{
  return std::make_shared<Texture>(path, params);
}

TexturePtr Texture::load(const std::string& path) { return Texture::load(path, {}); }

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

TextureArray::TextureArray(const Params& params)
    : m_format(params.format), m_texture_size(params.texture_size), m_array_size(params.array_size)
{
  glBindTexture(target, m_id);

  set_parameter(GL_TEXTURE_WRAP_S, params.wrap_s);
  set_parameter(GL_TEXTURE_WRAP_T, params.wrap_t);
  set_parameter(GL_TEXTURE_MIN_FILTER, params.min_filter);
  set_parameter(GL_TEXTURE_MAG_FILTER, params.mag_filter);

  glTexImage3D(target, 0, m_format, m_texture_size.x, m_texture_size.y, m_array_size, 0, m_format, GL_UNSIGNED_BYTE,
               NULL);
}

void TextureArray::bind() const { glBindTexture(target, m_id); }

void TextureArray::bind(GLuint texture_unit) const
{
  glActiveTexture(GL_TEXTURE0 + texture_unit);
  glBindTexture(target, m_id);
}

void TextureArray::unbind() const { glBindTexture(target, 0); }

void TextureArray::set_parameter(GLenum pname, GLint param) { glTexParameteri(target, pname, param); }

void TextureArray::set_parameter(GLenum pname, GLfloat param) { glTexParameterf(target, pname, param); }

void TextureArray::set_parameter(GLenum pname, const GLfloat* param) { glTexParameterfv(target, pname, param); }

void TextureArray::add_image(const Image& image)
{
  assert(m_format == image.format());
  assert(m_image_index < m_array_size);
  assert(m_texture_size.x == image.width() && m_texture_size.y == image.height());

  bind();
  glTexSubImage3D(target, 0, 0, 0, m_image_index++, image.width(), image.height(), 1, image.format(), GL_UNSIGNED_BYTE,
                  image.data());
  glGenerateMipmap(target);
  unbind();
}

}  // namespace gl
}  // namespace gfx
