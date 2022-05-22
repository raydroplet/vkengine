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
  class RenderSystem
  {
  public:
    RenderSystem(Device& device, RenderSystemContext context);
    ~RenderSystem();

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

  struct SimplePushConstantData
  {
    // glm::mat4 transform{1.f}; // it seems this can not be a mat3, otherwise the shader doesn't work
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
    //  glm::vec2 offset{};
    //  alignas(16) glm::vec3 color{1.f, 1.f, 1.f};
  };
} // namespace vke
