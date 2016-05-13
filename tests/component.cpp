#include <Kunlaboro/EntitySystem.inl>

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

SCENARIO("Creating a component")
{
	Kunlaboro::EntitySystem es;
	
	GIVEN("A simple POD component")
	{
		WHEN("The component is trivially constructed")
		{
			auto component = es.componentCreate<TestComponent>();
			auto copy = component;

			THEN("The created component is contained successfully in the entity system")
			{
				CHECK(component->getEntitySystem() == &es);
				CHECK(es.getComponent(component->getId()) == component);
				CHECK(es.componentAlive(component->getId()));

				AND_THEN("The created component has its default constructor called appropriately")
				{
					CHECK(component->getData() == -1);
				}
			}
		}
		WHEN("The component is non-trivially constructed")
		{
			auto component = es.componentCreate<TestComponent>(42);

			THEN("The created component has its non-trivial constructor called appropriately")
			{
				CHECK(component->getData() == 42);
			}
		}

		THEN("The component can receive messages correctly")
		{
			auto component = es.componentCreate<TestComponent>(42);
			CHECK(component->getData() == 42);

			TestComponent::Message newData(1);

			component->onMessage(&newData);
			CHECK(component->getData() == 1);

			newData.NewData = 23;
			es.sendMessage(component->getId(), &newData);

			CHECK(component->getData() == 23);
		}
	}
}

SCENARIO("Creating several components")
{
	Kunlaboro::EntitySystem es;

	GIVEN("A simple POD component")
	{

	}
}
