#include <gtest/gtest.h>
#include <Kunlaboro/Kunlaboro.hpp>

class GTestComponent : public Kunlaboro::Component
{
public:
    GTestComponent(const std::string& str) : Kunlaboro::Component(str) { }

    virtual void setPriority(int prio) = 0;
    

    virtual void TestMessage(const Kunlaboro::Message& msg) = 0;
    virtual void TestMessagePriority(const Kunlaboro::Message& msg) = 0;
    virtual void TestMessageHandling(Kunlaboro::Message& msg) = 0;
};

class GTestComponentSucceed : public GTestComponent {
public:
    GTestComponentSucceed(): GTestComponent("GTestComponentSucceed") {}

    virtual void setPriority(int prio)
    {
        changeRequestPriority("TestPrio", prio);
        changeRequestPriority("TestHandle", prio);
    }

    virtual void addedToEntity()
    {
        requestMessage("Test", &GTestComponentSucceed::TestMessage);
        requestMessage("TestPrio", &GTestComponentSucceed::TestMessagePriority);
        requestMessage("TestHandle", &GTestComponentSucceed::TestMessageHandling);
    }

    virtual void TestMessage(const Kunlaboro::Message& msg) { float& f = *boost::any_cast<float*>(msg.payload); f += 10; }
    virtual void TestMessagePriority(const Kunlaboro::Message& msg) { float& f = *boost::any_cast<float*>(msg.payload); f += 30; }
    virtual void TestMessageHandling(Kunlaboro::Message& msg) { msg.handled = true; }
};

class GTestComponentFail : public GTestComponent {
public:
    GTestComponentFail(): GTestComponent("GTestComponentFail") {}

    virtual void setPriority(int prio)
    {
        changeRequestPriority("TestPrio", prio);
        changeRequestPriority("TestHandle", prio);
    }

    virtual void addedToEntity()
    {
        requestMessage("Test", &GTestComponentFail::TestMessage);
        requestMessage("TestPrio", &GTestComponentFail::TestMessagePriority);
        requestMessage("TestHandle", &GTestComponentFail::TestMessageHandling);
    }

    virtual void TestMessage(const Kunlaboro::Message& msg) { float& f = *boost::any_cast<float*>(msg.payload); f += 1; }
    virtual void TestMessagePriority(const Kunlaboro::Message& msg) { float& f = *boost::any_cast<float*>(msg.payload); f /= 2; }
    virtual void TestMessageHandling(Kunlaboro::Message& msg) { FAIL(); }
};

class KunlaboroTest : public testing::Test {
protected:

    virtual void SetUp()
    {
        System.registerComponent<GTestComponentSucceed>("GTestComponentSucceed");
        System.registerComponent<GTestComponentFail>("GTestComponentFail");

        Entity = System.createEntity();

        ASSERT_TRUE(Entity != 0);
        EXPECT_NO_THROW(SucceedComponent = System.createComponent("GTestComponentSucceed"));
        EXPECT_NO_THROW(FailComponent    = System.createComponent("GTestComponentFail"));

        System.addComponent(Entity, SucceedComponent);
        System.addComponent(Entity, FailComponent);

        EXPECT_EQ(2, System.getAllComponentsOnEntity(Entity).size());

        EXPECT_TRUE(System.finalizeEntity(Entity));
    }

    virtual void TearDown()
    {

    }

    Kunlaboro::EntitySystem System;
    Kunlaboro::EntityId     Entity;

    Kunlaboro::Component*   FailComponent;
    Kunlaboro::Component*   SucceedComponent;
};

TEST_F(KunlaboroTest, DefaultConstructor) {
    EXPECT_FALSE(System.isFrozen());
}

TEST_F(KunlaboroTest, TestMessage) {
    float data = 2;

    System.sendGlobalMessage("Test", &data);

    EXPECT_EQ(13, data);
}

TEST_F(KunlaboroTest, TestPriority) {
    float data = 2;

    ((GTestComponent*)FailComponent)->setPriority(0);
    ((GTestComponent*)SucceedComponent)->setPriority(10);

    System.sendGlobalMessage("TestPrio", &data);

    EXPECT_EQ(31, data);

    ((GTestComponent*)FailComponent)->setPriority(10);
    ((GTestComponent*)SucceedComponent)->setPriority(0);

    data = 2;
    System.sendGlobalMessage("TestPrio", &data);

    EXPECT_EQ(16, data);
}

TEST_F(KunlaboroTest, TestHandling) {
    ((GTestComponent*)FailComponent)->setPriority(1);
    ((GTestComponent*)SucceedComponent)->setPriority(-1);

    void* data;
    System.sendGlobalMessage("TestHandle", data);
}