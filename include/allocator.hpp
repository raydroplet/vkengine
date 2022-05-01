#pragma once

#include "core.hpp"
#include "device.hpp"

namespace vke
{
  struct MemoryInfo
  {
    VkMemoryType type;
    VkMemoryHeap heap;
  };

  class MemAllocator
  {
    // void initMemoryPoolsInfo(const VkPhysicalDeviceMemoryProperties& memProperties);

  public:
    MemAllocator() = delete;

    void dispatch();

    static uint32_t findMemoryType(const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties, VkMemoryPropertyFlags optimalProperties = 0);

    static void createBuffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer* buffer);
    static void copyBuffer(Device& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);

    static void allocate(Device& device, VkDeviceSize size, uint32_t memoryTypeBits, VkDeviceMemory* memory);
    static void bindBufferMemory(Device& device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize offset);

    static void allocateBuffer(Device& device, VkBuffer buffer, VkDeviceMemory* memory, VkMemoryPropertyFlags requiredProperties, VkMemoryPropertyFlags optimalProperties = -1);
    static void allocateBuffers(Device& device, std::span<VkBuffer> buffers, VkDeviceMemory& memory, VkMemoryPropertyFlags requiredProperties, VkMemoryPropertyFlags optimalProperties = -1);

    static void createImage(Device& device, const VkImageCreateInfo& createInfo, VkMemoryPropertyFlags properties, VkDeviceMemory* memory, VkImage* image);

    // std::vector<MemoryInfo> m_memoryInfo;
    // std::vector<Memory> m_memory;
    // std::vector<AllocationInfo> allocInfo;
  };
} // namespace vke
