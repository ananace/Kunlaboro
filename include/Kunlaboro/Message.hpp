#pragma once

#include <functional>

namespace Kunlaboro
{

	class BaseMessage
	{
	public:

	};

	template<typename... Args>
	class Message : public BaseMessage
	{
	public:
		template<typename Functor>
		Message(Functor&& func);
		template<typename Functor>
		Message(const Functor& func);

		void call(Args... args);

	private:
		std::function<void(Args...)> mFunc;
	};

}
