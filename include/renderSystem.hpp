#pragma once

#include "camera.hpp"
#include "components.hpp"
#include "core.hpp"
#include "device.hpp"
#include "ecs.hpp"
#include "modelManager.hpp"
#include "pipeline.hpp"
#include "utilities.hpp"

namespace vke
{
  class RenderSystem
  {
  public:
    // RenderSystem(Device& device, ModelManager& modelManager, VkRenderPass renderPass, VkExtent2D extent);
    RenderSystem(Device& device, RenderSystemContext context, VkRenderPass renderPass, VkExtent2D extent);
    ~RenderSystem();

    // void loadModel(std::shared_ptr<Model>& models);
    void loadEntities();
    void renderEntities(VkCommandBuffer commandBuffer, const Camera& camera, std::span<EntityID> entities);

    void recreateGraphicsPipeline(event::InvalidPipeline& event);

  private:
    void createGraphicsPipeline(VkRenderPass renderPass, VkExtent2D extent);
    void createPipelineLayout();

    void cleanup();

    void createDescriptorSetLayout();
    void createDescriptorPool(uint32_t swapchainImageCount);
    void createDescriptorSets(uint32_t swapchainImageCount);

  private:
    Device& m_device;
    RenderSystemContext m_context;

    VkPipelineLayout m_pipelineLayout;
    std::unique_ptr<Pipeline> m_pipeline;

    // ModelManager& m_modelManager;

    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorPool m_descriptorPool{VK_NULL_HANDLE};
    std::vector<VkDescriptorSet> m_descriptorSets;
  };

  struct SimplePushConstantData
  {
    glm::mat4 transform{1.f}; // it seems this can not be a mat3, otherwise the shader doesn't work
    glm::mat4 normalMatrix{1.f};
    //glm::mat4 modelMatrix{1.f};
    // glm::vec2 offset{};
    // alignas(16) glm::vec3 color{1.f, 1.f, 1.f};
  };
} // namespace vke
