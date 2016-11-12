#pragma once

#include "ID.hpp"
#include "Message.hpp"
#include "detail/Delegate.hpp"

#include <unordered_map>
#include <deque>

namespace Kunlaboro
{

	/// \todo Look into moving out of public API, only necessary for internal MessageSystem hashing.
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

	class EntitySystem;

	/** Message passing system
	 *
	 * \note Uses hashed strings as message IDs
	 * \todo Do collision testing on debug builds
	 */
	class MessageSystem
	{
	public:
		/** The locality of the message.
		 *
		 * This is used to differentiate between global and local
		 * message requests, as well as ones that should accept
		 * both global as well as local messages.
		 */
		enum MessageLocality : uint8_t
		{
			/// The message is only accepted when sent globally.
			Message_Global = 1 << 0,
			/// The message is only accepted when sent locally.
			Message_Local  = 1 << 1,
			/// Accept both global and local messages.
			Message_Either = Message_Global | Message_Local
		};

		MessageSystem(const MessageSystem&) = delete;
		MessageSystem(MessageSystem&&) = delete;
		~MessageSystem();

		MessageSystem& operator=(const MessageSystem&) = delete;

		/** Register a message with the given arguments and name.
		 *
		 * \tparam Args The arguments for the message.
		 * \param name The name of the message.
		 *
		 * \todo Hash and add possible debug-time call argument checking,
		 *       to prevent passing messages with invalid arguments.
		 */
		template<typename... Args>
		void registerMessage(const char* const name, MessageLocality locality = Message_Either);

		/** Add a request for messages with the given name and arguments.
		 *
		 * \tparam Args The arguments of the message.
		 * \param cId The ID of the component to receive the message.
		 * \param message The name of the message to request.
		 * \param func The functor to be called when the message is received.
		 * \param prio The priority of the request, in ascending order.
		 *
		 * \todo Check that the arguments match the ones the message
		 *       was originally registered with.
		 * \todo Add distinction between global and local messages.
		 */
		template<typename... Args, typename Functor>
		void requestMessage(ComponentId cId, const char* const message, Functor&& func, float prio = 0);
		/** Remove a request for messages with the given name.
		 *
		 * \param cId The ID of the component that posted the original request.
		 * \param message The name of the message that was requested.
		 */
		void unrequestMessage(ComponentId cId, const char* const message);

		/** Change the request priority for the message with the given name.
		 *
		 * \param cId The ID of the component that posted the original request.
		 * \param message The name of the message that was requested.
		 * \param prio The new priority of the message request.
		 *
		 * \todo Reduce performance impact of this functionality, preferrably
		 *       it should be zero-copy.
		 */
		void reprioritizeMessage(ComponentId cId, const char* const message, float prio);

		/** Send a global message with the given name and arguments.
		 *
		 * \param message The name of the message to send.
		 * \param args The arguments of the message to send.
		 *
		 * \todo Check that the message can accept the given arguments,
		 *       preferrably through run/compile-time switch.
		 */
		template<typename... Args>
		void sendMessage(const char* const message, Args&&... args) const;
		/** Send a local message to the given component, with name and arguments.
		 *
		 * \param message The name of the message to send.
		 * \param cId The ID of the component to receive the message.
		 * \param args The arguments of the message.
		 *
		 * \todo Check that the message can accept the given arguments,
		 *       preferrably through run/compile-time switch.
		 */
		template<typename... Args>
		void sendMessageTo(const char* const message, ComponentId cId, Args&&... args) const;

		// TODO:
		// template<typename... Args>
		// void messageSendIdTo(const char* const message, EntityId eId, Args... args) const;

		/** Register a message with the given ID and arguments.
		 *
		 * \tparam Args The arguments for the message.
		 * \param mId The ID of the message to register.
		 * \param name The name of the message, used for debugging if provided.
		 */
		template<typename... Args>
		void registerMessage(MessageId mId, const char* const name = nullptr, MessageLocality locality = Message_Either);

		/** Request a message by ID.
		 *
		 * \tparam Args The arguments of the message.
		 * \param cId The ID of the component to receive the message.
		 * \param mId The ID of the message to request.
		 * \param func The functor to call upon receiving the message.
		 * \param prio The priority of the request, in ascending order.
		 *
		 * \todo Check that the message can accept the given arguments,
		 *       preferrably through run/compile-time switch.
		 */
		template<typename... Args, typename Functor>
		void requestMessage(ComponentId cId, MessageId mId, Functor&& func, float prio = 0);
		/** Remove a request for a message by ID.
		 *
		 * \param cId The ID of the component that posted the original request.
		 * \param mId The ID of the message that was requested.
		 */
		void unrequestMessage(ComponentId cId, MessageId mId);
		/** Change the request priority for the message with the given ID.
		 *
		 * \param cId The ID of the component that posted the original request.
		 * \param mId The ID of the message that was requested.
		 * \param prio The new priority of the message request.
		 *
		 * \todo Reduce performance impact of this functionality, preferrably
		 *       it should be zero-copy.
		 */
		void reprioritizeMessage(ComponentId cId, MessageId mId, float prio);

		/** Send a global message with the given ID and arguments.
		 *
		 * \param mId The ID of the message to send.
		 * \param args The arguments of the message to send.
		 *
		 * \todo Check that the message can accept the given arguments,
		 *       preferrably through run/compile-time switch.
		 */
		template<typename... Args>
		void sendMessage(MessageId mId, Args&&... args) const;
		/** Send a local message with the given ID and arguments.
		 *
		 * \param mId The ID of the message to send.
		 * \param cId The ID of the component to receive the message.
		 * \param args The arguments of the message to send.
		 *
		 * \todo Check that the message can accept the given arguments,
		 *       preferrably through run/compile-time switch.
		 */
		template<typename... Args>
		void sendMessageTo(MessageId mId, ComponentId cId, Args&&... args) const;
		// TODO:
		// template<typename... Args>
		// void messageSendTo(MessageId mId, EntityId eId, Args... args) const;

		/** Remove all messages requested by the given component ID.
		 *
		 * \param cId The ID of the component to remove all requests from.
		 */
		void unrequestAllMessages(ComponentId cId);

		/** Hash a message string into a MessageId at compile time.
		 *
		 * \param msg The c-string to hash
		 */
		inline static constexpr MessageId hash(const char* const msg)
		{
			return detail::hash_func<MessageId>::hash(msg);
		}
		/** Hash a message string into a MessageId at runtime.
		 *
		 * \param msg The c++ stl string to hash
		 */
		inline static MessageId hash(const std::string& msg)
		{
			return detail::hash_func<MessageId>::hashF(msg.c_str(), msg.size());
		}

	private:
		MessageSystem(EntitySystem* es);

		friend class EntitySystem;

		EntitySystem* mES;

		struct BaseMessageCallback
		{
			BaseMessageCallback(ComponentId cId, float p)
				: Component(cId)
				, Priority(p)
			{ }

			ComponentId Component;
			float Priority;

			inline bool operator<(const BaseMessageCallback& cb) const
			{
				return Priority < cb.Priority;
			}
		};
		template<typename... Args>
		struct MessageCallback : public BaseMessageCallback
		{
			MessageCallback(ComponentId cId, float p, detail::Delegate<void(Args...)>&& func)
				: BaseMessageCallback(cId, p)
				, Func(func)
			{ }

			detail::Delegate<void(Args...)> Func;
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

		struct MessageData
		{
			MessageLocality Locality;
			BaseMessageType* Type;

			std::deque<BaseMessageCallback*> Callbacks;
		};
		std::unordered_map<MessageId, MessageData> mMessages;
	};

}
