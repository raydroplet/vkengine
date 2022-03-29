#pragma once

#include "core.hpp"

#include "allocator.hpp"
#include "ecs.hpp"
#include "model.hpp"

namespace vke
{
  class ModelManager
  {
  public:
    struct Model2DCreateInfo;
    struct Model3DCreateInfo;

    ModelManager(Device& device);
    ~ModelManager();

    void give(Model3DCreateInfo info);
    Model& get(EntityID entity);

    std::vector<Model::Vertex3D> cubeModel();
    std::vector<Model::Vertex3D> modelVertices();
    void createModels();

    //Model loadModel() { assert(!( __FUNCTION__ && "not implemented.")); }

  private:
    Device& m_device;

    std::unordered_map<EntityID, size_t> m_modelIndexes;
    std::vector<Model> m_models;

    Buffer m_buffer{VK_NULL_HANDLE};
    Memory m_memory{VK_NULL_HANDLE};

//    std::vector<Model2DCreateInfo> m_model2DCreateInfo;
    std::vector<Model3DCreateInfo> m_model3DCreateInfo;
  };

  struct ModelManager::Model2DCreateInfo
  {
    std::vector<EntityID> entities;
    std::vector<Model::Vertex2D> vertices2d;
    std::vector<uint32_t> indices;
  };

  struct ModelManager::Model3DCreateInfo
  {
    std::vector<EntityID> entities;
    std::vector<Model::Vertex3D> vertices3d;
    std::vector<uint32_t> indices;
  };
} // namespace vke
