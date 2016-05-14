#pragma once

#include <cstdint>
#include <vector>

#include "DynamicBitfield.hpp"

namespace Kunlaboro
{

	namespace detail
	{
		
		class BaseComponentPool
		{
		public:
			BaseComponentPool(std::size_t componentSize, std::size_t chunkSize = 256);
			virtual ~BaseComponentPool();

			inline std::size_t getSize() const { return mSize; }
			inline std::size_t getComponentSize() const { return mComponentSize; }
			inline std::size_t getChunkSize() const { return mChunkSize; }

			void ensure(std::size_t count);
			void resize(std::size_t count);

			inline bool hasBit(std::size_t index) const { return mBits.hasBit(index); }
			inline void setBit(std::size_t index) { mBits.setBit(index); }
			inline void resetBit(std::size_t index) { mBits.clearBit(index); }

			inline void* getData(std::size_t index) {
				return mBlocks[index / mChunkSize] + (index % mChunkSize) * mComponentSize;
			}
			inline const void* getData(std::size_t index) const {
				return mBlocks[index / mChunkSize] + (index % mChunkSize) * mComponentSize;
			}
			virtual void destroy(std::size_t index) = 0;

		private:
			std::vector<uint8_t*> mBlocks;
			DynamicBitfield mBits;
			std::size_t mComponentSize, mChunkSize, mSize, mCapacity;
		};


		template<typename T>
		class ComponentPool : public BaseComponentPool
		{
		public:
			ComponentPool()
				: BaseComponentPool(sizeof(T), T::sPreferredChunkSize)
			{ }
			virtual ~ComponentPool() { }

			virtual void destroy(std::size_t index) override
			{
				static_cast<T*>(getData(index))->~T();
			}
		};

	}

}
