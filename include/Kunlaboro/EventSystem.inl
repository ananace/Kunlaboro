#pragma once

#include "EventSystem.hpp"

#include <algorithm>
#include <type_traits>

namespace Kunlaboro
{

	namespace detail
	{

		template<typename Event>
		struct EventType
		{
			std::function<void(const Event&)> Func;
		};

		struct BaseComponentEvent : public EventSystem::BaseEvent
		{
			ComponentId Component;
		};
		template<typename Event>
		struct ComponentEvent : public BaseComponentEvent, public EventType<Event>
		{ };

		struct BaseLooseEvent : public EventSystem::BaseEvent
		{
			std::size_t ID;
		};
		template<typename Event>
		struct LooseEvent : public BaseLooseEvent, public EventType<Event>
		{ };

	}

	template<typename Event, typename Functor>
	void EventSystem::registerEvent(ComponentId cId, Functor&& func)
	{
		auto& list = mEvents[typeid(Event)];

		auto* ev = new detail::ComponentEvent<Event>();
		ev->Component = cId;
		ev->Func = std::move(func);
		ev->Type = sComponentEvent;

		list.push_back(ev);
	}
	template<typename Event, typename Functor>
	EventSystem::ListenerId EventSystem::registerEvent(Functor&& func)
	{
		auto& list = mEvents[typeid(Event)];

		auto* ev = new detail::LooseEvent<Event>();
		ev->Func = std::move(func);
		ev->ID = list.size();
		ev->Type = sLooseEvent;

		list.push_back(ev);
		return ev->ID;
	}
	template<typename Event>
	void EventSystem::unregisterEvent(ComponentId cId)
	{
		auto& list = mEvents[typeid(Event)];

		auto it = std::find_if(list.cbegin(), list.cend(), [cId](const BaseEvent* ev) {
			return ev->Type == sComponentEvent && static_cast<const detail::BaseComponentEvent*>(ev)->Component == cId;
		});

		if (it != list.cend())
		{
			delete *it;
			list.erase(it);
		}
	}
	template<typename Event>
	void EventSystem::unregisterEvent(ListenerId id)
	{
		auto& list = mEvents[typeid(Event)];

		auto it = std::find_if(list.cbegin(), list.cend(), [id](const BaseEvent* ev) {
			return ev->Type == sLooseEvent && static_cast<const detail::BaseLooseEvent*>(ev)->ID == id;
		});

		if (it != list.cend())
		{
			delete *it;
			list.erase(it);
		}
	}
	template<typename Event>
	void EventSystem::emitEvent(const Event& toSend) const
	{
		if (mEvents.count(typeid(Event)) == 0)
			return;

		const auto& list = mEvents.at(typeid(Event));

		for (auto& ev : list)
		{
			if (ev->Type == sLooseEvent)
				static_cast<const detail::LooseEvent<Event>*>(ev)->Func(toSend);
			else
				static_cast<const detail::ComponentEvent<Event>*>(ev)->Func(toSend);
		}
	}
	template<typename Event, typename... Args>
	void EventSystem::emitEvent(Args... args) const
	{
		// static_assert(std::is_trivial<Event>::value, "Must be a POD type.");

		Event toSend{ std::forward<Args>(args)... };
		emitEvent(toSend);
	}

}