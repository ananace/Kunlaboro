#include <Kunlaboro/detail/DynamicBitfield.hpp>
#include <algorithm>

using namespace Kunlaboro::detail;

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

void DynamicBitfield::ensure(std::size_t bit)
{
	const auto count = bit + 1;
	const std::size_t bytes = static_cast<std::size_t>(std::ceil(count / 64.f));

	if (mCapacity < bytes)
	{
		mBits.resize(bytes, 0);
		mCapacity = bytes;
	}

	mSize = count;
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
