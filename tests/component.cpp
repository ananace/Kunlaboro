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

            es.sendGlobalMessage(Kunlaboro::hashRequest("GlobalTestMessage"), msg);

            THEN("The message is handled correctly")
            {
                CHECK(msg.Handled);
                CHECK(boost::any_cast<int>(msg.Data) == 314159);
            }
        }

        WHEN("A local message is sent to the entity")
        {
            Kunlaboro::Message msg;

            es.sendLocalMessage(eid, Kunlaboro::hashRequest("LocalTestMessage"), msg);

            THEN("The message is handled correctly")
            {
                CHECK(msg.Handled);
                CHECK(boost::any_cast<std::string>(msg.Data) == "Hello World");
            }
        }

        WHEN("A local message is sent globally")
        {
            Kunlaboro::Message msg;

            es.sendGlobalMessage(Kunlaboro::hashRequest("LocalTestMessage"), msg);

            THEN("The message is left unhandled")
            {
                CHECK(!msg.Handled);
            }
        }

        WHEN("A global message is sent locally to the entity")
        {
            Kunlaboro::Message msg;

            es.sendLocalMessage(eid, Kunlaboro::hashRequest("GlobalTestMessage"), msg);

            THEN("The message is left unhandled")
            {
                CHECK(!msg.Handled);
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

            es.sendGlobalMessage(Kunlaboro::hashRequest("GlobalTestMessage"), msg);

            THEN("The message is handled correctly")
            {
                CHECK(msg.Handled);
            }
        }

        WHEN("The priority is changed")
        {
            testComponent2->changeRequestPriority("GlobalTestMessage", -1);

            Kunlaboro::Message msg;

            es.sendGlobalMessage(Kunlaboro::hashRequest("GlobalTestMessage"), msg);

            THEN("The message is recieved by the correct component")
            {
                CHECK(msg.Handled);
                CHECK((msg.Sender == testComponent2));
            }
        }
    }
}

SCENARIO("Requests changing during calls")
{
    Kunlaboro::EntitySystem es;
    es.registerComponent<TestComponent>("TestComponent");
    es.registerComponent<TestComponent2>("TestComponent2");

    es.registerTemplate("Test", { "TestComponent" });

    std::vector<Kunlaboro::EntityId> eids = {
        es.createEntity("Test"),
        es.createEntity("Test")
    };

    WHEN("Adding message requests in the middle of calls")
    {
        auto c1 = es.getAllComponentsOnEntity(eids[0])[0];
        auto c2 = es.getAllComponentsOnEntity(eids[1])[0];

        bool handled = false;

        c1->requestMessage("Message", [&](Kunlaboro::Message& msg) {
            CHECK_NOTHROW(msg.Sender->requestMessage("Message", [&](const Kunlaboro::Message& msg) {
                handled = true;
            }, true));
            msg.handle(true);
        }, true);

        THEN("The message doesn't exist to begin with")
        {
            c2->sendMessage("Message");
            CHECK(!handled);
        }

        c2->sendMessageToEntity(c1->getOwnerId(), "Message");

        THEN("The message exists afterwards")
        {
            c2->sendMessage("Message");
            CHECK(handled);
        }
    }

    WHEN("Removing message requests in the middle of calls")
    {
        auto c1 = es.getAllComponentsOnEntity(eids[0])[0];

        bool handled = false;

        c1->requestMessage("Message", [&](const Kunlaboro::Message& msg) {
            handled = !handled;

            CHECK_NOTHROW(c1->unrequestMessage("Message", true));
        }, true);

        THEN("The message exists to begin with")
        {
            c1->sendMessage("Message");
            CHECK(handled);

            AND_THEN("The message doesn't exist afterwards")
            {
                c1->sendMessage("Message");
                CHECK(handled);
            }
        }
    }

    GIVEN("Two entities calling eachother")
    {
        auto c1 = es.getAllComponentsOnEntity(eids[0])[0];
        auto c2 = es.getAllComponentsOnEntity(eids[1])[0];

        Kunlaboro::Component* comps[2] = { c1, c2 };

        bool add = true;

        for (auto comp : comps)
        {
            comp->requestMessage("Ping", [&](const Kunlaboro::Message& msg) {
                comp->sendMessageToEntity(msg.Sender->getOwnerId(), "Pong");
            }, true);

            comp->requestMessage("Pong", [&](const Kunlaboro::Message& msg) {
                if (msg.Sender == comp)
                    return;

                if (add)
                    es.addComponent(msg.Sender->getOwnerId(), "TestComponent");
                else
                    es.removeComponent(msg.Sender->getOwnerId(), es.getAllComponentsOnEntity(msg.Sender->getOwnerId())[0]);
            }, true);

            comp->requestComponent("TestComponent", [&](const Kunlaboro::Message& msg){
                comp->sendMessage("Ping");
            });
        }

        WHEN("A component is added during the call")
        {
            CHECK_NOTHROW(c2->sendMessageToEntity(c1->getOwnerId(), "Ping"));
        }

        WHEN("A component is removed during the call")
        {
            add = false;
            CHECK_NOTHROW(c2->sendMessageToEntity(c1->getOwnerId(), "Ping"));
        }
    }
}
