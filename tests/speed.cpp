#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>
#include "catch.hpp"

uint64_t calls;

SCENARIO("Message passing benchmark",
    "[benchmark]")
{
    calls = 0;

    class BasicComponent : public Kunlaboro::Component
    {
    public:
        BasicComponent() : Kunlaboro::Component("Basic") { }
        void addedToEntity() {
            requestMessage("Ping", &BasicComponent::ping);
            requestMessage("Pong", &BasicComponent::pong);
        }

    private:
        void ping(const Kunlaboro::Message& msg)
        {
            sendGlobalMessage("Pong");

            ++calls;
        }

        void pong(const Kunlaboro::Message& msg)
        {
            ++calls;
        }
    };

    class LambdaComponent : public Kunlaboro::Component
    {
    public:
        LambdaComponent() : Kunlaboro::Component("Lambda") { }
        void addedToEntity() {
            requestMessage("Ping", [=](const Kunlaboro::Message& msg) { sendGlobalMessage("Pong"); ++calls; });
            requestMessage("Pong", [=](const Kunlaboro::Message& msg) { ++calls; });
        }
    };

    Kunlaboro::EntitySystem es;

    es.registerComponent<BasicComponent>("Basic");
    es.registerComponent<LambdaComponent>("Lambda");
    es.registerTemplate("Basic", { "Basic" });
    es.registerTemplate("Lambda", { "Lambda" });

    WHEN("Using direct function mapping")
    {
        std::vector<Kunlaboro::EntityId> eids = {
            es.createEntity("Basic"),
            es.createEntity("Basic"),
            es.createEntity("Basic"),
            es.createEntity("Basic")
        };

        AND_WHEN("Unsafe calls")
        {
            AND_WHEN("Using direct string calls")
            {
                for (uint32_t i = 0; i < 100000; ++i)
                    es.sendGlobalMessage("Ping");

                CHECK(calls == 2000000);
            }

            AND_WHEN("Using cached request IDs")
            {
                Kunlaboro::Message msg;
                Kunlaboro::RequestId rid = Kunlaboro::hash::hash_func1::hash("Ping");
                for (uint32_t i = 0; i < 100000; ++i)
                    es.sendUnsafeGlobalMessage(rid, msg);

                CHECK(calls == 2000000);
            }
        }

        AND_WHEN("Safe calls")
        {
            AND_WHEN("Using cached request IDs")
            {
                Kunlaboro::Message msg;
                Kunlaboro::RequestId rid = Kunlaboro::hash::hash_func1::hash("Ping");
                for (uint32_t i = 0; i < 100000; ++i)
                    es.sendSafeGlobalMessage(rid, msg);

                CHECK(calls == 2000000);
            }
        }
    }

    WHEN("Using lambda mappings")
    {
        std::vector<Kunlaboro::EntityId> eids = {
            es.createEntity("Lambda"),
            es.createEntity("Lambda"),
            es.createEntity("Lambda"),
            es.createEntity("Lambda")
        };

        AND_WHEN("Unsafe calls")
        {
            AND_WHEN("Using direct string calls")
            {
                for (uint32_t i = 0; i < 100000; ++i)
                    es.sendGlobalMessage("Ping");

                CHECK(calls == 2000000);
            }

            AND_WHEN("Using cached request IDs")
            {
                Kunlaboro::Message msg;
                Kunlaboro::RequestId rid = Kunlaboro::hash::hash_func1::hash("Ping");
                for (uint32_t i = 0; i < 100000; ++i)
                    es.sendUnsafeGlobalMessage(rid, msg);

                CHECK(calls == 2000000);
            }
        }

        AND_WHEN("Safe calls")
        {
            AND_WHEN("Using cached request IDs")
            {
                Kunlaboro::Message msg;
                Kunlaboro::RequestId rid = Kunlaboro::hash::hash_func1::hash("Ping");
                for (uint32_t i = 0; i < 100000; ++i)
                    es.sendSafeGlobalMessage(rid, msg);

                CHECK(calls == 2000000);
            }
        }
    }
}