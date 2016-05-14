#include <Kunlaboro/Views.hpp>
#include <Kunlaboro/Views.inl>

using namespace Kunlaboro;

EntityView::EntityView(const EntitySystem* es)
	: BaseView<EntityView, Entity>(es)
{

}

void EntityView::forEach(const Function& func)
{
	auto& list = mES->entityGetList();

	for (size_t i = 0; i < list.size(); ++i)
	{
		auto& entData = list[i];
		EntityId eid(i, entData.Generation);

		Entity ent(const_cast<EntitySystem*>(mES), eid);
		if (mES->entityAlive(eid) && !mPred || mPred(ent))
			func(ent);
	}
}

EntityView::Iterator::Iterator(EntitySystem* sys, EntityId::IndexType index)
	: BaseView<EntityView, Entity>::Iterator<Iterator>(sys, index, mPred)
{

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