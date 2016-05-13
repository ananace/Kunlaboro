#include <Kunlaboro/detail/ComponentPool.hpp>

using namespace Kunlaboro::detail;

BaseComponentPool::BaseComponentPool(size_t componentSize, size_t chunkSize)
	: mComponentSize(componentSize)
	, mChunkSize(chunkSize)
	, mSize(0)
	, mCapacity(0)
{

}
BaseComponentPool::~BaseComponentPool()
{
	for (auto& block : mBlocks)
		delete[] block;
}

void BaseComponentPool::ensure(size_t count)
{
	if (count < mSize)
		return;

	if (count > mCapacity)
		resize(count);

	mSize = count;
}
void BaseComponentPool::resize(size_t count)
{
	while (mCapacity < count)
	{
		auto* chunk = new uint8_t[mComponentSize * mChunkSize];
		mBlocks.push_back(chunk);

		mCapacity += mChunkSize;
	}

	mBits.resize(size_t(std::ceil(mCapacity / 64.f)), 0);
}

bool BaseComponentPool::hasBit(size_t index) const
{
	if (mBits.size() < index / 64)
		return false;

	return (mBits[index / 64] & (1ull << (index % 64))) != 0;
}
void BaseComponentPool::setBit(size_t index)
{
	if (mBits.size() < index / 64ul)
		return;

	mBits[index / 64] |= (1ull << (index % 64));
}
void BaseComponentPool::resetBit(size_t index)
{
	if (mBits.size() < index / 64ul)
		return;

	mBits[index / 64] &= ~(1ull << (index % 64));
}