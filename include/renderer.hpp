#pragma once

#include "core.hpp"
//
#include "allocator.hpp"
#include "device.hpp"
#include "systems/renderSystem.hpp"
#include "swapchain.hpp"
#include "window.hpp"

namespace vke
{
  class Renderer
  {
  public:
    Renderer(Device& device, Window& window, EventRelayer& relayer);
    ~Renderer();

    bool beginFrame();
    void endFrame();
    void beginRenderPass();
    void endRenderPass();
    void present();

    bool isFrameInProgress() const
    {
      return m_hasFrameStarted;
    }
    auto currentCommandBuffer() const -> VkCommandBuffer;

    auto renderPass() const -> VkRenderPass
    {
      return m_swapchain->renderPass();
    }
    auto swapchainAspectRatio() const -> float
    {
      return m_swapchain->aspectRatio();
    }
    auto swapchainExtent() -> VkExtent2D const
    {
      return m_swapchain->extent();
    }
    auto swapchainImageCount() const -> uint32_t
    {
      return m_swapchain->imageCount();
    }
    auto maxFramesInFlight() const -> uint32_t
    {
      return m_maxFramesInFlight;
    }
    auto frameIndex() const -> uint32_t
    {
      return m_currentFrameIndex;
    }

  private:
    void allocateCommandBuffers();
    void freeCommandBuffers();
    // void recordCommandBuffers(uint32_t imageIndex);

    void createSyncObjects();
    void recreateSwapchain();
    // void drawFrame(uint32_t* pImageIndex);

    // void renderEntities(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void cleanup();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer) = delete;

  private:
    Device& m_device;
    Window& m_window;

    EventRelayer& m_eventRelayer;

    std::unique_ptr<Swapchain> m_swapchain;
    std::vector<VkCommandBuffer> m_commandBuffers;

    // TODO: you could move the submitCommandBuffers and present functionality into the swapchain class
    std::vector<VkSemaphore> m_imageAvailableSemaphore; //  signal that an image has been acquired and is ready for rendering
    std::vector<VkSemaphore> m_renderFinishedSemaphore; //  signal that rendering has finished and presentation can happen
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;

    uint32_t m_maxFramesInFlight{};
    uint32_t m_currentFrameIndex{};
    uint32_t m_currentImageIndex{};
    bool m_hasFrameStarted{};

    std::function<void()> incompatibleRenderPassCallback;
    std::function<void(void* const, VkRenderPass, VkExtent2D)> recreatePipelineCallback;
  };
} // namespace vke
