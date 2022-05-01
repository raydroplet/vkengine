#pragma once

#include "allocator.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "events.hpp"
#include "input.hpp"
#include "model.hpp"
#include "modelManager.hpp"
#include "renderSystem.hpp"
#include "descriptor.hpp"
#include "renderer.hpp"
#include "window.hpp"

namespace vke
{
  struct GlobalUbo
  {
    alignas(16) glm::mat4 projectionView{1.f};
    alignas(16) glm::vec3 lightDirection{glm::normalize(glm::vec3{1.f, -3.f, -1.f})};
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

    std::unique_ptr<RenderSystem> m_renderSystem;
    //{m_device, m_modelManager, m_renderer.renderPass(), m_renderer.swapchainExtent()};

    std::unique_ptr<DescriptorPool> m_globalDescriptorPool{};
    std::vector<EntityID> m_entities;
  };
} // namespace vke
