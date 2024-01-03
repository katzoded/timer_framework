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

#define CONVERT_2_NANO(from_type, from) std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::from_type(from)).count()

using TIMER_HANDLE = void *;
using timer_callback_t = std::function<void (TIMER_HANDLE timer_handle, void *cookie)>;
#define TICKS_BETWEEN_UNITS 1000

enum time_unit_t {
    nano_sec,
    micro_sec,
    milli_sec,
    single_sec,
    kilo_sec,
    mega_sec,
    time_unit_last = mega_sec,
    time_unit_array_size

};

class NoThreadTimerManager_t
{
public:
    NoThreadTimerManager_t(time_unit_t minimal_tick_unit = nano_sec,
                           time_unit_t maximal_tick_unit = time_unit_last,
                           uint8_t optimal_tick[time_unit_array_size] = {}) {
        minimal_tick_unit_ = minimal_tick_unit;
        uint64_t unit_divider = 1;
        TimerUnitData_t *next_unit = nullptr;
        get_current_tick_pfn_ = get_current_nano_ticks;

        previous_processed_tick_ = get_current_tick_pfn_(this);
        for (int time_unit = nano_sec; time_unit < maximal_tick_unit + 1; time_unit++) {
            unit_divider *= TICKS_BETWEEN_UNITS;
        }

        for (int time_unit = maximal_tick_unit; time_unit >= nano_sec; time_unit--) {
            uint8_t tick_optimizer = (optimal_tick && optimal_tick[time_unit]) ? optimal_tick[time_unit] : 1;

            while(tick_optimizer > 1) {
                if (TICKS_BETWEEN_UNITS % tick_optimizer == 0) {
                    break;
                }
                tick_optimizer--;
            }

            unit_divider /= TICKS_BETWEEN_UNITS;
            timer_unit_data_[time_unit].set_next_unit(next_unit);
            timer_unit_data_[time_unit].set_unit_divider(unit_divider);
            timer_unit_data_[time_unit].set_manager(this);
            timer_unit_data_[time_unit].set_tick_optimizer(tick_optimizer);
            timer_unit_data_[time_unit].set_last_processed_tick(previous_processed_tick_);
            next_unit = &timer_unit_data_[time_unit];
        }
    }

    ~NoThreadTimerManager_t() {
    }

    virtual TIMER_HANDLE start_timer(uint64_t timeout_in_nano, timer_callback_t timer_callback, void *timer_cookie) {
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

    virtual void stop_timer(TIMER_HANDLE timer_handle) {
        TimerEntry_t *entry = (TimerEntry_t *)timer_handle;

        if (entry) {
            remove_timer(entry);
        }
    }

    void process_tick() {
        previous_processed_tick_ = get_current_tick_pfn_(this);
        timer_unit_data_[minimal_tick_unit_].process_tick();
//        auto now = get_current_tick_pfn_(this);
//
//        if (now < previous_processed_tick_) {
//            previous_processed_tick_ = now;
//            return;
//        }
//        auto time_diff = now - previous_processed_tick_;
//
//        uint64_t ticks_since_last_process_tick = time_diff / timer_unit_data_[minimal_tick_unit_].get_unit_divider();
//
//        if (ticks_since_last_process_tick) {
//            previous_processed_tick_ = now;
//            timer_unit_data_[minimal_tick_unit_].process_tick(ticks_since_last_process_tick);
//        }
    }

    inline uint64_t get_previous_processed_tick() {
        return previous_processed_tick_;
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
            for (int i = 0; i < TICKS_BETWEEN_UNITS; i++) {
                dllInit(&timer_array_[i]);
            }
        }

        ~TimerUnitData_t() {
            for (int i = 0; i < TICKS_BETWEEN_UNITS; i++) {
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

        inline void set_last_processed_tick(uint64_t last_processed_tick) {
            last_processed_tick_ = last_processed_tick;
        }

        inline void set_tick_optimizer(uint8_t optimizer) {
            tick_optimizer_ = optimizer;
        }

        inline uint8_t get_tick_optimizer() {
            return tick_optimizer_;
        }

        void add_timer(TimerEntry_t *entry) {
            uint16_t optimized_ticks_per_unit = TICKS_BETWEEN_UNITS / tick_optimizer_;
            uint16_t index = (entry->adjusted_interval_nano_sec / unit_divider_) % optimized_ticks_per_unit;
            if (0 != entry->adjusted_interval_nano_sec) {
                if (0 == index) {
                    if (next_unit_) {
                        next_unit_->add_timer(entry);
                        return;
                    }
                    else {
                        entry->adjusted_interval_nano_sec--;
                        index = optimized_ticks_per_unit;
                    }
                }
                index--;
            }
            uint16_t offset = (current_index_ + index) % optimized_ticks_per_unit;

            entry->list = &timer_array_[offset];
            dllAdd(entry->list, &entry->node);
        }

        void process_tick() {
            uint16_t optimized_ticks_for_overlap = TICKS_BETWEEN_UNITS / tick_optimizer_;
            auto now = manager_->get_previous_processed_tick();

            if (now < last_processed_tick_) {
                set_last_processed_tick(now);
                next_unit_->process_tick();
                return;
            }
            auto time_diff = now - last_processed_tick_;

            uint64_t ticks_since_last_process_tick = time_diff / unit_divider_ / tick_optimizer_;
            uint64_t tick_left_overs_for_next = ((time_diff / unit_divider_) % tick_optimizer_);

            if (ticks_since_last_process_tick) {
                TimerEntry_t* timer_entry;
                set_last_processed_tick(now - (unit_divider_ * tick_left_overs_for_next));

                for(int i = 0; i < ticks_since_last_process_tick; i++)
                {
                    // Empty the bin
                    while((timer_entry = get_obj_by_node(dllGet(&timer_array_[current_index_])))) {
                        if (timer_entry->adjusted_interval_nano_sec / (unit_divider_ * TICKS_BETWEEN_UNITS) > 0) {
                            dllRemove(&timer_array_[current_index_], &timer_entry->node);
                            timer_entry->adjusted_interval_nano_sec -= timer_entry->adjusted_interval_nano_sec % (unit_divider_ * TICKS_BETWEEN_UNITS);
                            add_timer(timer_entry);
                        }
                        else {
                            timer_entry->app_callback((TIMER_HANDLE)timer_entry, timer_entry->app_cookie);
                            manager_->remove_timer(timer_entry);
                        }
                    }
                    current_index_++;
                    if (current_index_ == optimized_ticks_for_overlap) {
                        current_index_ = 0;
                    }
                }
            }
            if (next_unit_) {
                next_unit_->process_tick();
            }
        }

        static TimerEntry_t *get_obj_by_node(DL_NODE *apNode)
        {
            return (apNode) ? (TimerEntry_t *)((char *)apNode - OFFSET(TimerEntry_t, node)) : NULL;
        }

        static int each_stop_timer(DL_NODE *apNode, TimerUnitData_t *pThis) {
            pThis->manager_->stop_timer((TIMER_HANDLE) get_obj_by_node(apNode));
            return 1;
        }

    private:
        DL_LIST timer_array_[TICKS_BETWEEN_UNITS] = {};
        uint16_t current_index_ = 0;
        uint8_t tick_optimizer_ = 1;
        uint64_t unit_divider_ = 0;
        uint64_t last_processed_tick_ = 0;

        TimerUnitData_t *next_unit_ = nullptr;
        NoThreadTimerManager_t *manager_ = nullptr;

    };

    time_unit_t minimal_tick_unit_;
    TimerUnitData_t timer_unit_data_[time_unit_array_size];

private: // functions
    void remove_timer(TimerEntry_t *entry) {
        dllRemove(entry->list, &entry->node);
        delete entry;

        size_--;
    }

    static uint64_t get_current_nano_ticks(NoThreadTimerManager_t *this_obj) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

private: // members
    using get_current_time_cb_t = uint64_t(*)(NoThreadTimerManager_t *this_obj);

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
    void advance(int64_t nano_ticks_to_advance) {
        set_next_tick(nano_ticks_to_advance);
        process_tick();
    }

    inline uint32_t number_of_active_timers() {
        return size_;
    }
#endif // UNIT_TESTING
};
