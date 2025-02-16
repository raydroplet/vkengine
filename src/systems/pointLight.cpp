#include "systems/pointLight.hpp"

namespace vke
{
  PointLightSystem::PointLightSystem(Device& device, RenderSystemContext context) :
    m_device{device},
    m_eventRelayer{context.eventRelayer}
  // m_modelManager{context.modelManager}
  {
    m_eventRelayer.setCallback(this, &PointLightSystem::recreateGraphicsPipeline);

    createPipelineLayout(context.globalDescriptorSetLayout);
    createGraphicsPipeline(context.renderPass, context.extent);
  }

  PointLightSystem::~PointLightSystem()
  {
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
  }

  void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout)
  {
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalDescriptorSetLayout};

    VkPipelineLayoutCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
      .pSetLayouts = descriptorSetLayouts.data(),
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr,
    };

    if(vkCreatePipelineLayout(m_device, &createInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
      throw std::runtime_error("Failed to create pipelineLayout");
  }

  void PointLightSystem::createGraphicsPipeline(VkRenderPass renderPass, VkExtent2D extent)
  {
    Pipeline::Config config{};
    Pipeline::defaultConfig(extent, false, &config);

    Pipeline::ShaderPaths shaderPaths{
      .vert = m_device.assetsPath().string() + "/build/shaders/pointLight.vert.spv",
      .frag = m_device.assetsPath().string() + "/build/shaders/pointLight.frag.spv",
    };

    config.renderPass = renderPass;
    config.subpass = 0;
    config.pipelineLayout = m_pipelineLayout;

    m_pipeline = std::make_unique<Pipeline>(m_device, shaderPaths, config);
  }

  void PointLightSystem::recreateGraphicsPipeline(event::InvalidPipeline& event)
  {
    createGraphicsPipeline(event.renderPass, event.extent);
  }

  void PointLightSystem::render(FrameInfo info)
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

    vkCmdDraw(info.commandBuffer, 6, 1, 0, 0);
  }
} // namespace vke
