#pragma once

#include "MessageSystem.hpp"
#include "EntitySystem.hpp"

#include <algorithm>

namespace Kunlaboro
{
	template<typename... Args>
	inline void MessageSystem::registerMessage(const char* const name, MessageLocality locality)
	{
		auto mId = hash(name);
		registerMessage<Args...>(mId, name, locality);
	}

	template<typename... Args, typename Functor>
	inline void MessageSystem::requestMessage(ComponentId cId, const char* const message, Functor&& func, float prio)
	{
		auto mId = hash(message);
		requestMessage<Args...>(cId, mId, std::forward<Functor&&>(func), prio);
	}
	inline void MessageSystem::unrequestMessage(ComponentId cId, const char* const message)
	{
		auto mId = hash(message);
		unrequestMessage(cId, mId);
	}
	inline void MessageSystem::reprioritizeMessage(ComponentId cId, const char* const message, float prio)
	{
		auto mId = hash(message);
		reprioritizeMessage(cId, mId, prio);
	}
	template<typename... Args>
	inline void MessageSystem::sendMessage(const char* const message, Args&&... args) const
	{
		auto mId = hash(message);
		sendMessage<Args...>(mId, std::forward<Args>(args)...);
	}
	template<typename... Args>
	inline void MessageSystem::sendMessageTo(const char* const message, ComponentId cId, Args&&... args) const
	{
		auto mId = hash(message);
		sendMessageTo<Args...>(mId, cId, std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void MessageSystem::registerMessage(MessageId mId, const char* const name, MessageLocality locality)
	{
		if (mMessages.count(mId) > 0)
			return;

		auto& message = mMessages[mId];
		message.Type = new MessageType<Args...>(
			mId
			);
		message.Locality = locality;

#ifdef _DEBUG
		if (name)
			message.Type->Name = name;
#endif
	}

	template<typename... Args, typename Functor>
	inline void MessageSystem::requestMessage(ComponentId cId, MessageId mId, Functor&& func, float prio)
	{
		if (mMessages.count(mId) == 0)
			return;

		auto& message = mMessages[mId];
		auto* msgType = static_cast<MessageType<Args...>*>(message.Type);

		if (!msgType->isValid(func))
			return;

		auto it = std::find_if(message.Callbacks.cbegin(), message.Callbacks.cend(), [cId](const BaseMessageCallback* cb) { return cb->Component == cId; });
		if (it != message.Callbacks.end())
		{
			delete *it;
			message.Callbacks.erase(it);
		}

		message.Callbacks.insert(
			std::lower_bound(message.Callbacks.cbegin(), message.Callbacks.cend(), prio, [](const BaseMessageCallback* cb, float p) { return cb->Priority < p; }),
			new MessageCallback<Args...>(
				cId,
				prio,
				func
				));
	}
	inline void MessageSystem::unrequestMessage(ComponentId cId, MessageId mId)
	{
		if (mMessages.count(mId) == 0)
			return;

		auto& message = mMessages[mId];
		auto it = std::find_if(message.Callbacks.cbegin(), message.Callbacks.cend(), [cId](const BaseMessageCallback* cb) { return cb->Component == cId; });
		if (it == message.Callbacks.end())
			return;

		delete *it;
		message.Callbacks.erase(it);
	}

	inline void MessageSystem::reprioritizeMessage(ComponentId cId, MessageId mId, float prio)
	{
		if (mMessages.count(mId) == 0)
			return;

		auto& message = mMessages[mId];
		auto it = std::find_if(message.Callbacks.cbegin(), message.Callbacks.cend(), [cId](const BaseMessageCallback* cb) { return cb->Component == cId; });
		if (it == message.Callbacks.end())
			return;

		message.Callbacks.erase(it);

		auto newPos = std::lower_bound(message.Callbacks.cbegin(), message.Callbacks.cend(), prio, [](const BaseMessageCallback* cb, float p) { return cb->Priority < p; });
		message.Callbacks.insert(newPos, *it);
	}
	template<typename... Args>
	inline void MessageSystem::sendMessage(MessageId mId, Args&&... args) const
	{
		if (mMessages.count(mId) == 0)
			return;

		auto& message = mMessages.at(mId);
		if (message.Locality & Message_Global)
		{
			auto copy = message.Callbacks;

			for (auto& cb : copy)
				static_cast<MessageCallback<Args...>*>(cb)->Func(std::forward<Args>(args)...);
		}
	}
	template<typename... Args>
	inline void MessageSystem::sendMessageTo(MessageId mId, ComponentId cId, Args&&... args) const
	{
		if (mMessages.count(mId) == 0)
			return;

		auto& message = mMessages.at(mId);
		if (message.Locality & Message_Local)
		{
			auto it = std::find_if(message.Callbacks.cbegin(), message.Callbacks.cend(), [cId](const BaseMessageCallback* cb) { return cb->Component == cId; });

			if (it == message.Callbacks.cend())
				return;

			static_cast<MessageCallback<Args...>*>(*it)->Func(std::forward<Args>(args)...);
		}
	}
	template<typename... Args>
	inline void MessageSystem::sendMessageTo(MessageId mId, EntityId eId, Args&&... args) const
	{
		if (mMessages.count(mId) == 0)
			return;

		auto& message = mMessages.at(mId);
		if (message.Locality & Message_Local)
		{
			for (auto& cb : message.Callbacks)
			{
				if (!mES->isAttached(cb->Component, eId))
					continue;

				static_cast<MessageCallback<Args...>*>(cb)->Func(std::forward<Args>(args)...);
			}
		}
	}
}
