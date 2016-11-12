#include <Kunlaboro/EntitySystem.inl>
#include <Kunlaboro/Component.inl>
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

TEST_CASE("Component handling", "[component]")
{
	Kunlaboro::EntitySystem es;

	SECTION("Component creation")
	{
		auto component = es.createComponent<TestComponent>();

		REQUIRE(component->getEntitySystem() == &es);
		REQUIRE(es.getComponent(component->getId()) == component);
		REQUIRE(es.isAlive(component->getId()));
		REQUIRE(component.getRefCount() == 1);
		REQUIRE(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 1);
	}

	SECTION("Ref counting")
	{
		auto component = es.createComponent<TestComponent>();

		{
			auto copy = component;

			REQUIRE(component.getRefCount() == 2);
			REQUIRE(copy == component);

			copy.release();

			REQUIRE(component.getRefCount() == 1);
			CHECK(copy == component);

			copy.unlink();

			CHECK(component.getRefCount() == 1);
			CHECK(copy == component);
		}

		REQUIRE(component.getRefCount() == 1);

		{
			auto copy = component;

			REQUIRE(component.getRefCount() == 2);
			REQUIRE(copy == component);

			copy.unlink();

			REQUIRE(copy == component);
		}

		CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 1);
		REQUIRE(component.getRefCount() == 2);

		auto id = component->getId();

		component.release();
		component.release();

		REQUIRE(component.getRefCount() == 0);
		REQUIRE(!es.isAlive(id));
		CHECK(component->getId() == id);

		CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 0);
	}

	SECTION("Component destruction")
	{
		auto component = es.createComponent<TestComponent>();
		es.destroyComponent(component->getId());

		CHECK(component->getEntitySystem() == &es);
		REQUIRE(es.getComponent(component->getId()) != component);
		REQUIRE(!es.isAlive(component->getId()));
		CHECK(component.getRefCount() == 0);
		REQUIRE(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 0);
	}

	SECTION("Non-trivial construction")
	{
		auto component = es.createComponent<TestComponent>(42);
		auto component2 = es.createComponent<TestComponent>(23);

		REQUIRE(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 2);

		CHECK(component->getEntitySystem() == &es);
		CHECK(es.getComponent(component->getId()) == component);
		CHECK(es.isAlive(component->getId()));
		CHECK(component.getRefCount() == 1);

		REQUIRE(component->getData() == 42);
		REQUIRE(component2->getData() == 23);
	}

	SECTION("Non-trivial copy construction")
	{
		auto component = es.createComponent<TestComponent>();
		auto copy = es.createComponent<TestComponent>(*component);

		REQUIRE(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 2);

		CHECK(copy->getEntitySystem() == &es);
		CHECK(es.getComponent(copy->getId()) == copy);
		CHECK(es.isAlive(copy->getId()));
		CHECK(copy.getRefCount() == 1);

		REQUIRE(copy != component);
		REQUIRE(copy->getData() == component->getData());
	}
}

TEST_CASE("Component views", "[component][view]")
{
	Kunlaboro::EntitySystem es;

	std::vector<Kunlaboro::ComponentHandle<TestComponent>> components;
	components.reserve(10);
	for (int i = 0; i < 10; ++i)
		components.push_back(es.createComponent<TestComponent>(i));

	CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 10);

	auto collection = Kunlaboro::ComponentView<TestComponent>(es);

	SECTION("View iteration with range-based for")
	{
		int count = 0;
		for (auto& comp : collection)
		{
			REQUIRE(comp.getData() == count);
			++count;
		}

		REQUIRE(count == 10);
	}

	SECTION("View iteration with predicated range-based for")
	{
		int combinedValue = 0;
		for (auto& comp : collection.where([](const TestComponent& comp) { return comp.getData() > 5; }))
			combinedValue += comp.getData();

		REQUIRE(combinedValue == 30);
	}

	SECTION("View iteration with forEach")
	{
		int combinedValue = 0;
		collection.forEach([&combinedValue](TestComponent& comp) { combinedValue += comp.getData(); });

		REQUIRE(combinedValue == 45);
	}

	SECTION("View iteration with predicated forEach")
	{
		int combinedValue = 0;
		collection.where([](const TestComponent& comp) { return comp.getData() < 5; })
		          .forEach([&combinedValue](TestComponent& comp) { combinedValue += comp.getData(); });

		REQUIRE(combinedValue == 10);
	}
}
