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

void EventSystem::eventUnregisterAll(ComponentId cId)
{
	for (auto& kv : mEvents)
	{
		auto it = std::find_if(kv.second.cbegin(), kv.second.cend(), [cId](const BaseEvent* ev) {
			return ev->Type == sComponentEvent && static_cast<const detail::BaseComponentEvent*>(ev)->Component == cId;
		});

		if (it != kv.second.cend())
			kv.second.erase(it);
	}
}