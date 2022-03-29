#include "model.hpp"

namespace vke
{
  Model::Model(Device& device, const Buffer& buffer) :
      m_device{device}, c_buffer{buffer}
  {
  }

  void Model::createIndexBuffer(std::span<uint32_t> indices, uint32_t memoryOffset)
  {
    VkDeviceSize bufferSize{sizeof(indices[0]) * indices.size()};

    Buffer stagingBuffer;
    Memory stagingBufferMemory;

    MemAllocator::createBuffer(m_device, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer);
    MemAllocator::allocateBuffer(m_device, stagingBuffer, stagingBufferMemory, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    MemAllocator::mapBufferMemory(m_device, stagingBufferMemory, bufferSize, memoryOffset, indices.data());
    //    MemAllocator::mapBufferMemory(m_device, stagingBufferMemory, indexBufferSize, vertexBufferSize, indices.data());

    MemAllocator::copyBuffer(m_device, stagingBuffer, 0, c_buffer, memoryOffset, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
  }

  /*
  void Model::updateUniformBuffers(uint32_t currentImage, VkExtent2D swapChainExtent)
  {
    static auto startTime{std::chrono::high_resolution_clock::now()};

    auto currentTime{std::chrono::high_resolution_clock::now()};
    float time{std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count()};

    UniformBufferObject ubo{
      .model = glm::rotate(glm::mat4(1.0f), time * glm::half_pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f)),
      .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
      .proj = glm::perspective(glm::radians(45.0f), (swapChainExtent.width / (float)swapChainExtent.height), 0.1f, 10.0f),
    };

    ubo.proj[1][1] *= -1;

    MemAllocator::mapBufferMemory(m_device, m_uniformBuffersMemory, sizeof(ubo), sizeof(UniformBufferObject) * currentImage, &ubo);
  }
  */

  void Model::bindVertexBuffer(VkCommandBuffer commandBuffer)
  {
    assert(m_verticesCount && "Vertex buffer not created.");
    VkBuffer vertexBuffers[] = {c_buffer};
    VkDeviceSize offsets[]{m_vertexBufferOffset};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  }

  void Model::bindIndexBuffer(VkCommandBuffer commandBuffer)
  {
    assert(m_indicesCount && "Index buffer not created.");
    vkCmdBindIndexBuffer(commandBuffer, c_buffer, m_indexBufferOffset, VK_INDEX_TYPE_UINT32);
  }

  void Model::draw(VkCommandBuffer commandBuffer)
  {
    assert(!m_indicesCount && "Using a index buffer, drawIndexed() instead.");
    vkCmdDraw(commandBuffer, m_verticesCount, 1, 0, 0);
  }

  void Model::drawIndexed(VkCommandBuffer commandBuffer)
  {
    assert(m_indicesCount && "Index buffer not created, draw() instead.");
    vkCmdDrawIndexed(commandBuffer, m_indicesCount, 1, 0, 0, 0);
  }

  std::vector<VkVertexInputAttributeDescription> Model::Vertex2D::getVertexInputAttributeDescription()
  {
    return {
      {
        .location = 0, //  references the shader location
        .binding = 0,  //  which binding the per-vertex data comes
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(Vertex2D, pos),
      },
      {
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex2D, color),
      },
    };
  }

  std::vector<VkVertexInputBindingDescription> Model::Vertex2D::getVertexInputBindingDescription()
  {
    // All of our per-vertex data is packed together in one array, so we're only going to have one binding
    return {
      {
        // VkVertexInputBindingDescription bindingDescription
        .binding = 0, // specifies the index of the binding in the array of bindings
        .stride = sizeof(Vertex2D),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
      },
    };
  }

  std::vector<VkVertexInputAttributeDescription> Model::Vertex3D::getVertexInputAttributeDescription()
  {
    return {
      {
        .location = 0, //  references the shader location
        .binding = 0,  //  which binding the per-vertex data comes
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex3D, pos),
      },
      {
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex3D, color),
      },
    };
  }

  std::vector<VkVertexInputBindingDescription> Model::Vertex3D::getVertexInputBindingDescription()
  {
    // All of our per-vertex data is packed together in one array, so we're only going to have one binding
    return {
      {
        // VkVertexInputBindingDescription bindingDescription
        .binding = 0, // specifies the index of the binding in the array of bindings
        .stride = sizeof(Vertex3D),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
      },
    };
  }

  std::vector<VkDescriptorSetLayoutBinding> Model::UniformBufferObject::getDescriptorSetLayoutBinding()
  {
    return {
      {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = {},
      },
    };
  }

  std::vector<VkDescriptorPoolSize> Model::UniformBufferObject::getDescriptorPoolSize(uint32_t descriptorCount)
  {
    return {
      {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = descriptorCount,
      },
    };
  }

  /*
  std::vector<VkDescriptorBufferInfo> Model::getDescriptorBufferInfo()
  {
    std::vector<VkDescriptorBufferInfo> descriptorBufferInfo;
    descriptorBufferInfo.reserve(m_uniformBuffers.size());

    for(size_t i{}; i < m_uniformBuffers.size(); ++i)
    {
      VkDescriptorBufferInfo bufferInfo{
        .buffer = m_uniformBuffers[i],
        .offset = 0,
        .range = VK_WHOLE_SIZE, // sizeof(Model::UniformBufferObject),
      };

      descriptorBufferInfo.push_back(bufferInfo);
    }

    return descriptorBufferInfo;
  }

  std::vector<VkWriteDescriptorSet> Model::getWriteDescriptorSet(std::span<VkDescriptorSet> descriptorSets, std::span<VkDescriptorBufferInfo> bufferInfo)
  {
    std::vector<VkWriteDescriptorSet> descriptorWrite;
    descriptorWrite.reserve(m_uniformBuffers.size());

    for(size_t i{}; i < m_uniformBuffers.size(); ++i)
    {
      VkWriteDescriptorSet writeDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1, // static_cast<uint32_t>(bufferInfo.size()),
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = {},
        .pBufferInfo = &bufferInfo[i],
        .pTexelBufferView = {},
      };

      descriptorWrite.push_back(writeDescriptorSet);
    }

    return descriptorWrite;
  }

  void Model::createUniformBuffers(uint32_t swapChainImageCount)
  {
    throw std::runtime_error("uniform buffer creation not implemented");

    // VkDeviceSize bufferSize{sizeof(UniformBufferObject)};

    //   m_uniformBuffers.resize(swapChainImageCount);

    //    for(size_t i{}; i < swapChainImageCount; ++i)
    //      MemAllocator::createBuffer(m_device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &m_uniformBuffers[i]);

    // constexpr VkMemoryPropertyFlags optimalProperties{VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
    // constexpr VkMemoryPropertyFlags requiredProperties{VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    //    MemAllocator::allocateBuffers(m_device, m_uniformBuffers, m_uniformBuffersMemory, optimalProperties, requiredProperties);

    // uint32_t offset{};
    // for(size_t i{}; i < m_uniformBuffers.size(); ++i)
    //{
    //   // vkBindBufferMemory(m_device, m_uniformBuffers[i], m_uniformBuffersMemory, m_uniformBuffers[i].memoryOffset);
    //   MemAllocator::bindBufferMemory(m_device, m_uniformBuffers[i], m_uniformBuffersMemory);
    //   // offset += bufferSize;
    // }
  }

  void Model::recreateUniformBuffers(uint32_t swapChainImageCount)
  {
    throw std::runtime_error("uniform buffer recreation not implemented");
    for(VkBuffer buffer : m_uniformBuffers)
      vkDestroyBuffer(m_device, buffer, nullptr);

    vkFreeMemory(m_device, m_uniformBuffersMemory, nullptr);
    createUniformBuffers(swapChainImageCount);
  }
  */

} // namespace vke
