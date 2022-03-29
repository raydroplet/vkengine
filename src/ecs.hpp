#pragma once

#include "core.hpp"
#include "multilist.hpp"
#include "components.hpp" //always use when i include ecs

namespace vke
{
  using EntityID = uint32_t;
  using Signature = std::bitset<8>; // entity signature

  // TODO: implement a unordered_map stored in contiguous memory

  class EntityManager
  {
  public:
    EntityManager(uint32_t entityCount = 256);
    void generateIndexes();
    EntityID createEntity();
    void destroyEntity(EntityID e);
    void setSignature(EntityID e, Signature s);
    Signature getSignature(EntityID e);

  private:
    uint32_t m_indexCount{}; // total number of entity indexes
    uint32_t m_livingEntities{};

    std::queue<EntityID> m_availableEntities; // index queue // what about std::stack?
    std::vector<Signature> m_signatures;      // component signatures for each entity // < v < e < c < t < o < r
  };

  class ComponentArrayInterface
  {
  public:
    virtual ~ComponentArrayInterface() = default;
    virtual void notifyEntityDestruction(EntityID entity) = 0;
  };

  template<typename T>
  class ComponentArray : public ComponentArrayInterface
  {
    using ComponentIndex = typename multilist<T>::index;

  public:
    ComponentArray(int size = 2);
    void insertData(EntityID e, T component);
    void removeData(EntityID e);
    T& getData(EntityID e);
    void notifyEntityDestruction(EntityID e);

  private:
    multilist<T> m_components;
    std::unordered_map<EntityID, ComponentIndex> m_entityToIndex; // a contiguous container would be better
    std::unordered_map<ComponentIndex, EntityID, typename multilist<T>::IndexHash> m_indexToEntity;
  };

  // TODO: implement your own RTTI system

  // return type id's
  template<typename T>
  static inline size_t getId()
  {
    return typeid(T).hash_code();
  }

  class ComponentManager
  {
    using ComponentId = decltype(typeid(void).hash_code()); // size_t
    using ComponentSignature = std::bitset<8>;
    //using destruction_function = std::function<void(ComponentArrayInterface*, EntityID)>;

  public:
    template<typename T>
    void registerComponent();

    template<typename T>
    ComponentSignature getComponentSignature();

    template<typename T>
    void addComponent(EntityID entity, T component);

    template<typename T>
    void removeComponent(EntityID entity);

    template<typename T>
    T& getComponent(EntityID entity);

    void notifyEntityDestruction(EntityID entity);

  private:
    template<typename T>
    //std::shared_ptr<ComponentArray<T>> getComponentArray()
    ComponentArray<T>& getComponentArray()
    {
      ComponentId id = getId<T>();
      assert(m_componentArrays.find(id) != m_componentArrays.end() && "Component not registered before use.");

      return *(static_cast<ComponentArray<T>*>(m_componentArrays[id]));
    }

  private:
    // Map from type string pointer to a component type
    std::unordered_map<ComponentId, ComponentSignature> m_componentSignatures{}; // mainly used in assertions

    // Map from type string pointer to a component array
//    std::unordered_map<ComponentId, std::shared_ptr<ComponentArrayInterface>> m_componentArrays{};
    std::unordered_map<ComponentId, ComponentArrayInterface*> m_componentArrays{};

    // removeData() function pointers to notify the generic componentArrays that a entity has been destroyed
    // "Another method of handling this is to use events, so that every ComponentArray can subscribe to an Entity Destroyed event and then respond accordingly."
    //std::unordered_map<ComponentId, destruction_function> m_destructionNotification;

    // The component type to be assigned to the next registered component - starting at 0
    ComponentSignature m_availableSignature{0b1}; // bit 0
  };

  class System
  {
    // Every system needs a list of entities, and we want some logic outside of the system (in the form of a manager) so we use a System base class that has only a std::set of entities.

  public:
    std::set<EntityID> m_entities;
  };

  class SystemManager
  {
    using SystemId = size_t; // decltype(typeid(void).hash_code())

  public:
    template<typename T>
    std::unique_ptr<T> registerSystem();

    template<typename T>
    void setSignature(Signature signature);

    void notifyEntityDestruction(EntityID entity);
    void notifyEntitySignatureChange(EntityID entity, const Signature& entitySignature);

  private:
    std::unordered_map<SystemId, Signature> m_signatures{};
    std::unordered_map<SystemId, std::unique_ptr<System>> m_systems{};
  };

  class Coordinator
  {
  public:
    // Entity methods
    EntityID createEntity();
    void destroyEntity(EntityID e);

    // Component methods
    template<typename T>
    void registerComponent();

    template<typename T>
    void addComponent(EntityID entity, T component);

    template<typename T>
    void removeComponent(EntityID entity);

    template<typename T>
    T& getComponent(EntityID entity);

    template<typename T>
    Signature getComponentSignature();

    // System methods
    template<typename T>
    std::unique_ptr<T> registerSystem();

    template<typename T>
    void setSystemSignature(Signature signature);

  private:
    EntityManager m_entityManager;
    ComponentManager m_componentManager;
    SystemManager m_systemManager;
  };


  //ComponentManager
  template<typename T>
  void ComponentManager::registerComponent()
  {
    size_t id = getId<T>();
    assert(m_componentArrays.find(id) == m_componentArrays.end() && "Registering component type more than once");
    assert(m_availableSignature != std::numeric_limits<ComponentSignature>::max() && "Signature limit reached, you should change the ComponentSignature type");

    // Add this component type to the component type map
    //m_componentSignatures.insert({id, m_availableSignature});
    m_componentSignatures[id] = m_availableSignature;

    // Create a ComponentArray and add it to the component arrays map
    //m_componentArrays.insert({id, std::make_unique<ComponentArray<T>>()});
    m_componentArrays[id] =  new ComponentArray<T>; //std::make_shared<ComponentArray<T>>();

    //change the signature so that the next component registered will be different
    m_availableSignature <<= 1;

    //get the removeData function pointer
    //m_destructionNotification[id] = &reinterpret_cast<ComponentArray<T>>(m_componentArrays[id])->notifyEntityDestruction;
    //m_destructionNotification[id] = reinterpret_cast<void*>(std::static_pointer_cast<ComponentArray<T>>(m_componentArrays[id])->notifyEntityDestruction_ptr());
  }

  template<typename T>
  ComponentManager::ComponentSignature ComponentManager::getComponentSignature()
  {
    assert(m_componentSignatures.find(getId<T>()) != m_componentSignatures.end() && "Component not registered before use");

    return m_componentSignatures[getId<T>()];
  }

  template<typename T>
  void ComponentManager::addComponent(EntityID entity, T component)
  {
    getComponentArray<T>().insertData(entity, component);
    //reinterpret_cast<T>(m_componentArrays[getId<T>()])->insertData(entity, component);
  }

  template<typename T>
  void ComponentManager::removeComponent(EntityID entity)
  {
    // Remove a component from the array for an entity
    getComponentArray<T>().removeData(entity);
    //reinterpret_cast<T>(m_componentArrays[getId<T>()])->removeData(entity);
  }

  template<typename T>
  T& ComponentManager::getComponent(EntityID entity)
  {
    //return reinterpret_cast<T>(m_componentArrays[getId<T>()])->getData(entity);
    return getComponentArray<T>().getData(entity);
  }


  //SystemManager
  template<typename T>
  std::unique_ptr<T> SystemManager::registerSystem()
  {
    assert(m_systems.find(getId<T>) != m_systems.end() && "Registering system more than once");

    auto system = std::make_unique<T>();
    m_systems.insert(system);
    return system;
  }

  template<typename T>
  void SystemManager::setSignature(Signature signature)
  {
    assert(m_systems.find(getId<T>()) != m_systems.end() && "System used before registered");

    m_signatures[getId<T>()] = signature;
  }


  //ComponentArray
  template<typename T>
  ComponentArray<T>::ComponentArray(int size) :
      m_components{size}, m_entityToIndex{}, m_indexToEntity{}
  {
  }

  template<typename T>
  void ComponentArray<T>::insertData(EntityID e, T component)
  {
    // does not support multiple components of the same type. consider adding this possibility, maybe.
    assert(m_entityToIndex.find(e) == m_entityToIndex.end() && "Component added to same entity more than once");

    // push new component and return the index
    ComponentIndex newIndex = m_components.push(component);

    // update the mappings
    m_entityToIndex[e] = newIndex;
    m_indexToEntity[newIndex] = e;
  }

  template<typename T>
  void ComponentArray<T>::removeData(EntityID e)
  {
    assert(m_entityToIndex.find(e) != m_entityToIndex.end() && "Removing non-existant component");

    ComponentIndex removedEntityIndex = m_entityToIndex[e];
    ComponentIndex lastEntityIndex = m_components.lastIndex();

    m_components.erase(removedEntityIndex);

    // Update map to point to moved spot
    EntityID lastEntity = m_indexToEntity[lastEntityIndex];
    m_entityToIndex[lastEntity] = removedEntityIndex;
    m_indexToEntity[removedEntityIndex] = lastEntity;

    m_entityToIndex.erase(e);
    m_indexToEntity.erase(lastEntityIndex);
  }

  template<typename T>
  T& ComponentArray<T>::getData(EntityID e)
  {
    assert(m_entityToIndex.find(e) != m_entityToIndex.end() && "Retrieving non-existent component.");

    return m_components[m_entityToIndex[e]];
  }

  template<typename T>
  void ComponentArray<T>::notifyEntityDestruction(EntityID e)
  {
    if(m_entityToIndex.find(e) != m_entityToIndex.end())
    {
      removeData(e); //  Remove the entity's component if it existed
    }
  }


  //Coordinator - Component methods
  template<typename T>
  inline void Coordinator::registerComponent()
  {
    m_componentManager.registerComponent<T>();
  }

  template<typename T>
  inline void Coordinator::addComponent(EntityID entity, T component)
  {
    m_componentManager.addComponent<T>(entity, component);

    auto signature = m_entityManager.getSignature(entity);
    signature &= m_componentManager.getComponentSignature<T>();
    m_entityManager.setSignature(entity, signature);

    m_systemManager.notifyEntitySignatureChange(entity, signature);
  }

  template<typename T>
  inline void Coordinator::removeComponent(EntityID entity)
  {
    m_componentManager.removeComponent<T>(entity);

    Signature signature = m_entityManager.getSignature(entity);
    signature &= getComponentSignature<T>();
    m_entityManager.setSignature(entity, signature);

    m_systemManager.notifyEntitySignatureChange(entity, signature);
  }

  template<typename T>
  inline T& Coordinator::getComponent(EntityID entity)
  {
    return m_componentManager.getComponent<T>(entity);
  }

  template<typename T>
  inline Signature Coordinator::getComponentSignature()
  {
    return m_componentManager.getComponentSignature<T>();
  }


  // Coordinator - System methods
  template<typename T>
  inline std::unique_ptr<T> Coordinator::registerSystem()
  {
    return m_systemManager.registerSystem<T>();
  }

  template<typename T>
  inline void Coordinator::setSystemSignature(Signature signature)
  {
    m_systemManager.setSignature<T>(signature);
  }
} // namespace vke
