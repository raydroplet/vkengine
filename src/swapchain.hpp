#pragma once

#include "core.hpp"

#include "device.hpp"
#include "allocator.hpp"

namespace vke
{
  struct supportDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    supportDetails& operator=(supportDetails&&) = default;
  };

  class Swapchain
  {
  public:
    struct Info
    {
      VkSurfaceFormatKHR surfaceFormat; //VkFormat?
      VkFormat depthFormat{VK_FORMAT_MAX_ENUM};   //VkFormat?
      VkPresentModeKHR presentMode;
      uint32_t imageCount;
      VkExtent2D extent;
    };

    Swapchain(Device& device, Window& window, std::unique_ptr<Swapchain> pOldSwapchain = nullptr);
    ~Swapchain();

    operator VkSwapchainKHR() { return m_swapchain; }

    auto info() const -> const Info& { return m_info; }
    auto extent() const -> VkExtent2D { return m_info.extent; }
    auto aspectRatio() const -> float;
    auto imageCount() const -> uint32_t { return m_info.imageCount; }  //images.size()
    auto renderPass() const -> VkRenderPass { return m_renderPass; };
    auto framebuffers() -> std::span<VkFramebuffer> { return m_framebuffers; }
    auto acquireNextImage(VkSemaphore imageAvailableSemaphore, uint32_t* imageIndex) -> VkResult;

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    friend bool operator==(const VkSurfaceFormatKHR& first, const VkSurfaceFormatKHR& second);
    bool compareSwapchainFormats(const Info& other) const;

    static bool hasStencilComponent(VkFormat format);

  private:
    void createSwapchain(VkSwapchainKHR oldSwapchain);
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();

    void querySupportDetails(VkSurfaceCapabilitiesKHR* capabilities = nullptr);
    void chooseSurfaceFormat();
    void choosePresentMode();
    void chooseExtent();

    void findDepthFormat(VkFormat* format);
    void createDepthResources();
    void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView);

  private:
    Device& m_device;
    Window& m_window;

    VkSwapchainKHR m_swapchain;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    supportDetails m_details;
    Info m_info;

    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_framebuffers;

    //for 3d
    VkImage m_depthImage;
    VkImageView m_depthImageView;
    VkDeviceMemory m_depthImageMemory;
  };
}
