#pragma once 

#include <vector>
#include <cmath>
#include <cstdint>

namespace Kunlaboro
{

	namespace detail
	{

		/** Dynamic size bitfield.
		 *
		 * \todo Look into moving out of API.
		 */
		class DynamicBitfield
		{
		public:
			DynamicBitfield();
			~DynamicBitfield() = default;

			void clear();
			inline void ensure(std::size_t bit)
			{
				const auto count = bit + 1;
				const std::size_t bytes = static_cast<std::size_t>(std::ceil(count / 64.f));

				if (mCapacity < bytes)
				{
					mBits.resize(bytes, 0);
					mCapacity = bytes;
				}

				if (mSize < count)
					mSize = count;
			}
			inline void shrink()
			{
				// TODO: Shrink size properly
				while (mSize > 0 && !hasBit(mSize - 1))
					--mSize;
			}

			inline std::size_t getSize() const { return mSize; }
			std::size_t countBits() const;

			bool operator==(const DynamicBitfield& rhs) const;
			bool operator!=(const DynamicBitfield& rhs) const;

			inline bool hasBit(std::size_t bit) const { return mSize > bit && (mBits[bit / 64] & (1ull << (bit % 64))) != 0; }
			inline void setBit(std::size_t bit) { ensure(bit); mBits[bit / 64] |= (1ull << (bit % 64)); }
			inline void clearBit(std::size_t bit) {
				if (mSize < bit)
					return;

				mBits[bit / 64] &= ~(1ull << (bit % 64));
				shrink();
			}

		private:
			std::vector<std::uint64_t> mBits;
			std::size_t mSize, mCapacity;
		};

	}

}
