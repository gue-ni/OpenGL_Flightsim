#pragma once

#include "gl.h"

namespace gfx {



class BaseGeometry
{
 public:
  enum DrawType { DRAW_ARRAYS, DRAW_ELEMENTS, /* TODO: instanced */ };

  // stride
  enum VertexLayout : int {
    POS = 3,         // pos
    POS_UV = 5,      // pos, uv
    POS_NORM = 6,    // pos, normal
    POS_NORM_UV = 8  // pos, normal, uv
  };

  BaseGeometry(DrawType type) : draw_type(type) {}

  GLsizei count;
  gl::VertexArrayObject vao;
  const DrawType draw_type;
};


using GeometryPtr = std::shared_ptr<BaseGeometry>;

class Geometry : public BaseGeometry
{
 public:
  Geometry(const std::vector<float>& vertices, const VertexLayout& layout);
  Geometry(const std::vector<gl::Vertex>& vertices);

  static GeometryPtr load(const std::string& path);
  static GeometryPtr quad();
  static GeometryPtr plane();
  static GeometryPtr box();

 private:
  gl::VertexBuffer vbo;
};

class IndexedGeometry : public BaseGeometry
{
 public:
  IndexedGeometry(const std::vector<gl::Vertex>& vertices, const std::vector<GLuint>& indices);

 private:
  gl::VertexBuffer vbo;
  gl::ElementBuffer ebo;
};



}
