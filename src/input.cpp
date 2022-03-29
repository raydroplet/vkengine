#include "input.hpp"

namespace vke
{
  KeyboardInput::KeyboardInput(Coordinator& coord) :
      m_ecs{coord}
  {
  }

  void KeyboardInput::moveInPlaneXZ(GLFWwindow* window, TimeStep ts, EntityID cameraEntity)
  {
    //FIXME: inverted xy

    glm::vec3 rotate{0.f};

    if(glfwGetKey(window, m_keys.lookRight) == GLFW_PRESS)
      rotate.y += 1.f;
    if(glfwGetKey(window, m_keys.lookLeft) == GLFW_PRESS)
      rotate.y -= 1.f;
    if(glfwGetKey(window, m_keys.lookUp) == GLFW_PRESS)
      rotate.x -= 1.f;
    if(glfwGetKey(window, m_keys.lookDown) == GLFW_PRESS)
      rotate.x += 1.f;

    auto& transform{m_ecs.getComponent<component::Transform3D>(cameraEntity)};
    if(glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
    {
      transform.rotation += m_lookSpeed * static_cast<float>(ts.count()) * glm::normalize(rotate);
    }
    transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());
    transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);

    float yaw = transform.rotation.y;
    //const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::vec3 rightDir{std::cos(yaw), 0.f, std::sin(yaw)};
    const glm::vec3 upDir{0.f, -1.f, 0.f};
    const glm::vec3 forwardDir{-std::sin(yaw), 0.f, std::cos(yaw)};

    /*
    std::cout << "yaw\t[" << yaw << "]" << std::endl;
    std::cout << "sin(yaw)\t[" << std::sin(yaw) << "]" << std::endl;
    std::cout << "cos(yaw)\t[" << std::cos(yaw) << "]" << std::endl;
    std::cout << "Forward\t[" << forwardDir.x << ' ' << forwardDir.y << ' ' << forwardDir.z << "]" << std::endl;
    std::cout << "Right\t[" << rightDir.x << ' ' << rightDir.y << ' ' << rightDir.z << "]" << std::endl;
    std::cout << "Up\t[" << upDir.x << ' ' << upDir.y << ' ' << upDir.z << "]" << std::endl;
    */

    glm::vec3 moveDir{0.f};

    if(glfwGetKey(window, m_keys.moveFoward) == GLFW_PRESS)
      moveDir += forwardDir;
    if(glfwGetKey(window, m_keys.moveBackward) == GLFW_PRESS)
      moveDir -= forwardDir;
    if(glfwGetKey(window, m_keys.moveRight) == GLFW_PRESS)
      moveDir += rightDir;
    if(glfwGetKey(window, m_keys.moveLeft) == GLFW_PRESS)
      moveDir -= rightDir;
    if(glfwGetKey(window, m_keys.moveUp) == GLFW_PRESS)
      moveDir += upDir;
    if(glfwGetKey(window, m_keys.moveDown) == GLFW_PRESS)
      moveDir -= upDir;

    if(glm::dot(moveDir, moveDir) > glm::epsilon<float>())
    {
      transform.translation += m_moveSpeed * static_cast<float>(ts.count()) * glm::normalize(moveDir);
    }
  }
}; // namespace vke
