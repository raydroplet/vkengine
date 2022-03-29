#pragma once

#include "allocator.hpp"
#include "model.hpp"
#include "window.hpp"
#include "renderer.hpp"
#include "modelManager.hpp"
#include "renderSystem.hpp"
#include "camera.hpp"
#include "events.hpp"

#include "input.hpp"

namespace vke
{
  class Program
  {
  public:
    Program();
    ~Program();

    void run();
    void dispatchEvents();
//    static void notifySwapchainRecreation(void* object, VkRenderPass renderPass, VkExtent2D extent);

  private:
    std::unique_ptr<Model> createCubeModel(Device& device, glm::vec3 offset);
    void loadEntities();

//   void bindListeners();
//   void poolEvents();
  private:
    EventRelayer m_eventRelayer;

    Window m_window;
    Device m_device;

    ModelManager m_modelManager;
    Coordinator m_ecs;

    Renderer m_renderer;

    Camera camera;
    std::unique_ptr<RenderSystem> m_renderSystem;
    //{m_device, m_modelManager, m_renderer.renderPass(), m_renderer.swapchainExtent()};

    std::vector<EntityID> m_entities;
  };
} // namespace vke
