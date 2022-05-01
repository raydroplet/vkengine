#pragma once

#include "camera.hpp"
#include "core.hpp"
#include "eventListeners.hpp"
#include "ecs.hpp"

namespace vke
{
  struct FrameInfo
  {
    using TimeStep = std::chrono::duration<double, std::chrono::seconds::period>;

    uint32_t frameIndex{};
    TimeStep timeStep{};
    VkCommandBuffer commandBuffer{};
    Camera& camera;
    VkDescriptorSet globalDescriptorSet{};
  };
} // namespace vke

//
