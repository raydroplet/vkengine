#include "allocator.hpp"

namespace vke
{
  uint32_t MemAllocator::findMemoryType(const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties, VkMemoryPropertyFlags optimalProperties)
  {
    const uint32_t& memoryCount = memoryProperties.memoryTypeCount;
    VkMemoryPropertyFlags requestedProperties[]{optimalProperties, requiredProperties};

    for(size_t i{}; i < std::size(requestedProperties); ++i)
    {
      for(uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex)
      {
        const uint32_t memoryTypeBits = (1 << memoryIndex);
        const bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;

        const VkMemoryPropertyFlags& properties = memoryProperties.memoryTypes[memoryIndex].propertyFlags;
        const bool hasRequestedProperties = (properties & requestedProperties[i]) == requestedProperties[i];

        if(isRequiredMemoryType && hasRequestedProperties)
          return static_cast<int32_t>(memoryIndex);
      }
    }

    throw std::runtime_error("Failed to find memory type");
    // return -1;
  }

  void MemAllocator::allocate(Device& device, VkDeviceSize size, uint32_t memoryTypeBits, VkDeviceMemory* memory)
  {
    // size = ((size / alignment) + int((size % alignment) != 0)) * alignment;

    VkMemoryAllocateInfo allocateInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = size,
      .memoryTypeIndex = memoryTypeBits,
    };

    if(vkAllocateMemory(device, &allocateInfo, nullptr, memory) != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate buffer memory");

    // vkAllocateMemory is guaranteed to return an allocation that is aligned to the largest alignment requirement for your Vulkan implementation(ie: if one resource type needs to be 16 byte aligned, and another type 128 byte aligned, all vkAllocateMemory calls will be 128 bit aligned), so you never have to worry about the alignment of these allocs.
  }


  void MemAllocator::allocateBuffer(Device& device, Buffer buffer, Memory& memory, VkMemoryPropertyFlags requiredProperties, VkMemoryPropertyFlags optimalProperties)
  {
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    uint32_t memoryType{findMemoryType(device.physicalInfo().memoryProperties, memRequirements.memoryTypeBits, optimalProperties, requiredProperties)};
    allocate(device, memRequirements.size, memoryType, &memory.memory);
    buffer.size = memRequirements.size;
    buffer.memoryOffset = 0;

    bindBufferMemory(device, buffer, memory);

    memory.size = buffer.size;
  }

  inline void MemAllocator::bindBufferMemory(Device& device, Buffer& buffer, Memory& memory)
  {
    vkBindBufferMemory(device, buffer, memory.memory, buffer.memoryOffset);
  }

  void MemAllocator::allocateBuffers(Device& device, std::span<Buffer> buffers, Memory& memory, VkMemoryPropertyFlags requiredProperties, VkMemoryPropertyFlags optimalProperties)
  {
    // uint32_t alignment{};
    uint32_t memoryTypeBits{};
    VkDeviceSize allocationSize{};
    VkMemoryRequirements memRequirements;

    for(uint32_t i{}; i < buffers.size(); ++i)
    {
      vkGetBufferMemoryRequirements(device, buffers[i], &memRequirements);
      memoryTypeBits |= memRequirements.memoryTypeBits;

      const auto& alignment{memRequirements.alignment};
      const auto& size{memRequirements.size};

      buffers[i].memoryOffset = allocationSize;
      buffers[i].size = memRequirements.size;
      allocationSize += ((size / alignment) + (int)((size % alignment) != 0)) * alignment;

      /*
         buffers[i].memoryOffset = ((allocationSize / alignment) + (int)((allocationSize % alignment) != 0)) * alignment;
         buffers[i].size = memRequirements.size;
         */

      /*
         if(memRequirements.alignment > alignment)
         alignment = memRequirements.alignment;
         */
    }

    uint32_t memoryType{findMemoryType(device.physicalInfo().memoryProperties, memoryTypeBits, requiredProperties, optimalProperties)};

    VkMemoryAllocateInfo allocateInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = allocationSize,
      .memoryTypeIndex = memoryType,
    };

    if(vkAllocateMemory(device, &allocateInfo, nullptr, &memory.memory) != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate buffer memory");

    // vkAllocateMemory is guaranteed to return an allocation that is aligned to the largest alignment requirement for your Vulkan implementation(ie: if one resource type needs to be 16 byte aligned, and another type 128 byte aligned, all vkAllocateMemory calls will be 128 bit aligned), so you never have to worry about the alignment of these allocs.

    memory.size = allocationSize;
  }

  void MemAllocator::createBuffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer* buffer)
  {
    VkBufferCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,   // sizeof(Vertex) * vertices.size()
      .usage = usage, // VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      //.queueFamilyIndexCount = ,
      //.pQueueFamilyIndices = ,
    };

    uint32_t indices[]{device.queues().graphicsFamily, device.queues().transferFamily};
    if(device.queues().graphicsFamily != device.queues().transferFamily)
    {
      createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = std::size(indices);
      createInfo.pQueueFamilyIndices = indices;
    }

    if(vkCreateBuffer(device, &createInfo, nullptr, buffer) != VK_SUCCESS)
      throw std::runtime_error("Failed to create vertex buffer");
  }

  void MemAllocator::copyBuffer(Device& device, VkBuffer srcBuffer, VkDeviceSize srcOffset, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size)
  {
    VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = device.commandPools().transfer,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VkBufferCopy copyRegion{
      .srcOffset = srcOffset,
      .dstOffset = dstOffset,
      .size = size,
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
    };

    vkQueueSubmit(device.queues().transfer, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device.queues().transfer);

    vkFreeCommandBuffers(device, device.commandPools().transfer, 1, &commandBuffer);
  }

  void MemAllocator::mapBufferMemory(Device& device, VkDeviceMemory memory, VkDeviceSize size, VkDeviceSize offset, void* inputData)
  {
    void* data;

    vkMapMemory(device, memory, offset, size, 0, &data);
    memcpy(data, inputData, size);
    vkUnmapMemory(device, memory);
  }

  // TODO: maybe cache the create info just like in modelManager
  void MemAllocator::createImage(Device& device, const VkImageCreateInfo& createInfo, VkMemoryPropertyFlags properties, VkDeviceMemory* memory, VkImage* image)
  {
    if(vkCreateImage(device, &createInfo, nullptr, image) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create image");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = findMemoryType(device.physicalInfo().memoryProperties, memRequirements.memoryTypeBits, properties, 0),
    };

    if(vkAllocateMemory(device, &allocInfo, nullptr, memory) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to allocate image memory");
    }

    vkBindImageMemory(device, *image, *memory, 0);
  }

  void MemAllocator::dispatch()
  {
    ;
  }

  /*
     MemAllocator::MemAllocator()
     {
     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT = 0x00000001,
     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT = 0x00000002,
     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT = 0x00000004,
     VK_MEMORY_PROPERTY_HOST_CACHED_BIT = 0x00000008,
     VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT = 0x00000010,
     VK_MEMORY_PROPERTY_PROTECTED_BIT = 0x00000020,
     VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD = 0x00000040,
     VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD = 0x00000080,
     VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV = 0x00000100,
     VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF

     VkBuffer buffer;
     m_device.createBuffer(128, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &buffer);

     VkMemoryRequirements memRequirements;
     vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

     VkMemoryPropertyFlags optimalProperties{VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
     VkMemoryPropertyFlags requiredProperties{VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

     int32_t memoryType = findMemoryType(memProperties, memRequirements.memoryTypeBits, optimalProperties, requiredProperties);

     memoryType++;

     VkDeviceMemory memory;
     allocate(device, 4 * MB, memRequirements.memoryTypeBits, &memory);

     vkFreeMemory(m_device, memory, nullptr);
     }
     */

  /*
     void MemAllocator::initMemoryPoolsInfo(const VkPhysicalDeviceMemoryProperties& memProperties)
     {
     const uint32_t& memTypeCount{memProperties.memoryTypeCount};

     m_memoryInfo.resize(memTypeCount);

     for(uint32_t i{}; i < memProperties.memoryTypeCount; ++i)
     {
     const auto& heapIndex{memProperties.memoryTypes[i].heapIndex};

     m_memoryInfo[i].type = memProperties.memoryTypes[i];
     m_memoryInfo[i].heap = memProperties.memoryHeaps[heapIndex];
     }
     }
     */
} // namespace vke
