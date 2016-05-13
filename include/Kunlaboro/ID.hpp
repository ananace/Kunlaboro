#pragma once

#include <cstdint>

namespace Kunlaboro
{

	template<typename idType, typename indexType, uint8_t IndexBits, typename generationType, uint8_t GenerationBits>
	class Id
	{
	public:
		typedef idType IdType;
		typedef indexType IndexType;
		typedef generationType GenerationType;

		static_assert(IndexBits <= sizeof(indexType) * 8, "Index bit count is larger than index type can store");
		static_assert(GenerationBits <= sizeof(generationType) * 8, "Generation bit count is larger than generation type can store");

		static const Id INVALID;

	private:
		enum : uint8_t
		{
			sIndexBits = IndexBits,
			sGenerationBits = GenerationBits
		};
		enum : idType
		{
			sIndexMask = (idType(1) << sIndexBits) - 1,
			sGenerationMask = (idType(1) << sGenerationBits) - 1
		};

	public:
		enum : indexType
		{
			sMaxIndex = indexType(sIndexMask)
		};
		enum : generationType
		{
			sMaxGeneration = generationType(sGenerationMask)
		};

		static_assert(sIndexBits + sGenerationBits == sizeof(idType) * 8, "Id bit counts must add up to the total Id size.");

		Id() : mId(0) { }
		Id(indexType index, generationType generation)
			: mId(static_cast<idType>(index & sIndexMask) | (static_cast<idType>(generation & sGenerationMask) << sIndexBits))
		{ }

		explicit Id(idType fullId) : mId(fullId) { }

		bool operator==(const Id& other) const { return mId == other.mId; }
		bool operator!=(const Id& other) const { return mId != other.mId; }
		bool operator<(const Id& other) const { return mId < other.mId; }

		indexType getIndex() const { return static_cast<indexType>(mId & sIndexMask); }
		generationType getGeneration() const { return static_cast<generationType>((mId >> sIndexBits) & sGenerationMask); }

	private:
		friend class EntitySystem;

		idType mId;
	};

#if 0
	typedef Id<uint64_t, uint32_t, 32, uint32_t, 32> EntityId;
	typedef Id<uint64_t, uint32_t, 32, uint32_t, 32> ComponentId;
#else
	typedef Id<uint32_t, uint32_t, 20, uint16_t, 12> EntityId;
	typedef Id<uint32_t, uint32_t, 18, uint16_t, 14> ComponentId;
#endif

	static_assert(sizeof(EntityId) == sizeof(EntityId::IdType), "EntityId has extra padding, this might break things");
	static_assert(sizeof(ComponentId) == sizeof(ComponentId::IdType), "ComponentId has extra padding, this might break things");

}
