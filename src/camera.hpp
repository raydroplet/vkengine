#pragma once

#include "core.hpp"

namespace vke
{
  class Camera
  {
  public:
    // (near_x, near_y, near_z) (far_x, far_y, far_z)
    //   (left, bottom, near)     (right, top, far)
    void setOrthographicProjection(float left_x, float right_x, float top_y, float bottom_y, float near_z, float far_z);
    void setOrthographicProjection(VkExtent2D extent, float near_z, float far_z);

    void setPerspectiveProjection(float x_aspectRatio, float y_aspectRatio, float fov, float near_z, float far_z);
    void setPerspectiveProjection(VkExtent2D extent, float fov, float near, float far);

    void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = {0.f, 1.f, 0.f});
    void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = {0.f, 1.f, 0.f});


    void broken_setViewXYZ(glm::vec3 position, glm::vec3 rotation);
    void setViewYXZ(glm::vec3 position, glm::vec3 rotation);


    glm::mat4 projection() const { return projectionMatrix; }
    glm::mat4 view() const { return viewMatrix; }

  private:
    glm::mat4 viewMatrix{1.f};
    glm::mat4 projectionMatrix{1.f};
  };

  inline void Camera::setOrthographicProjection(VkExtent2D extent, float near_z, float far_z)
  {
    float x_aspectRatio{static_cast<float>(extent.width) / extent.height};
    float y_aspectRatio{static_cast<float>(extent.height) / extent.width};
    setOrthographicProjection(-x_aspectRatio, x_aspectRatio, -y_aspectRatio, y_aspectRatio, near_z, far_z);
  }

  inline void Camera::setPerspectiveProjection(VkExtent2D extent, float fov, float near_z, float far_z)
  {
    float x_aspectRatio{static_cast<float>(extent.width) / extent.height};
    float y_aspectRatio{static_cast<float>(extent.height) / extent.width};
    setPerspectiveProjection(x_aspectRatio, y_aspectRatio, fov, near_z, far_z);
  }

  inline void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up)
  {
    setViewDirection(position, target - position, up);
  }
} // namespace vke
