#pragma once

#include "allocator.hpp"
#include "core.hpp"
#include "device.hpp"
#include "utils.hpp"

namespace vke
{
  class Model
  {
  public:
    struct Vertex;
    struct UniformBufferObject;
    struct Builder;

    Model(Device& device, Builder& builder);
    ~Model();

    // << // void updateUniformBuffers(uint32_t currentImage, VkExtent2D swapChainExtent);
    // << // void recreateUniformBuffers(uint32_t swapChainImageCount);

    void createVertexBuffer(std::span<Model::Vertex> vertices, Buffer buffer, Memory memory);
    void createIndexBuffer(std::span<uint32_t> indices, Buffer buffer, Memory memory);

    void bindBuffers(VkCommandBuffer commandBuffer);
    //  void bindVertexBuffer(VkCommandBuffer commandBuffer);
    //  void bindIndexBuffer(VkCommandBuffer commandBuffer);

    void draw(VkCommandBuffer commandBuffer);

    // std::span<Buffer> uniformBuffers() { return m_uniformBuffers; }

    Model(Model const&) = delete;
    Model(Model&&) = default;
    // Model& operator=(const Model&) = default;
    // void createUniformBuffers(uint32_t swapChainImageCount);

  private:
    //    //  TODO: move this
    // << //  std::vector<VkDescriptorBufferInfo> getDescriptorBufferInfo();
    // << //  std::vector<VkWriteDescriptorSet> getWriteDescriptorSet(std::span<VkDescriptorSet> descriptorSets, std::span<VkDescriptorBufferInfo> bufferInfo);

  private:
    Device& m_device;

    size_t m_verticesCount{};
    size_t m_indicesCount{};

    Buffer m_vertexBuffer{};
    Buffer m_indexBuffer{};

    Memory m_vertexBufferMemory{};
    Memory m_indexBufferMemory{};

    // TODO: optimize
    bool m_hasIndexBuffer{};

    // std::vector<Buffer> m_uniformBuffers;
    // Memory m_uniformBuffersMemory;
  };


  struct Vertex2D
  {
    glm::vec2 position;
    glm::vec3 color;

    static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescription();
    static std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescription();
  };

  struct Model::Vertex
  {
    glm::vec3 position{};
    glm::vec3 color{};
    glm::vec3 normal{};
    glm::vec2 uv{};

    static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescription();
    static std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescription();

    bool operator==(Vertex const& other) const
    {
      return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
    }
  };

  struct alignas(16) Model::UniformBufferObject
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

    static std::vector<VkDescriptorSetLayoutBinding> getDescriptorSetLayoutBinding();
    static std::vector<VkDescriptorPoolSize> getDescriptorPoolSize(uint32_t descriptorCount);
  };

  // TODO: either remove Vertex2D or find a way to insert them here
  struct Model::Builder
  {
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};

    Buffer vertexBuffer{};
    Buffer indexBuffer{};

    Memory vertexBufferMemory{};
    Memory indexBufferMemory{};

    void loadModel(std::filesystem::path path);
  };
} // namespace vke
