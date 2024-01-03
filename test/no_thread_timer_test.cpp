#define UNIT_TESTING
#include "no_thread_timer_manager.hpp"
#include "catch.hpp"

using namespace std;


//static NoThreadTimerManager_t test_subject_sec(single_sec);

SCENARIO("nano-sec-no-thread-timer")
{
    NoThreadTimerManager_t test_subject_nano(nano_sec);
    SECTION("test 100 nano sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_previous_processed_tick();
        test_subject_nano.start_timer(CONVERT_2_NANO(nanoseconds, 100), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
        }, nullptr);
        test_subject_nano.advance(CONVERT_2_NANO(nanoseconds, 99));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(nanoseconds, 99) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(nanoseconds, 2));
        REQUIRE(expired == true);
        REQUIRE(CONVERT_2_NANO(nanoseconds, 101) == test_subject_nano.get_previous_processed_tick() - start_time);
    }
    SECTION("test 10080 nano sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_previous_processed_tick();
        test_subject_nano.start_timer(CONVERT_2_NANO(nanoseconds, 10080), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
        }, nullptr);
        test_subject_nano.advance(CONVERT_2_NANO(nanoseconds, 80));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(nanoseconds, 80) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(microseconds, 3));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(microseconds, 3) +
                CONVERT_2_NANO(nanoseconds, 80) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(microseconds, 7));
        REQUIRE(expired == true);
        REQUIRE(CONVERT_2_NANO(microseconds, 10) +
                CONVERT_2_NANO(nanoseconds, 80) == test_subject_nano.get_previous_processed_tick() - start_time);
    }
    SECTION("test 10000 nano sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_previous_processed_tick();
        test_subject_nano.start_timer(CONVERT_2_NANO(nanoseconds, 10000), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
        }, nullptr);
        test_subject_nano.advance(CONVERT_2_NANO(nanoseconds, 80));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(nanoseconds, 80) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(microseconds, 3));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(microseconds, 3) +
                CONVERT_2_NANO(nanoseconds, 80) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(microseconds, 7));
        REQUIRE(expired == true);
        REQUIRE(CONVERT_2_NANO(microseconds, 10) +
                CONVERT_2_NANO(nanoseconds, 80) == test_subject_nano.get_previous_processed_tick() - start_time);
    }
    SECTION("test 100milli + 50usec + 300nano sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_previous_processed_tick();
        test_subject_nano.start_timer(CONVERT_2_NANO(nanoseconds, 300) +
                                    CONVERT_2_NANO(microseconds, 50) +
                                    CONVERT_2_NANO(milliseconds, 100),
                                    [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
        }, nullptr);
        test_subject_nano.advance(CONVERT_2_NANO(nanoseconds, 300));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(nanoseconds, 300) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(microseconds, 50));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(microseconds, 50) +
                CONVERT_2_NANO(nanoseconds, 300) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(milliseconds, 100));
        REQUIRE(expired == true);
        REQUIRE(CONVERT_2_NANO(milliseconds, 100) +
                CONVERT_2_NANO(microseconds, 50) +
                CONVERT_2_NANO(nanoseconds, 300) == test_subject_nano.get_previous_processed_tick() - start_time);
    }
}

SCENARIO("mill-sec-no-thread-timer")
{
    NoThreadTimerManager_t test_subject_milli(milli_sec);
    SECTION("test 100 nano sec, not valid")
    {
        test_subject_milli.start_timer(CONVERT_2_NANO(nanoseconds , 100), [&](TIMER_HANDLE timer, void *cookie) {
        }, nullptr);
        REQUIRE(0 == test_subject_milli.number_of_active_timers());
    }
    SECTION("test 1000 milli sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_milli.get_previous_processed_tick();
        test_subject_milli.start_timer(CONVERT_2_NANO(milliseconds, 100), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
            uint64_t expiration_time = test_subject_milli.get_previous_processed_tick() - start_time;
        }, nullptr);
        test_subject_milli.advance(CONVERT_2_NANO(milliseconds, 99));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(milliseconds, 99) == test_subject_milli.get_previous_processed_tick() - start_time);
        test_subject_milli.advance(CONVERT_2_NANO(milliseconds, 1));
        REQUIRE(expired == true);
        REQUIRE(CONVERT_2_NANO(milliseconds, 100) == test_subject_milli.get_previous_processed_tick() - start_time);
    }
}

SCENARIO("nano-sec-only-no-thread-timer")
{
    NoThreadTimerManager_t test_subject_nano(nano_sec, nano_sec);
    SECTION("test 1000 micro sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_previous_processed_tick();
        test_subject_nano.start_timer(CONVERT_2_NANO(microseconds, 1000), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
        }, nullptr);
        test_subject_nano.advance(CONVERT_2_NANO(microseconds, 999));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(microseconds, 999) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(microseconds, 1));
        REQUIRE(expired == true);
        REQUIRE(CONVERT_2_NANO(microseconds, 1000) == test_subject_nano.get_previous_processed_tick() - start_time);
    }
}

SCENARIO("nano-sec-with-100-ns-optimization-thread-timer")
{
    uint8_t tick_optimization[time_unit_array_size] = {0};
    tick_optimization[nano_sec] = 100;
    tick_optimization[micro_sec] = 10;
    NoThreadTimerManager_t test_subject_nano(nano_sec, single_sec, tick_optimization);

    SECTION("test 1000 micro sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_previous_processed_tick();
        test_subject_nano.start_timer(CONVERT_2_NANO(microseconds, 1000), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
        }, nullptr);
        test_subject_nano.advance(CONVERT_2_NANO(microseconds, 999) + CONVERT_2_NANO(nanoseconds, 115));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(microseconds, 999) +
                CONVERT_2_NANO(nanoseconds, 115) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(nanoseconds, 885));
        REQUIRE(expired == true);
        REQUIRE(CONVERT_2_NANO(microseconds, 1000) == test_subject_nano.get_previous_processed_tick() - start_time);
    }
}
