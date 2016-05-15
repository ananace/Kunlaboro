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

TEST_CASE("component creation", "[component]")
{
	Kunlaboro::EntitySystem es;
	auto component = es.componentCreate<TestComponent>();

	CHECK(component->getEntitySystem() == &es);
	CHECK(es.getComponent(component->getId()) == component);
	CHECK(es.componentAlive(component->getId()));
	CHECK(component.getRefCount() == 1);
	CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 1);

	SECTION("ref counted handles")
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

	SECTION("component cleanup")
	{
		component.release();

		REQUIRE(component->getEntitySystem() == &es);
		REQUIRE(es.getComponent(component->getId()) != component);
		REQUIRE(!es.componentAlive(component->getId()));
		REQUIRE(component.getRefCount() == 0);
		REQUIRE(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 0);
	}

	SECTION("non-trivial construction")
	{
		auto component = es.componentCreate<TestComponent>(42);

		REQUIRE(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 2);

		CHECK(component->getEntitySystem() == &es);
		CHECK(es.getComponent(component->getId()) == component);
		CHECK(es.componentAlive(component->getId()));
		CHECK(component.getRefCount() == 1);

		REQUIRE(component->getData() == 42);
	}

	SECTION("copy construction")
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

	SECTION("low-level message passing")
	{
		CHECK(es.componentAlive(component->getId()));
		CHECK(component->getData() == -1);

		TestComponent::Message newData(1);

		component->onMessage(&newData);
		REQUIRE(component->getData() == 1);

		newData.NewData = 23;
		es.componentSendMessage(component->getId(), &newData);

		REQUIRE(component->getData() == 23);
	}
}

TEST_CASE("component iteration", "[component][view]")
{
	Kunlaboro::EntitySystem es;

	std::vector<Kunlaboro::ComponentHandle<TestComponent>> components;
	components.reserve(10);
	for (int i = 0; i < 10; ++i)
		components.push_back(es.componentCreate<TestComponent>(i));

	CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<TestComponent>::getFamily()).countBits() == 10);

	auto collection = Kunlaboro::ComponentView<TestComponent>(es);

	SECTION("view iteration with range-based for")
	{
		int count = 0;
		for (auto& comp : collection)
		{
			REQUIRE(comp.getData() == count);
			++count;
		}

		REQUIRE(count == 10);
	}

	SECTION("view iteration with predicated range-based for")
	{
		int combinedValue = 0;
		for (auto& comp : collection.where([](const TestComponent& comp) { return comp.getData() > 5; }))
			combinedValue += comp.getData();

		REQUIRE(combinedValue == 30);
	}

	SECTION("view iteration with forEach")
	{
		int combinedValue = 0;
		collection.forEach([&combinedValue](TestComponent& comp) { combinedValue += comp.getData(); });

		REQUIRE(combinedValue == 45);
	}

	SECTION("view iteration with predicated forEach")
	{
		int combinedValue = 0;
		collection
			.where([](const TestComponent& comp) { return comp.getData() < 5; })
			.forEach([&combinedValue](TestComponent& comp) { combinedValue += comp.getData(); });

		REQUIRE(combinedValue == 10);
	}
}
