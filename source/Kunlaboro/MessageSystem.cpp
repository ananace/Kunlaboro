#include <Kunlaboro/MessageSystem.hpp>
#include <Kunlaboro/MessageSystem.inl>
#include <Kunlaboro/EntitySystem.hpp>

using namespace Kunlaboro;

MessageSystem::MessageSystem(EntitySystem* es)
	: mES(es)
{
}

MessageSystem::~MessageSystem()
{
	for (auto& msg : mMessages)
	{
		for (auto& cb : msg.second.Callbacks)
			delete cb;

		delete msg.second.Type;
	}
}

void MessageSystem::unrequestAllMessages(ComponentId cId)
{
	for (auto& msg : mMessages)
	{
		auto& cb = msg.second.Callbacks;

		auto it = std::find_if(cb.cbegin(), cb.cend(), [cId](const BaseMessageCallback* cb) { return cb->Component == cId; });
		if (it != cb.cend())
			cb.erase(it);
	}
}

void MessageSystem::unrequestAllMessages(EntityId eId)
{
	for (auto& msg : mMessages)
	{
		auto& cb = msg.second.Callbacks;

		auto it = std::remove_if(cb.begin(), cb.end(), [this, eId](const BaseMessageCallback* cb) {
			return mES->isAttached(cb->Component, eId);
		});

		if (it != cb.end())
			for (auto rit = it; rit != cb.end(); ++rit)
				cb.erase(rit);
	}
}
