#pragma once

#include "ID.hpp"

#include <functional>
#include <typeindex>
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
		struct BaseEvent {
			std::uint8_t Type;
		};

		EventSystem(const EventSystem&) = delete;
		EventSystem(EventSystem&&) = delete;
		~EventSystem();

		EventSystem& operator=(const EventSystem&) = delete;

		template<typename Functor, typename Event>
		void eventRegister(ComponentId cId, Functor&& func);
		template<typename Functor, typename Event>
		std::size_t eventRegister(Functor&& func);
		template<typename Event>
		void eventUnregister(ComponentId cId);
		template<typename Event>
		void eventUnregister(std::size_t id);
		template<typename Event>
		void eventEmit(const Event& ev) const;
		template<typename Event, typename... Args>
		void eventEmit(Args... args) const;

	private:
		enum
		{
			sComponentEvent = 1,
			sLooseEvent = 2
		};

		EventSystem(EntitySystem* es);

		friend class EntitySystem;

		EntitySystem* mES;

		std::unordered_map<std::type_index, std::vector<BaseEvent*>> mEvents;
	};

}
