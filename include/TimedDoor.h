// Copyright 2021 GHA Test Team
#ifndef INCLUDE_TIMEDDOOR_H_
#define INCLUDE_TIMEDDOOR_H_

#include <thread>

class DoorTimerAdapter;
class Timer;
class Door;
class TimedDoor;

class TimerClient {
 public:
    virtual void Timeout() = 0;
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
    explicit DoorTimerAdapter(TimedDoor&);
    void Timeout() override;
};

class TimedDoor : public Door {
    friend class DoorTimerAdapter;

 private:
    DoorTimerAdapter* adapter;
    int iTimeout;
    bool isOpened;
    std::thread* th;

 public:
    explicit TimedDoor(int);
    ~TimedDoor() override;
    bool isDoorOpened() override;
    void unlock() override;
    void lock() override;
    int getTimeOut() const;
    void throwState();
    void checkTimeout();
};

class Timer {
    TimerClient* client = nullptr;
    void sleep(int);

 public:
    void tregister(int, TimerClient*);
};

#endif  // INCLUDE_TIMEDDOOR_H_
