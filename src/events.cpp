#include "events.hpp"

namespace vke::event
{
  WindowResized::WindowResized(int w, int h) :
      width{w}, height{h}
  {
  }

  InvalidPipeline::InvalidPipeline(VkRenderPass renderPass, VkExtent2D extent) :
      renderPass{renderPass}, extent{extent}
  {
  }
} // namespace vke::event
