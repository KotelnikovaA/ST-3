// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <thread>
#include <chrono>
#include <stdexcept>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::NiceMock;

class MockTimerClient : public TimerClient {
 public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
 public:
    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
    MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class MockTimedDoor : public TimedDoor {
 public:
    explicit MockTimedDoor(int timeout) : TimedDoor(timeout) {}
    MOCK_METHOD(bool, isDoorOpened, (), (override));
    MOCK_METHOD(void, throwState, (), (override));
};

class TimedDoorTest : public ::testing::Test {
 protected:
    void SetUp() override {
        door = new TimedDoor(1);
        adapter = new DoorTimerAdapter(*door);
        timer = new Timer();
    }

    void TearDown() override {
        delete door;
        delete adapter;
        delete timer;
    }

    TimedDoor* door;
    DoorTimerAdapter* adapter;
    Timer* timer;
};

TEST_F(TimedDoorTest, DoorCreation) {
    EXPECT_NE(door, nullptr);
    EXPECT_EQ(door->getTimeOut(), 1);
}

TEST_F(TimedDoorTest, InitialDoorState) {
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, DoorUnlock) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, DoorLock) {
    door->unlock();
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, AdapterWithOpenDoor) {
    door->unlock();
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, AdapterWithClosedDoor) {
    door->lock();
    EXPECT_NO_THROW(adapter->Timeout());
}

TEST_F(TimedDoorTest, TimerWithOpenDoorThrows) {
    door->unlock();

    Timer testTimer;
    EXPECT_THROW(testTimer.tregister(1, adapter), std::runtime_error);
}

TEST_F(TimedDoorTest, TimerWithClosedDoorNoThrow) {
    door->lock();

    Timer testTimer;
    EXPECT_NO_THROW(testTimer.tregister(1, adapter));
}

TEST(TimerClientTest, TimeoutCalled) {
    NiceMock<MockTimerClient> mockClient;
    Timer timer;

    EXPECT_CALL(mockClient, Timeout())
        .Times(1);

    std::thread timerThread([&timer, &mockClient]() {
        timer.tregister(0, &mockClient);
        });

    timerThread.join();
}

TEST(AdapterTest, DoorAdapterInteraction) {
    MockTimedDoor mockDoor(1);
    DoorTimerAdapter adapter(mockDoor);

    EXPECT_CALL(mockDoor, isDoorOpened())
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(mockDoor, throwState())
        .Times(1);

    adapter.Timeout();
}

TEST_F(TimedDoorTest, MultipleOpenClose) {
    for (int i = 0; i < 5; i++) {
        door->unlock();
        EXPECT_TRUE(door->isDoorOpened());
        door->lock();
        EXPECT_FALSE(door->isDoorOpened());
    }
}

TEST(TimedDoorParamTest, DifferentTimeouts) {
    TimedDoor door1(1);
    TimedDoor door2(5);
    TimedDoor door3(10);

    EXPECT_EQ(door1.getTimeOut(), 1);
    EXPECT_EQ(door2.getTimeOut(), 5);
    EXPECT_EQ(door3.getTimeOut(), 10);
}

TEST(MockChainTest, VerifyMethodCalls) {
    NiceMock<MockDoor> mockDoor;
    MockTimerClient mockClient;
    DoorTimerAdapter adapter(static_cast<TimedDoor&>(mockDoor));

    {
        ::testing::InSequence seq;
        EXPECT_CALL(mockDoor, unlock())
            .Times(1);
        EXPECT_CALL(mockDoor, isDoorOpened())
            .Times(1)
            .WillOnce(Return(true));
    }

    mockDoor.unlock();
    EXPECT_TRUE(mockDoor.isDoorOpened());
}

TEST(TimerTest, NullClient) {
    Timer timer;
    EXPECT_NO_THROW(timer.tregister(0, nullptr));
}

TEST_F(TimedDoorTest, ThrowStateDoesntChangeDoorState) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    adapter->Timeout();
    EXPECT_TRUE(door->isDoorOpened());
}