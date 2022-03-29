#include "ecs.hpp"

namespace vke
{
  //  EntityManager Coordinator::m_entityManager{};
  //  ComponentManager Coordinator::m_componentManager{};
  //  SystemManager Coordinator::m_systemManager{};

  EntityManager::EntityManager(uint32_t entityCount) :
      m_indexCount{entityCount}
  {
    m_signatures.reserve(m_indexCount);
    generateIndexes();
  }

  void EntityManager::generateIndexes()
  {
    m_indexCount *= 2;
    for(EntityID e = m_livingEntities; e < m_indexCount; ++e) {
      m_availableEntities.push(e); // maybe could be better
    }
  }

  EntityID EntityManager::createEntity()
  {
    ////assert(mEntitiesCount < mIndexCount && "Invalid entity. Out of range");
    // if(m_livingEntities == m_indexCount)
    if(m_availableEntities.empty())
    {
      generateIndexes();
    }

    EntityID id = m_availableEntities.front();
    m_availableEntities.pop();
    ++m_livingEntities;

    return id;
  }

  void EntityManager::destroyEntity(EntityID e)
  {
    assert(e < m_indexCount && "Invalid entity. Out of range");

    m_signatures[e].reset();
    m_availableEntities.push(e);
    --m_livingEntities;
  }

  void EntityManager::setSignature(EntityID e, Signature s)
  {
    assert(e < m_indexCount && "Invalid entity. Out of range");

    m_signatures[e] = s;
  }

  Signature EntityManager::getSignature(EntityID e)
  {
    assert(e < m_indexCount && "Invalid entity. Out of range");

    return m_signatures[e];
  }

  void ComponentManager::notifyEntityDestruction(EntityID entity)
  {
    // Notify each component array that an entity has been destroyed
    // If it has a component for that entity, it will remove it
    /*
    for(const auto& pair : m_destructionNotification)
    {
      //const auto& componentArray = pair.second;
      const auto& fun = pair.second;

      fun(entity);
    }
    */

    for(const auto& pair : m_componentArrays)
    {
      const auto& componentArray = pair.second;
      componentArray->notifyEntityDestruction(entity);
    }
  }

  void SystemManager::notifyEntityDestruction(EntityID entity)
  {
    // Erase a destroyed entity from all system lists
    // mEntities is a set so no check needed
    for(const auto& pair : m_systems)
    {
      const auto& system = pair.second;

      system->m_entities.erase(entity);
    }
  }

  void SystemManager::notifyEntitySignatureChange(EntityID entity, const Signature& entitySignature)
  {
    // Notify each system that an entity's signature changed
    for(const auto& [id, system] : m_systems)
    {
      Signature systemSignature = m_signatures[id];

      // Entity signature matches system signature - insert into set
      if((entitySignature & systemSignature) == systemSignature)
      {
        system->m_entities.insert(entity);
      }
      // Entity signature does not match system signature - erase from set
      else
      {
        system->m_entities.erase(entity);
      }
    }
  }

  EntityID Coordinator::createEntity()
  {
    return m_entityManager.createEntity();
  }

  void Coordinator::destroyEntity(EntityID e)
  {
    if(e != std::numeric_limits<EntityID>::max())
    {
      m_entityManager.destroyEntity(e);
      m_componentManager.notifyEntityDestruction(e);
      m_systemManager.notifyEntityDestruction(e);
    }
  }
} // namespace vke
