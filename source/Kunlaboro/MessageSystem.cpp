#include <Kunlaboro/MessageSystem.hpp>
#include <Kunlaboro/EntitySystem.hpp>

using namespace Kunlaboro;

MessageSystem::MessageSystem(EntitySystem* es)
	: mES(es)
{
}

MessageSystem::~MessageSystem()
{
}
