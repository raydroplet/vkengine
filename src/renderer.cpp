#include "renderer.hpp"

namespace vke
{
  Renderer::Renderer(Device& device, Window& window, EventRelayer& relayer) :
      m_device{device},
      m_window{window},
      m_eventRelayer{relayer},
      m_swapchain{std::make_unique<Swapchain>(m_device, m_window)},
      m_maxFramesInFlight{m_swapchain->imageCount()}
  {
    createSyncObjects();
    allocateCommandBuffers();
    // recordCommandBuffers();
  }

  Renderer::~Renderer()
  {
    freeCommandBuffers();

    for(uint32_t i{0}; i < m_maxFramesInFlight; ++i)
    {
      vkDestroySemaphore(m_device, m_renderFinishedSemaphore[i], nullptr);
      vkDestroySemaphore(m_device, m_imageAvailableSemaphore[i], nullptr);
      vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
    }
  }

  void Renderer::createSyncObjects()
  {
    m_imageAvailableSemaphore.resize(m_maxFramesInFlight);
    m_renderFinishedSemaphore.resize(m_maxFramesInFlight);
    m_inFlightFences.resize(m_maxFramesInFlight);
    m_imagesInFlight.resize(m_swapchain->imageCount(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(size_t i = 0; i < m_maxFramesInFlight; ++i)
    {
      if(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore[i]) ||
         vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore[i]) ||
         vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create synchronization objects");
      }
    }
  }

  void Renderer::freeCommandBuffers()
  {
    vkFreeCommandBuffers(m_device, m_device.commandPools().graphics, m_commandBuffers.size(), m_commandBuffers.data());

    // m_commandBuffers.clear();
    for(auto& commandBuffer : m_commandBuffers)
      commandBuffer = nullptr;
  }

  void Renderer::allocateCommandBuffers()
  {
    //not using a command buffer per swapchain image anymore
    m_commandBuffers.resize(m_maxFramesInFlight);
    //m_commandBuffers.resize(m_swapchain->imageCount());

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = m_device.commandPools().graphics;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = m_commandBuffers.size();

    if(vkAllocateCommandBuffers(m_device, &allocateInfo, m_commandBuffers.data()) != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate command buffers");
  }

  void Renderer::recreateSwapchain()
  {
    m_window.handleMinimization();

    vkDeviceWaitIdle(m_device);

    Swapchain::Info oldSwapchainInfo{m_swapchain->info()};
    m_swapchain = std::make_unique<Swapchain>(m_device, m_window, std::move(m_swapchain));

    // the pipeline is dependant on the swapchain because of the viewport and renderpass.
    // TODO: if the render pass is compatible, and a dynamic viewport is being used, do nothing.
    // (https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/chap8.html#renderpass-compatibility)

    if(!m_window.isFullscreen() || !m_swapchain->compareSwapchainFormats(oldSwapchainInfo))
    {
      m_eventRelayer.queue(event::InvalidPipeline{m_swapchain->renderPass(), m_swapchain->extent()});
      //createGraphicsPipeline();
    }

    // createDescriptorSets();

    //  [not using a command buffer per swapchain image anymore]
    /*
    if(oldSwapchainInfo.imageCount != m_swapchain->imageCount())
    {
      freeCommandBuffers();
      allocateCommandBuffers();
      // recordCommandBuffers();
      //m_model->recreateUniformBuffers(m_swapchain->imageCount());
    }
    */
  }

  VkCommandBuffer Renderer::currentCommandBuffer() const
  {
    assert(m_hasFrameStarted && "Frame has not been started.");
    return m_commandBuffers[m_currentFrame];
  }

  bool Renderer::beginFrame()
  {
    //uint32_t& imageIndex = *pImageIndex;
    assert(!m_hasFrameStarted && "Frame already started. ");

    //////////////////////////

    vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
    VkResult result{m_swapchain->acquireNextImage(m_imageAvailableSemaphore[m_currentFrame], &m_currentImageIndex)};

    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
      recreateSwapchain();
      return false;
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
      throw std::runtime_error("Failed to acquire swap chain image");
    }

    if(m_imagesInFlight[m_currentImageIndex] != VK_NULL_HANDLE)
      vkWaitForFences(m_device, 1, &m_imagesInFlight[m_currentImageIndex], VK_TRUE, UINT64_MAX);

    m_imagesInFlight[m_currentImageIndex] = m_inFlightFences[m_currentFrame];

    m_hasFrameStarted = true;

    //////////////////////////////////////

    auto commandBuffer{ currentCommandBuffer() };

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = {};
    beginInfo.pInheritanceInfo = {};

    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
      throw std::runtime_error("Failed to begin recording command buffer.");

    return true;
  }

  void Renderer::beginRenderPass()
  {
    assert(m_hasFrameStarted && "Frame not started.");

    auto commandBuffer{ currentCommandBuffer() };
    VkClearValue clearValues[2];
    clearValues[0].color = {{0.000f, 0.000f, 0.005f, 0.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    // for(size_t i = 0; i < m_commandBuffers.size(); i++) {
    //
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_swapchain->renderPass();
    renderPassInfo.framebuffer = m_swapchain->framebuffers()[m_currentImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapchain->extent();
    renderPassInfo.clearValueCount = std::size(clearValues);
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    if(!m_window.isFullscreen())
    {
      VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(m_swapchain->extent().width),
        .height = static_cast<float>(m_swapchain->extent().height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
      };

      VkRect2D scissor = {
        .offset = {0, 0},
        .extent = m_swapchain->extent(),
      };

      vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
      vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    // renderEntities(commandBuffer, m_currentImageIndex);
  }

  void Renderer::endRenderPass()
  {
    assert(m_hasFrameStarted && "Frame not started.");

    vkCmdEndRenderPass( m_commandBuffers[m_currentFrame] );
  }

  void Renderer::endFrame()
  {
    assert(m_hasFrameStarted && "Frame not started.");

    auto commandBuffer{ currentCommandBuffer() };

    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
      throw std::runtime_error("Failed to record command buffer");

    VkSemaphore waitSemaphores[]{m_imageAvailableSemaphore[m_currentFrame]};
    VkSemaphore signalSemaphores[]{m_renderFinishedSemaphore[m_currentFrame]};
    VkPipelineStageFlags waitStages[]{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    /*m_model->updateUniformBuffers(imageIndex, m_swapchain->extent());*/

    VkSubmitInfo submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = std::size(waitSemaphores),
      .pWaitSemaphores = waitSemaphores,
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
      .signalSemaphoreCount = std::size(signalSemaphores),
      .pSignalSemaphores = signalSemaphores,
    };

    vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

    if(vkQueueSubmit(m_device.queues().graphics, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
      throw std::runtime_error("Failed to submit draw command buffer");

    m_hasFrameStarted = false;
  }

  void Renderer::present()
  {
    assert(!m_hasFrameStarted && "Frame not finished.");

    VkSemaphore signalSemaphores[]{m_renderFinishedSemaphore[m_currentFrame]};
    VkSwapchainKHR swapchains[]{*m_swapchain};

    VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = std::size(signalSemaphores),
      .pWaitSemaphores = signalSemaphores,
      .swapchainCount = std::size(swapchains),
      .pSwapchains = swapchains,
      .pImageIndices = &m_currentImageIndex,
    };

    VkResult result{vkQueuePresentKHR(m_device.queues().present, &presentInfo)};

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasResized())
    {
      m_window.resetResizedFlag();
      recreateSwapchain();
    }
    else if(result != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to present swap chain image");
    }

    ++m_currentFrame %= m_maxFramesInFlight;
  }
} // namespace vke
