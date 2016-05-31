#pragma once

#include "Message.hpp"
#include "EntitySystem.hpp"
#include "EventSystem.inl"
#include "MessageSystem.inl"

namespace Kunlaboro
{

	MessagingComponent::MessagingComponent()
	{
		getEntitySystem()->getEventSystem().eventRegister<EntitySystem::ComponentAttachedEvent>(getId(), [this](const EntitySystem::ComponentAttachedEvent& ev) {
			if (ev.Component == getId())
				addedToEntity();
		});
	}

	template<typename... Args, typename Functor>
	void MessagingComponent::requestMessage(MessageId id, Functor&& func, float prio)
	{
		getEntitySystem()->getMessageSystem().messageRequest<Args...>(getId(), id, func, prio);
	}
	template<typename... Args, typename Obj>
	void MessagingComponent::requestMessage(MessageId id, void (Obj::*func)(Args...), float prio)
	{
		getEntitySystem()->getMessageSystem().messageRequest<Args...>(getId(), id, std::bind1st(std::mem_fn(func), static_cast<Obj*>(this)), prio);
	}
	void MessagingComponent::unrequestMessage(MessageId id)
	{
		getEntitySystem()->getMessageSystem().messageUnrequest(getId(), id);
	}
	void MessagingComponent::reprioritizeMessage(MessageId id, float prio)
	{
		getEntitySystem()->getMessageSystem().messageReprioritize(getId(), id, prio);
	}

	template<typename... Args>
	void MessagingComponent::sendMessage(MessageId id, Args... args) const
	{
		getEntitySystem()->getMessageSystem().messageSend(id, std::forward<Args>(args)...);
	}
	// template<typename... Args>
	// void MessagingComponent::sendMessageTo(MessageId id, EntityId ent, Args... args) const
	// {
	//     getEntitySystem()->getMessageSystem().messageSend(id, std::forward<Args>(args)...);
	// }

	template<typename... Args>
	void MessagingComponent::sendMessageTo(MessageId id, ComponentId comp, Args... args) const
	{
		getEntitySystem()->getMessageSystem().messageSendTo(id, comp, std::forward<Args>(args)...);
	}

	template<typename... Args, typename Functor>
	void MessagingComponent::requestMessageId(const char* const id, Functor&& func, float prio)
	{
		requestMessage(BaseMessageType::Hash(id), func, prio);
	}
	template<typename... Args, typename Obj>
	void MessagingComponent::requestMessageId(const char* const id, void (Obj::*func)(Args...), float prio)
	{
		requestMessage(BaseMessageType::Hash(id), func, prio);
	}
	void MessagingComponent::unrequestMessageId(const char* const id)
	{
		unrequestMessage(BaseMessageType::Hash(id));
	}
	void MessagingComponent::reprioritizeMessageId(const char* const id, float prio)
	{
		reprioritizeMessage(BaseMessageType::Hash(id), prio);
	}

	template<typename... Args>
	void MessagingComponent::sendMessageId(const char* const id, Args... args) const
	{
		sendMessage(BaseMessageType::Hash(id), std::forward<Args>(args)...);
	}
	template<typename... Args>
	void MessagingComponent::sendMessageIdTo(const char* const id, EntityId ent, Args... args) const
	{
		sendMessageTo(ent, BaseMessageType::Hash(id), std::forward<Args>(args)...);
	}
	template<typename... Args>
	void MessagingComponent::sendMessageIdTo(const char* const id, ComponentId comp, Args... args) const
	{
		sendMessageTo(comp, BaseMessageType::Hash(id), std::forward<Args>(args)...);
	}
}
