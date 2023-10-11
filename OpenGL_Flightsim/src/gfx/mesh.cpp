#include "mesh.h"

namespace gfx
{

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

Skybox::Skybox(const std::array<std::string, 6>& faces)
    : Mesh(make_cube_geometry(1.0f),
           std::make_shared<Material>("shaders/skybox", std::make_shared<gl::CubemapTexture>(faces)))
{
}

}  // namespace gfx