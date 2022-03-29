#pragma once

#include "model.hpp"

namespace vke::component
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

    // ZYX to rotate the axis
    // XYZ to rotate in the axis
    // glm::mat4 XZY_mat4() const { throw std::runtime_error("not implemented"); }
    glm::mat4 XYZ_mat4() const;
    // glm::mat4 YXZ_mat4() const;
    // glm::mat4 YZX_mat4() const;
    // glm::mat4 ZYX_mat4() const;
    // glm::mat4 ZXY_mat4() const;
  };

  struct Color
  {
    glm::vec3 color{1.f, 1.f, 1.f};
    std::vector<Model::Vertex3D> vertices{};

    operator glm::vec3();
  };
} // namespace vke::component
