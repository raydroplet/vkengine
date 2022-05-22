#pragma once

#include "core.hpp"
#include "ecs.hpp"
#include "modelManager.hpp"

//
namespace vke
{
  //idea

  struct EngineContext
  {
    //Coordinator ecs;
    //EventRelayer eventRelayer;
  };

  struct RenderSystemContext
  {
    EventRelayer& eventRelayer;
    VkRenderPass renderPass;
    VkExtent2D extent;
    VkDescriptorSetLayout globalDescriptorSetLayout;
  };
};
