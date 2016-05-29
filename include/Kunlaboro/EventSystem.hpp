#pragma once

#include "ID.hpp"

#include <functional>
#include <unordered_map>
#include <vector>

namespace Kunlaboro
{

	class EntitySystem;

	struct ComponentCreatedEvent
	{
		ComponentId Component;
		EntitySystem* EntitySystem;
	};
	struct ComponentDestroyedEvent
	{
		ComponentId Component;
		EntitySystem* EntitySystem;
	};
	struct EntityCreatedEvent
	{
		EntityId Entity;
		EntitySystem* EntitySystem;
	};
	struct EntityDestroyedEvent
	{
		EntityId Entity;
		EntitySystem* EntitySystem;
	};

	class EventSystem
	{
	public:
		EventSystem(const EventSystem&) = delete;
		EventSystem(EventSystem&&) = delete;
		~EventSystem();

		EventSystem& operator=(const EventSystem&) = delete;

		template<typename Functor, typename Event>
		void eventRegister(ComponentId, Functor&& func);
		template<typename Functor, typename Event>
		void eventRegister(EntityId, Functor&& func);
		template<typename Event>
		void eventUnregister(ComponentId);
		template<typename Event>
		void eventUnregister(EntityId);
		template<typename Event>
		void eventEmit(const Event& ev) const;
		template<typename Event, typename... Args>
		void eventEmit(Args... args) const;


	private:
		EventSystem(EntitySystem* es);

		friend class EntitySystem;

		EntitySystem* mES;
	};

}
