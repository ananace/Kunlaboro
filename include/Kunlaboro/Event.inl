#pragma once

#include "Event.hpp"

namespace Kunlaboro
{

	template<typename T>
	static std::size_t Event::getId()
	{
		static std::size_t sId = sFamilyCounter++;
		return sId;
	}

}
