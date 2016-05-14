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
	
	SECTION("simple POD component")
	{
		auto component = es.componentCreate<TestComponent>();

		REQUIRE(component->getEntitySystem() == &es);
		REQUIRE(es.getComponent(component->getId()) == component);
		REQUIRE(es.componentAlive(component->getId()));
		REQUIRE(component.getRefCount() == 1);

		SECTION("ref counting")
		{
			auto copy = component;
			REQUIRE(component.getRefCount() == 2);
			REQUIRE(copy == component);

			copy.release();
			copy.unlink();

			REQUIRE(component.getRefCount() == 1);
			REQUIRE(copy == component);
		}

		SECTION("cleanup")
		{
			component.release();

			REQUIRE(component->getEntitySystem() == &es);
			REQUIRE(es.getComponent(component->getId()) != component);
			REQUIRE(!es.componentAlive(component->getId()));
			REQUIRE(component.getRefCount() == 0);
		}

		SECTION("non-trivially construction")
		{
			auto component = es.componentCreate<TestComponent>(42);

			REQUIRE(component->getData() == 42);
		}

		SECTION("copy construction")
		{
			auto copy = es.componentCreate<TestComponent>(*component);

			CHECK(copy != component);
			CHECK(copy->getData() == component->getData());
		}

		SECTION("low-level message passing")
		{
			component = es.componentCreate<TestComponent>(42);
			CHECK(component->getData() == 42);

			TestComponent::Message newData(1);

			component->onMessage(&newData);
			CHECK(component->getData() == 1);

			newData.NewData = 23;
			es.componentSendMessage(component->getId(), &newData);

			CHECK(component->getData() == 23);
		}
	}
}

TEST_CASE("component iteration", "[component][view]")
{
	Kunlaboro::EntitySystem es;

	std::vector<Kunlaboro::ComponentHandle<TestComponent>> components;
	components.reserve(10);
	for (int i = 0; i < 10; ++i)
		components.push_back(es.componentCreate<TestComponent>(i));

	auto collection = es.components<TestComponent>();

	SECTION("range-based for")
	{
		int count = 0;
		for (auto& comp : collection)
		{
			REQUIRE(comp.getData() == count);
			++count;
		}

		REQUIRE(count == 10);
	}

	SECTION("view forEach")
	{
		int combinedValue = 0;
		collection.forEach([&combinedValue](TestComponent& comp) { combinedValue += comp.getData(); });

		REQUIRE(combinedValue == 45);
	}

	SECTION("view where forEach")
	{
		collection.where([](const TestComponent& comp) { return comp.getData() < 5; });

		int combinedValue = 0;
		collection.forEach([&combinedValue](TestComponent& comp) { combinedValue += comp.getData(); });

		REQUIRE(combinedValue == 10);
	}

	SECTION("view where range-based")
	{
		collection.where([](const TestComponent& comp) { return comp.getData() > 5; });

		int combinedValue = 0;
		for (auto& comp : collection)
			combinedValue += comp.getData();

		REQUIRE(combinedValue == 30);
	}
}
