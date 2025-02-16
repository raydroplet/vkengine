#include "swapchain.hpp"

namespace vke
{
  Swapchain::Swapchain(Device& device, Window& window, std::unique_ptr<Swapchain> pOldSwapchain) :
    m_device{device}, m_window{window}
  {
    // VkSwapchainKHR oldSwapchain = (pOldSwapchain ? *pOldSwapchain : VK_NULL_HANDLE);
    VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;
    if(pOldSwapchain) {
      VkSwapchainKHR oldSwapchain = *pOldSwapchain;
    }

    if(!oldSwapchain) {
      querySupportDetails();
      chooseSurfaceFormat();
      choosePresentMode();
      chooseExtent();
      findDepthFormat(&m_info.depthFormat);
    } else {
      m_details = std::move(pOldSwapchain->m_details);
      m_info = std::move(pOldSwapchain->m_info);

      // update details and info
      querySupportDetails(&m_details.capabilities);
      chooseExtent();
    }

    createSwapchain(oldSwapchain);
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
  }

  Swapchain::~Swapchain()
  {
    for(const auto& framebuffer : m_framebuffers)
      vkDestroyFramebuffer(m_device, framebuffer, nullptr);

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);

    vkDestroyImageView(m_device, m_depthImageView, nullptr);
    vkDestroyImage(m_device, m_depthImage, nullptr);
    vkFreeMemory(m_device, m_depthImageMemory, nullptr);

    for(const auto& imageView : imageViews)
      vkDestroyImageView(m_device, imageView, nullptr);
  }

  void Swapchain::createSwapchain(VkSwapchainKHR oldSwapchain)
  {
    const auto& capabilities = m_details.capabilities;
    uint32_t imageCount{capabilities.minImageCount + 1};

    if((capabilities.maxImageCount > 0) && (imageCount > capabilities.maxImageCount))
      imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_window.surface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = m_info.surfaceFormat.format;
    createInfo.imageColorSpace = m_info.surfaceFormat.colorSpace;
    createInfo.imageExtent = m_info.extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t indices[]{m_device.queues().graphicsFamily, m_device.queues().presentFamily};
    if(m_device.queues().graphicsFamily != m_device.queues().presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = indices;
    } else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = {}; //  0;
      createInfo.pQueueFamilyIndices = {};   // nullptr;
    }
    createInfo.preTransform = m_details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = m_info.presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;

    if(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
      throw std::runtime_error("Failed to create swapchain");

    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, images.data());

    m_info.imageCount = images.size();
  }

  void Swapchain::querySupportDetails(VkSurfaceCapabilitiesKHR* capabilities)
  {
    if(capabilities) {
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device.physical(), m_window.surface(), capabilities);
    } else {
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device.physical(), m_window.surface(), &m_details.capabilities);

      uint32_t count{};

      vkGetPhysicalDeviceSurfaceFormatsKHR(m_device.physical(), m_window.surface(), &count, nullptr);
      m_details.formats.resize(count);
      vkGetPhysicalDeviceSurfaceFormatsKHR(m_device.physical(), m_window.surface(), &count, m_details.formats.data());

      vkGetPhysicalDeviceSurfacePresentModesKHR(m_device.physical(), m_window.surface(), &count, nullptr);
      m_details.presentModes.resize(count);
      vkGetPhysicalDeviceSurfacePresentModesKHR(m_device.physical(), m_window.surface(), &count, m_details.presentModes.data());
    }
  }

  void Swapchain::choosePresentMode()
  {
    /*
       for(const auto& presentMode : m_details.presentModes)
       {
       if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
       {
       m_presentMode = presentMode;
       return;
       }
       }
       */
    m_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  }

  void Swapchain::chooseSurfaceFormat()
  {
    for(const auto& surfaceFormat : m_details.formats) {
      if(surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        m_info.surfaceFormat = surfaceFormat;
    }

    m_info.surfaceFormat = m_details.formats[0];
  }

  void Swapchain::chooseExtent()
  {
    const auto& capabilities = m_details.capabilities;
    if(capabilities.currentExtent.width /*!=*/ == UINT32_MAX) {
      m_info.extent = capabilities.currentExtent;
    } else {
      int width, height;
      glfwGetFramebufferSize(m_window, &width, &height);

      m_info.extent = {
        std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),    // (value, min, max)
        std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height), // (value, min, max)
      };
    }
  }

  void Swapchain::createImageViews()
  {
    imageViews.resize(images.size());
    for(size_t i = 0; i < images.size(); ++i) {
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = images[i];
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = m_info.surfaceFormat.format;
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      if(vkCreateImageView(m_device, &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image views");
      }
    }
  }

  void Swapchain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView)
  {
    VkImageViewCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = format,
    };

    createInfo.components = {
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY,
    };

    createInfo.subresourceRange = {
      .aspectMask = aspectFlags,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
    };

    if(vkCreateImageView(m_device, &createInfo, nullptr, imageView) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create image view");
    }
  }

  void Swapchain::createRenderPass()
  {
    VkAttachmentDescription colorAttachment{
      .format = m_info.surfaceFormat.format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE, // VK_ATTACHMENT_STORE_OP_DONT_CARE
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentDescription depthAttachment{
      .format = m_info.depthFormat,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference colorAttachmentRef{
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depthAttachmentRef{
      .attachment = 1,
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };


    VkSubpassDescription subpass{
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      // subpass.inputAttachmentCount = ;
      // subpass.pInputAttachments = ;  //read from a shader
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentRef,
      // subpass.pResolveAttachments = ;  //  used for multisampling color attachments
      .pDepthStencilAttachment = &depthAttachmentRef, //  for depth and stencil data
      // subpass.preserveAttachmentCount = ;
      // subpass.pPreserveAttachments = //  not used by this subpass, but for which the data must be preserved
    };

    VkSubpassDependency dependency{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    VkAttachmentDescription attachments[]{colorAttachment, depthAttachment};
    VkRenderPassCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = std::size(attachments),
      .pAttachments = attachments,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency,
    };

    if(vkCreateRenderPass(m_device, &createInfo, nullptr, &m_renderPass) != VK_SUCCESS)
      throw std::runtime_error("Failed to create render pass");
  }

  void Swapchain::createFramebuffers()
  {
    m_framebuffers.resize(imageViews.size());

    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = m_renderPass;
    createInfo.attachmentCount = {};
    createInfo.pAttachments = {};
    createInfo.width = m_info.extent.width;
    createInfo.height = m_info.extent.height;
    createInfo.layers = 1;

    for(size_t i = 0; i < imageViews.size(); ++i) {
      VkImageView attachments[]{
        imageViews[i],
        m_depthImageView,
      };

      createInfo.attachmentCount = std::size(attachments);
      createInfo.pAttachments = attachments;

      if(vkCreateFramebuffer(m_device, &createInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
        throw std::runtime_error("Failed to framebuffers");
    }
  }

  VkResult Swapchain::acquireNextImage(VkSemaphore imageAvailableSemaphore, uint32_t* imageIndex)
  {
    return vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, imageIndex);
  }

  void Swapchain::findDepthFormat(VkFormat* format)
  {
    std::vector formats{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    *format = m_device.findSupportedFormat(formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }

  // TODO: move this to the device class
  bool Swapchain::hasStencilComponent(VkFormat format)
  {
    return format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D24_UNORM_S8_UINT;
  }

  void Swapchain::createDepthResources()
  {
    VkImageCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .flags = {}, // optional
      .imageType = VK_IMAGE_TYPE_2D,
      .format = m_info.depthFormat,
      .extent = {m_info.extent.width, m_info.extent.height, 1},
      .mipLevels = 1, // maybe?
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    MemAllocator::createImage(m_device, createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_depthImageMemory, &m_depthImage);

    createImageView(m_depthImage, m_info.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &m_depthImageView);
  }

  bool operator==(const VkSurfaceFormatKHR first, const VkSurfaceFormatKHR second)
  {
    return (first.format == second.format) && (first.colorSpace == second.colorSpace);
  }

  //  (Used for render pass compatibility checking)
  bool Swapchain::compareSwapchainFormats(const Info& other) const
  {
    //  Check which internal dependencies the createRenderPass() has.
    //  Currently (maybe not when reading this again), the only dependency is the surface format (for the color attachment),
    //  which doesn't change because the available surface formats doesn't change because the selected device doesn't change.

    return (m_info.depthFormat == other.depthFormat) &&
           (m_info.surfaceFormat == other.surfaceFormat);

    // depth will be implemented latter
  }

  float Swapchain::aspectRatio() const
  {
    const uint32_t& w = m_info.extent.width;
    const uint32_t& h = m_info.extent.height;
    // return static_cast<float>(std::max(w, h)) / static_cast<float>(std::min(w, h));
    return static_cast<float>(w) / static_cast<float>(h);
  }
} // namespace vke
