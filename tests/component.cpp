#include <Kunlaboro/EntitySystem.inl>
#include <Kunlaboro/Component.inl>
#include <Kunlaboro/Views.inl>

#include "catch.hpp"

class TestComponent : public Kunlaboro::Component
{
public:
	struct Message : public BaseMessage
	{
		static constexpr uint16_t Id = 0x0532;

		Message()
			: BaseMessage{ Id }
			, NewData(0)
		{}
		Message(int data)
			: BaseMessage{ Id }
			, NewData(data)
		{}

		int NewData;
	};

	TestComponent()
		: mData(-1)
	{

	}

	TestComponent(int data)
		: mData(data)
	{

	}

	int getData() const { return mData; }

	virtual void onMessage(BaseMessage* msg) override
	{
		if (msg->MessageId == Message::Id)
			mData = static_cast<Message*>(msg)->NewData;
	}

private:
	int mData;
};

TEST_CASE("Component handling", "[component]")
{
	Kunlaboro::EntitySystem es;
	auto component = es.componentCreate<TestComponent>();

	SECTION("Component creation")
	{
		REQUIRE(component->getEntitySystem() == &es);
		REQUIRE(es.getComponent(component->getId()) == component);
		REQUIRE(es.componentAlive(component->getId()));
		REQUIRE(component.getRefCount() == 1);
		REQUIRE(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 1);
	}

	SECTION("Ref counting")
	{
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
		REQUIRE(!es.componentAlive(id));
		CHECK(component->getId() == id);

		CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 0);
	}

	SECTION("Component destruction")
	{
		es.componentDestroy(component->getId());

		CHECK(component->getEntitySystem() == &es);
		REQUIRE(es.getComponent(component->getId()) != component);
		REQUIRE(!es.componentAlive(component->getId()));
		CHECK(component.getRefCount() == 0);
		REQUIRE(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 0);
	}

	SECTION("Non-trivial construction")
	{
		auto component = es.componentCreate<TestComponent>(42);

		REQUIRE(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 2);

		CHECK(component->getEntitySystem() == &es);
		CHECK(es.getComponent(component->getId()) == component);
		CHECK(es.componentAlive(component->getId()));
		CHECK(component.getRefCount() == 1);

		REQUIRE(component->getData() == 42);
	}

	SECTION("Non-trivial copy construction")
	{
		auto copy = es.componentCreate<TestComponent>(*component);

		REQUIRE(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 2);

		CHECK(copy->getEntitySystem() == &es);
		CHECK(es.getComponent(copy->getId()) == copy);
		CHECK(es.componentAlive(copy->getId()));
		CHECK(copy.getRefCount() == 1);

		REQUIRE(copy != component);
		REQUIRE(copy->getData() == component->getData());
	}

	SECTION("Low-level message handling")
	{
		REQUIRE(es.componentAlive(component->getId()));
		CHECK(component->getData() == -1);

		TestComponent::Message newData(1);

		component->onMessage(&newData);
		REQUIRE(component->getData() == 1);

		newData.NewData = 23;
		es.componentSendMessage(component->getId(), &newData);

		REQUIRE(component->getData() == 23);
	}
}

TEST_CASE("Component views", "[component][view]")
{
	Kunlaboro::EntitySystem es;

	std::vector<Kunlaboro::ComponentHandle<TestComponent>> components;
	components.reserve(10);
	for (int i = 0; i < 10; ++i)
		components.push_back(es.componentCreate<TestComponent>(i));

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
		collection
			.where([](const TestComponent& comp) { return comp.getData() < 5; })
			.forEach([&combinedValue](TestComponent& comp) { combinedValue += comp.getData(); });

		REQUIRE(combinedValue == 10);
	}
}
