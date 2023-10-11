
#include "geometry.h"

#include "../../lib/tiny_obj_loader.h"

namespace gfx
{
Geometry::Geometry(const std::vector<float>& vertices, const VertexLayout& layout) : BaseGeometry(DRAW_ARRAYS)
{
  const int stride = static_cast<int>(layout);
  count = static_cast<int>(vertices.size()) / (stride);

  vao.bind();
  vbo.buffer(vertices);

  unsigned int index = 0;
  glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
  glEnableVertexAttribArray(index);

  if (layout == POS_NORM || layout == POS_NORM_UV)  // add normal
  {
    index++;
    glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(index);
  }

  if (layout == POS_UV || layout == POS_NORM_UV)  // add uv
  {
    index++;
    glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float),
                          (void*)(static_cast<int>(index) * 3 * sizeof(float)));
    glEnableVertexAttribArray(index);
  }

  vbo.unbind();
  vao.bind();
}

Geometry::Geometry(const std::vector<gl::Vertex>& vertices) : BaseGeometry(DRAW_ARRAYS)
{
  count = vertices.size();

  vao.bind();

  vbo.bind();
  vbo.buffer(vertices);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Normal));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords));
  glEnableVertexAttribArray(2);

  vao.unbind();
}

IndexedGeometry::IndexedGeometry(const std::vector<gl::Vertex>& vertices, const std::vector<GLuint>& indices)
    : BaseGeometry(DRAW_ELEMENTS)
{
  count = indices.size();

  vao.bind();

  vbo.bind();
  vbo.buffer(vertices);

  ebo.bind();
  ebo.buffer(indices);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Normal));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords));
  glEnableVertexAttribArray(2);

  vao.unbind();
}

#if 0
GeometryPtr Geometry::load(const std::string& path)
{
  std::vector<gl::Vertex> vertices;

  std::istringstream source(load_text_file(path));

  std::string warning, error;
  tinyobj::attrib_t attributes;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, &source)) {
    std::cout << "loadObj::Error: " << warning << error << std::endl;
    return nullptr;
  }

#if 1
  printf("path = %s\n", path.c_str());
  printf("# of vertices  = %d\n", (int)(attributes.vertices.size()) / 3);
  printf("# of normals   = %d\n", (int)(attributes.normals.size()) / 3);
  printf("# of texcoords = %d\n", (int)(attributes.texcoords.size()) / 2);
  printf("# of materials = %d\n", (int)materials.size());
  printf("# of shapes    = %d\n", (int)shapes.size());
#endif

  for (size_t s = 0; s < shapes.size(); s++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      // hardcode loading to triangles

      // Loop over vertices in the face.
      for (size_t v = 0; v < 3; v++) {
        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

        gl::Vertex vertex;
        // vertex position
        vertex.Position.x = attributes.vertices[3 * idx.vertex_index + 0];
        vertex.Position.y = attributes.vertices[3 * idx.vertex_index + 1];
        vertex.Position.z = attributes.vertices[3 * idx.vertex_index + 2];
        // vertex normal
        vertex.Normal.x = attributes.normals[3 * idx.normal_index + 0];
        vertex.Normal.y = attributes.normals[3 * idx.normal_index + 1];
        vertex.Normal.z = attributes.normals[3 * idx.normal_index + 2];

        vertex.TexCoords.x = attributes.texcoords[2 * idx.texcoord_index + 0];
        vertex.TexCoords.y = attributes.texcoords[2 * idx.texcoord_index + 1];

        vertices.push_back(vertex);
      }
      index_offset += 3;
    }
  }

  return std::make_shared<Geometry>(vertices);
}
#endif

GeometryPtr Geometry::quad()
{
  const std::vector<gl::Vertex> quad_vertices = {
      {{-1, 1, 0}, {0, 0, 1}, {0, 1}},   // top left
      {{-1, -1, 0}, {0, 0, 1}, {0, 0}},  // bottom left
      {{1, 1, 0}, {0, 0, 1}, {1, 1}},    // top right

      {{1, 1, 0}, {0, 0, 1}, {1, 1}},    // top right
      {{-1, -1, 0}, {0, 0, 1}, {0, 0}},  // bottom left
      {{1, -1, 0}, {0, 0, 1}, {1, 0}},   // bottom right
  };

  return std::make_shared<Geometry>(quad_vertices);
}

void push_back(std::vector<float>& vector, const glm::vec3& v)
{
  vector.push_back(v.x);
  vector.push_back(v.y);
  vector.push_back(v.z);
}

void push_back(std::vector<float>& vector, const glm::vec2& v)
{
  vector.push_back(v.x);
  vector.push_back(v.y);
}

void push_back(std::vector<float>& vector, const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv)
{
  push_back(vector, pos);
  push_back(vector, normal);
  push_back(vector, uv);
}

GeometryPtr Geometry::plane(int x_elements, int y_elements, float size)
{
  const float width = size, height = size;

  std::vector<float> vertices;

  glm::vec3 normal(0.0f, 1.0f, 0.0f);

  for (int y = 0; y < y_elements; y++) {
    for (int x = 0; x < x_elements; x++) {
      auto bottom_left = glm::vec3((x + 0) * width, 0.0f, (y + 0) * height);
      auto bottom_right = glm::vec3((x + 1) * width, 0.0f, (y + 0) * height);
      auto top_left = glm::vec3((x + 0) * width, 0.0f, (y + 1) * height);
      auto top_right = glm::vec3((x + 1) * width, 0.0f, (y + 1) * height);

      auto tex_coord = glm::vec2(static_cast<float>(x) / static_cast<float>(x_elements),
                                 static_cast<float>(y) / static_cast<float>(y_elements));

      // triangle 1
      push_back(vertices, top_right, normal, glm::vec2(1.0f, 1.0f));
      push_back(vertices, bottom_right, normal, glm::vec2(1, 0.0f));
      push_back(vertices, bottom_left, normal, glm::vec2(0.0f, 0.0f));

      // triangle 2
      push_back(vertices, bottom_left, normal, glm::vec2(0.0f, 0.0f));
      push_back(vertices, top_left, normal, glm::vec2(0.0f, 1.0f));
      push_back(vertices, top_right, normal, glm::vec2(1.0f, 1.0f));
    }
  }

  return std::make_shared<Geometry>(vertices, Geometry::POS_NORM_UV);
}

GeometryPtr Geometry::box(float size)
{
  float s = size / 2;

  // clang-format off
  std::vector<float> vertices = {
      // left
      -s, -s, -s, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
      s, -s, -s, 0.0f, 0.0f,-1.0f,1.0f,0.0f,
      s,s,-s,0.0f,0.0f,-1.0f,1.0f,1.0f,
      s,s,-s,0.0f,0.0f,-1.0f,1.0f,1.0f,
      -s,s,-s,0.0f,0.0f,-1.0f,0.0f,1.0f,
      -s,-s,-s,0.0f,0.0f,-1.0f,0.0f,0.0f,

      // right
      -s,-s,s,0.0f,0.0f,1.0f,0.0f,0.0f,
      s,-s,s,0.0f,0.0f,1.0f,1.0f,0.0f,
      s,s,s,0.0f,0.0f,1.0f,1.0f,1.0f,
      s,s,s,0.0f,0.0f,1.0f,1.0f,1.0f,
      -s,s,s,0.0f,0.0f,1.0f,0.0f,1.0f,
      -s,-s,s,0.0f,0.0f,1.0f,0.0f,0.0f,

      // backward
      -s,s,s,-1.0f,0.0f,0.0f,0.0f,0.0f,
      -s,s,-s,-1.0f,0.0f,0.0f,1.0f,0.0f,
      -s,-s,-s,-1.0f,0.0f,0.0f,1.0f,1.0f,
      -s,-s,-s,-1.0f,0.0f,0.0f,1.0f,1.0f,
      -s,-s,s,-1.0f,0.0f,0.0f,0.0f,1.0f,
      -s,s,s,-1.0f,0.0f,0.0f,0.0f,0.0f,

      // forward
      s,s,s,1.0f,0.0f,0.0f,0.0f,0.0f,
      s,s,-s,1.0f,0.0f,0.0f,1.0f,0.0f,
      s,-s,-s,1.0f,0.0f,0.0f,1.0f,1.0f,
      s,-s,-s,1.0f,0.0f,0.0f,1.0f,1.0f,
      s,-s,s,1.0f,0.0f,0.0f,0.0f,1.0f,
      s,s,s,1.0f,0.0f,0.0f,0.0f,0.0f,

      // down
      -s,-s,-s,0.0f,-1.0f,0.0f,0.0f,0.0f,
      s,-s,-s,0.0f,-1.0f,0.0f,1.0f,0.0f,
      s,-s,s,0.0f,-1.0f,0.0f,1.0f,1.0f,
      s,-s,s,0.0f,-1.0f,0.0f,1.0f,1.0f,
      -s,-s,s,0.0f,-1.0f,0.0f,0.0f,1.0f,
      -s,-s,-s,0.0f,-1.0f,0.0f,0.0f,0.0f,

      // up
      -s,s,-s,0.0f,1.0f,0.0f,0.0f,0.0f,
      s,s,-s,0.0f,1.0f,0.0f,1.0f,0.0f,
      s,s,s,0.0f,1.0f,0.0f,1.0f,1.0f,
      s,s,s,0.0f,1.0f,0.0f,1.0f,1.0f,
      -s,s,s,0.0f,1.0f,0.0f,0.0f,1.0f,
      -s,s,-s,0.0f,1.0f,0.0f,0.0f,0.0f,

  };
  // clang-format on
  return std::make_shared<Geometry>(vertices, Geometry::POS_NORM_UV);
}

}  // namespace gfx