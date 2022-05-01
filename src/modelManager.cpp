#define TINYOBJLOADER_IMPLEMENTATION
#include "modelManager.hpp"

namespace vke
{
  ModelManager::ModelManager(Device& device, Coordinator& ecs) :
      m_device{device},
      m_ecs{ecs}
  {
  }

  ModelManager::~ModelManager()
  {
    m_models.clear();
  }

  void ModelManager::give(Model::Builder builder, std::vector<EntityID> entities)
  {
    assert("Empty model (no vertices)" && builder.vertices.size() > 0);
    m_modelBuilders.push_back({builder, entities});
  }

  void ModelManager::createModels()
  {
    //TODO: vkFlushMappedMemoryRanges();

    for(auto& builder : m_modelBuilders)
    {
      m_models.emplace_back(m_device, builder);
      for(auto& e : builder.entities)
      {
        auto& c{m_ecs.getComponent<cmp::Common>(e)};
        c.setModel(&m_models.back());
      }
    }

    m_modelBuilders.clear();
  }

  Model& ModelManager::get(EntityID entity)
  {
    throw std::runtime_error("unimplemented function");
  }
} // namespace vke
