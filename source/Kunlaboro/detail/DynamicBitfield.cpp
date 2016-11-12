#include <Kunlaboro/detail/DynamicBitfield.hpp>
#include <algorithm>

using namespace Kunlaboro::detail;

namespace
{
#if defined _MSC_VER
#define popcount __popcnt
#elif defined __GNUC__
#define popcount __builtin_popcount
#else

	// SWAR
	unsigned popcount(unsigned i)
	{
		i = i - ((i >> 1) & 0x55555555);
		i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
		return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
	}

#endif
}

DynamicBitfield::DynamicBitfield()
	: mSize(0)
	, mCapacity(0)
{

}

void DynamicBitfield::clear()
{
	mBits.clear();
	mSize = mCapacity = 0;
}

std::size_t DynamicBitfield::countBits() const
{
	const std::size_t bytes = static_cast<std::size_t>(std::ceil(mSize / 64.f));
	std::size_t count = 0;
	for (std::size_t i = 0; i < bytes; ++i)
	{
		count += popcount(static_cast<uint32_t>(mBits[i])) + popcount(static_cast<uint32_t>(mBits[i] >> 32));
	}

	return count;
}

bool DynamicBitfield::operator==(const DynamicBitfield& rhs) const
{
	const size_t larger = std::max(mCapacity, rhs.mCapacity);
	const size_t lS = mCapacity;
	const size_t rS = rhs.mCapacity;
	const uint64_t* lB = mBits.data();
	const uint64_t* rB = rhs.mBits.data();

	for (size_t i = 0; i < larger; ++i)
	{
		uint64_t res = (i > lS ? 0ull : lB[i]) ^ (i > rS ? 0ull : rB[i]);
		if (res > 0)
			return false;
	}

	return true;
}
bool DynamicBitfield::operator!=(const DynamicBitfield& rhs) const
{
	return !(*this == rhs);
}
