#include "EventSystem.hpp"

#include <type_traits>

namespace Kunlaboro
{

	template<typename Functor, typename Event>
	void EventSystem::eventRegister(ComponentId cId, Functor&& func)
	{

	}
	template<typename Functor, typename Event>
	void EventSystem::eventRegister(EntityId eId, Functor&& func)
	{

	}
	template<typename Event>
	void EventSystem::eventUnregister(ComponentId cId)
	{

	}
	template<typename Event>
	void EventSystem::eventUnregister(EntityId eId)
	{

	}
	template<typename Event>
	void EventSystem::eventEmit(const Event& ev) const
	{

	}
	template<typename Event, typename... Args>
	void EventSystem::eventEmit(Args... args) const
	{
		// static_assert(std::is_trivial<Event>::value, "Must be a POD type.");

		Event toSend{ std::forward<Args>(args)... };
		eventEmit(toSend);
	}

}