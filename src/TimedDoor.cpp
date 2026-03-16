// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>

DoorTimerAdapter::DoorTimerAdapter(TimedDoor& d) : door(d) {}

void DoorTimerAdapter::Timeout() {
    door.checkTimeout();
}

TimedDoor::TimedDoor(int timeout) : iTimeout(timeout), isOpened(false) {
    adapter = new DoorTimerAdapter(*this);
}

TimedDoor::~TimedDoor() {
    if (th && th->joinable()) {
        th->join();
        delete th;
    }
    delete adapter;
}

bool TimedDoor::isDoorOpened() {
    return isOpened;
}

void TimedDoor::unlock() {
    isOpened = true;
    if (th && th->joinable()) {
        th->join();
        delete th;
    }
    th = new std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::seconds(iTimeout));
        if (isOpened) {
            adapter->Timeout();
        }
        });
}

void TimedDoor::lock() {
    if (isOpened) {
        checkTimeout();
    }
    isOpened = false;
    if (th && th->joinable()) {
        th->join();
        delete th;
        th = nullptr;
    }
}

int TimedDoor::getTimeOut() const {
    return iTimeout;
}

void TimedDoor::throwState() {
    if (isOpened) {
        throw std::runtime_error("Door is still opened after timeout!");
    }
}

void TimedDoor::checkTimeout() {
    if (isOpened) {
        throw std::runtime_error("Door is still opened after timeout!");
    }
}

void Timer::sleep(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void Timer::tregister(int timeout, TimerClient* client) {
    this->client = client;
    std::thread([this, timeout]() {
        sleep(timeout);
        if (this->client) {
            this->client->Timeout();
        }
        }).detach();
}
