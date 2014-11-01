#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>
#include "catch.hpp"

#include <iostream>

SCENARIO("Testing")
{
    class TestComponent : public Kunlaboro::Component
    {
    public:
        TestComponent() : Kunlaboro::Component("Test") { }

        void addedToEntity() {
            requestMessage("Add", &TestComponent::add);
        }

    private:
        Kunlaboro::Optional<int> add(int a, int b)
        {
            if (a > 0)
                return a + b;

            return nullptr;
        }
    };

    class TestComponent2 : public Kunlaboro::Component
    {
    public:
        TestComponent2() : Kunlaboro::Component("Test2") { }

        void addedToEntity() {
            requestMessage("Add", &TestComponent2::add);
        }

    private:
        Kunlaboro::Optional<int> add(float a, float b)
        {
            if (a == 0)
                return a - b;

            return nullptr;
        }
    };

    Kunlaboro::EntitySystem es;
    es.registerComponent<TestComponent>("Test");
    es.registerComponent<TestComponent2>("Test2");
        
    auto eid = es.createEntity();
    es.addComponent(eid, "Test");
    es.addComponent(eid, "Test2");
    es.finalizeEntity(eid);

    auto ret = es.sendSafeGlobalMessage<int>(Kunlaboro::hash::hashString("Add"), 0.2f, 5.2f);

    if (ret)
    {
        std::cout << "Returned " << *ret << std::endl;
    }
}