#pragma once

#include <functional>
#include <memory>
#include <string>
#include <chrono>
#include <iostream>
#include <string.h>
#include <shared_mutex>
#include <thread>
#include "no_thread_timer_manager.hpp"

class WithThreadTimerManager : public NoThreadTimerManager_t
{
public:
    WithThreadTimerManager(time_unit_t minimal_tick_unit, int64_t sleep_tick_multiplier = 1) : NoThreadTimerManager_t(minimal_tick_unit){
        uint64_t sec_in_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)).count();
        uint64_t tick_sleep = ((sleep_tick_multiplier != 0 ) ? sleep_tick_multiplier : 1) * timer_unit_data_[minimal_tick_unit_].get_unit_divider();
        sleep_request_.tv_sec = tick_sleep / sec_in_nano;
        sleep_request_.tv_nsec = tick_sleep % sec_in_nano;
        running_ = true;

        timer_thread_ = std::thread([=]() {
            while(running_) {
                struct timespec remaining;
                if (0 == nanosleep(&sleep_request_, &remaining)) {
                    std::lock_guard<std::shared_mutex> lock(mutex_);
                    ProcessTick();
                }
            }
            ended_ = true;
        });
        timer_thread_.detach();
    }

    ~WithThreadTimerManager() {
        running_ = false;
        while (!ended_) {
            struct timespec remaining;
            struct timespec request = {1, 0};
            nanosleep(&request, &remaining);
        }
    }
    virtual TIMER_HANDLE StartTimer(uint64_t timeout_in_nano, timer_callback_t timer_callback, void *timer_cookie) {
        std::lock_guard<std::shared_mutex> lock(mutex_);
        return NoThreadTimerManager_t::StartTimer(timeout_in_nano, timer_callback, timer_cookie);
    }
    virtual void StopTimer(TIMER_HANDLE timer_handle) {
        std::lock_guard<std::shared_mutex> lock(mutex_);
        NoThreadTimerManager_t::StopTimer(timer_handle);
    }
#ifdef UNIT_TESTING
    void advance(int64_t nano_ticks_to_advance) {
        set_next_tick(nano_ticks_to_advance);
        struct timespec remaining;
        struct timespec sleep_request;

        sleep_request.tv_sec = (sleep_request_.tv_sec) ? sleep_request_.tv_sec * 2 : 1;
        sleep_request.tv_nsec = sleep_request_.tv_nsec;
        nanosleep(&sleep_request, &remaining);
    }

#endif // UNIT_TESTING

private:
    std::thread timer_thread_;
    std::shared_mutex mutex_;
    struct timespec sleep_request_;
    bool running_ = false;
    bool ended_ = false;
};
