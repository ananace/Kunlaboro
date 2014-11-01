#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>
#include "catch.hpp"

SCENARIO("Message passing benchmark",
    "[benchmark]")
{
    class BasicComponent : public Kunlaboro::Component
    {
    public:
        BasicComponent() : Kunlaboro::Component("Basic") { }
        void addedToEntity() {
            requestMessage("Ping", [this](const Kunlaboro::Message& msg) {
                sendGlobalMessage("Pong");
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

    Kunlaboro::Message msg;
    Kunlaboro::RequestId rid = Kunlaboro::hash::hash_func1::hash("Ping");
    for (uint32_t i = 0; i < 1000000; ++i)
        es.sendSafeGlobalMessage(rid, msg);
}