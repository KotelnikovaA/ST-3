// Copyright 2021 GHA Test Team

#include "TimedDoor.h"
#include <iostream>
#include <chrono>
#include <stdexcept>

DoorTimerAdapter::DoorTimerAdapter(TimedDoor& d) : door(d) {}

void DoorTimerAdapter::Timeout() {
    door.checkTimeout();
}

TimedDoor::TimedDoor(int timeout)
    : iTimeout(timeout), isOpened(false), isTimerActive(false) {
    adapter = new DoorTimerAdapter(*this);
}

TimedDoor::~TimedDoor() {
    lock();
    delete adapter;
}

bool TimedDoor::isDoorOpened() {
    return isOpened.load();
}

void TimedDoor::unlock() {
    if (isOpened.exchange(true)) {
        return;
    }

    if (timerThread && timerThread->joinable()) {
        isTimerActive = false;
        timerThread->join();
    }

    isTimerActive = true;
    timerThread = std::make_unique<std::thread>([this]() {
        std::this_thread::sleep_for(std::chrono::seconds(iTimeout));
        if (isTimerActive.load() && isOpened.load()) {
            adapter->Timeout();
        }
        });
}

void TimedDoor::lock() {
    isTimerActive = false;
    if (timerThread && timerThread->joinable()) {
        timerThread->join();
    }

    if (isOpened.load()) {
        checkTimeout();
    }

    isOpened = false;
}

int TimedDoor::getTimeOut() const {
    return iTimeout;
}

void TimedDoor::throwState() {
    checkTimeout();
}

void TimedDoor::checkTimeout() {
    if (isOpened.load()) {
        throw std::runtime_error("Door is still opened after timeout!");
    }
}

Timer::~Timer() {
    if (timerThread && timerThread->joinable()) {
        timerThread->join();
    }
}

void Timer::sleep(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void Timer::tregister(int timeout, TimerClient* client) {
    if (timerThread && timerThread->joinable()) {
        timerThread->join();
    }

    timerThread = std::make_unique<std::thread>([this, timeout, client]() {
        sleep(timeout);
        if (client) {
            client->Timeout();
        }
        });
}
