#pragma once

#include <cstdint>

namespace Kunlaboro
{

	template<typename idType, typename indexType, uint8_t IndexBits, typename generationType, uint8_t GenerationBits>
	class BaseEntityId
	{
	public:
		typedef idType IdType;
		typedef indexType IndexType;
		typedef generationType GenerationType;

		static_assert(IndexBits <= sizeof(indexType) * 8, "Index bit count is larger than index type can store");
		static_assert(GenerationBits <= sizeof(generationType) * 8, "Generation bit count is larger than generation type can store");

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

		BaseEntityId() : mId(~idType(0)) { }
		BaseEntityId(indexType index, generationType generation)
			: mId(static_cast<idType>(index & sIndexMask) | (static_cast<idType>(generation & sGenerationMask) << sIndexBits))
		{ }

		explicit BaseEntityId(idType fullId) : mId(fullId) { }

		inline bool operator==(const BaseEntityId& other) const { return mId == other.mId; }
		inline bool operator!=(const BaseEntityId& other) const { return mId != other.mId; }
		inline bool operator<(const BaseEntityId& other) const { return mId < other.mId; }

		inline indexType getIndex() const { return static_cast<indexType>(mId & sIndexMask); }
		inline generationType getGeneration() const { return static_cast<generationType>((mId >> sIndexBits) & sGenerationMask); }

	private:
		friend class EntitySystem;

		idType mId;
	};

	template<typename idType, typename indexType, uint8_t IndexBits, typename generationType, uint8_t GenerationBits, typename familyType, uint8_t FamilyBits>
	class BaseComponentId
	{
	public:
		typedef idType IdType;
		typedef indexType IndexType;
		typedef generationType GenerationType;
		typedef familyType FamilyType;

		static_assert(IndexBits <= sizeof(indexType) * 8, "Index bit count is larger than index type can store");
		static_assert(GenerationBits <= sizeof(generationType) * 8, "Generation bit count is larger than generation type can store");
		static_assert(FamilyBits <= sizeof(familyType) * 8, "Family bit count is larger than family type can store");

	private:
		enum : uint8_t
		{
			sIndexBits = IndexBits,
			sGenerationBits = GenerationBits,
			sFamilyBits = FamilyBits
		};
		enum : idType
		{
			sIndexMask = (idType(1) << sIndexBits) - 1,
			sGenerationMask = (idType(1) << sGenerationBits) - 1,
			sFamilyMask = (idType(1) << sFamilyBits) - 1
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
		enum : familyType
		{
			sMaxFamily = familyType(sFamilyMask)
		};

		static_assert(sIndexBits + sGenerationBits + sFamilyBits == sizeof(idType) * 8, "Id bit counts must add up to the total Id size.");

		BaseComponentId() : mId(~idType(0)) { }
		static BaseComponentId Invalid()
		{
			return BaseComponentId(~indexType(0), ~generationType(0), ~familyType(0));
		}
		BaseComponentId(indexType index, generationType generation, familyType family)
			: mId(static_cast<idType>(index & sIndexMask) |
				(static_cast<idType>(generation & sGenerationMask) << sIndexBits) |
				(static_cast<idType>(family & sFamilyMask) << (sIndexBits + sGenerationBits)))
		{ }

		explicit BaseComponentId(idType fullId) : mId(fullId) { }

		inline bool operator==(const BaseComponentId& other) const { return mId == other.mId; }
		inline bool operator!=(const BaseComponentId& other) const { return mId != other.mId; }
		inline bool operator<(const BaseComponentId& other) const { return mId < other.mId; }

		inline indexType getIndex() const { return static_cast<indexType>(mId & sIndexMask); }
		inline generationType getGeneration() const { return static_cast<generationType>((mId >> sIndexBits) & sGenerationMask); }
		inline familyType getFamily() const { return static_cast<familyType>((mId >> (sIndexBits + sGenerationBits)) & sFamilyMask); }

	private:
		friend class EntitySystem;

		idType mId;
};

#if 0
	typedef BaseEntityId<uint64_t, uint32_t, 32, uint32_t, 32> EntityId;
	typedef BaseComponentId<uint64_t, uint32_t, 32, uint32_t, 24, uint8_t, 8> ComponentId;
#else
	typedef BaseEntityId<uint32_t, uint32_t, 20, uint16_t, 12> EntityId;
	typedef BaseComponentId<uint32_t, uint32_t, 21, uint8_t, 5, uint8_t, 6> ComponentId;
#endif

	static_assert(sizeof(EntityId) == sizeof(EntityId::IdType), "EntityId has extra padding, this might break things");
	static_assert(sizeof(ComponentId) == sizeof(ComponentId::IdType), "ComponentId has extra padding, this might break things");

}
