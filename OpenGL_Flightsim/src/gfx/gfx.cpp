#include "gfx.h"

#include "util.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../../lib/tiny_obj_loader.h"

#if 0
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#endif

namespace gfx
{



glm::mat4 Camera::get_view_matrix() const { return glm::inverse(m_transform); }

glm::mat4 Camera::get_projection_matrix() const { return m_projection; }

void Camera::look_at(const glm::vec3& target) { set_transform(glm::inverse(glm::lookAt(m_position, target, m_up))); }

glm::mat4 Light::light_space_matrix()
{
  float near_plane = 0.1f, far_plane = 10.0f, m = 10.0f;
  auto wp = get_world_position();
  glm::mat4 light_view = glm::lookAt(wp, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 light_projection = glm::ortho(-m, m, -m, m, near_plane, far_plane);
  return light_projection * light_view;
}

GeometryPtr make_cube_geometry(float size)
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

GeometryPtr make_plane_geometry(int x_elements, int y_elements, float size)
{
  // TODO
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

GeometryPtr make_quad_geometry()
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



void ShaderCache::add_shader(const std::string& path)
{
  if (!m_cache.contains(path)) {
    m_cache.insert(std::make_pair(path, std::make_shared<gl::Shader>(path)));
  }
}

gl::ShaderPtr ShaderCache::get_shader(const std::string& path)
{
  if (!m_cache.contains(path)) {
    add_shader(path);
  }
  return m_cache.at(path);
}

RenderTarget::RenderTarget(int w, int h) : RenderTargetBase(w, h)
{
#if 1
  // setup texture
  texture = std::make_shared<gl::Texture>();
  texture->bind();
  glTexImage2D(texture->target, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  texture->set_parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  texture->set_parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  texture->unbind();

  // setup depthmap
  depthbuffer.bind();
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  depthbuffer.unbind();

  framebuffer.bind();

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->target, texture->id(), 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthbuffer.id());

  if (!framebuffer.complete()) {
    std::cout << "Framebuffer is not complete!" << std::endl;
  }

  framebuffer.unbind();
#endif
}

ShadowMap::ShadowMap(int w, int h) : RenderTargetBase(w, h)
{
  texture = std::make_shared<gl::Texture>();
  texture->bind();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(1.0f)));
  texture->unbind();

  framebuffer.bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->target, texture->id(), 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  if (!framebuffer.complete()) {
    std::cout << "ShadowMap framebuffer is not complete\n";
  }

  framebuffer.unbind();
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

  const gfx::gl::Texture::Params params = {.flip_vertically = true, .texture_mag_filter = GL_LINEAR};

  if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, &source)) {
    std::cout << "Error: " << warning << error << std::endl;
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



Renderer::Renderer(GLsizei width, GLsizei height) : m_width(width), m_height(height)
{
  // init renderer
  std::cout << "========== OpenGL ==========\n";
  std::cout << glGetString(GL_VERSION) << std::endl;
  std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
  std::cout << glGetString(GL_VENDOR) << std::endl;
  std::cout << glGetString(GL_RENDERER) << std::endl;
  std::cout << "============================\n";

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
  glViewport(0, 0, m_width, m_height);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  bool shadowmap = true;

  // init skybox
#if 1
  const std::string skybox_path = "assets/textures/skybox/2/";
  const std::array<std::string, 6>& faces = {
      skybox_path + "right.png",  skybox_path + "left.png",  skybox_path + "top.png",
      skybox_path + "bottom.png", skybox_path + "front.png", skybox_path + "back.png",
  };
  m_fog_color = gfx::rgb(0x5e5e6e);
#else
  const std::string skybox_path = "assets/textures/skybox/1/";
  const std::array<std::string, 6>& faces = {
      skybox_path + "right.jpg",  skybox_path + "left.jpg",  skybox_path + "top.jpg",
      skybox_path + "bottom.jpg", skybox_path + "front.jpg", skybox_path + "back.jpg",
  };
  m_fog_color = gfx::rgb(222, 253, 255);
#endif

  m_skybox = std::make_shared<gfx::Skybox>(faces);
  m_skybox->set_scale(glm::vec3(10.0f));
  m_skybox->update_transform();

  if (shadowmap) {
    m_shadowmap = std::make_shared<ShadowMap>(2048, 2048);
  }

#if 1
  m_rendertarget = std::make_shared<gfx::RenderTarget>(m_width, m_height);

  auto mat1 = std::make_shared<gfx::Material>("shaders/screen", m_rendertarget->texture);
  // auto mat2 = std::make_shared<gfx::Material>("shaders/shadow_debug", m_shadowmap->texture);

  m_screenquad = std::make_shared<gfx::Mesh>(gfx::make_quad_geometry(), mat1);
#else
  m_rendertarget = m_screenquad = nullptr;
#endif
}

void Renderer::render_shadows(RenderContext& context)
{
  // TODO
}

void Renderer::render_skybox(RenderContext& context)
{
  glDepthMask(GL_FALSE);
  m_skybox->draw(context);
  glDepthMask(GL_TRUE);
}

void Renderer::render_screenquad(RenderContext& context) { m_screenquad->draw(context); }

void Renderer::render(Camera* camera, Object3D* scene)
{
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // glEnable(GL_DEPTH_TEST);

  RenderContext context;
  context.camera = camera;
  context.shadow_pass = false;
  context.shaders = &m_shaders;
  context.env_map = m_skybox->get_material()->get_texture();
  context.depth_map = m_shadowmap->texture;

  glm::vec3 center;
  glm::mat4 light_space_matrix;

  // find the light
  scene->traverse([&center](Object3D* obj) {
    Light* light = dynamic_cast<Light*>(obj);
    if (light != NULL) {
      center = light->get_world_position();
    }
    return true;
  });

  float m = 50.0f;
  float near_plane = 1.0f, far_plane = 5000.0f;
  glm::vec3 light_pos = center + glm::vec3(-2.0f, 18.0f, -1.0f);
  glm::mat4 light_proj = glm::ortho(-m, m, -m, m, near_plane, far_plane);
  glm::mat4 light_view = glm::lookAt(light_pos, center, glm::vec3(0.0f, 1.0f, 0.0f));
  light_space_matrix = light_proj * light_view;

  context.light_space_matrix = light_space_matrix;

  // depends on skybox
  context.fog_color = m_fog_color;

  // update transforms
  scene->update_transform();

  // render to shadowmap
#if 1
  {
    glViewport(0, 0, m_shadowmap->width, m_shadowmap->height);
    m_shadowmap->framebuffer.bind();
    // glClear(GL_DEPTH_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene->traverse([&context](Object3D* obj) {
      Mesh* mesh = dynamic_cast<Mesh*>(obj);

      if (mesh != NULL && mesh->cast_shadow == true) {
        gl::ShaderPtr shader = context.shaders->get_shader("shaders/mesh");
        shader->bind();
        shader->set_uniform("u_Model", mesh->get_transform());
        shader->set_uniform("u_ShadowPass", true);
        shader->set_uniform("u_LightSpaceMatrix", context.light_space_matrix);

        auto geo = mesh->get_geometry();

        // TODO: also handle indexed geometry
        geo->vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, geo->count);
        geo->vao.unbind();

        shader->unbind();
      }

      return true;
    });

    m_shadowmap->framebuffer.unbind();
  }
#endif

  // render skybox

  if (m_rendertarget && m_screenquad) {
    glViewport(0, 0, m_rendertarget->width, m_rendertarget->height);
    m_rendertarget->framebuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_skybox(context);

    scene->draw(context);

    m_rendertarget->framebuffer.unbind();

    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_screenquad(context);

  } else {  // do not render to texture
    render_skybox(context);
    scene->draw(context);
  }
}

void Renderer::render(Camera* camera, Object3D* scene, RenderTarget* target)
{
  RenderContext context;
  context.camera = camera;
  context.shadow_pass = false;
  context.shaders = &m_shaders;
  context.env_map = m_skybox->get_material()->get_texture();
  context.depth_map = m_shadowmap->texture;

  glViewport(0, 0, target->width, target->height);
  target->framebuffer.bind();
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  scene->draw(context);
  target->framebuffer.unbind();
}

}  // namespace gfx
