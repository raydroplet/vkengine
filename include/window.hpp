#pragma once

#include "core.hpp"
#include "events.hpp"

namespace vke
{
  class Window
  {
  public:
    Window(EventRelayer& relayer);
    ~Window();

    void create(VkInstance instance);
    void destroySurface(VkInstance instance);
    void handleMinimization();
    bool shouldClose();
    inline void poolEvents() { glfwPollEvents(); }

    static void framebufferResizeCallback(GLFWwindow*, int width, int height);
    static void cursorCallback(GLFWwindow* glfwWindow, double posX, double posY);

    std::pair<double, double> cursorPos();
    int width() { return m_width; }
    int height() { return m_height; }

    VkSurfaceKHR surface() const { return m_surface; }
    operator GLFWwindow*() const { return m_window; }
    bool wasResized() const { return m_framebufferResized; }
    void resetResizedFlag() { m_framebufferResized = false; }
    bool isFullscreen() const { return m_fullscreen; }
    //    const MultiEventListener<event::WindowEvent>& multiListener() const { return m_multiListener; }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

  private:
    EventRelayer& m_eventRelayer;

    GLFWwindow* m_window;
    VkSurfaceKHR m_surface;

    int m_width{320};
    int m_height{180};

    double m_cursorPosY{};
    double m_cursorPosX{};

    int m_pixelsWidth{};
    int m_pixelsHeight{};

    bool m_fullscreen{};
    bool m_framebufferResized{};
  };
}
