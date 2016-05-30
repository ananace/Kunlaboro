#include <Kunlaboro/Component.inl>
#include <Kunlaboro/Entity.inl>
#include <Kunlaboro/EntitySystem.inl>
#include <Kunlaboro/EventSystem.inl>
#include <Kunlaboro/Views.inl>
#include "catch.hpp"

class TestComponent : public Kunlaboro::Component
{
public:
	TestComponent()
		: mData(-1)
	{

	}

	TestComponent(int data)
		: mData(data)
	{

	}

	int getData() const { return mData; }

private:
	int mData;
};

TEST_CASE("entity creation", "[entity]")
{
	Kunlaboro::EntitySystem es;

	int i = 0;
	auto func = [&i](const Kunlaboro::EntitySystem::EntityCreatedEvent& ev) {
		++i;
	};
	es.getEventSystem().eventRegister<Kunlaboro::EntitySystem::EntityCreatedEvent>(func);

	auto ent = es.entityCreate();
	ent.addComponent<TestComponent>(42);

	REQUIRE(ent.hasComponent<TestComponent>());
	REQUIRE(i == 1);

	auto comp = ent.getComponent<TestComponent>();

	REQUIRE(comp->getData() == 42);
}
