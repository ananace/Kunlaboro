#pragma once

#include "Component.hpp"
#include <functional>

namespace Kunlaboro
{

	namespace detail
	{
		/** Compile-time capable implementation of the Fowler-Null-Vo string hash method.
		 *
		 * Implemented both for 32-bit and 64-bit hashes.
		 */
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
			/** Hash a C-string using the Fowler-Null-Vo hashing method.
			 *
			 * \param string The string to hash
			 * \param val The default FNV offset
			 */
			constexpr static inline S hash(const char* const string, const S val = hash_internal<S>::default_offset)
			{
				return (string[0] == 0) ? val : hash(string + 1, (val * hash_internal<S>::prime) ^ S(string[0]));
			}
			/** Hash a fixed-length string using the Fowler-Null-Vo hashing method.
			 *
			 * \param string The string to hash
			 * \param strlen The length of the string to hash
			 * \param val The default FNV offset
			 */
			static inline S hashF(const char* const string, const size_t strlen, const S val = hash_internal<S>::default_offset)
			{
				return (strlen == 0) ? val : hashF(string + 1, strlen - 1, (val * hash_internal<S>::prime) ^ S(string[0]));
			}
		};
	}

	class MessageSystem;
	typedef uint32_t MessageId;

	/** Helper component for handling messaging.
	 */
	class MessagingComponent : public Component
	{
	public:
		MessagingComponent();

	protected:
		/** This function is called every time the component is added to an entity.
		 *
		 * In here you want to request the messages you want to receive during the
		 * lifetime of your attachment to the current entity.
		 *
		 * Example of how such functionality can be done is;
		 * \code{.cpp}
		 * void Example::addedToEntity()
		 * {
		 *   requestMessageId<float>("System.Tick", &Example::Tick);
		 *   requestMessageId("System.Reset", &Example::Reset);
		 *   requestMessageId<std::string&>("System.Display", &Example::Display, priority);
		 *   // Can also be changed later on through
		 *   // reprioritizeMessageId("System.Display", priority);
		 * }
		 *
		 * void Example::Tick(float dt)
		 * {
		 *   // Run an update tick
		 * }
		 * void Example::Reset()
		 * {
		 *   // Reset the system state
		 * }
		 * void Example::Display(std::string& out)
		 * {
		 *   // Append state to output variable
		 * }
		 * \endcode
		 */
		virtual void addedToEntity() = 0;

		/** Request to receive messages with the given ID.
		 *
		 * \param id The ID of the message to receive.
		 * \param func The functor to receive the message.
		 * \param prio The priority of the request, in ascending order.
		 * \tparam Args The message arguments, must match the functor input.
		 */
		template<typename... Args, typename Functor>
		void requestMessage(MessageId id, Functor&& func, float prio = 0);
		/** Helper method for requesting against a member method.
		 *
		 * \param id The ID of the message to receive.
		 * \param func The member method to receive the message.
		 * \param prio The priority of the request, in ascending order.
		 * \tparam Args The message arguments, must match the method arguments.
		 */
		template<typename... Args, typename Obj>
		void requestMessage(MessageId id, void (Obj::*func)(Args...), float prio = 0);
		/** Unrequest the message with the given ID.
		 */
		void unrequestMessage(MessageId id);
		/** Change the priority of the message with the given ID.
		 *
		 * \todo Look into possible performance increases.
		 */
		void reprioritizeMessage(MessageId id, float prio);

		/** Sends a global message with the given ID.
		 *
		 * \param id The ID of the message to send.
		 * \param args The message arguments.
		 */
		template<typename... Args>
		void sendMessage(MessageId id, Args... args) const;
		/** Sends a local message with the given ID to the given entity ID.
		 *
		 * \param id The ID of the message to send.
		 * \param ent The ID of the entity to receive the message.
		 * \param args The message arguments.
		 */
		template<typename... Args>
		void sendMessageTo(MessageId id, EntityId ent, Args... args) const;
		/** Sends a local message with the given ID to the given component ID.
		 *
		 * \param id The ID of the message to send.
		 * \param comp The ID of the component to receive the message.
		 * \param args The message arguments.
		 */
		template<typename... Args>
		void sendMessageTo(MessageId id, ComponentId comp, Args... args) const;

		/** Helper method to request a message from an unhashed string.
		 *
		 * \param id The ID string of the message to request.
		 * \param func The functor to receive the call.
		 * \param prio The priority of the request, in ascending order.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		template<typename... Args, typename Functor>
		void requestMessageId(const char* const id, Functor&& func, float prio = 0);
		/** Helper method to request a message from an unhashed string.
		 *
		 * \param id The ID string of the message to request.
		 * \param func The member function to receive the call.
		 * \param prio The priority of the request, in ascending order.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		template<typename... Args, typename Obj>
		void requestMessageId(const char* const id, void (Obj::*func)(Args...), float prio = 0);
		/** Helper method to unrequest a message from an unhashed string.
		 *
		 * \param id The ID string of the message to unrequest.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		void unrequestMessageId(const char* const id);
		/** Helper method to change message priority from an unhashed string.
		 *
		 * \param id The ID string of the message to reprioritize.
		 * \param prio The new priority of the request, in ascending order.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		void reprioritizeMessageId(const char* const id, float prio);

		/** Helper method to send a global message by an unhashed string.
		 *
		 * \param id The ID string of the message to send.
		 * \param args The arguments of the message.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		template<typename... Args>
		void sendMessageId(const char* const id, Args... args) const;
		/** Helper method to send a entity-local message by an unhashed string.
		 *
		 * \param id The ID string of the message to send.
		 * \param ent The ID of the entity to receive the message.
		 * \param args The arguments of the message.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		template<typename... Args>
		void sendMessageIdTo(const char* const id, EntityId ent, Args... args) const;
		/** Helper method to send a component-local message by an unhashed string.
		 *
		 * \param id The ID string of the message to send.
		 * \param comp The ID of the component to receive the message.
		 * \param args The arguments of the message.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		template<typename... Args>
		void sendMessageIdTo(const char* const id, ComponentId comp, Args... args) const;
	};

	/** Helper struct for storing registered messages.
	 *
	 */
	struct BaseMessageType
	{
		BaseMessageType(MessageId mId)
			: Id(mId)
		{ }

		/// The ID of the message.
		MessageId Id;
#ifdef _DEBUG
		/// The unhashed name of the message, for Debug-time collision checking.
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
		/** Checks that the given functor can be called with the message arguments.
		 */
		template<typename Functor>
		static constexpr bool isValid(Functor&&) { return can_call<Functor>{}; }

		MessageType(MessageId mId)
			: BaseMessageType(mId)
		{ }
	};

}
