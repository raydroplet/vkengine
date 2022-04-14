#include "window.hpp"

namespace vke
{
  Window::Window(EventRelayer& relayer) :
      m_eventRelayer{relayer}
  {
    glfwInit();
  }

  Window::~Window()
  {
    glfwTerminate();
  }

  void Window::destroySurface(VkInstance instance)
  {
    vkDestroySurfaceKHR(instance, m_surface, nullptr);
  }

  void Window::create(VkInstance instance)
  {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, "", nullptr, nullptr);
    glfwGetFramebufferSize(m_window, &m_pixelsWidth, &m_pixelsHeight);
    glfwCreateWindowSurface(instance, m_window, nullptr, &m_surface);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, &framebufferResizeCallback);

    ////////////////////////////
    // tell GLFW that it should hide the cursor and capture it.
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(m_window, &Window::cursorCallback); // called each time the mouse moves
    glfwGetCursorPos(m_window, &m_cursorPosX, &m_cursorPosY);
    ////////////////////////////

    // glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

  }

  bool Window::shouldClose()
  {
    return glfwWindowShouldClose(m_window);
  }

  void Window::handleMinimization()
  {
    // glfwGetFramebufferSize(m_window, &width, &height);
    while(m_pixelsWidth == 0 || m_pixelsHeight == 0)
    {
      // glfwGetFramebufferSize(m_window, &width, &height);  //  already updated with the resize callback
      glfwWaitEvents();
    }
  }

  void Window::framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height)
  {
    Window* window{reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow))};
    window->m_framebufferResized = true;
    window->m_pixelsWidth = width;
    window->m_pixelsHeight = height;
//    window->m_eventRelayer.queue(event::WindowResized{width, height});
  }

  void Window::cursorCallback(GLFWwindow* glfwWindow, double posX, double posY)
  {
    Window* window{reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow))};
    window->m_cursorPosX = posX;
    window->m_cursorPosY = posY;
  }

  std::pair<double, double> Window::cursorPos()
  {
    return {m_cursorPosX, m_cursorPosY};
  }
} // namespace vke
