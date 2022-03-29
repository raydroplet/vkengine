#pragma once

#include "core.hpp"
//
#include "ecs.hpp"
#include "window.hpp"

namespace vke
{
  using TimeStep = std::chrono::duration<double>;

  class KeyboardInput
  {
    struct KeyMappings
    {
      int moveFoward{GLFW_KEY_W};
      int moveLeft{GLFW_KEY_A};
      int moveBackward{GLFW_KEY_S};
      int moveRight{GLFW_KEY_D};
      int moveUp{GLFW_KEY_SPACE};
      int moveDown{GLFW_KEY_LEFT_SHIFT};

      int lookLeft{GLFW_KEY_LEFT};
      int lookRight{GLFW_KEY_RIGHT};
      int lookUp{GLFW_KEY_UP};
      int lookDown{GLFW_KEY_DOWN};
    };

  public:
    KeyboardInput(Coordinator& coord);

    void moveInPlaneXZ(GLFWwindow* window, TimeStep ts, EntityID cameraEntity);

  private:
    Coordinator& m_ecs;
    KeyMappings m_keys{};
    float m_moveSpeed{1.f};
    float m_lookSpeed{1.5f};
  };
}; // namespace vke
