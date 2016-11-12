#pragma once

#include <cstdint>

namespace Kunlaboro
{

	/** Defines an entity ID.
	 *
	 * Entities are indexed based on two variables - to help uniqueness.
	 * \li Index The index part is the position in memory where the entity is created.
	 * \li Generation The generation part exists to allow reuse of small index values while
	 *                still generating unique entity IDs.
	 *
	 * \note Depending on the generation width, entity IDs may become non-unique
	 *       after reuse. If ID uniqueness is not an issue then generation width
	 *       may be set to 1.
	 * \todo Allow for 0-width generation values, when uniqueness is a non-issue.
	 *
	 * \tparam idType
	 * \parblock
	 * The underlying ID type, recommended to be an unsigned integral.
	 *
	 * (Guaranteed valid types are; uint8_t, uint16_t, uint32_t, uint64_t)
	 * \endparblock
	 * \tparam indexType A type wide enough to contain the index part of the ID.
	 * \tparam IndexBits The width of the index part of the ID, must fit in the indexType.
	 * \tparam generationType A type wide enough to contain the generation part of the ID.
	 * \tparam GenerationBits The with of the generation part of the ID, must fit in the generationType.
	 *
	 * \note Total bit count of index and generation must match the number of bits in the underlying type.
	 */
	template<typename idType, typename indexType, uint8_t IndexBits, typename generationType, uint8_t GenerationBits>
	class BaseEntityId
	{
	public:
		/// The type of the underlying ID.
		typedef idType IdType;
		/// A type wide enough to contain the index part of the ID.
		typedef indexType IndexType;
		/// A type wide enough to contain the generation part of the ID.
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
			/// The maximum index value of the ID.
			sMaxIndex = indexType(sIndexMask)
		};
		enum : generationType
		{
			/// The maximum generation value of the ID.
			sMaxGeneration = generationType(sGenerationMask)
		};

		static_assert(sIndexBits + sGenerationBits == sizeof(idType) * 8, "Id bit counts must add up to the total Id size.");

		BaseEntityId() { }
		/** Create an ID from an index and a generation.
		 *
		 * \param index The index of the ID.
		 * \param generation The generation of the ID.
		 */
		BaseEntityId(indexType index, generationType generation)
			: mId(static_cast<idType>(index & sIndexMask) | (static_cast<idType>(generation & sGenerationMask) << sIndexBits))
		{ }
		/// Gets a reference to the the invalid entity ID.
		static const BaseEntityId& Invalid()
		{
			static BaseEntityId invalid(~indexType(0), ~generationType(0));
			return invalid;
		}

		inline bool operator==(const BaseEntityId& other) const { return mId == other.mId; }
		inline bool operator!=(const BaseEntityId& other) const { return mId != other.mId; }
		inline bool operator<(const BaseEntityId& other) const { return mId < other.mId; }

		/// Gets the index part of the ID.
		inline indexType getIndex() const { return static_cast<indexType>(mId & sIndexMask); }
		/// Gets the generation part of the ID.
		inline generationType getGeneration() const { return static_cast<generationType>((mId >> sIndexBits) & sGenerationMask); }

	private:
		friend class EntitySystem;

		idType mId;
	};

	/** Defines a component ID.
	 *
	 * Components are indexed based off of three variables;
	 * \li Family The family is a 0-indexed type identifier, defining the maximum number of different
	 *            component types that can be in use at the same time.
	 * \li Generation The generation is an increasing value used to help keep the uniquness of IDs.
	 * \li Index The index is a lookup value for the position in the memory where the component lays.
	 *
	 * \note Depending on the generation width, generation IDs may become non-unique
	 *       after reuse. If ID uniqueness is not an issue then generation width
	 *       may be set to 1.
	 * \todo Allow for 0-width generation values, when uniqueness is a non-issue.
	 *
	 * \tparam idType
	 * \parblock
	 * The underlying ID type, recommended to be an unsigned integral.
	 *
	 * (Guaranteed valid types are; uint8_t, uint16_t, uint32_t, uint64_t)
	 * \endparblock
	 * \tparam indexType A type wide enough to contain the index part of the ID.
	 * \tparam IndexBits The width of the index part of the ID, must fit in the indexType.
	 * \tparam generationType A type wide enough to contain the generation part of the ID.
	 * \tparam GenerationBits The with of the generation part of the ID, must fit in the generationType.
	 * \tparam familyType A type wide enough to contain the family part of the ID.
	 * \tparam FamilyBits The with of the family part of the ID, must fit in the generationType.
	 *
	 * \note Total bit count of index, generation, and family must match the number of bits in the underlying type.
	 */
	template<typename idType, typename indexType, uint8_t IndexBits, typename generationType, uint8_t GenerationBits, typename familyType, uint8_t FamilyBits>
	class BaseComponentId
	{
	public:
		/// The underlying ID type.
		typedef idType IdType;
		/// A type wide enough to contain the index part of the ID.
		typedef indexType IndexType;
		/// A type wide enough to contain the generation part of the ID.
		typedef generationType GenerationType;
		/// A type wide enough to contain the family part of the ID.
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
			/// The maximum index value allowed by the index width.
			sMaxIndex = indexType(sIndexMask)
		};
		enum : generationType
		{
			/// The maximum generation value allowed by the generation width.
			sMaxGeneration = generationType(sGenerationMask)
		};
		enum : familyType
		{
			/// The maximum family value allowed by the family width.
			sMaxFamily = familyType(sFamilyMask)
		};

		static_assert(sIndexBits + sGenerationBits + sFamilyBits == sizeof(idType) * 8, "Id bit counts must add up to the total Id size.");

		BaseComponentId() { }
		/** Creates a component ID based on index, generation, and family.
		 *
		 * \param index The index part of the ID.
		 * \param generation The generation part of the ID.
		 * \param family The family part of the ID.
		 */
		BaseComponentId(indexType index, generationType generation, familyType family)
			: mId(static_cast<idType>(index & sIndexMask) |
				(static_cast<idType>(generation & sGenerationMask) << sIndexBits) |
				(static_cast<idType>(family & sFamilyMask) << (sIndexBits + sGenerationBits)))
		{ }
		/// Gets a reference to the invalid component ID.
		static const BaseComponentId& Invalid()
		{
			static BaseComponentId invalid(~indexType(0), ~generationType(0), ~familyType(0));
			return invalid;
		}

		inline bool operator==(const BaseComponentId& other) const { return mId == other.mId; }
		inline bool operator!=(const BaseComponentId& other) const { return mId != other.mId; }
		inline bool operator<(const BaseComponentId& other) const { return mId < other.mId; }

		/// Gets the index part of the ID.
		inline indexType getIndex() const { return static_cast<indexType>(mId & sIndexMask); }
		/// Gets the generation part of the ID.
		inline generationType getGeneration() const { return static_cast<generationType>((mId >> sIndexBits) & sGenerationMask); }
		/// Gets the family part of the ID.
		inline familyType getFamily() const { return static_cast<familyType>((mId >> (sIndexBits + sGenerationBits)) & sFamilyMask); }

	private:
		friend class EntitySystem;

		idType mId;
};

#if !defined(KUNLABORO_32BIT)
	/** The default entity ID type.
	*
	* \note 32 bits for index, meaning ~4.3 billion entities possible.
	* \note 32 bits for generation, meaning ~4.3 billion entity slots can be reused before IDs become non-unique.
	*/
	typedef BaseEntityId<uint64_t, uint32_t, 32, uint32_t, 32> EntityId;
	/** The default component ID type.
	*
	* \note 32 bits for index, meaning ~4.3 billion components possible.
	* \note 24 bits for generation, meaning ~16.7 million component slots can be reused before component IDs become non-unique.
	* \note 8 bits for family, meaning 256 distinct component types can exist.
	*/
	typedef BaseComponentId<uint64_t, uint32_t, 32, uint32_t, 24, uint8_t, 8> ComponentId;
#else
	/** The default entity ID type.
	 *
	 * \note 20 bits for index, meaning ~1 million entities possible.
	 * \note 12 bits for generation, meaning 4096 entity slots can be reused before IDs become non-unique.
	 */
	typedef BaseEntityId<uint32_t, uint32_t, 20, uint16_t, 12> EntityId;
	/** The default component ID type.
	 *
	 * \note 21 bits for index, meaning ~2 million components possible.
	 * \note 5 bits for generation, meaning 32 component slots can be reused before component IDs become non-unique.
	 * \note 6 bits for family, meaning 64 distinct component types can exist.
	 */
	typedef BaseComponentId<uint32_t, uint32_t, 21, uint8_t, 5, uint8_t, 6> ComponentId;
#endif

	static_assert(sizeof(EntityId) == sizeof(EntityId::IdType), "EntityId has extra padding, this might break things");
	static_assert(sizeof(ComponentId) == sizeof(ComponentId::IdType), "ComponentId has extra padding, this might break things");

}
