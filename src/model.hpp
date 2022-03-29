#pragma once

#include "core.hpp"
#include "device.hpp"
#include "allocator.hpp"

namespace vke
{
  class Model
  {
  public:
    struct Vertex2D;
    struct Vertex3D;
    struct UniformBufferObject;

    Model(Device& device, const Buffer& buffer);

 // << // void updateUniformBuffers(uint32_t currentImage, VkExtent2D swapChainExtent);
 // << // void recreateUniformBuffers(uint32_t swapChainImageCount);

    template<typename T>
    void createVertexBuffer(T& vertices, uint32_t memoryOffset); //std::span<T>
    void createIndexBuffer(std::span<uint32_t> indices, uint32_t offset);

    void bindVertexBuffer(VkCommandBuffer commandBuffer);
    void bindIndexBuffer(VkCommandBuffer commandBuffer);

    void draw(VkCommandBuffer commandBuffer);
    void drawIndexed(VkCommandBuffer commandBuffer);

    //std::span<Buffer> uniformBuffers() { return m_uniformBuffers; }

    Model(const Model&) = default;
    //Model& operator=(const Model&) = default;
    //void createUniformBuffers(uint32_t swapChainImageCount);

  private:

//    //  TODO: move this
// << //  std::vector<VkDescriptorBufferInfo> getDescriptorBufferInfo();
// << //  std::vector<VkWriteDescriptorSet> getWriteDescriptorSet(std::span<VkDescriptorSet> descriptorSets, std::span<VkDescriptorBufferInfo> bufferInfo);

  private:
    Device& m_device;

    size_t m_verticesCount{};
    size_t m_indicesCount{};

    const Buffer& c_buffer;

    VkDeviceSize m_vertexBufferOffset{};
    VkDeviceSize m_indexBufferOffset{};

    //std::vector<Buffer> m_uniformBuffers;
    //Memory m_uniformBuffersMemory;
  };


  struct Model::Vertex2D
  {
    glm::vec2 pos;
    glm::vec3 color;

    static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescription();
    static std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescription();
  };

  struct Model::Vertex3D
  {
    glm::vec3 pos;
    glm::vec3 color;

    static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescription();
    static std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescription();
  };

  struct alignas(16) Model::UniformBufferObject
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

    static std::vector<VkDescriptorSetLayoutBinding> getDescriptorSetLayoutBinding();
    static std::vector<VkDescriptorPoolSize> getDescriptorPoolSize(uint32_t descriptorCount);
  };


  template<typename T>
  void Model::createVertexBuffer(T& vertices, uint32_t memoryOffset) //std::span<T>
  {
    //assert((std::is_class<Vertex2D>(vertices[0]) || std::is_class<Vertex3D>(vertices[0])) && "Must be a Vertex type.");
    //assert((typeid(vertices).hash_code() == typeid(Vertex2D).hash_code()) || (typeid(vertices).hash_code() == typeid(Vertex3D).hash_code()));

    assert(vertices.size() >= 3 && "Vertex count must be at least 3");

    VkDeviceSize bufferSize{sizeof(vertices[0]) * vertices.size()};

    Buffer stagingBuffer;
    Memory stagingBufferMemory;

    MemAllocator::createBuffer(m_device, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer);
    MemAllocator::allocateBuffer(m_device, stagingBuffer, stagingBufferMemory, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    MemAllocator::mapBufferMemory(m_device, stagingBufferMemory, bufferSize, 0, vertices.data());
    //    MemAllocator::mapBufferMemory(m_device, stagingBufferMemory, indexBufferSize, vertexBufferSize, indices.data());

    MemAllocator::copyBuffer(m_device, stagingBuffer, 0, c_buffer, memoryOffset, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);

    m_verticesCount = vertices.size();
    m_vertexBufferOffset = memoryOffset;
  }
}
