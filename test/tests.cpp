// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include <thread>
#include "TimedDoor.h"

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

class TimedDoorTest : public ::testing::Test {
 protected:
    void SetUp() override {
        door = new TimedDoor(1);
        adapter = new DoorTimerAdapter(*door);
        door->lock();
    }

    void TearDown() override {
        delete adapter;
        delete door;
    }
    TimedDoor* door;
    DoorTimerAdapter* adapter;
};

TEST_F(TimedDoorTest, Test_full_cycle) {
    EXPECT_FALSE(door->isDoorOpened());
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
    door->unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_NO_THROW(door->throwState());
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    EXPECT_THROW(door->lock(), std::runtime_error);
}

TEST_F(TimedDoorTest, Test_door_created_closed) {
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, Test_lock_sets_closed) {
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, Test_unlock_sets_opened) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    door->lock();
}

TEST_F(TimedDoorTest, Test_get_timeout_returns_correct_value) {
    EXPECT_EQ(door->getTimeOut(), 1);
}

TEST_F(TimedDoorTest, Test_adapter_calls_timeout) {
    MockTimerClient mockClient;
    Timer timer;
    EXPECT_CALL(mockClient, Timeout()).Times(1);
    timer.tregister(1, &mockClient);
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

TEST_F(TimedDoorTest, Test_when_door_is_closed) {
    door->lock();
    EXPECT_NO_THROW(door->throwState());
}

TEST_F(TimedDoorTest, Test_when_door_is_opened_for_too_long) {
    door->unlock();
    ASSERT_ANY_THROW({
      std::this_thread::sleep_for(std::chrono::milliseconds(1100));
      door->lock();
        });
}

TEST_F(TimedDoorTest, Test_mock_door_state) {
    MockDoor mockDoor;
    EXPECT_CALL(mockDoor, isDoorOpened()).WillOnce(::testing::Return(false));
    EXPECT_FALSE(mockDoor.isDoorOpened());
}

TEST_F(TimedDoorTest, Test_unlock_triggers_timer) {
    MockTimerClient mockClient;
    Timer timer;
    EXPECT_CALL(mockClient, Timeout()).Times(1);
    timer.tregister(1, &mockClient);
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

TEST_F(TimedDoorTest, Test_mock_door_lock) {
    MockDoor mockDoor;
    EXPECT_CALL(mockDoor, lock()).Times(1);
    mockDoor.lock();
}

TEST_F(TimedDoorTest, Test_mock_door_unlock) {
    MockDoor mockDoor;
    EXPECT_CALL(mockDoor, unlock()).Times(1);
    mockDoor.unlock();
}
