#include "modelManager.hpp"

namespace vke
{
  ModelManager::ModelManager(Device& device) :
      m_device{device}
  {
  }

  ModelManager::~ModelManager()
  {
    m_models.clear();
    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_memory, nullptr);
  }

  void ModelManager::give(Model3DCreateInfo info)
  {
    size_t index{m_models.size()};

    for(EntityID entity : info.entities)
    {
      m_modelIndexes[entity] = index;
    }

    m_model3DCreateInfo.push_back(info);

    Model model{m_device, m_buffer};
    m_models.push_back(model);
  }

  void ModelManager::createModels()
  {
    uint32_t bufferSize{};

    for(const auto& info : m_model3DCreateInfo)
    {
      bufferSize += sizeof(info.vertices3d[0]) * info.vertices3d.size();
    }

    VkBufferUsageFlags usage{VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    MemAllocator::createBuffer(m_device, bufferSize, usage, &m_buffer);
    MemAllocator::allocateBuffer(m_device, m_buffer, m_memory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceSize memoryOffset{};
    for(size_t index{}; auto& info : m_model3DCreateInfo)
    {
      m_models[index].createVertexBuffer(info.vertices3d, memoryOffset);
      memoryOffset += (sizeof(info.vertices3d[0]) * info.vertices3d.size());

      if(index == 1)
        throw std::runtime_error("memory offset may be broken here");
    }

    //    m_model2DCreateInfo.clear();
    m_model3DCreateInfo.clear();
  }

  Model& ModelManager::get(EntityID entity)
  {
    size_t index{m_modelIndexes[entity]};
    return m_models[index];
  }

  std::vector<Model::Vertex3D> ModelManager::cubeModel()
  {
    return std::vector<Model::Vertex3D>
    {
      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

      // top face (orange, remember y axis points down)
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

      // nose face (blue)
      {{-.5f, -.5f, .5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, .5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, .5f}, {.1f, .1f, .8f}},
      {{-.5f, -.5f, .5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, .5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, .5f}, {.1f, .1f, .8f}},

      // tail face (green)
      {{-.5f, -.5f, -.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -.5f}, {.1f, .8f, .1f}},
      {{-.5f, -.5f, -.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.1f, .8f, .1f}},
    };
  };

    std::vector<Model::Vertex3D> ModelManager::modelVertices()
    {
      return std::vector<Model::Vertex3D>{
        {{-0.4f, -0.4f, .5f}, {1.0f, 0.0f, 0.4f}},
        {{0.4f, -0.4f, .5f}, {0.0f, 0.4f, 1.0f}},
        {{0.4f, 0.4f, .5f}, {0.4f, 1.0f, 0.0f}},
        {{0.4f, 0.4f, .5f}, {1.0f, 0.0f, 0.4f}},
        {{-0.4f, 0.4f, .5f}, {0.0f, 0.4f, 1.0f}},
        {{-0.4f, -0.4f, .5f}, {0.4f, 1.0f, 0.0f}},

        {{-0.4f, -0.4f, -.5f}, {1.0f, 0.0f, 0.4f}},
        {{0.4f, -0.4f, -.5f}, {0.0f, 0.4f, 1.0f}},
        {{0.4f, 0.4f, -.5f}, {0.4f, 1.0f, 0.0f}},
        {{0.4f, 0.4f, -.5f}, {1.0f, 0.0f, 0.4f}},
        {{-0.4f, 0.4f, -.5f}, {0.0f, 0.4f, 1.0f}},
        {{-0.4f, -0.4f, -.5f}, {0.4f, 1.0f, 0.0f}},
      };
    }
  } // namespace vke
