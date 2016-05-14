#include <Kunlaboro/detail/ComponentPool.hpp>
#include <cmath>

using namespace Kunlaboro::detail;
using std::size_t;

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
	mBits.ensure(count);
}
void BaseComponentPool::resize(size_t count)
{
	while (mCapacity < count)
	{
		auto* chunk = new uint8_t[mComponentSize * mChunkSize];
		mBlocks.push_back(chunk);

		mCapacity += mChunkSize;
	}
}
