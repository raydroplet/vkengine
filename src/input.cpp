#include "input.hpp"

namespace vke
{
  KeyboardInput::KeyboardInput(Coordinator& coord, Window& window) :
      m_window{window},
      m_ecs{coord},
      lastCursorPos{m_window.cursorPos()}
  {
  }

  void KeyboardInput::moveInPlaneXZ(TimeStep ts, EntityID cameraEntity)
  {
    glm::vec3 rotate{0.f};

    auto cursorPos{m_window.cursorPos()};
    rotate.y = ((cursorPos.first - lastCursorPos.first)) * m_lookSpeed;
    rotate.x = ((lastCursorPos.second - cursorPos.second)) * m_lookSpeed;
    lastCursorPos = cursorPos;

    //  if(glfwGetKey(window, m_keys.lookRight) == GLFW_PRESS)
    //    rotate.y += rotationY;
    //  if(glfwGetKey(window, m_keys.lookLeft) == GLFW_PRESS)
    //    rotate.y -= rotationY;
    //  if(glfwGetKey(window, m_keys.lookUp) == GLFW_PRESS)
    //    rotate.x += rotationX;
    //  if(glfwGetKey(window, m_keys.lookDown) == GLFW_PRESS)
    //    rotate.x -= rotationX;

    // Rotate only when 'rotate' is a non-zero vector. the 'dot(r, r) > 0' is just a way to check for that
    auto& transform{m_ecs.getComponent<cmp::Transform3D>(cameraEntity)};
    if(glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
    {
      //// rotate is normalized so that it doesn't rotate faster diagonally, than solely in the horizontal or vertical directions
      // transform.rotation += m_lookSpeed * static_cast<float>(ts.count()) * glm::normalize(rotate);
      transform.rotation += m_lookSpeed * static_cast<float>(ts.count()) * (rotate);
    }

    float radians{glm::radians(85.f)};
    transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());
    transform.rotation.x = glm::clamp(transform.rotation.x, -radians, radians);

    float yaw = transform.rotation.y;
    // const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::vec3 rightDir{std::cos(yaw), 0.f, -std::sin(yaw)};
    const glm::vec3 upDir{0.f, -1.f, 0.f};
    const glm::vec3 forwardDir{std::sin(yaw), 0.f, std::cos(yaw)};

    glm::vec3 moveDir{0.f};
    if(glfwGetKey(m_window, m_keys.moveFoward) == GLFW_PRESS)
      moveDir += forwardDir;
    if(glfwGetKey(m_window, m_keys.moveBackward) == GLFW_PRESS)
      moveDir -= forwardDir;
    if(glfwGetKey(m_window, m_keys.moveRight) == GLFW_PRESS)
      moveDir += rightDir;
    if(glfwGetKey(m_window, m_keys.moveLeft) == GLFW_PRESS)
      moveDir -= rightDir;
    if(glfwGetKey(m_window, m_keys.moveUp) == GLFW_PRESS)
      moveDir += upDir;
    if(glfwGetKey(m_window, m_keys.moveDown) == GLFW_PRESS)
      moveDir -= upDir;

    // same as before. moveDir can't be a 0 vector.
    if(glm::dot(moveDir, moveDir) > glm::epsilon<float>())
    {
      transform.translation += m_moveSpeed * static_cast<float>(ts.count()) * glm::normalize(moveDir);
    }
  }
}; // namespace vke
