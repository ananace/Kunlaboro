#pragma once

#include "Component.hpp"
#include <functional>

namespace Kunlaboro
{

	namespace detail
	{
		template<typename S>
		struct hash_internal;

		template<>
		struct hash_internal<uint64_t>
		{
			constexpr static uint64_t default_offset = 14695981039346656037ULL;
			constexpr static uint64_t prime = 1099511628211ULL;
		};
		template<>
		struct hash_internal<uint32_t>
		{
			constexpr static uint32_t default_offset = 0x811C9DC5;
			constexpr static uint32_t prime = 0x01000193;
		};

		template<typename S>
		struct hash_func : public hash_internal<S>
		{
			constexpr static inline S hash(const char* const string, const S val = hash_internal<S>::default_offset)
			{
				return (string[0] == 0) ? val : hash(string + 1, (val * hash_internal<S>::prime) ^ S(string[0]));
			}
			static inline S hashF(const char* const string, const size_t strlen, const S val = hash_internal<S>::default_offset)
			{
				return (strlen == 0) ? val : hashF(string + 1, strlen - 1, (val * hash_internal<S>::prime) ^ S(string[0]));
			}
		};
	}

	class MessageSystem;
	typedef uint32_t MessageId;

	class MessagingComponent : public Component
	{
	public:
		MessagingComponent();

	protected:
		virtual void addedToEntity() = 0;

		template<typename... Args, typename Functor>
		void requestMessage(MessageId id, Functor&& func, float prio = 0);
		template<typename... Args, typename Obj>
		void requestMessage(MessageId id, void (Obj::*func)(Args...), float prio = 0);
		void unrequestMessage(MessageId id);
		void reprioritizeMessage(MessageId id, float prio);

		template<typename... Args>
		void sendMessage(MessageId id, Args... args) const;
		template<typename... Args>
		void sendMessageTo(MessageId id, EntityId ent, Args... args) const;
		template<typename... Args>
		void sendMessageTo(MessageId id, ComponentId comp, Args... args) const;

		template<typename... Args, typename Functor>
		void requestMessageId(const char* const id, Functor&& func, float prio = 0);
		template<typename... Args, typename Obj>
		void requestMessageId(const char* const id, void (Obj::*func)(Args...), float prio = 0);
		void unrequestMessageId(const char* const id);
		void reprioritizeMessageId(const char* const id, float prio);

		template<typename... Args>
		void sendMessageId(const char* const id, Args... args) const;
		template<typename... Args>
		void sendMessageIdTo(const char* const id, EntityId ent, Args... args) const;
		template<typename... Args>
		void sendMessageIdTo(const char* const id, ComponentId comp, Args... args) const;
	};

	struct BaseMessageType
	{
		BaseMessageType(MessageId mId)
			: Id(mId)
		{ }

		MessageId Id;
#ifdef _DEBUG
		std::string Name;
#endif

	public:
		static constexpr MessageId Hash(const char* const msg)
		{
			return detail::hash_func<MessageId>::hash(msg);
		}
		static MessageId Hash(const std::string& msg)
		{
			return detail::hash_func<MessageId>::hashF(msg.c_str(), msg.size());
		}
	};

	template<typename... Args>
	struct MessageType : public BaseMessageType
	{
	private:
		struct can_call_test
		{
			template<typename F>
			static decltype(std::declval<F>()(std::declval<Args>()...), std::true_type())
				f(int);

			template<typename F>
			static std::false_type
				f(...);
		};

		template<typename F>
		using can_call = decltype(can_call_test::template f<F>(0));

	public:
		template<typename Functor>
		static constexpr bool isValid(Functor&&) { return can_call<Functor>{}; }

		MessageType(MessageId mId)
			: BaseMessageType(mId)
		{ }
	};

}
