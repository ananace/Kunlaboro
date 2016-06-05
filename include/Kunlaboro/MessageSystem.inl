#pragma once

#include "MessageSystem.hpp"

#include <algorithm>

namespace Kunlaboro
{
	template<typename... Args>
	inline void MessageSystem::messageRegisterId(const char* const name)
	{
		auto mId = BaseMessageType::Hash(name);
		messageRegister<Args...>(mId);

#ifdef _DEBUG
		mMessages[mId].Type->Name = name;
#endif
	}

	template<typename... Args, typename Functor>
	inline void MessageSystem::messageRequestId(ComponentId cId, const char* const message, Functor&& func, float prio)
	{
		auto mId = BaseMessageType::Hash(message);
		messageRequest<Args...>(cId, mId, std::forward<Functor&&>(func), prio);
	}
	inline void MessageSystem::messageUnrequestId(ComponentId cId, const char* const message)
	{
		auto mId = BaseMessageType::Hash(message);
		messageUnrequest(cId, mId);
	}
	inline void MessageSystem::messageReprioritizeId(ComponentId cId, const char* const message, float prio)
	{
		auto mId = BaseMessageType::Hash(message);
		messageReprioritize(cId, mId, prio);
	}
	template<typename... Args>
	inline void MessageSystem::messageSendId(const char* const message, Args... args) const
	{
		auto mId = BaseMessageType::Hash(message);
		messageSend<Args...>(mId, std::forward<Args>(args)...);
	}
	template<typename... Args>
	inline void MessageSystem::messageSendIdTo(const char* const message, ComponentId cId, Args... args) const
	{
		auto mId = BaseMessageType::Hash(message);
		messageSendTo<Args...>(mId, cId, std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void MessageSystem::messageRegister(MessageId mId, const char* const name)
	{
		if (mMessages.count(mId) > 0)
			return;

		mMessages[mId].Type = new MessageType<Args...>(
			mId
		);

#ifdef _DEBUG
		if (name)
			mMessages[mId].Type->Name = name;
#endif
	}

	template<typename... Args, typename Functor>
	inline void MessageSystem::messageRequest(ComponentId cId, MessageId mId, Functor&& func, float prio)
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
	inline void MessageSystem::messageUnrequest(ComponentId cId, MessageId mId)
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

	inline void MessageSystem::messageReprioritize(ComponentId cId, MessageId mId, float prio)
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
	inline void MessageSystem::messageSend(MessageId mId, Args... args) const
	{
		if (mMessages.count(mId) == 0)
			return;

		auto& message = mMessages.at(mId);
		auto copy = message.Callbacks;

		for (auto& cb : copy)
			static_cast<MessageCallback<Args...>*>(cb)->Func(std::forward<Args>(args)...);
	}
	template<typename... Args>
	inline void MessageSystem::messageSendTo(MessageId mId, ComponentId cId, Args... args) const
	{
		if (mMessages.count(mId) == 0)
			return;

		auto& message = mMessages.at(mId);
		auto it = std::find_if(message.Callbacks.cbegin(), message.Callbacks.cend(), [cId](const BaseMessageCallback* cb) { return cb->Component == cId; });

		if (it == message.Callbacks.cend())
			return;

		static_cast<MessageCallback<Args...>*>(*it)->Func(std::forward<Args>(args)...);
	}
}
