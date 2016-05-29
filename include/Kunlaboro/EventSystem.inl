#include "EventSystem.hpp"

#include <type_traits>

namespace Kunlaboro
{

	namespace detail
	{

		template<typename Event>
		struct EventType : public EventSystem::BaseEvent
		{
			std::function<void(const Event&)> Func;
		};

		template<typename Event>
		struct BaseComponentEvent : public EventType<Event>
		{
			ComponentId Component;
		};
		template<typename Event>
		struct BaseLooseEvent : public EventSystem::BaseEvent
		{
			std::size_t ID;
		};
	}

	template<typename Functor, typename Event>
	void EventSystem::eventRegister(ComponentId cId, Functor&& func)
	{
		auto& list = mEvents[typeid(Event)];

		auto* ev = new detail::ComponentEvent<Functor>();
		ev->Component = cId;
		ev->func = std::move(func);
		ev->Type = sComponentEvent;
		list.push_back(ev);
	}
	template<typename Functor, typename Event>
	std::size_t EventSystem::eventRegister(Functor&& func)
	{
		auto& list = mEvents[typeid(Event)];

		auto* ev = new detail::LooseEvent<Functor>();
		ev->func = std::move(func);
		ev->ID = list.size();
		ev->Type = sLooseEvent;
		list.push_back(ev);

		return ev->ID;
	}
	template<typename Event>
	void EventSystem::eventUnregister(ComponentId cId)
	{
		auto& list = mEvents[typeid(Event)];

		auto it = std::find_if(list.cbegin(), list.cend(), [cId](BaseEvent* ev) {
			return ev->Type == sComponentEvent && static_cast<detail::BaseComponentEvent*>(ev)->Component == cId;
		});

		if (it != list.cend())
			list.erase(it);
	}
	template<typename Event>
	void EventSystem::eventUnregister(std::size_t id)
	{
		auto& list = mEvents[typeid(Event)];

		auto it = std::find_if(list.cbegin(), list.cend(), [cId](BaseEvent* ev) {
			return ev->Type == sLooseEvent && static_cast<detail::BaseLooseEvent*>(ev)->Component == cId;
		});

		if (it != list.cend())
			list.erase(it);
	}
	template<typename Event>
	void EventSystem::eventEmit(const Event& toSend) const
	{
		if (mEvents.count(typeid(Event)) == 0)
			return;

		const auto& list = mEvents.at(typeid(Event));

		for (auto& ev : list)
			static_cast<detail::EventType<Event>*>(ev)->Func(toSend);
	}
	template<typename Event, typename... Args>
	void EventSystem::eventEmit(Args... args) const
	{
		// static_assert(std::is_trivial<Event>::value, "Must be a POD type.");

		Event toSend{ std::forward<Args>(args)... };
		eventEmit(toSend);
	}

}