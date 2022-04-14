#pragma once

#include "core.hpp"

#include "allocator.hpp"
#include "ecs.hpp"
#include "model.hpp"

namespace vke
{
  // TODO: make this a system in the ecs, and add a component with a shared model managed be this system. futurely, this could become some sort of asset manager (for textures, models, etc, that are shared between entities)
  class ModelManager
  {
    struct Builder : public Model::Builder
    {
      std::vector<EntityID> entities;
    };

  public:
    ModelManager(Device& device, Coordinator& ecs);
    ~ModelManager();

    void give(Model::Builder builder, std::vector<EntityID> entities);
    Model& get(EntityID entity);

    void createModels();
    void loadModel(std::filesystem::path path);

  private:
    Device& m_device;

    std::unordered_map<EntityID, size_t> m_modelIndexes;
    std::unordered_map<EntityID, size_t> m_modelCount;
    std::list<Model> m_models;
    Coordinator& m_ecs;

//  Buffer  m_vertexBuffer{};
//  Memory  m_vertexMemory{};
//
//  Buffer  m_indexBuffer{};
//  Memory  m_indexMemory{};

//    std::vector<Model2DCreateInfo> m_model2DCreateInfo;
//    std::vector<Model3DCreateInfo> m_model3DCreateInfo;
    std::vector<Builder> m_modelBuilders{};
  };
} // namespace vke
