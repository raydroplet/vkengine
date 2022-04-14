#define TINYOBJLOADER_IMPLEMENTATION
#include "modelManager.hpp"

namespace vke
{
  ModelManager::ModelManager(Device& device, Coordinator& ecs) :
      m_device{device},
      m_ecs{ecs}
  {
  }

  ModelManager::~ModelManager()
  {
    m_models.clear();
    // vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    // vkFreeMemory(m_device, m_vertexMemory, nullptr);

    // vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
    // vkFreeMemory(m_device, m_indexMemory, nullptr);
  }

  void ModelManager::give(Model::Builder builder, std::vector<EntityID> entities)
  {
    assert("Empty model (no vertices)" && builder.vertices.size() > 0);

    //    size_t index{m_modelBuilders.size()};
    //
    //  for(EntityID entity : entities)
    //  {
    //    m_modelIndexes[entity] = index;
    //  }

    m_modelBuilders.push_back({builder, entities});
  }

  void ModelManager::createModels()
  {
    VkBufferUsageFlags vertexUsage{VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    VkBufferUsageFlags indexUsage{VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};


    VkDeviceSize stagingBufferSize{};
    VkDeviceSize vertexOffset{};
    VkDeviceSize indexOffset{};
    for(auto& builder : m_modelBuilders)
    {
      builder.vertexBuffer.memoryOffset = {vertexOffset};
      builder.indexBuffer.memoryOffset = {indexOffset};
      ////

      builder.vertexBuffer.size = sizeof(builder.vertices[0]) * builder.vertices.size();
      builder.indexBuffer.size = sizeof(builder.indices[0]) * builder.indices.size();

      vertexOffset += builder.vertexBuffer.size;
      indexOffset += builder.indexBuffer.size;

      stagingBufferSize = std::max(stagingBufferSize, builder.vertexBuffer.size + builder.indexBuffer.size);
    }

    Buffer stagingBuffer{};
    Memory stagingBufferMemory{};

    MemAllocator::createBuffer(m_device, stagingBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer);
    MemAllocator::allocateBuffer(m_device, stagingBuffer, stagingBufferMemory, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    for(auto& builder : m_modelBuilders)
    {
      //buffer
      MemAllocator::createBuffer(m_device, builder.vertexBuffer.size, vertexUsage, &builder.vertexBuffer.buffer);
      MemAllocator::allocateBuffer(m_device, builder.vertexBuffer, builder.vertexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

      MemAllocator::mapBufferMemory(m_device, stagingBufferMemory.memory, builder.vertexBuffer.size, 0, builder.vertices.data());
      MemAllocator::copyBuffer(m_device, stagingBuffer, 0, builder.vertexBuffer, 0, builder.vertexBuffer.size);

      if(!builder.indexBuffer.size)
        continue;

      MemAllocator::createBuffer(m_device, builder.indexBuffer.size, indexUsage, &builder.indexBuffer.buffer);
      MemAllocator::allocateBuffer(m_device, builder.indexBuffer, builder.indexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

      MemAllocator::mapBufferMemory(m_device, stagingBufferMemory.memory, builder.indexBuffer.size, 0, builder.indices.data());
      MemAllocator::copyBuffer(m_device, stagingBuffer, 0, builder.indexBuffer, builder.indexBuffer.memoryOffset, builder.indexBuffer.size);
    }

    for(auto& builder : m_modelBuilders)
    {
      m_models.emplace_back(m_device, builder);
      for(auto& e : builder.entities)
      {
        auto& c{m_ecs.getComponent<cmp::Common>(e)};
        c.setModel(&m_models.back());
      }
    }

    m_modelBuilders.clear();
  }

  Model& ModelManager::get(EntityID entity)
  {
    //   size_t index{m_modelIndexes[entity]};
    //   return m_models[index];
    throw std::runtime_error("unimplemented function");
  }
} // namespace vke
