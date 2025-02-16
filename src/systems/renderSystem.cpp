#include "systems/renderSystem.hpp"

namespace vke
{
  RenderSystem::RenderSystem(Device& device, RenderSystemContext context) :
    m_device{device},
    m_eventRelayer{context.eventRelayer}
  // m_modelManager{context.modelManager}
  {
    m_eventRelayer.setCallback(this, &RenderSystem::recreateGraphicsPipeline);

    createPipelineLayout(context.globalDescriptorSetLayout);
    createGraphicsPipeline(context.renderPass, context.extent);
  }

  RenderSystem::~RenderSystem()
  {
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
  }

  void RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout)
  {
    VkPushConstantRange pushConstantRange{
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = sizeof(SimplePushConstantData),
    };

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalDescriptorSetLayout};

    VkPipelineLayoutCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
      .pSetLayouts = descriptorSetLayouts.data(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange,
    };

    if(vkCreatePipelineLayout(m_device, &createInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
      throw std::runtime_error("Failed to create pipelineLayout");
  }

  void RenderSystem::createGraphicsPipeline(VkRenderPass renderPass, VkExtent2D extent)
  {
    Pipeline::Config config{};
    Pipeline::defaultConfig(extent, false, &config);

    Pipeline::ShaderPaths shaderPaths{
      .vert = m_device.assetsPath().string() + "/build/shaders/shader.vert.spv",
      .frag = m_device.assetsPath().string() + "/build/shaders/shader.frag.spv",
    };

    config.bindingDescriptions = Model::Vertex::getVertexInputBindingDescription();
    config.attributeDescriptions = Model::Vertex::getVertexInputAttributeDescription();
    config.renderPass = renderPass;
    config.subpass = 0;
    config.pipelineLayout = m_pipelineLayout;

    m_pipeline = std::make_unique<Pipeline>(m_device, shaderPaths, config);
  }

  void RenderSystem::recreateGraphicsPipeline(event::InvalidPipeline& event)
  {
    createGraphicsPipeline(event.renderPass, event.extent);
  }

  void RenderSystem::render(FrameInfo info)
  {
    m_pipeline->bind(info.commandBuffer);
    /*m_pipeline->bindDescriptorSets(commandBuffer, &m_descriptorSets[imageIndex], m_pipelineLayout);*/

    // each object will use the same projectionView, so we calculate it once outside the loop
    // auto const& projectionView{info.camera.projection() * info.camera.view()}; //

    vkCmdBindDescriptorSets(
      info.commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_pipelineLayout,
      0, 1,
      &info.globalDescriptorSet,
      0, nullptr);

    // auto projectionView{info.camera.projection() * info.camera.view()};

    for(auto& entity : info.entities) {
      using namespace cmp;
      Transform3D& transform{info.ecs.getComponent<Transform3D>(entity)};
      Common& common{info.ecs.getComponent<cmp::Common>(entity)};

      // auto modelMatrix{transform.mat4()};
      SimplePushConstantData push{
        //.transform = projectionView * modelMatrix,
        .modelMatrix = transform.mat4(),
        .normalMatrix = transform.normalMatrix(), // glm automatically converts the mat3 to mat4
      };

      vkCmdPushConstants(info.commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

      // model.model->bindBuffers(commandBuffer);
      // model.model->drawIndexed(commandBuffer);

      if(!common.model())
        throw std::runtime_error("fix-me non-existent-model on-rendersystem-renderEntities()");

      common.model()->bindBuffers(info.commandBuffer);
      common.model()->draw(info.commandBuffer);
      // model.bindIndexBuffer(commandBuffer);
    }
  }
} // namespace vke
