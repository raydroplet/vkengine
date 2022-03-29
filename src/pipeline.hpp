#pragma once

#include "core.hpp"

#include "device.hpp"
#include "swapchain.hpp"

namespace vke
{
  class Pipeline
  {
  public:
    // using Config = PipelineConfig;
    // using ShaderPaths = ShaderPaths;

    struct Config;
    struct ShaderPaths;

    Pipeline(Device& device, const ShaderPaths shaderPaths, const Config& config);
    ~Pipeline();

    static void defaultConfig(VkExtent2D extent, bool fullscreen, Config* config);

    // auto descriptorSetLayouts() -> VkDescriptorSetLayout& { return m_descriptorSetLayout; };
    void bindDescriptorSets(VkCommandBuffer commandBuffer, VkDescriptorSet* descriptorSet, VkPipelineLayout layout);
    void bind(VkCommandBuffer commandBuffer);

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    operator VkPipeline()
    {
      return m_pipeline;
    };

  private:
    void createPipeline(const ShaderPaths& shaderPaths, const Config& config);
    // void createDescriptorSetLayout(const Config& config); //std::span<VkDescriptorSetLayoutBinding> uboLayoutBindings
    // void createPipelineLayout();
    void createShaderModule(std::vector<char>& buffer, VkShaderModule* module);

    static auto readFile(std::filesystem::path path) -> std::vector<char>;

    Device& m_device;
    // VkExtent2D m_swapchainExtent;
    VkPipeline m_pipeline;
    // VkPipelineLayout m_pipelineLayout;
    // VkDescriptorSetLayout m_descriptorSetLayout;
  };

  struct Pipeline::Config
  {
    Config() = default;

    //  you can optionally cache the config (when not in fullscreen) and recreate the pipeline using it
    //  i am not doing this for now. do only if creating a new config is expensive enough
    Config(const Config&);
    Config& operator=(const Config&);

    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

    // std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
    VkViewport viewport{};
    VkRect2D scissor{};
    VkPipelineVertexInputStateCreateInfo vertexInput{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkPipelineViewportStateCreateInfo viewportState{};
    VkPipelineRasterizationStateCreateInfo rasterization{};
    VkPipelineMultisampleStateCreateInfo multisample{};
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlend{};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    std::vector<VkDynamicState> dynamicStateEnables{};

    // std::vector<VkDescriptorSetLayoutBinding> uboLayoutBindings{};

    VkPipelineLayout pipelineLayout{nullptr};
    VkRenderPass renderPass{nullptr};
    uint32_t subpass{0};
  };

  struct Pipeline::ShaderPaths
  {
    std::filesystem::path vert;
    std::filesystem::path frag;
  };
} // namespace vke
