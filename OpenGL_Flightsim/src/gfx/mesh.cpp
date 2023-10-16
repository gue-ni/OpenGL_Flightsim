#include "mesh.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "../../lib/tiny_obj_loader.h"

namespace gfx
{

Mesh* process_assimp_mesh(aiMesh* mesh, const aiScene* scene)
{
  std::vector<GLuint> indices;
  std::vector<gl::Vertex> vertices;

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    gl::Vertex vertex;
    vertex.Position.x = mesh->mVertices[i].x;
    vertex.Position.y = mesh->mVertices[i].y;
    vertex.Position.z = mesh->mVertices[i].z;

    vertex.Normal.x = mesh->mNormals[i].x;
    vertex.Normal.y = mesh->mNormals[i].y;
    vertex.Normal.z = mesh->mNormals[i].z;

    if (mesh->mTextureCoords[0]) {
      vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
      vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
    } else {
      vertex.TexCoords = glm::vec2(0.0f);
    }

    vertices.push_back(vertex);
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  auto geometry = std::make_shared<IndexedGeometry>(vertices, indices);
  auto material = std::make_shared<Material>("shaders/mesh", "assets/textures/falcon.jpg");

  if (mesh->mMaterialIndex >= 0) {
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
  } else {
  }
  // TODO
  return new Mesh(geometry, material);
}

glm::mat4 convert_matrix(const aiMatrix4x4& aiMat)
{
  return {aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1, aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
          aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3, aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4};
}

void process_assimp_node(aiNode* node, const aiScene* scene, Object3D* result)
{
  assert(node->mNumMeshes <= 1);

  aiMatrix4x4 m = node->mTransformation;

  // glm::mat4 transformation = AiMatrix4x4ToGlm(&node->mTransformation);
  // glm::mat4 globalTransformation = transformation * parentTransformation;

  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    Mesh* parsed = process_assimp_mesh(mesh, scene);
    parsed->set_transform(convert_matrix(m));
    result->add(parsed);
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    process_assimp_node(node->mChildren[i], scene, result);
  }
}

Mesh::Mesh(const GeometryPtr& geometry, const MaterialPtr& material)
    : m_material(material), m_geometry(geometry), Object3D(MESH)
{
}

void Mesh::draw_self(RenderContext& context)
{
  if (disable_depth_test) glDisable(GL_DEPTH_TEST);

  if (!context.shadow_pass && context.shaders) {
    auto camera = context.camera;

    // get shader from cache
    std::string shader_name = m_material->get_shader_name();
    gl::ShaderPtr shader = context.shaders->get_shader(shader_name);

    shader->bind();

    // transform
    shader->set_uniform("u_Model", get_transform());
    shader->set_uniform("u_Normal", get_normal_transform());

    // camera
    shader->set_uniform("u_View", camera->get_view_matrix());
    shader->set_uniform("u_Projection", camera->get_projection_matrix());
    shader->set_uniform("u_CameraPos", camera->get_world_position());

    // lights
    glm::vec3 light_dir = glm::vec3(0, 1, 0);
    glm::vec3 light_color = glm::vec3(1, 1, 1);
    shader->set_uniform("u_LightDir", light_dir);
    shader->set_uniform("u_LightColor", light_color);

    // material texture
    m_material->get_texture()->bind(5);
    shader->set_uniform("u_Texture_01", 5);
    shader->set_uniform("u_UseTexture", true);

    // environment map
    context.env_map->bind(6);
    shader->set_uniform("u_EnvMap", 6);

    // shadows
    context.depth_map->bind(7);
    shader->set_uniform("u_ShadowMap", 7);
    shader->set_uniform("u_ReceiveShadow", receive_shadow ? 1 : 0);
    shader->set_uniform("u_ShadowPass", false);

    glm::vec3 rgb = glm::vec3(1.0f, 0.0f, 0.0f);
    shader->set_uniform("u_SolidObjectColor", rgb);

    // material pbr properties
    shader->set_uniform("u_Opacity", m_material->opacity);
    shader->set_uniform("u_Shininess", m_material->shininess);

    m_geometry->vao.bind();

    switch (m_geometry->draw_type) {
      case BaseGeometry::DRAW_ARRAYS:
        glDrawArrays(GL_TRIANGLES, 0, m_geometry->count);
        break;
      case BaseGeometry::DRAW_ELEMENTS:
        glDrawElements(GL_TRIANGLES, m_geometry->count, GL_UNSIGNED_INT, 0);
        break;
      default:
        assert(false);
        break;
    }

    m_geometry->vao.unbind();

    shader->unbind();
  }

  if (disable_depth_test) glEnable(GL_DEPTH_TEST);
}

Object3D* Mesh::load_mesh(const std::string& path)
{
  Assimp::Importer importer;
  unsigned int flags = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs;

  // flags |= aiProcess_PreTransformVertices;

  const aiScene* scene = importer.ReadFile(path, flags);

  if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    printf("Failed to load %s: %s\n", path.c_str(), importer.GetErrorString());
  }

  Object3D* root = new Object3D;
  process_assimp_node(scene->mRootNode, scene, root);
  return root;
}

Object3D* Mesh::load(const std::string& path, const std::string& texture)
{
  Object3D* root = new Object3D;

  // TODO: load obj, but load every shape in its own Mesh

  std::istringstream source(load_text_file(path));
  std::string warning, error;

  tinyobj::attrib_t attributes;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  for (auto& material : materials) {
    std::cout << material.name << std::endl;
  }

  const gfx::gl::Texture::Params params = {.flip_vertically = true, .mag_filter = GL_LINEAR};

  if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, &source)) {
    std::cout << "Error: " << warning << error << std::endl;
    return nullptr;
  }

#if 0
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
    std::vector<gl::Vertex> vertices;

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
        // uv coords
        vertex.TexCoords.x = attributes.texcoords[2 * idx.texcoord_index + 0];
        vertex.TexCoords.y = attributes.texcoords[2 * idx.texcoord_index + 1];

        vertices.push_back(vertex);
      }
      index_offset += 3;
    }

    MaterialPtr material = std::make_shared<Material>("shaders/mesh", texture);
    GeometryPtr geometry = std::make_shared<Geometry>(vertices);

    Mesh* mesh = new Mesh(geometry, material);
    root->add(mesh);
  }

  return root;
}

Skybox::Skybox(const std::array<std::string, 6>& faces)
    : Mesh(Geometry::box(1.0f),
           std::make_shared<Material>("shaders/skybox", std::make_shared<gl::CubemapTexture>(faces)))
{
}

}  // namespace gfx