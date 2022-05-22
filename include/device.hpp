#pragma once

#include "core.hpp"

#include "window.hpp"
#include "LogUtils.h"

namespace vke
{
class PhysicalDeviceInfo
{
public:
  std::vector<VkQueueFamilyProperties> queueFamilyProperties;
  std::vector<VkLayerProperties> availableLayers;
  std::vector<VkExtensionProperties> availableExtensions;
  VkPhysicalDeviceMemoryProperties memoryProperties;
  VkPhysicalDeviceProperties deviceProperties;
  std::vector<VkFormatProperties> formatProperties;

  PhysicalDeviceInfo& operator=(PhysicalDeviceInfo&&) = default;
  //PhysicalDeviceInfo& operator=(PhysicalDeviceInfo& device) = delete;
};

/*
class QueueFamily
{
  friend class Device;
public:
  //operator VkQueue() const; //  handy

  bool operator!=(const QueueFamily& other) const;
  bool operator<(const QueueFamily& other) const { return m_index != other.m_index; }
  void operator=(VkQueue queue);
  void operator=(uint32_t i);

  //VkQueue* operator&();
  bool hasValue() const { return m_hasValue; }
  VkQueue queue() const { return m_queue; }
  VkQueue* queue() { return &m_queue; } // :\ //
  uint32_t index() const { return m_index; }

  float priority{1.0f};

private:
  uint32_t m_index;
  VkQueue m_queue;
  bool m_hasValue{};
};

class Queues
{
public:
  QueueFamily graphics;
  QueueFamily present;
  QueueFamily transfer;

  bool isComplete();
  std::set<uint32_t> uniqueIndices();

  Queues& operator=(Queues&&) = default;
};
*/

class QueueFamily
{
public:
  //auto index() -> uint32_t { return m_index; }
  operator uint32_t() const { return m_index; }
  bool hasValue() const { return m_hasValue; }

  bool operator!=(const QueueFamily& other) const { return m_index != other.m_index; }
  bool operator<(const QueueFamily& other) const { return operator!=(other); }
  void operator=(uint32_t i)
  {
    m_index = i;
    m_hasValue = true;
  }

  operator bool() const { return m_hasValue; }  //handy

private:
  uint32_t m_index;
  bool m_hasValue;
};

struct Queue
{
public:
  operator VkQueue() const { return m_queue; }
  void operator=(VkQueue queue) { m_queue = queue; }
  VkQueue* operator&() { return &m_queue; }

  //float priority{1.0};

private:
  VkQueue m_queue;
};

struct Queues
{
  QueueFamily graphicsFamily;
  QueueFamily transferFamily;
  QueueFamily presentFamily;

  Queue graphics;
  Queue transfer;
  Queue present;

  bool isComplete() {
    return graphicsFamily.hasValue() &&
           transferFamily.hasValue() &&
           presentFamily.hasValue();
  }
};

class CommmandPools
{
public:
  VkCommandPool graphics;
  VkCommandPool transfer;
};

class Device
{
public:
  Device(Window& window);
  ~Device();

  void logMemoryInfo();

  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;

  auto findSupportedFormat(std::span<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features) -> VkFormat;

private:
  void createInstance();
  void setupDebugMessenger();
  void choosePhysicalDevice();
  void createLogicalDevice();
  void createCommandPools();

  bool checkValidationLayersSupport();
  bool checkDeviceExtensionsSupport();
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
  auto getRequiredExtensions() -> std::vector<const char*>;
  bool isDeviceSuitable();
  void queryPhysicalDeviceInfo(VkPhysicalDevice physicalDevice);
  void findQueueIndices();

  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
  VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
  void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

public:
  auto physical() const -> const VkPhysicalDevice& { return m_physicalDevice; }
  auto physicalInfo() const -> const PhysicalDeviceInfo& { return m_physicalDeviceInfo; }
  auto queues() const -> const Queues& { return m_queues;}
  auto commandPools() const -> const CommmandPools& { return m_commandPools; };

  operator VkDevice() { return m_device; }

private:
  VkInstance m_instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkDevice m_device;
  Window& window; //  maybe not the best place
  Queues m_queues;
  PhysicalDeviceInfo m_physicalDeviceInfo;
  VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
  CommmandPools m_commandPools;

  bool enableValidationLayers{true};

  static constexpr std::array m_deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  static constexpr std::array m_validationLayers{"VK_LAYER_KHRONOS_validation"};
};
}
