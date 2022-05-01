#include "descriptor.hpp"

#include <algorithm>
#include <cassert>

namespace vke
{
  //// DescriptorSetLayout ////

  DescriptorSetLayout::Builder::Builder(Device& device) :
      m_device{device}
  {
    // m_bindings.reserve(8);
  }

  DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count)
  {
    m_bindings.push_back(VkDescriptorSetLayoutBinding{
      .binding = binding,
      .descriptorType = descriptorType,
      .descriptorCount = count,
      .stageFlags = stageFlags,
      .pImmutableSamplers = {},
    });

    return *this;
  }

  auto DescriptorSetLayout::Builder::build() -> std::unique_ptr<DescriptorSetLayout>
  {
    return std::make_unique<DescriptorSetLayout>(m_device, &m_bindings);
  }

  DescriptorSetLayout::DescriptorSetLayout(Device& device, std::vector<VkDescriptorSetLayoutBinding>* bindings) :
      m_device{device},
      m_bindings{std::move(*bindings)}
  {
    std::sort(m_bindings.begin(), m_bindings.end(),
      [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b) {
        return a.binding < b.binding;
      });

    assert(
      "Duplicate bindings" &&
      std::adjacent_find(m_bindings.begin(), m_bindings.end(),
        [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b) {
          return a.binding == b.binding;
        }) == m_bindings.end());

    VkDescriptorSetLayoutCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = {},
      .flags = {},
      .bindingCount = static_cast<uint32_t>(m_bindings.size()),
      .pBindings = m_bindings.data(),
    };

    if(vkCreateDescriptorSetLayout(m_device, &createInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create descriptor set layout");
    }
  }

  DescriptorSetLayout::~DescriptorSetLayout()
  {
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
  }

//// DescriptorWriter ////

  DescriptorWriter::DescriptorWriter(DescriptorSetLayout& descriptorSetLayout, DescriptorPool& descriptorPool) :
      m_descriptorSetLayout{descriptorSetLayout},
      m_descriptorPool{descriptorPool}
  {
  }

  DescriptorWriter& DescriptorWriter::addBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
  {
    assert("Binding single buffer descriptor info, but binding expects multiple" && m_descriptorSetLayout.m_bindings[binding].descriptorCount == 1);

    add(binding, 0, nullptr, bufferInfo, nullptr);
    return *this;
  }

  DescriptorWriter& DescriptorWriter::addImage(uint32_t binding, VkDescriptorImageInfo* imageInfo)
  {
    assert("Binding single image descriptor info, but binding expects multiple" && m_descriptorSetLayout.m_bindings[binding].descriptorCount == 1);

    add(binding, 0, imageInfo, nullptr, nullptr);
    return *this;
  }

  void DescriptorWriter::add(uint32_t binding, uint32_t arrayElement, VkDescriptorImageInfo* pImageInfo, VkDescriptorBufferInfo* pBufferInfo, VkBufferView* pTexelBufferView)
  {
    assert(
      "DescriptorSetLayout does not contain the specified binding" &&
      std::binary_search(m_descriptorSetLayout.m_bindings.begin(), m_descriptorSetLayout.m_bindings.end(),
        VkDescriptorSetLayoutBinding{.binding = binding},
        [](VkDescriptorSetLayoutBinding const& a, VkDescriptorSetLayoutBinding const& b) {
          return a.binding < b.binding;
        }));

    auto& bindingDescription{m_descriptorSetLayout.m_bindings[binding]};
    m_writes.push_back(
      VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = {},
        .dstSet = nullptr, // specified in this->overwrite()
        .dstBinding = bindingDescription.binding,
        .dstArrayElement = arrayElement,
        .descriptorCount = bindingDescription.descriptorCount,
        .descriptorType = bindingDescription.descriptorType,
        .pImageInfo = pImageInfo,
        .pBufferInfo = pBufferInfo,
        .pTexelBufferView = pTexelBufferView,
      });
  }

  void DescriptorWriter::write(VkDescriptorSet* descriptorSet)
  {
    m_descriptorPool.allocate(m_descriptorSetLayout, descriptorSet);
    update(*descriptorSet);
  }

  void DescriptorWriter::update(VkDescriptorSet& descriptorSet)
  {
    for(auto& e : m_writes)
      e.dstSet = descriptorSet;

    vkUpdateDescriptorSets(m_descriptorSetLayout.m_device, m_writes.size(), m_writes.data(), 0, nullptr);
  }

  //// DescriptorPool ////

  DescriptorPool::Builder::Builder(Device& device) :
      m_device{device}
  {
  }

  DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
  {
    m_poolSizes.push_back({descriptorType, count});
    return *this;
  }

  DescriptorPool::Builder& DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags)
  {
    m_poolFlags = flags;
    return *this;
  }

  DescriptorPool::Builder& DescriptorPool::Builder::setMaxDescriptorSets(uint32_t count)
  {
    m_maxDescriptorSets = count;
    return *this;
  }

  auto DescriptorPool::Builder::build() const -> std::unique_ptr<DescriptorPool>
  {
    return std::make_unique<DescriptorPool>(m_device, m_maxDescriptorSets, m_poolFlags, m_poolSizes);
  }

  DescriptorPool::DescriptorPool(Device& device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, std::vector<VkDescriptorPoolSize> const& poolSizes) :
      m_device{device}
  {
    VkDescriptorPoolCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext = {},
      .flags = poolFlags,
      .maxSets = maxSets,
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.data(),
    };

    if(vkCreateDescriptorPool(m_device, &createInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create descriptor pool");
    }
  };

  DescriptorPool::~DescriptorPool()
  {
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
  }

  void DescriptorPool::allocate(std::vector<VkDescriptorSetLayout> const& descriptorSetLayouts, std::vector<VkDescriptorSet>* descriptorSets)
  {
    allocate(
      descriptorSetLayouts.data(),
      static_cast<uint32_t>(descriptorSetLayouts.size()),
      descriptorSets->data());
  }

  void DescriptorPool::allocate(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet* descriptorSet)
  {
    allocate(&descriptorSetLayout, 1, descriptorSet);
  }

  void DescriptorPool::allocate(VkDescriptorSetLayout const* descriptorSetLayouts, uint32_t count, VkDescriptorSet* descriptors)
  {
    VkDescriptorSetAllocateInfo allocateInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = {},
      .descriptorPool = m_descriptorPool,
      .descriptorSetCount = count,
      .pSetLayouts = descriptorSetLayouts,
    };

    if(vkAllocateDescriptorSets(m_device, &allocateInfo, descriptors) != VK_SUCCESS)
    {
      // TODO:
      //  if the pool fills up it will crash.
      //  A common solution is to create a DescriptorPoolManager: (see vkguide.dev)
      //   . it keeps tracks of all pools objects
      //   . and create a new pool whenever all pools are full
      throw std::runtime_error("Failed to allocate descriptor sets");
    }
  }

  void DescriptorPool::free(std::vector<VkDescriptorSet>* descriptorSets)
  {
    free(descriptorSets->size(), descriptorSets->data());
  }

  void DescriptorPool::free(VkDescriptorSet* descriptorSet)
  {
    free(1, descriptorSet);
  }

  void DescriptorPool::free(uint32_t count, VkDescriptorSet const* descriptorSets)
  {
    vkFreeDescriptorSets(m_device, m_descriptorPool, count, descriptorSets);
  }

  void DescriptorPool::reset()
  {
    VkDescriptorPoolResetFlags flags{};
    vkResetDescriptorPool(m_device, m_descriptorPool, flags);
  }
} // namespace vke
