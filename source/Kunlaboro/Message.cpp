#include <Kunlaboro/Message.hpp>
#include <Kunlaboro/Message.inl>
#include <Kunlaboro/EventSystem.inl>

using namespace Kunlaboro;

MessagingComponent::MessagingComponent()
{
	getEntitySystem()->getEventSystem().eventRegister<EntitySystem::ComponentAttachedEvent>(getId(), [this](const EntitySystem::ComponentAttachedEvent& ev) {
		if (ev.Component == getId())
			addedToEntity();
	});
}

void MessagingComponent::unrequestMessage(MessageId id)
{
	getEntitySystem()->getMessageSystem().messageUnrequest(getId(), id);
}
void MessagingComponent::reprioritizeMessage(MessageId id, float prio)
{
	getEntitySystem()->getMessageSystem().messageReprioritize(getId(), id, prio);
}

void MessagingComponent::unrequestMessageId(const char* const id)
{
	unrequestMessage(MessageSystem::hash(id));
}
void MessagingComponent::reprioritizeMessageId(const char* const id, float prio)
{
	reprioritizeMessage(MessageSystem::hash(id), prio);
}
