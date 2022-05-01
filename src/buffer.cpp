#include "buffer.hpp"

namespace vke
{
  Buffer::Buffer(
    Device& device,
    uint32_t elementCount,
    VkDeviceSize elementSize,
    VkBufferUsageFlags bufferUsage,
    VkMemoryPropertyFlags requiredMemoryProperties,
    VkMemoryPropertyFlags optimalMemoryProperties,
    VkDeviceSize minOffsetAlignment) :
      m_device{device},
      m_elementCount{elementCount},
      m_elementSize{elementSize},
      m_alignmentSize{getAlignment(elementSize, minOffsetAlignment)},
      m_bufferSize{m_alignmentSize * m_elementCount},
      m_usageFlags{bufferUsage}
  // m_memoryPropertyFlags{memoryPropertyFlags}
  {
    MemAllocator::createBuffer(device, m_bufferSize, bufferUsage, &m_buffer);
    MemAllocator::allocateBuffer(device, m_buffer, &m_memory, requiredMemoryProperties, optimalMemoryProperties);
  }

  Buffer::~Buffer()
  {
    unmapMemory();

    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_memory, nullptr);
  }

  VkResult Buffer::mapMemory(VkDeviceSize size, VkDeviceSize offset)
  {
    constexpr VkMemoryPropertyFlags flags{}; // reserved for future use
    return vkMapMemory(m_device, m_memory, offset, size, flags, &m_mappedMemory);
  }

  void Buffer::unmapMemory()
  {
    if(!m_mappedMemory)
      return;

    vkUnmapMemory(m_device, m_memory);
    m_mappedMemory = nullptr;
  }

  void Buffer::write(void* data, VkDeviceSize size, VkDeviceSize offset)
  {
    assert("Writing to non-mapped memory" && m_mappedMemory);

    if(size == VK_WHOLE_SIZE)
    {
      memcpy(m_mappedMemory, data, m_bufferSize);
    }
    else
    {
      memcpy(reinterpret_cast<char*>(m_mappedMemory) + offset, data, size);
    }
  }

  void Buffer::writeByIndex(void* data, VkDeviceSize size, VkDeviceSize index)
  {
    write(data, m_alignmentSize * index);
  }

  // Flush a memory range of the buffer to make it visible to the device
  VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
  {
    VkMappedMemoryRange ranges[]{{
      VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
      nullptr,
      m_memory,
      offset,
      size,
    }};

    return vkFlushMappedMemoryRanges(m_device, std::size(ranges), ranges);
  }

  // Invalidate a memory range of the buffer to make it visible to the host
  VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
  {
    VkMappedMemoryRange ranges[]{{
      VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
      nullptr,
      m_memory,
      offset,
      size,
    }};

    return vkInvalidateMappedMemoryRanges(m_device, std::size(ranges), ranges);
  }

  VkResult Buffer::flushByIndex(VkDeviceSize size, VkDeviceSize index)
  {
    return flush(m_alignmentSize, index * m_alignmentSize);
  }

  VkResult Buffer::invalidateByIndex(VkDeviceSize size, VkDeviceSize index)
  {
    return invalidate(m_alignmentSize, index * m_alignmentSize);
  }

  // see 'algorithms.doc -> { boundary number alignment }' for the full explanation.
  // Returns the minimum elementSize required to be compatible with devices minOffsetAlignment.
  VkDeviceSize Buffer::getAlignment(VkDeviceSize elementSize, VkDeviceSize minOffsetAlignment)
  {
    assert("minOffsetAlignment is not 1 or a multiple of 2" && minOffsetAlignment == 1 || !(minOffsetAlignment % 2));

    if(minOffsetAlignment > 0)
      return (elementSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);

    return elementSize;
  }

  auto Buffer::descriptorInfo(VkDeviceSize range, VkDeviceSize offset) const -> VkDescriptorBufferInfo
  {
    return VkDescriptorBufferInfo{
      .buffer = m_buffer,
      .offset = offset,
      .range = range,
    };
  }

  auto Buffer::descriptorInfoByIndex(VkDeviceSize index) const -> VkDescriptorBufferInfo
  {
    return descriptorInfo(m_alignmentSize, m_alignmentSize * index);
  }
} // namespace vke


///////////////////


#if 0
namespace lve
{
/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

  #include "lve_buffer.hpp"

// std
  #include <cassert>
  #include <cstring>

namespace lve
{
  /**
   * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
   *
   * @param instanceSize The size of an instance
   * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
   * minUniformBufferOffsetAlignment)
   *
   * @return VkResult of the buffer mapping call
   */
  VkDeviceSize LveBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
  {
    if(minOffsetAlignment > 0) {
      return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
  }

  LveBuffer::LveBuffer(
    LveDevice& device,
    VkDeviceSize instanceSize,
    uint32_t instanceCount,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkDeviceSize minOffsetAlignment) :
      lveDevice{device},
      instanceSize{instanceSize},
      instanceCount{instanceCount},
      usageFlags{usageFlags},
      memoryPropertyFlags{memoryPropertyFlags}
  {
    alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
    bufferSize = alignmentSize * instanceCount;
    device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
  }

  LveBuffer::~LveBuffer()
  {
    unmap();
    vkDestroyBuffer(lveDevice.device(), buffer, nullptr);
    vkFreeMemory(lveDevice.device(), memory, nullptr);
  }

  /**
   * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
   *
   * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
   * buffer range.
   * @param offset (Optional) Byte offset from beginning
   *
   * @return VkResult of the buffer mapping call
   */
  VkResult LveBuffer::map(VkDeviceSize size, VkDeviceSize offset)
  {
    assert(buffer && memory && "Called map on buffer before create");
    return vkMapMemory(lveDevice.device(), memory, offset, size, 0, &mapped);
  }

  /**
   * Unmap a mapped memory range
   *
   * @note Does not return a result as vkUnmapMemory can't fail
   */
  void LveBuffer::unmap()
  {
    if(mapped) {
      vkUnmapMemory(lveDevice.device(), memory);
      mapped = nullptr;
    }
  }

  /**
   * Copies the specified data to the mapped buffer. Default value writes whole buffer range
   *
   * @param data Pointer to the data to copy
   * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
   * range.
   * @param offset (Optional) Byte offset from beginning of mapped region
   *
   */
  void LveBuffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset)
  {
    assert(mapped && "Cannot copy to unmapped buffer");

    if(size == VK_WHOLE_SIZE) {
      memcpy(mapped, data, bufferSize);
    }
    else {
      char* memOffset = (char*)mapped;
      memOffset += offset;
      memcpy(memOffset, data, size);
    }
  }

  /**
   * Flush a memory range of the buffer to make it visible to the device
   *
   * @note Only required for non-coherent memory
   *
   * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
   * complete buffer range.
   * @param offset (Optional) Byte offset from beginning
   *
   * @return VkResult of the flush call
   */
  VkResult LveBuffer::flush(VkDeviceSize size, VkDeviceSize offset)
  {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(lveDevice.device(), 1, &mappedRange);
  }

  /**
   * Invalidate a memory range of the buffer to make it visible to the host
   *
   * @note Only required for non-coherent memory
   *
   * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
   * the complete buffer range.
   * @param offset (Optional) Byte offset from beginning
   *
   * @return VkResult of the invalidate call
   */
  VkResult LveBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
  {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(lveDevice.device(), 1, &mappedRange);
  }

  /**
   * Create a buffer info descriptor
   *
   * @param size (Optional) Size of the memory range of the descriptor
   * @param offset (Optional) Byte offset from beginning
   *
   * @return VkDescriptorBufferInfo of specified offset and range
   */
  VkDescriptorBufferInfo LveBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset)
  {
    return VkDescriptorBufferInfo{
      buffer,
      offset,
      size,
    };
  }

  /**
   * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
   *
   * @param data Pointer to the data to copy
   * @param index Used in offset calculation
   *
   */
  void LveBuffer::writeToIndex(void* data, int index)
  {
    writeToBuffer(data, instanceSize, index * alignmentSize);
  }

  /**
   *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
   *
   * @param index Used in offset calculation
   *
   */
  VkResult LveBuffer::flushIndex(int index)
  {
    return flush(alignmentSize, index * alignmentSize);
  }

  /**
   * Create a buffer info descriptor
   *
   * @param index Specifies the region given by index * alignmentSize
   *
   * @return VkDescriptorBufferInfo for instance at index
   */
  VkDescriptorBufferInfo LveBuffer::descriptorInfoForIndex(int index)
  {
    return descriptorInfo(alignmentSize, index * alignmentSize);
  }

  /**
   * Invalidate a memory range of the buffer to make it visible to the host
   *
   * @note Only required for non-coherent memory
   *
   * @param index Specifies the region to invalidate: index * alignmentSize
   *
   * @return VkResult of the invalidate call
   */
  VkResult LveBuffer::invalidateIndex(int index)
  {
    return invalidate(alignmentSize, index * alignmentSize);
  }
} // namespace lve
#endif
