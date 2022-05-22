#pragma once

#include "allocator.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "events.hpp"
#include "input.hpp"
#include "model.hpp"
#include "modelManager.hpp"
#include "systems/renderSystem.hpp"
#include "systems/pointLight.hpp"
#include "descriptor.hpp"
#include "renderer.hpp"
#include "window.hpp"

namespace vke
{
  struct GlobalUbo
  {
    glm::mat4 projectionMatrix{1.f};
    glm::mat4 ViewMatrix{1.f};
    glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f};
    glm::vec3 lightPosition{-1.f, -1.f, -1.f};
    alignas(16) glm::vec4 lightColor{1.f, 1.f, 1.f, 1.0f};
    glm::vec4 cameraPosition{1.f};
  };

  class Program
  {
  public:
    Program();
    ~Program();

    void run();
    void dispatchEvents();
    //    static void notifySwapchainRecreation(void* object, VkRenderPass renderPass, VkExtent2D extent);

  private:
    void loadEntities();

    //   void bindListeners();
    //   void poolEvents();

  private:
    EventRelayer m_eventRelayer;

    Window m_window;
    Device m_device;

    Coordinator m_ecs;
    ModelManager m_modelManager;

    Renderer m_renderer;
    //{m_device, m_modelManager, m_renderer.renderPass(), m_renderer.swapchainExtent()};

    std::unique_ptr<DescriptorPool> m_globalDescriptorPool{};
    std::vector<EntityID> m_entities;
  };
} // namespace vke
