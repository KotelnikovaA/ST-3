// Copyright 2021 GHA Test Team

#ifndef INCLUDE_TIMEDDOOR_H_
#define INCLUDE_TIMEDDOOR_H_

#include <thread>
#include <atomic>
#include <memory>

class DoorTimerAdapter;
class Timer;
class Door;
class TimedDoor;

class TimerClient {
 public:
    virtual void Timeout() = 0;
    virtual ~TimerClient() = default;
};

class Door {
 public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual bool isDoorOpened() = 0;
    virtual ~Door() = default;
};

class DoorTimerAdapter : public TimerClient {
 private:
    TimedDoor& door;

 public:
    explicit DoorTimerAdapter(TimedDoor& d);
    void Timeout() override;
};

class TimedDoor : public Door {
    friend class DoorTimerAdapter;

 private:
    DoorTimerAdapter* adapter;
    int iTimeout;
    std::atomic<bool> isOpened;
    std::atomic<bool> isTimerActive;
    std::unique_ptr<std::thread> timerThread;

 public:
    explicit TimedDoor(int timeout);
    ~TimedDoor() override;

    bool isDoorOpened() override;
    void unlock() override;
    void lock() override;
    int getTimeOut() const;
    void throwState();
    void checkTimeout();
    TimedDoor(const TimedDoor&) = delete;
    TimedDoor& operator=(const TimedDoor&) = delete;
};

class Timer {
    std::unique_ptr<std::thread> timerThread;

 public:
    ~Timer();
    void tregister(int timeout, TimerClient* client);
    void sleep(int seconds);
};

#endif  // INCLUDE_TIMEDDOOR_H_
