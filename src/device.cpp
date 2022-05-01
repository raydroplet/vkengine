#include "device.hpp"

namespace vke
{
  Device::Device(Window& window) :
      window(window)
  {
    createInstance();
    setupDebugMessenger();
    window.create(m_instance); //  also creates a surface
    choosePhysicalDevice();
    createLogicalDevice();
    createCommandPools();
  }

  Device::~Device()
  {
    window.destroySurface(m_instance);
    vkDestroyCommandPool(m_device, m_commandPools.graphics, nullptr);
    vkDestroyCommandPool(m_device, m_commandPools.transfer, nullptr);
    vkDestroyDevice(m_device, nullptr);
    destroyDebugUtilsMessengerEXT(m_instance, debugMessenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);
  }

  void Device::createInstance()
  {
    if(enableValidationLayers && !checkValidationLayersSupport())
      throw std::runtime_error("Validation layers requested but not available");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0); //  VK_API_VERSION_1_0

    std::vector extensions{getRequiredExtensions()};

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    if(enableValidationLayers)
    {
      createInfo.enabledLayerCount = m_validationLayers.size();
      createInfo.ppEnabledLayerNames = m_validationLayers.data();
      populateDebugMessengerCreateInfo(debugMessengerCreateInfo);
      createInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugMessengerCreateInfo);
    }
    else
    {
      createInfo.enabledLayerCount = 0;
      createInfo.ppEnabledLayerNames = nullptr;
    }
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    if(vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
      throw std::runtime_error("Failed to create instance");
  }

  bool Device::checkValidationLayersSupport()
  {
    std::vector<VkLayerProperties> availableLayers;

    uint32_t layerCount{};
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    availableLayers.resize(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const auto& layer : m_validationLayers)
    {
      if(std::find_if(availableLayers.begin(), availableLayers.end(), [&layer](const VkLayerProperties& availableLayer) {
           return std::strcmp(layer, availableLayer.layerName) == 0;
         }) == availableLayers.end())
      {
        return false;
      }
    }

    return true;
  }

  bool Device::checkDeviceExtensionsSupport()
  {
    const auto& availableExtensions = m_physicalDeviceInfo.availableExtensions;
    for(const auto& deviceExtension : m_deviceExtensions)
    {
      if(std::find_if(availableExtensions.begin(), availableExtensions.end(), [&deviceExtension](const auto& availableExtension) {
           return std::strcmp(deviceExtension, availableExtension.extensionName) == 0;
         }) == availableExtensions.end())
      {
        return false;
      }
    }

    return true;
  }

  void Device::setupDebugMessenger()
  {
    if(!enableValidationLayers)
      return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if(createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
      throw std::runtime_error("Failed to setup debug messenger");
  }

  void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
  {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = &debugCallback;
  }

  std::vector<const char*> Device::getRequiredExtensions()
  {
    uint32_t glfwExtensionCount{};
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if(enableValidationLayers)
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
  }

  void Device::choosePhysicalDevice()
  {
    uint32_t deviceCount{};
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if(!deviceCount)
      throw std::runtime_error("Failed to find a GPU with Vulkan support");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    for(const auto& device : devices)
    {
      m_physicalDevice = device;

      queryPhysicalDeviceInfo(device);
      findQueueIndices();
      if(isDeviceSuitable())
      {
        // logMemoryInfo();
        return;
      }
    }

    if(m_physicalDevice == VK_NULL_HANDLE)
      throw std::runtime_error("Failed to find a suitable GPU");
  }

  void Device::logMemoryInfo()
  {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    std::cout << "\n[Memory Types]  [" << memProperties.memoryTypeCount << "]\n\n";
    for(uint32_t i{}; i < memProperties.memoryTypeCount; ++i)
    {
      const auto memoryType = memProperties.memoryTypes;
      const auto memoryHeap = memProperties.memoryHeaps;

      std::cout << 0 + (memoryHeap[memoryType[i].heapIndex].size) / 1024 / 1024 << " MB" << '\t';

      if(memoryHeap[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        std::cout << "(VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)";

      if(memoryHeap[i].flags == 0)
        std::cout << "(HEAP_0)";

      std::cout << std::endl;

      if(memoryType[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        std::cout << "> VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" << std::endl;

      if(memoryType[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        std::cout << "> VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT" << std::endl;

      if(memoryType[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        std::cout << "> VK_MEMORY_PROPERTY_HOST_COHERENT_BIT" << std::endl;

      if(memoryType[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
        std::cout << "> VK_MEMORY_PROPERTY_HOST_CACHED_BIT" << std::endl;

      if(memoryType[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
        std::cout << "> VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT" << std::endl;

      std::cout << std::endl;
    }

    std::cout << "\n[Memory Heaps]   [" << memProperties.memoryHeapCount << "]\n";
    for(uint32_t i{}; i < memProperties.memoryHeapCount; ++i)
    {
      const auto memoryHeap = memProperties.memoryHeaps;

      std::cout << '\n';
      std::cout << (memoryHeap[i].size) / 1024 / 1024 << " MB" << std::endl;

      if(memoryHeap[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        std::cout << "> VK_MEMORY_HEAP_DEVICE_LOCAL_BIT" << std::endl;

      if(memoryHeap[i].flags == 0)
        std::cout << "> HEAP_0" << std::endl;
    }

    std::cout << "nonCoherentAtomSize -> " << physicalInfo().deviceProperties.limits.nonCoherentAtomSize << '\n';
    std::cout << "maxAllocations -> " << physicalInfo().deviceProperties.limits.maxMemoryAllocationCount;

    std::cout.flush();
  }

  bool Device::isDeviceSuitable()
  {
    return m_queues.isComplete() && checkDeviceExtensionsSupport();
  }

  void Device::queryPhysicalDeviceInfo(VkPhysicalDevice physicalDevice)
  {
    auto& info = m_physicalDeviceInfo;
    uint32_t count{};

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
    info.queueFamilyProperties.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, info.queueFamilyProperties.data());

    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    info.availableExtensions.resize(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, info.availableExtensions.data());

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &info.memoryProperties);
    vkGetPhysicalDeviceProperties(physicalDevice, &info.deviceProperties);
  }

  void Device::createLogicalDevice()
  {
    std::set<QueueFamily> uniqueQueueFamilies{m_queues.graphicsFamily, m_queues.transferFamily}; //  could be better
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(uniqueQueueFamilies.size());

    float priorities[]{1.0}; // TODO: consider the support for multiple queues (and handle priorities better)
    for(auto& queueFamily : uniqueQueueFamilies)
    {
      VkDeviceQueueCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      createInfo.flags = {};
      createInfo.queueFamilyIndex = queueFamily;
      createInfo.queueCount = std::size(priorities);
      createInfo.pQueuePriorities = priorities;
      queueCreateInfos.push_back(createInfo);
    }

    VkPhysicalDeviceFeatures features{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    /*deprecated*/ createInfo.enabledLayerCount = m_validationLayers.size();
    /*deprecated*/ createInfo.ppEnabledLayerNames = m_validationLayers.data();
    createInfo.enabledExtensionCount = m_deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();
    createInfo.pEnabledFeatures = &features;

    if(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
      throw std::runtime_error("Failed to create logical device");

    vkGetDeviceQueue(m_device, m_queues.graphicsFamily, 0, &m_queues.graphics);
    vkGetDeviceQueue(m_device, m_queues.presentFamily, 0, &m_queues.present);
    vkGetDeviceQueue(m_device, m_queues.transferFamily, 0, &m_queues.transfer);
  }

  void Device::findQueueIndices()
  {
    const auto& info = m_physicalDeviceInfo;

    for(size_t i{0}; i < info.queueFamilyProperties.size(); ++i)
    {
      if(info.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        m_queues.graphicsFamily = i;

      if(info.queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT && info.queueFamilyProperties[i].queueFlags != VK_QUEUE_GRAPHICS_BIT)
        m_queues.transferFamily = i;

      VkBool32 presentSupport{};
      vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, window.surface(), &presentSupport);

      if(presentSupport)
        m_queues.presentFamily = i;

      if(m_queues.isComplete())
        break;
    }

    if(!m_queues.transferFamily)
      m_queues.transfer = m_queues.graphics;
  }

  void Device::createCommandPools()
  {
    VkCommandPoolCreateInfo createInfo[]{
      {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_queues.graphicsFamily,
      },
      {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = m_queues.transferFamily,
      },
    };

    VkCommandPool* commandPools[]{&m_commandPools.graphics, &m_commandPools.transfer};

    for(size_t i = 0; i < std::size(commandPools); ++i)
    {
      if(vkCreateCommandPool(m_device, &createInfo[i], nullptr, commandPools[i]) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool");
    }
  }

  VkFormat Device::findSupportedFormat(std::span<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
  {
    for(VkFormat format : candidates)
    {
      VkFormatProperties properties;
      vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &properties);

      if(((properties.optimalTilingFeatures & tiling) == tiling) && ((properties.optimalTilingFeatures & features) == features))
      {
        return format;
      }
      else if(((properties.linearTilingFeatures & tiling) == tiling) && ((properties.linearTilingFeatures & features) == features))
      {
        return format;
      }
    }

    throw std::runtime_error("Failed to find supported format.");
  }

VKAPI_ATTR VkBool32 VKAPI_CALL Device::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
  static std::unordered_map<int, const char*> severity{
    {VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT, "Verbose"},
    {VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, "Info"},
    {VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT, "Warning"},
    {VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "Error"}};

  if(messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
  {
    std::cerr << clr::red << "[" << severity[messageSeverity] << "] " << clr::white
              << pCallbackData->pMessage << std::endl;
  }

  return VK_FALSE;
}

VkResult Device::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
  auto function{reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"))};
  return function ? function(instance, pCreateInfo, pAllocator, pDebugMessenger) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Device::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
  auto function{reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"))};
  if(function)
    function(instance, debugMessenger, pAllocator);
}

////////////////////////////////////////////////////////////

void QueueFamily::operator=(uint32_t i)
{
  m_index = i;
  m_hasValue = true;
}

bool Queues::isComplete()
{
  return graphicsFamily.hasValue() && transferFamily.hasValue() && presentFamily.hasValue();
}

////////////////////////////////////////////////////
}
