#include <Kunlaboro/Views.hpp>
#include <Kunlaboro/Views.inl>

using namespace Kunlaboro;

bool impl::matchBitfield(const detail::DynamicBitfield& entity, const detail::DynamicBitfield& bitField, EntityView::MatchType match)
{
	if (match == EntityView::Match_All)
		return entity == bitField;

	for (std::size_t i = 0; i < bitField.getSize(); ++i)
	{
		if (!bitField.hasBit(i))
			continue;

		if (entity.hasBit(i))
			return true;
	}

	return false;
}

EntityView::EntityView(const EntitySystem* es)
	: BaseView<EntityView, Entity>(es)
{

}

EntityView::Iterator EntityView::begin() const
{
	return Iterator(mES, 0, mPred, mBitField, mMatchType);
}
EntityView::Iterator EntityView::end() const
{
	auto& list = mES->entityGetList();
	return Iterator(mES, list.size(), mPred, mBitField, mMatchType);
}

void EntityView::forEach(const Function& func)
{
	auto& list = mES->entityGetList();

	for (size_t i = 0; i < list.size(); ++i)
	{
		auto& entData = list[i];
		EntityId eid(i, entData.Generation);

		Entity ent(const_cast<EntitySystem*>(mES), eid);
		if (BaseView<EntityView, Entity>::mES->entityAlive(eid) && (!mPred || mPred(ent)))
			func(ent);
	}
}

EntityView::Iterator::Iterator(const EntitySystem* sys, EntityId::IndexType index, const Predicate& pred, const detail::DynamicBitfield& bits, MatchType match)
	: BaseView<EntityView, Entity>::BaseIterator<Iterator>(sys, index, pred)
	, mBitField(bits)
	, mMatchType(match)
{
	nextStep();
}

bool EntityView::Iterator::basePred() const
{
	return mCurEntity.isValid() && (mBitField.getSize() == 0 || impl::matchBitfield(mES->entityGetList()[mCurEntity.getId().getIndex()].ComponentBits, mBitField, mMatchType));
}

void EntityView::Iterator::moveNext()
{
	auto& list = mES->entityGetList();

	if (list.size() > mIndex)
	{
		EntityId eid(static_cast<EntityId::IndexType>(mIndex), list[static_cast<std::size_t>(mIndex)].Generation);
		mCurEntity = Entity(const_cast<EntitySystem*>(mES), eid);
	}
	else
		mCurEntity = Entity();
}
uint64_t EntityView::Iterator::maxLength() const
{
	return mES->entityGetList().size();
}
