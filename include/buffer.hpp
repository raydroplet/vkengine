#pragma once

#include "allocator.hpp"
#include "core.hpp"
#include "device.hpp"

namespace vke
{
  class Buffer
  {
  public:
    Buffer(
      Device& device,
      uint32_t elementCount,
      VkDeviceSize elementSize,
      VkBufferUsageFlags bufferUsage,
      VkMemoryPropertyFlags requiredMemoryProperties,
      VkMemoryPropertyFlags optimalMemoryProperties = 0,
      VkDeviceSize minOffsetAlignment = 1);

    ~Buffer();

    auto mapMemory(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> VkResult;
    void unmapMemory();

    void write(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void writeByIndex(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize index = 0);

    auto flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> VkResult;
    auto invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> VkResult;

    auto flushByIndex(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize index = 0) -> VkResult;
    auto invalidateByIndex(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize index = 0) -> VkResult;

    auto descriptorInfo(VkDeviceSize range = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const -> VkDescriptorBufferInfo;
    auto descriptorInfoByIndex(VkDeviceSize index) const -> VkDescriptorBufferInfo;

    auto handle() const -> VkBuffer { return m_buffer; }
    auto elementCount() const -> uint32_t { return m_elementCount; }
    auto elementSize() const -> uint32_t { return m_elementSize; }
    auto alignmentSize() const -> VkDeviceSize { return m_alignmentSize; }
    auto size() const -> VkDeviceSize { return m_bufferSize; }
    auto usage() const -> VkBufferUsageFlags { return m_usageFlags; }
    // auto mappedMemory() const -> void* { return m_mappedMemory; }
    // auto memoryPropertyFlags() const -> VkMemoryPropertyFlags { return m_memoryPropertyFlags; }

  private:
    static VkDeviceSize getAlignment(VkDeviceSize elementSize, VkDeviceSize minOffsetAlignment);

    void* m_mappedMemory{nullptr};
    Device& m_device;
    VkBuffer m_buffer{VK_NULL_HANDLE};
    VkDeviceMemory m_memory{VK_NULL_HANDLE};
    uint32_t m_elementCount{};
    VkDeviceSize m_elementSize{};
    VkDeviceSize m_alignmentSize{}; // elementSize + padding
    VkDeviceSize m_bufferSize{};
    //VkMemoryPropertyFlags m_memoryPropertyFlags{};
    VkBufferUsageFlags m_usageFlags{};
  };
} // namespace vke


#if 0
namespace lve
{
  class LveBuffer
  {
  public:
    LveBuffer(
      Device& device,
      VkDeviceSize instanceSize,
      uint32_t instanceCount,
      VkBufferUsageFlags usageFlags,
      VkMemoryPropertyFlags memoryPropertyFlags,
      VkDeviceSize minOffsetAlignment = 1);
    ~LveBuffer();

    LveBuffer(const LveBuffer&) = delete;
    LveBuffer& operator=(const LveBuffer&) = delete;

    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();

    void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void writeToIndex(void* data, int index);
    VkResult flushIndex(int index);
    VkDescriptorBufferInfo descriptorInfoForIndex(int index);
    VkResult invalidateIndex(int index);

    void* getMappedMemory() const
    {
      return mapped;
    }
    VkMemoryPropertyFlags getMemoryPropertyFlags() const
    {
      return memoryPropertyFlags;
    }

  private:
    static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

    LveDevice& lveDevice;
    void* mapped = nullptr;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;

    VkDeviceSize bufferSize;
    uint32_t instanceCount;
    VkDeviceSize instanceSize;
    VkDeviceSize alignmentSize;
    VkBufferUsageFlags usageFlags;
    VkMemoryPropertyFlags memoryPropertyFlags;
  };

} // namespace lve
#endif
