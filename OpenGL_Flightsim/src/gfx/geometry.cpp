
#include "geometry.h"

//#define TINYOBJLOADER_IMPLEMENTATION
//#include "../../lib/tiny_obj_loader.h"



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

GeometryPtr Geometry::load(const std::string& path)
{
#if 0
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
#else
  return nullptr;
#endif
}

GeometryPtr Geometry::quad()
{
  const std::vector<gl::Vertex> vertices = {

  };
  return std::make_shared<Geometry>(vertices);
}

GeometryPtr Geometry::plane() { return nullptr; }

GeometryPtr Geometry::box()
{
  const std::vector<gl::Vertex> vertices = {};
  return std::make_shared<Geometry>(vertices);
}

}  // namespace gfx