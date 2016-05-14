#pragma once 

#include <vector>
#include <cstdint>

namespace Kunlaboro
{

	namespace detail
	{

		class DynamicBitfield
		{
		public:
			DynamicBitfield();
			~DynamicBitfield() = default;

			void clear();
			void ensure(std::size_t count);
			inline std::size_t getSize() const { return mSize; }

			bool operator==(const DynamicBitfield& rhs) const;
			bool operator!=(const DynamicBitfield& rhs) const;

			inline bool hasBit(std::size_t bit) const { return (mBits[bit / 64] & (1ull << (bit % 64))) != 0; }
			inline void setBit(std::size_t bit) { ensure(bit); mBits[bit / 64] |= (1ull << (bit % 64)); }
			inline void clearBit(std::size_t bit) { ensure(bit); mBits[bit / 64] &= ~(1ull << (bit % 64)); }

		private:
			std::vector<std::uint64_t> mBits;
			std::size_t mSize, mCapacity;
		};

	}

}