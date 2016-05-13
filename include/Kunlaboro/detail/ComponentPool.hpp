#pragma once

#include <vector>

namespace Kunlaboro
{

	namespace detail
	{
		
		class BaseComponentPool
		{
		public:
			BaseComponentPool(size_t componentSize, size_t chunkSize = 256);
			virtual ~BaseComponentPool();

			inline size_t getSize() const { return mSize; }
			inline size_t getComponentSize() const { return mComponentSize; }
			inline size_t getChunkSize() const { return mChunkSize; }

			void ensure(size_t count);
			void resize(size_t count);

			bool hasBit(size_t index) const;
			void setBit(size_t index);
			void resetBit(size_t index);

			inline void* getData(size_t index) {
				return mBlocks[index / mChunkSize] + (index % mChunkSize) * mComponentSize;
			}
			inline const void* getData(size_t index) const {
				return mBlocks[index / mChunkSize] + (index % mChunkSize) * mComponentSize;
			}
			virtual void destroy(size_t index) = 0;

		private:
			std::vector<uint8_t*> mBlocks;
			std::vector<uint64_t> mBits;
			size_t mComponentSize, mChunkSize, mSize, mCapacity;
		};


		template<typename T, size_t ChunkSize = 256>
		class ComponentPool : public BaseComponentPool
		{
		public:
			ComponentPool()
				: BaseComponentPool(sizeof(T), ChunkSize)
			{ }
			virtual ~ComponentPool() { }

			virtual void destroy(size_t index) override
			{
				static_cast<T*>(getData(index))->~T();
			}
		};

	}

}
