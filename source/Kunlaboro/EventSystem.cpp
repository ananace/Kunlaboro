#include <Kunlaboro/EventSystem.hpp>
#include <Kunlaboro/EntitySystem.hpp>

using namespace Kunlaboro;

EventSystem::EventSystem(EntitySystem* es)
	: mES(es)
{
}
