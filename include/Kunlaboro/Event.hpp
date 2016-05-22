#pragma once

#include <cstdlib>

namespace Kunlaboro
{

	class BaseEvent
	{
	protected:
		static std::size_t sEventCounter;
	};

	template<typename T>
	class Event
	{
	public:
		static std::size_t getId();
	};

}
