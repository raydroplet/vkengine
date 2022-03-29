#include "pipeline.hpp"

// Pipeline::

namespace vke
{
  Pipeline::Pipeline(Device& device, const ShaderPaths shaderPaths, const Pipeline::Config& config) :
      m_device{device}
  {
    // createDescriptorSetLayout(config);
    // createPipelineLayout();
    createPipeline(shaderPaths, config);
  }

  Pipeline::~Pipeline()
  {
    // vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    // vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
  }

  void Pipeline::defaultConfig(VkExtent2D extent, bool fullscreen, Config* config)
  {
    config->vertexInput = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 0,
      .pVertexBindingDescriptions = nullptr, // Optional
      .vertexAttributeDescriptionCount = 0,
      .pVertexAttributeDescriptions = nullptr, // Optional
    };

    config->inputAssembly = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
    };

    config->rasterization = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      /*
      // inverted because of glm
      // .rasterization.cullMode = VK_CULL_MODE_NONE,
      // .rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE,
      //.cullMode = VK_CULL_MODE_BACK_BIT,
      */
      .cullMode = VK_CULL_MODE_NONE,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .depthBiasConstantFactor = {},
      .depthBiasClamp = {},
      .depthBiasSlopeFactor = {},
      .lineWidth = 1.0f,
    };

    config->multisample = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
      .minSampleShading = 1.0f, // Optional
      .pSampleMask = {},
      .alphaToCoverageEnable = {}, // Optional
      .alphaToOneEnable = {},      // Optional
    };

    config->colorBlendAttachment = {
      .blendEnable = VK_FALSE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
      .colorBlendOp = VK_BLEND_OP_ADD,             // Optional
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
      .alphaBlendOp = VK_BLEND_OP_ADD,             // Optional
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    config->colorBlend = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY, // Optional
      .attachmentCount = 1,
      .pAttachments = &config->colorBlendAttachment,
      .blendConstants = {},
    };

    config->depthStencil = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp = VK_COMPARE_OP_LESS,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE,
      .front = {},
      .back = {},
      .minDepthBounds = 0.f, //optional
      .maxDepthBounds = 1.f, //optional
    };

    if(fullscreen)
    {
      config->dynamicStateEnables = {
        VK_DYNAMIC_STATE_VIEWPORT,
      };

      config->dynamicState = {
        .dynamicStateCount = static_cast<uint32_t>(config->dynamicStateEnables.size()),
        .pDynamicStates = config->dynamicStateEnables.data(),
      };
    }
    else
    {
      config->dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      };

      config->viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
      };

      config->scissor = {
        .offset = {0, 0},
        .extent = extent,
      };

      config->viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &config->viewport,
        .scissorCount = 1,
        .pScissors = &config->scissor,
      };
    }
  }

  void Pipeline::createPipeline(const ShaderPaths& shaderPaths, const Config& config)
  {
    // std::vector<char> vertShaderCode{readFile("build/shaders/shader.vert.spv")};
    // std::vector<char> fragShaderCode{readFile("build/shaders/shader.frag.spv")};

    std::vector<char> vertShaderCode{readFile(shaderPaths.vert)};
    std::vector<char> fragShaderCode{readFile(shaderPaths.frag)};

    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;

    createShaderModule(vertShaderCode, &vertShaderModule);
    createShaderModule(fragShaderCode, &fragShaderModule);

    VkPipelineShaderStageCreateInfo shaderStages[]{
      {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
      },
      {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
        .pSpecializationInfo = {},
      }};

    VkGraphicsPipelineCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = std::size(shaderStages),
      .pStages = shaderStages,
      .pVertexInputState = &config.vertexInput,
      .pInputAssemblyState = &config.inputAssembly,
      .pViewportState = &config.viewportState,
      .pRasterizationState = &config.rasterization,
      .pMultisampleState = &config.multisample,
      .pDepthStencilState = &config.depthStencil,
      .pColorBlendState = &config.colorBlend,
      .pDynamicState = &config.dynamicState,
      //.layout = m_pipelineLayout,
      .layout = config.pipelineLayout,
      .renderPass = config.renderPass,
      .subpass = config.subpass,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
    };

    if(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_pipeline))
      throw std::runtime_error("Failed to create pipeline");

    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
  }

  void Pipeline::createShaderModule(std::vector<char>& buffer, VkShaderModule* module)
  {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<uint32_t*>(buffer.data());

    if(vkCreateShaderModule(m_device, &createInfo, nullptr, module) != VK_SUCCESS)
      throw std::runtime_error("Failed to create shader module");
  }

  void Pipeline::bindDescriptorSets(VkCommandBuffer commandBuffer, VkDescriptorSet* descriptorSet, VkPipelineLayout layout)
  {
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, descriptorSet, 0, nullptr);
  }

  void Pipeline::bind(VkCommandBuffer commandBuffer)
  {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
  }

  std::vector<char> Pipeline::readFile(std::filesystem::path path)
  {
    std::ifstream file{path, std::ios::ate | std::ios::binary};

    if(!file.is_open())
      throw std::runtime_error("Failed to open file: " + path.string());

    // file.seekg(file.end);
    // buffer.resize(file.tellg());
    // size_t fileSize = static_cast<size_t>(file.tellg()); //  may be needed for alignment
    std::vector<char> buffer(file.tellg());

    file.seekg(file.beg);
    file.read(buffer.data(), buffer.size());
    // file.close(); //  RAII

    return buffer;
  }

  Pipeline::Config& Pipeline::Config::operator=(const Config& other)
  {
    viewport = other.viewport;
    scissor = other.scissor;
    vertexInput = other.vertexInput;
    inputAssembly = other.inputAssembly;

    viewportState = other.viewportState;
    viewportState.pViewports = &viewport;
    viewportState.pScissors = &scissor;

    rasterization = other.rasterization;
    multisample = other.multisample;
    depthStencil = other.depthStencil;

    colorBlendAttachment = other.colorBlendAttachment;
    colorBlend = other.colorBlend;
    colorBlend.pAttachments = &colorBlendAttachment;

    dynamicState = other.dynamicState;
    pipelineLayout = other.pipelineLayout;
    renderPass = other.renderPass;
    subpass = other.subpass;
    dynamicState = other.dynamicState;

    dynamicStateEnables = other.dynamicStateEnables;
    dynamicState.pDynamicStates = dynamicStateEnables.data();

    return *this;
  }

  Pipeline::Config::Config(const Config& other)
  {
    *this = other;
  }
} // namespace vke
