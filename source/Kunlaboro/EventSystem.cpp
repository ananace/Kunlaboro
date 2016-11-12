#include <Kunlaboro/EventSystem.hpp>
#include <Kunlaboro/EventSystem.inl>
#include <Kunlaboro/EntitySystem.hpp>

using namespace Kunlaboro;

EventSystem::EventSystem(EntitySystem* es)
	: mES(es)
{
}

EventSystem::~EventSystem()
{
}

void EventSystem::unregisterAllEvents(ComponentId cId)
{
	for (auto& kv : mEvents)
	{
		auto it = std::find_if(kv.second.cbegin(), kv.second.cend(), [cId](const BaseEvent* ev) {
			return ev->Type == sComponentEvent && static_cast<const detail::BaseComponentEvent*>(ev)->Component == cId;
		});

		if (it != kv.second.cend())
		{
			delete *it;
			kv.second.erase(it);
		}
	}
}