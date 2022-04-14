#pragma once

#include "model.hpp"

namespace vke::cmp
{
  struct Transform2D
  {
    glm::vec2 translation{0, 0};
    glm::vec2 scale{1.f, 1.f};
    float rotation{};

    glm::mat2 mat2() const;
  };

  struct Transform3D
  {
    glm::vec3 translation{};
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation{}; // Y points up

    glm::mat4 mat4() const;
    glm::mat4 optimized_mat4() const; //TODO: rename the method name

    glm::mat3 normalMatrix();
  };

  // for shared components?
  class Common
  {
  public:
    Model* model() { return m_model; }
    void setModel(Model* model) { m_model = model; }

  private:
    Model* m_model{};
  };

  struct Color
  {
    glm::vec3 color{1.f, 1.f, 1.f};
    std::vector<Model::Vertex> vertices{};

    operator glm::vec3();
  };

  struct PointLight
  {
    glm::vec3 position{};
    glm::vec3 intensity{};
  };
} // namespace vke::component
