#pragma once

#include "core.hpp"
#include "eventListeners.hpp"

namespace vke::event
{
  struct WindowEvent
  {
  };

  struct WindowResized : WindowEvent
  {
    WindowResized() = default;
    WindowResized(int w, int h);

    // in pixels
    int width;
    int height;

    static constexpr Event::TypeID ID{Event::windowResized};
  };

  struct RenderEvent
  {
  };

  struct InvalidPipeline : RenderEvent
  {
    InvalidPipeline() = default;
    InvalidPipeline(VkRenderPass renderPass, VkExtent2D extent);

    VkRenderPass renderPass;
    VkExtent2D extent;

    static constexpr Event::TypeID ID{Event::invalidPipeline};
  };

  struct Null
  {
    static constexpr Event::TypeID ID{Event::null};
  };

} // namespace vke::event
