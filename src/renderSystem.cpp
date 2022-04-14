#include "renderSystem.hpp"

namespace vke
{
  //RenderSystem::RenderSystem(Device& device, ModelManager& modelManager, VkRenderPass renderPass, VkExtent2D extent) :
  //    m_device{device}, m_modelManager{modelManager}
  RenderSystem::RenderSystem(Device& device, RenderSystemContext context, VkRenderPass renderPass, VkExtent2D extent) :
      m_device{device}, m_context{context}
  {
    context.eventRelayer.setCallback(this, &RenderSystem::recreateGraphicsPipeline);

    createPipelineLayout();
    createGraphicsPipeline(renderPass, extent);
  }

  RenderSystem::~RenderSystem()
  {
    /*
    for(auto& entity : entities)
    {
      Coordinator::destroyEntity(entity);
    }
    */

    // m_model.reset();

    /*
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr); // duplicate code
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    */

    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
  }

  void RenderSystem::createDescriptorSetLayout() // std::span<VkDescriptorSetLayoutBinding> uboLayoutBindings
  {
    std::vector<VkDescriptorSetLayoutBinding> uboLayoutBindings{Model::UniformBufferObject::getDescriptorSetLayoutBinding()};

    VkDescriptorSetLayoutCreateInfo layoutInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 0,    // static_cast<uint32_t>(uboLayoutBindings.size())
      .pBindings = nullptr, // uboLayoutBindings.data()
    };

    if(vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
      throw std::runtime_error("Failed to create descriptor set layout");
  }

  void RenderSystem::createPipelineLayout()
  {
    VkPushConstantRange pushConstantRange{
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = sizeof(SimplePushConstantData),
    };

    VkPipelineLayoutCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,    // 1,
      .pSetLayouts = nullptr, //&m_descriptorSetLayout,
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
      .vert = "build/shaders/shader.vert.spv",
      .frag = "build/shaders/shader.frag.spv",
    };

    auto bindingDescription{Model::Vertex::getVertexInputBindingDescription()};
    auto attributeDescription{Model::Vertex::getVertexInputAttributeDescription()};

    config.vertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
    config.vertexInput.pVertexBindingDescriptions = bindingDescription.data();
    config.vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
    config.vertexInput.pVertexAttributeDescriptions = attributeDescription.data();

    config.renderPass = renderPass;
    config.subpass = 0;
    config.pipelineLayout = m_pipelineLayout;

    m_pipeline = std::make_unique<Pipeline>(m_device, shaderPaths, config);
  }

  void RenderSystem::recreateGraphicsPipeline(event::InvalidPipeline& event)
  {
    createGraphicsPipeline(event.renderPass, event.extent);
  }

  void RenderSystem::createDescriptorPool(uint32_t swapchainImageCount)
  {
    // if(m_descriptorPool != VK_NULL_HANDLE)
    //   vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);

    auto descriptorPoolSizes{Model::UniformBufferObject::getDescriptorPoolSize(swapchainImageCount)};

    VkDescriptorPoolCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = {}, // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
      .maxSets = swapchainImageCount,
      .poolSizeCount = 0,    // static_cast<uint32_t>(descriptorPoolSizes.size()),
      .pPoolSizes = nullptr, // descriptorPoolSizes.data(),
    };

    if(vkCreateDescriptorPool(m_device, &createInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
      throw std::runtime_error("Failed to create descriptor pool");
  }

  void RenderSystem::createDescriptorSets(uint32_t swapchainImageCount)
  {
    vkResetDescriptorPool(m_device, m_descriptorPool, 0);

    // std::vector<VkDescriptorSetLayout> layouts(m_swapchain->imageCount(), m_pipeline->descriptorSetLayouts());
    std::vector<VkDescriptorSetLayout> layouts(swapchainImageCount, m_descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = m_descriptorPool,
      .descriptorSetCount = swapchainImageCount,
      .pSetLayouts = layouts.data(),
    };

    m_descriptorSets.resize(swapchainImageCount);

    if(vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate descriptor sets");


    //  auto bufferInfo{m_model->getDescriptorBufferInfo()};
    //  auto descritorWrite{m_model->getWriteDescriptorSet(m_descriptorSets, bufferInfo)};

    for(size_t i{}; i < swapchainImageCount; ++i)
    {
      vkUpdateDescriptorSets(m_device, 1, /*&descritorWrite[i]*/ nullptr, 0, nullptr);
    }
  }

  void RenderSystem::renderEntities(VkCommandBuffer commandBuffer, Camera const& camera, std::span<EntityID> entities)
  {
    m_pipeline->bind(commandBuffer);
    /*m_pipeline->bindDescriptorSets(commandBuffer, &m_descriptorSets[imageIndex], m_pipelineLayout);*/

    // each object will use the same projectionView, so we calculate it once outside the loop
    auto const& projectionView{camera.projection() * camera.view()}; //

    for(auto& entity : entities)
    {
      using namespace cmp;
      Transform3D& transform{m_context.ecs.getComponent<Transform3D>(entity)};
      Color& color{m_context.ecs.getComponent<Color>(entity)};
      Common& common{m_context.ecs.getComponent<cmp::Common>(entity)};

      // transform.rotation = (j) * glm::two_pi<float>();
      // transform.translation.x = {}; //i[entity];
      // transform.translation.y = {}; //i[entity];

//     transform.rotation.x = glm::mod(transform.rotation.x + 0.016f, glm::two_pi<float>());
//    transform.rotation.y = glm::mod(transform.rotation.y + 0.014f, glm::two_pi<float>());
//     transform.rotation.z = glm::mod(transform.rotation.z + 0.012f, glm::two_pi<float>());

      // transform.rotation.z = glm::quarter_pi<float>();

      auto modelMatrix{transform.mat4()};
      auto normalMatrix{transform.normalMatrix()};

      SimplePushConstantData push{
        .transform = projectionView * modelMatrix,
        //.modelMatrix = modelMatrix,
        .normalMatrix = normalMatrix, // automatically convert the mat3 to mat4
      };

      vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

      // model.model->bindBuffers(commandBuffer);
      // model.model->drawIndexed(commandBuffer);

      if(!common.model())
        throw std::runtime_error("fix-me non-existent-model on-rendersystem-renderEntities()");

      common.model()->bindBuffers(commandBuffer);
      common.model()->draw(commandBuffer);
      //model.bindIndexBuffer(commandBuffer);
    }
  }
} // namespace vke
