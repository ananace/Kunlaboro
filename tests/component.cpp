#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>
#include "catch.hpp"

class TestComponent : public Kunlaboro::Component
{
public:
    TestComponent() : Kunlaboro::Component("TestComponent") { }

    void addedToEntity()
    {
        requestMessage("GlobalTestMessage", [&](Kunlaboro::Message& msg) { msg.handle(314159); });
        requestMessage("LocalTestMessage", [&](Kunlaboro::Message& msg) { msg.handle(std::string("Hello World")); }, true);
    }
};

class TestComponent2 : public Kunlaboro::Component
{
public:
    TestComponent2() : Kunlaboro::Component("TestComponent2") { }

    void addedToEntity()
    {
        requestMessage("GlobalTestMessage", [&](Kunlaboro::Message& msg) { msg.handle(1234); });
        changeRequestPriority("GlobalTestMessage", 1);

        requestMessage("LocalTestMessage", [&](Kunlaboro::Message& msg) { msg.handle(std::string("Goodbye World")); }, true);
    }
};

SCENARIO("Creating a component")
{
    Kunlaboro::EntitySystem es;
    es.registerComponent<TestComponent>("TestComponent");

    GIVEN("A manually created component")
    {
        TestComponent* component = new TestComponent();

        THEN("The component has a correct name")
        {
            CHECK(component->getName() == "TestComponent");
        }
        THEN("The component is in an invalid state")
        {
            CHECK(!component->isValid());
        }
        THEN("The component is not part of any entity system")
        {
            CHECK((component->getEntitySystem() == nullptr));
        }

        delete component;
    }

    GIVEN("A correctly created component")
    {
        WHEN("Created by itself")
        {
            Kunlaboro::Component* component = es.createComponent("TestComponent");

            THEN("The component has correct name")
            {
                CHECK(component->getName() == "TestComponent");
            }
            THEN("The component is of the correct type")
            {
                CHECK((dynamic_cast<TestComponent*>(component) != nullptr));
            }
            THEN("The component is in an invalid state")
            {
                CHECK(!component->isValid());
            }
            THEN("The component is unowned")
            {
                CHECK(component->getOwnerId() == 0);
            }
        }

        WHEN("Created and stored in an entity")
        {
            Kunlaboro::Component* component = es.createComponent("TestComponent");

            auto eid = es.createEntity();
            es.addComponent(eid, component);

            THEN("The component is in a valid state")
            {
                CHECK(component->isValid());
            }
            THEN("The component has a valid owner")
            {
                CHECK(component->getOwnerId() > 0);
            }
        }
    }
}

SCENARIO("Message passing")
{
    Kunlaboro::EntitySystem es;
    es.registerComponent<TestComponent>("TestComponent");
    es.registerComponent<TestComponent2>("TestComponent2");

    GIVEN("A single entity containing a single test component")
    {
        auto eid = es.createEntity();
        es.addComponent(eid, "TestComponent");

        WHEN("A global message is sent")
        {
            Kunlaboro::Message msg;

            es.sendGlobalMessage(es.getMessageRequestId(Kunlaboro::Reason_Message, "GlobalTestMessage"), msg);

            THEN("The message is handled correctly")
            {
                CHECK(msg.handled);
                CHECK(boost::any_cast<int>(msg.payload) == 314159);
            }
        }

        WHEN("A local message is sent to the entity")
        {
            Kunlaboro::Message msg;

            es.sendLocalMessage(eid, es.getMessageRequestId(Kunlaboro::Reason_Message, "LocalTestMessage"), msg);

            THEN("The message is handled correctly")
            {
                CHECK(msg.handled);
                CHECK(boost::any_cast<std::string>(msg.payload) == "Hello World");
            }
        }

        WHEN("A local message is sent globally")
        {
            Kunlaboro::Message msg;

            es.sendGlobalMessage(es.getMessageRequestId(Kunlaboro::Reason_Message, "LocalTestMessage"), msg);

            THEN("The message is left unhandled")
            {
                CHECK(!msg.handled);
            }
        }

        WHEN("A global message is sent locally to the entity")
        {
            Kunlaboro::Message msg;

            es.sendLocalMessage(eid, es.getMessageRequestId(Kunlaboro::Reason_Message, "GlobalTestMessage"), msg);

            THEN("The message is left unhandled")
            {
                CHECK(!msg.handled);
            }
        }
    }

    GIVEN("A single entity containing two test components")
    {
        auto eid = es.createEntity();

        auto testComponent = es.createComponent("TestComponent");
        auto testComponent2 = es.createComponent("TestComponent2");

        es.addComponent(eid, testComponent);
        es.addComponent(eid, testComponent2);

        WHEN("A global message is sent")
        {
            Kunlaboro::Message msg;

            es.sendGlobalMessage(es.getMessageRequestId(Kunlaboro::Reason_Message, "GlobalTestMessage"), msg);

            THEN("The message is handled correctly")
            {
                CHECK(msg.handled);
            }
        }

        WHEN("The priority is changed")
        {
            testComponent2->changeRequestPriority("GlobalTestMessage", -1);

            Kunlaboro::Message msg;

            es.sendGlobalMessage(es.getMessageRequestId(Kunlaboro::Reason_Message, "GlobalTestMessage"), msg);

            THEN("The message is recieved by the correct component")
            {
                CHECK(msg.handled);
                CHECK((msg.sender == testComponent2));
            }
        }
    }
}
