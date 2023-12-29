#pragma once

#include <functional>
#include <memory>
#include <string>
#include <chrono>
#include <iostream>
#include <string.h>
#include <shared_mutex>
#include <thread>
#include "dllLib.h"

#ifndef OFFSET
#define OFFSET(structure, member)	/* byte offset of member in structure*/\
		((long) &(((structure *) 0) -> member))
#endif

using TIMER_HANDLE = void *;
using timer_callback_t = std::function<void (TIMER_HANDLE timer_handle, void *cookie)>;
#define MAX_TICKS_PER_UNIT 10000
#define TICKS_BETWEEN_UNITS 1000

enum time_unit_t {
    nano_sec,
    micro_sec,
    milli_sec,
    single_sec,
    kilo_sec,
    mega_sec,
    time_unit_last
};

class NoThreadTimerManager_t
{
public:
    NoThreadTimerManager_t(time_unit_t minimal_tick_unit) {
        minimal_tick_unit_ = minimal_tick_unit;
        uint64_t unit_divider = 1;
        TimerUnitData_t *next_unit = nullptr;
        get_current_tick_pfn_ = get_current_nano_ticks;

        for (int time_unit = nano_sec; time_unit < time_unit_last; time_unit++) {
            unit_divider *= TICKS_BETWEEN_UNITS;
        }

        for (int time_unit = time_unit_last - 1; time_unit >= nano_sec; time_unit--) {
            unit_divider /= TICKS_BETWEEN_UNITS;
            timer_unit_data_[time_unit].set_next_unit(next_unit);
            timer_unit_data_[time_unit].set_unit_divider(unit_divider);
            timer_unit_data_[time_unit].set_manager(this);
            next_unit = &timer_unit_data_[time_unit];
        }
        previous_processed_tick_ = get_current_tick_pfn_(this);
    }
    ~NoThreadTimerManager_t() {
    }
    virtual TIMER_HANDLE StartTimer(uint64_t timeout_in_nano, timer_callback_t timer_callback, void *timer_cookie) {
        if (timeout_in_nano < timer_unit_data_[minimal_tick_unit_].get_unit_divider()) {
            return nullptr;
        }
        TimerEntry_t *entry = new TimerEntry_t;

        entry->interval_nano_sec = timeout_in_nano;
        entry->adjusted_interval_nano_sec = timeout_in_nano;
        entry->app_callback = timer_callback;
        entry->app_cookie = timer_cookie;

        timer_unit_data_[minimal_tick_unit_].add_timer(entry);

        size_++;
        return (TIMER_HANDLE)entry;
    }
    virtual void StopTimer(TIMER_HANDLE timer_handle) {
        TimerEntry_t *entry = (TimerEntry_t *)timer_handle;
        remove_timer(entry);
    }
    void ProcessTick() {
        auto now = get_current_tick_pfn_(this);

        if (now < previous_processed_tick_) {
            previous_processed_tick_ = now;
            return;
        }
        auto time_diff = now - previous_processed_tick_;

        uint64_t ticks_since_last_process_tick = time_diff / timer_unit_data_[minimal_tick_unit_].get_unit_divider();

        if (ticks_since_last_process_tick) {
            timer_unit_data_[minimal_tick_unit_].process_tick(ticks_since_last_process_tick);
            previous_processed_tick_ = now;
        }
    }
protected:
    struct TimerEntry_t {
        uint64_t interval_nano_sec;
        uint64_t adjusted_interval_nano_sec;
        timer_callback_t app_callback;
        void *app_cookie;
        DL_NODE node;
        DL_LIST *list;
    };

    class TimerUnitData_t {
    public:
        TimerUnitData_t() {
            for (int i = 0; i < MAX_TICKS_PER_UNIT; i++) {
                dllInit(&timer_array_[i]);
            }
        }
        ~TimerUnitData_t() {
            for (int i = 0; i < MAX_TICKS_PER_UNIT; i++) {
                dllEach(&timer_array_[i], (DLL_EACH_FUNCPTR)each_stop_timer, this);
            }

        }
        inline void set_next_unit(TimerUnitData_t *next_unit) {
            next_unit_ = next_unit;
        }
        inline void set_unit_divider(uint64_t divider) {
            unit_divider_ = divider;
        }
        inline void set_manager(NoThreadTimerManager_t *manager) {
            manager_ = manager;
        }
        inline uint64_t get_unit_divider() {
            return unit_divider_;
        }
        void add_timer(TimerEntry_t *entry) {
            uint16_t index = (entry->adjusted_interval_nano_sec / unit_divider_) % MAX_TICKS_PER_UNIT;
            if (0 == index) {
                next_unit_->add_timer(entry);
                return;
            }
            index--;
            uint16_t offset = (current_index_ + index) % MAX_TICKS_PER_UNIT;

            entry->list = &timer_array_[offset];
            dllAdd(entry->list, &entry->node);
        }
        void process_tick(uint64_t ticks) {
            TimerEntry_t* timer_entry;

            for(int i = 0; i < ticks; i++)
            {
                // Empty the bin
                while((timer_entry = get_obj_by_node(dllGet(&timer_array_[current_index_])))) {
                    if (timer_entry->adjusted_interval_nano_sec / (unit_divider_ * MAX_TICKS_PER_UNIT) > 0) {
                        dllRemove(&timer_array_[current_index_], &timer_entry->node);
                        timer_entry->adjusted_interval_nano_sec -= timer_entry->adjusted_interval_nano_sec % (unit_divider_ * MAX_TICKS_PER_UNIT);
                        next_unit_->add_timer(timer_entry);
                    }
                    else {
                        timer_entry->app_callback((TIMER_HANDLE)timer_entry, timer_entry->app_cookie);
                        manager_->remove_timer(timer_entry);
                    }
                }
                current_index_++;
                if (current_index_ % TICKS_BETWEEN_UNITS == 0) {
                    if (current_index_ == MAX_TICKS_PER_UNIT) {
                        current_index_ = 0;
                    }
                    if (next_unit_) {
                        next_unit_->process_tick(1);
                    }
                }
            }
        }
        static TimerEntry_t *get_obj_by_node(DL_NODE *apNode)
        {
            return (apNode) ? (TimerEntry_t *)((char *)apNode - OFFSET(TimerEntry_t, node)) : NULL;
        }
        static int each_stop_timer(DL_NODE *apNode, TimerUnitData_t *pThis) {
            pThis->manager_->StopTimer((TIMER_HANDLE)get_obj_by_node(apNode));
            return 1;
        }
    private:
        DL_LIST timer_array_[MAX_TICKS_PER_UNIT] = {};
        uint16_t num_indexes_ = 0;
        uint16_t current_index_ = 0;
        uint64_t unit_divider_ = 0;

        TimerUnitData_t *next_unit_ = nullptr;
        NoThreadTimerManager_t *manager_ = nullptr;

    };

    time_unit_t minimal_tick_unit_;
    TimerUnitData_t timer_unit_data_[time_unit_last];

private:
    void remove_timer(TimerEntry_t *entry) {
        dllRemove(entry->list, &entry->node);
        delete entry;

        size_--;
    }
    static uint64_t get_current_nano_ticks(NoThreadTimerManager_t *this_obj) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
    using get_current_time_cb_t=uint64_t(*)(NoThreadTimerManager_t *this_obj);

    get_current_time_cb_t get_current_tick_pfn_;
    uint32_t size_ = 0;
    uint64_t previous_processed_tick_ = 0;
    
#ifdef UNIT_TESTING
private:
    uint64_t next_nano_tick = 0;
    static uint64_t ut_get_current_nano_ticks(NoThreadTimerManager_t *this_obj) {
        return this_obj->next_nano_tick;
    }

protected:
    void set_next_tick(int64_t nano_ticks_to_advance) {
        get_current_tick_pfn_ = ut_get_current_nano_ticks;
        next_nano_tick = previous_processed_tick_ + nano_ticks_to_advance;
    }

public:
    uint64_t get_current_tick() {
        return previous_processed_tick_;
    }
    void advance(int64_t nano_ticks_to_advance) {
        set_next_tick(nano_ticks_to_advance);
        ProcessTick();
    }
    inline uint32_t number_of_active_timers() {
        return size_;
    }
#endif // UNIT_TESTING
};
