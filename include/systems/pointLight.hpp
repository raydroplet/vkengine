#pragma once

#include "camera.hpp"
#include "components.hpp"
#include "core.hpp"
#include "device.hpp"
#include "ecs.hpp"
#include "frameInfo.hpp"
#include "modelManager.hpp"
#include "pipeline.hpp"
#include "utilities.hpp"

namespace vke
{
  class PointLightSystem
  {
  public:
    PointLightSystem(Device& device, RenderSystemContext context);
    ~PointLightSystem();

    // void loadModel(std::shared_ptr<Model>& models);
    void loadEntities();
    void render(FrameInfo info);

    void recreateGraphicsPipeline(event::InvalidPipeline& event);

  private:
    void createGraphicsPipeline(VkRenderPass renderPass, VkExtent2D extent);
    void createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);

    void cleanup();

  private:
    Device& m_device;
    EventRelayer& m_eventRelayer;
    // ModelManager& m_modelManager;

    VkPipelineLayout m_pipelineLayout;
    std::unique_ptr<Pipeline> m_pipeline;
  };
} // namespace vke
