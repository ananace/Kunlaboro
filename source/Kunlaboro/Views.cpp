#include <Kunlaboro/Views.hpp>
#include <Kunlaboro/Views.inl>

using namespace Kunlaboro;

bool impl::matchBitfield(const detail::DynamicBitfield& entity, const detail::DynamicBitfield& bitField, MatchType match)
{
	for (std::size_t i = 0; i < bitField.getSize(); ++i)
	{
		if (!bitField.hasBit(i))
			continue;

		if (match == Match_Any && entity.hasBit(i))
			return true;
		else if (match == Match_All && !entity.hasBit(i))
			return false;
	}

	return match == Match_All;
}

EntityView::EntityView(const EntitySystem& es)
	: BaseView<EntityView, Entity>(&es)
{

}

EntityView::Iterator EntityView::begin()
{
	return Iterator(mES, 0, mPred);
}
EntityView::Iterator EntityView::end()
{
	auto& list = mES->entityGetList();
	return Iterator(mES, list.size(), mPred);
}

void EntityView::forEach(const Function& func)
{
	auto* queue = impl::BaseView<EntityView, Entity>::mQueue;

	auto& list = mES->entityGetList();

	for (size_t i = 0; i < list.size(); ++i)
	{
		auto& entData = list[i];
		EntityId eid(i, entData.Generation);

		Entity ent(const_cast<EntitySystem*>(mES), eid);
		if (BaseView<EntityView, Entity>::mES->isAlive(eid) && (!mPred || mPred(ent)))
		{
			if (queue)
				queue->submit([func, ent]() { func(ent); });
			else
				func(ent);
		}
	}

	if (queue)
		queue->wait();
}

EntityView::Iterator::Iterator(const EntitySystem* sys, EntityId::IndexType index, const Predicate& pred)
	: impl::BaseIterator<Iterator, Entity>(sys, index, pred)
{
	Iterator::nextStep();
}

bool EntityView::Iterator::basePred() const
{
	return mCurEntity.isValid();
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
