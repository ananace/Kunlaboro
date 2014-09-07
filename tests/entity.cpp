#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>
#include "catch.hpp"

#include <random>

SCENARIO("Entities having their components messed with")
{
    class BasicComponent : public Kunlaboro::Component
    {
    public:
        BasicComponent() : Kunlaboro::Component("Basic") { }
        void addedToEntity() {
            requestMessage("Ping", [](const Kunlaboro::Message& msg) {
                msg.sender->sendGlobalMessage("Pong");
            });
        }
    };

    Kunlaboro::EntitySystem es;
    
    es.registerComponent<BasicComponent>("Basic");
    es.registerTemplate("Basic", { "Basic" });

    std::vector<Kunlaboro::EntityId> eids = {
        es.createEntity("Basic"),
        es.createEntity("Basic"),
        es.createEntity("Basic"),
        es.createEntity("Basic")
    };

    WHEN("Moving entities in the middle of calls")
    {
        auto c1 = es.getAllComponentsOnEntity(eids[0])[0];
        auto c2 = es.getAllComponentsOnEntity(eids[1])[0];

        c1->requestMessage("Pong", [&](Kunlaboro::Message& msg) {
            if (msg.sender == c1)
                return;

            CHECK_NOTHROW(es.removeComponent(c1->getOwnerId(), c1));
            CHECK_NOTHROW(es.addComponent(msg.sender->getOwnerId(), c1));
            msg.handle(true);
        });
        c2->requestMessage("Pong", [&](Kunlaboro::Message& msg) {
            if (msg.sender == c2)
                return;

            CHECK_NOTHROW(es.removeComponent(c2->getOwnerId(), c2));
            CHECK_NOTHROW(es.addComponent(msg.sender->getOwnerId(), c2));
            msg.handle(true);
        });

        std::random_device dev;
        std::uniform_int_distribution<int> dist(0, 10);

        for (uint16_t i = 0; i < 1000; ++i)
        {
            c1->changeRequestPriority("Pong", dist(dev));
            c2->changeRequestPriority("Pong", dist(dev));
            
            do
            {
                auto comps = es.getAllComponentsOnEntity(eids[dist(dev) % 4]);
                if (comps.empty())
                    continue;

                comps[0]->sendGlobalMessage("Ping");
            } while (false);
        }

        THEN("Entities should still be valid")
        {
            CHECK(c1->isValid());
            CHECK(c2->isValid());
        }
    }

    WHEN("Adding message requests in the middle of calls")
    {
        auto c1 = es.getAllComponentsOnEntity(eids[0])[0];
        auto c2 = es.getAllComponentsOnEntity(eids[1])[0];

        bool handled = false;

        c1->requestMessage("Pong", [&](Kunlaboro::Message& msg) {
            CHECK_NOTHROW(msg.sender->requestMessage("Pong", [&](const Kunlaboro::Message& msg) {
                handled = true;
            }, true));
            msg.handle(true);
        }, true);

        THEN("The message doesn't exist to begin with")
        {
            c2->sendMessage("Pong");
            CHECK(!handled);
        }

        c2->sendMessageToEntity(c1->getOwnerId(), "Pong");

        THEN("The message exists afterwards")
        {
            c2->sendMessage("Pong");
            CHECK(handled);
        }
    }

    WHEN("Removing message requests in the middle of calls")
    {
        auto c1 = es.getAllComponentsOnEntity(eids[0])[0];

        bool handled = false;

        c1->requestMessage("Pong", [&](const Kunlaboro::Message& msg) {
            handled = !handled;

            c1->unrequestMessage("Pong", true);
        }, true);

        THEN("The message exist to begin with")
        {
            c1->sendMessage("Pong");
            CHECK(handled);

            AND_THEN("The message doesn't exist afterwards")
            {
                c1->sendMessage("Pong");
                CHECK(handled);
            }
        }
    }
}