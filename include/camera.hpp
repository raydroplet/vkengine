#pragma once

#include "core.hpp"

namespace vke
{
  class Camera
  {
  public:
    void setOrthographicProjection(float left_x, float right_x, float top_y, float bottom_y, float near_z, float far_z);
    void setOrthographicProjection(float wh_aspectRatio, float near_z, float far_z);

    void setPerspectiveProjection(float fovY, float wh_aspectRatio, float near_z, float far_z);

    void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = {0.f, -1.f, 0.f});
    void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = {0.f, -1.f, 0.f});

    void setViewRotationXYZ(glm::vec3 position, glm::vec3 rotation); // Extrinsic XY
    void setViewRotationYXZ(glm::vec3 position, glm::vec3 rotation); // Intrinsic XY
    void setViewRotationZYX(glm::vec3 position, glm::vec3 rotation); // Intrinsic XY

    glm::mat4 const& projection() const
    {
      return projectionMatrix;
    }
    glm::mat4 const& view() const
    {
      return viewMatrix;
    }

  private:
    glm::mat4 viewMatrix{1.f};
    glm::mat4 projectionMatrix{1.f};
  };

  inline void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up)
  {
    // vec(direction) = vec(target) - vec(position) // -> positive forward z
    setViewDirection(position, target - position, up);
  }
} // namespace vke
