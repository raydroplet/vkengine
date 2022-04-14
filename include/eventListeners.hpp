#pragma once

#include "core.hpp"

namespace vke
{
  struct Event
  {
    using TypeID = size_t;
    enum
    {
      windowResized,
      invalidPipeline,
      input,
      null,
    };
  };

  // Can hear a single event type and send it to multiple classes
  template<typename EventType>
  class EventListener
  {
    using Object = void*;

  public:
    template<typename C>
    void setCallback(C* object, void (C::*fn)(EventType&));

    template<typename C>
    void unbind(C* object);

    void queue(EventType& event);
    void queue(EventType&& event);
    void dispatch();

    void operator()(EventType& event) const;
    inline void operator()(EventType&& event) const;

    //  // for the up-to-date event
    //  void hold(EventType& event);
    //  void hold(EventType&& event);
    //  void drop();
    //
    //  EventListener() :
    //      m_event{new EventType} {}
    //
    //  ~EventListener()
    //  {
    //    delete m_event;
    //  }

  private:
    std::unordered_map<Object, std::function<void(EventType&)>> m_callbacks;
    std::vector<EventType> m_eventQueue;
    //    EventType* m_event{};
  };

  // for handling events
  class EventRelayer
  {
    using EventListenerPtr = void*;

  public:
    template<typename EventType, typename C>
    void setCallback(C* object, void (C::*callback)(EventType&));

    template<typename EventType>
    void queue(EventType& event);

    template<typename EventType>
    void queue(EventType&& event);

    template<typename EventType>
    void dispatch();

  private:
    template<typename EventType>
    EventListener<EventType>* listener();

  private:
    std::unordered_map<Event::TypeID, EventListenerPtr> m_listeners;
  };


  // EventListener
  template<typename EventType>
  template<typename C>
  void EventListener<EventType>::setCallback(C* object, void (C::*fn)(EventType&))
  {
    assert("Binding the same class again" && m_callbacks.find(object) == m_callbacks.end());

    m_callbacks[object] = (std::bind(fn, object, std::placeholders::_1));
  }

  template<typename EventType>
  template<typename C>
  void EventListener<EventType>::unbind(C* object)
  {
    m_callbacks.erase(object);
  }

  template<typename EventType>
  void EventListener<EventType>::operator()(EventType& event) const
  {
    for(auto& [signature, callback] : m_callbacks)
    {
      callback(event);
    }
  }

  template<typename EventType>
  inline void EventListener<EventType>::operator()(EventType&& event) const
  {
    operator()(event);
  }

  template<typename EventType>
  void EventListener<EventType>::queue(EventType& event)
  {
    m_eventQueue.push_back(event);
  }

  template<typename EventType>
  void EventListener<EventType>::queue(EventType&& event)
  {
    queue(event);
  }

  template<typename EventType>
  void EventListener<EventType>::dispatch()
  {
    for(auto& event : m_eventQueue)
    {
      for(auto& [signature, callback] : m_callbacks)
      {
        callback(event);
      }
    }

    //  // the up-to-date event
    //  if(m_event)
    //    callback(m_event);

    m_eventQueue.clear();
  }

  // template<typename EventType>
  // void EventListener<EventType>::hold(EventType& event)
  //{
  //   *m_event = event;
  // }
  //
  // template<typename EventType>
  // inline void EventListener<EventType>::hold(EventType&& event)
  //{
  //   hold(event);
  // }
  //
  // template<typename EventType>
  // void EventListener<EventType>::drop()
  //{
  //   delete m_event;
  //   m_event = nullptr;
  // }


  // Event Relayer
  template<typename EventType, typename C>
  void EventRelayer::setCallback(C* object, void (C::*callback)(EventType&))
  {
    if(m_listeners.find(EventType::ID) == m_listeners.end())
      m_listeners[EventType::ID] = new EventListener<EventType>;

    // you may wanna check this
    //m_listeners.try_emplace(EventType::ID);

    listener<EventType>()->setCallback(object, callback);
  }

  template<typename EventType>
  void EventRelayer::queue(EventType& event)
  {
    assert("Queuing a event with no callback." && m_listeners.find(EventType::ID) != m_listeners.end());
    listener<EventType>()->queue(event);
  }

  template<typename EventType>
  inline void EventRelayer::queue(EventType&& event)
  {
    queue(event);
  }

  template<typename EventType>
  void EventRelayer::dispatch()
  {
    listener<EventType>()->dispatch();
  }

  template<typename EventType>
  EventListener<EventType>* EventRelayer::listener()
  {
    if(m_listeners.at(EventType::ID) == nullptr)
    {
      //throw std::runtime_error("TODO: fix me");
      //TODO: either throw properly or return a Null Object
      throw std::runtime_error("nullptr");
    }

    return reinterpret_cast<EventListener<EventType>*>(m_listeners[EventType::ID]);
  }
}; // namespace vke
