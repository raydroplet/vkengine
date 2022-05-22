#pragma once

#include "core.hpp"
#include "device.hpp"

namespace vke
{
  class DescriptorSetLayout
  {
    friend class DescriptorWriter;

  public:
    class Builder;
    class Writer;

    DescriptorSetLayout(Device& device, std::vector<VkDescriptorSetLayoutBinding>* bindings);
    ~DescriptorSetLayout();

    DescriptorSetLayout(DescriptorSetLayout const&) = delete;
    DescriptorSetLayout& operator=(DescriptorSetLayout const&) = delete;

    operator VkDescriptorSetLayout()
    {
      return m_descriptorSetLayout;
    }

  private:
    Device& m_device;
    VkDescriptorSetLayout m_descriptorSetLayout{};
    std::vector<VkDescriptorSetLayoutBinding> m_bindings{};
  };

  class DescriptorPool
  {
    friend class DescriptorWriter;

  public:
    class Builder;

    DescriptorPool(
      Device& device,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      std::vector<VkDescriptorPoolSize> const& poolSizes);
    ~DescriptorPool();
    DescriptorPool(DescriptorPool const&) = delete;
    DescriptorPool& operator=(DescriptorPool const&) = delete;

    void allocate(std::vector<VkDescriptorSetLayout> const& descriptorSetLayouts, std::vector<VkDescriptorSet>* descriptorSets);
    void allocate(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet* descriptorSet);

    // You don't need to explicitly clean up descriptor sets, because they will be automatically freed when the descriptor pool is destroyed
    void free(std::vector<VkDescriptorSet>* descriptorSets);
    void free(VkDescriptorSet* descriptorSet);

    void reset();

  private:
    void allocate(VkDescriptorSetLayout const* descriptorSetLayouts, uint32_t count, VkDescriptorSet* descriptors);
    void free(uint32_t count, VkDescriptorSet const* descriptorSets);

    Device& m_device;
    VkDescriptorPool m_descriptorPool{};
  };


  class DescriptorPool::Builder
  {
  public:
    Builder(Device& device);

    Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder& setMaxDescriptorSets(uint32_t count);

    auto build() const -> std::unique_ptr<DescriptorPool>;

  private:
    Device& m_device;
    std::vector<VkDescriptorPoolSize> m_poolSizes{};
    uint32_t m_maxDescriptorSets{2};
    VkDescriptorPoolCreateFlags m_poolFlags{};
  };

  class DescriptorSetLayout::Builder
  {
  public:
    Builder(Device& device);

    Builder& addBinding(
      uint32_t binding,
      VkDescriptorType descriptorType,
      VkShaderStageFlags stageFlags,
      uint32_t count = 1);

    void clear()
    {
      m_bindings.clear();
    }
    auto build() -> std::unique_ptr<DescriptorSetLayout>;

  private:
    Device& m_device;
    std::vector<VkDescriptorSetLayoutBinding> m_bindings{};
  };

  class DescriptorWriter
  {
  public:
    DescriptorWriter(DescriptorSetLayout& descriptorSetLayout, DescriptorPool& descriptorPool);

    // TODO: writeImages() and writeBuffers() (rename those below to that too)
    DescriptorWriter& addBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
    DescriptorWriter& addImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);
    // TODO: DescriptorWriter& writeTexelBufferView();

    void allocAndUpdate(VkDescriptorSet* descriptorSet);
    void update(VkDescriptorSet& descriptorSet);

  private:
    void add(uint32_t binding, uint32_t arrayElement, VkDescriptorImageInfo* pImageInfo, VkDescriptorBufferInfo* pBufferInfo, VkBufferView* pTexelBufferView);

    DescriptorSetLayout& m_descriptorSetLayout;
    DescriptorPool& m_descriptorPool;
    std::vector<VkWriteDescriptorSet> m_writes;
  };
} // namespace vke
