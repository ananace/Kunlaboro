#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>
#include "catch.hpp"

#include <atomic>
#include <thread>

SCENARIO("Threaded message passing")
{
    Kunlaboro::EntitySystem es(true);

    class TestComponent : public Kunlaboro::Component
    {
    public:
        TestComponent() : Kunlaboro::Component("TestComponent") { }

        void addedToEntity()
        {
            requestMessage("Increment", [](const Kunlaboro::Message& msg) {
                std::atomic_uint& value = *boost::any_cast<std::atomic_uint*>(msg.payload);

                ++value;
            }, true);

            requestMessage("Decrement", [](const Kunlaboro::Message& msg) {
                std::atomic_uint& value = *boost::any_cast<std::atomic_uint*>(msg.payload);

                --value;
            }, true);
        }
    };

    es.registerComponent<TestComponent>("TestComponent");
    es.registerTemplate("TestEntity", { "TestComponent" });

    GIVEN("An entity system with two entites")
    {
        auto eid1 = es.createEntity("TestEntity");
        auto eid2 = es.createEntity("TestEntity");

        WHEN("Both entities are being spammed with messages")
        {
            std::atomic_uint value;
            value.store(0);

            std::list<std::thread*> threads = {
                new std::thread([eid1, &es, &value]() {
                    auto rid = es.getMessageRequestId(Kunlaboro::Reason_Message, "Increment");
                    Kunlaboro::Message msg;
                    msg.payload = &value;

                    for (uint16_t i = 0; i < UINT16_MAX; ++i)
                    {
                        es.sendLocalMessage(eid1, rid, msg);
                    }
                }),
                new std::thread([eid2, &es, &value]() {
                    auto rid = es.getMessageRequestId(Kunlaboro::Reason_Message, "Decrement");
                    Kunlaboro::Message msg;
                    msg.payload = &value;

                    for (uint16_t i = 0; i < UINT16_MAX; ++i)
                    {
                        es.sendLocalMessage(eid2, rid, msg);
                    }
                })
            };

            for (auto thread : threads)
            {
                if (thread->joinable())
                    thread->join();

                delete thread;
            }

            THEN("All the messages are still recieved successfully")
            {
                CHECK(value == 0);
            }
        }

        WHEN("One entity ends up stuck in a loop")
        {
            bool run = true;
            es.getAllComponentsOnEntity(eid1)[0]->requestMessage("Stuck", [&run](const Kunlaboro::Message& msg) { while (run); });

            std::thread send([&es]() {
                es.sendGlobalMessage("Stuck");
            });

            THEN("Messages can still be sent to that entity from another thread")
            {
                uint16_t temp = 0;

                es.getAllComponentsOnEntity(eid1)[0]->requestMessage("Test", [&temp](const Kunlaboro::Message& msg) { ++temp; });

                for (uint16_t i = 0; i < 42; ++i)
                    es.sendGlobalMessage("Test");

                CHECK(temp == 42);
            }

            THEN("Messages can still be sent to the other entity")
            {
                uint16_t temp = 0;

                es.getAllComponentsOnEntity(eid2)[0]->requestMessage("Test", [&temp](const Kunlaboro::Message& msg) { ++temp; });

                for (uint16_t i = 0; i < 42; ++i)
                    es.sendGlobalMessage("Test");

                CHECK(temp == 42);
            }

            run = false;

            if (send.joinable())
                send.join();
        }
    }
}