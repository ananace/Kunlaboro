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

        std::atomic_uint value;
        value.store(0);

        WHEN("Both entities recieve messages at the same time")
        {
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

            THEN("All messages are recieved successfully")
            {
                CHECK(value == 0);
            }
        }
    }
}