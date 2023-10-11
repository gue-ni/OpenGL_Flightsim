#pragma once

#include "gfx.h"

namespace gfx
{
class Mesh : public Object3D
{
 public:
  Mesh(const GeometryPtr& geometry, const MaterialPtr& material);

  MaterialPtr get_material() { return m_material; }
  GeometryPtr get_geometry() { return m_geometry; }

  // TODO: return shared ptr
  static Object3D* load(const std::string& path, const std::string& texture);

  static Object3D* load_mesh(const std::string& path);

 protected:
  MaterialPtr m_material;
  GeometryPtr m_geometry;
  void draw_self(RenderContext& context) override;
};



class Skybox : public Mesh
{
 public:
  Skybox(const std::array<std::string, 6>& faces);
  Object3D& add(Object3D* child) = delete;
};

}  // namespace gfx
