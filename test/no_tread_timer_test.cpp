#define UNIT_TESTING
#include "timer_manager.hpp"
#include "catch.hpp"

using namespace std;


NoThreadTimerManager_t test_subject_nano(nano_sec);
NoThreadTimerManager_t test_subject_milli(milli_sec);
NoThreadTimerManager_t test_subject_sec(single_sec);

TEST_CASE("nano-sec-no-thread-timer", "nano-sec")
{
    SECTION("test 100 nano sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_current_tick();
        test_subject_nano.StartTimer(std::chrono::duration_cast<std::chrono::nanoseconds>(100ns).count(), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
            uint64_t expiration_time = test_subject_nano.get_current_tick() - start_time;
        }, nullptr);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(99ns).count());
        REQUIRE(expired == false);
        REQUIRE(99 == test_subject_nano.get_current_tick() - start_time);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(2ns).count());
        REQUIRE(expired == true);
        REQUIRE(101 == test_subject_nano.get_current_tick() - start_time);
    }
    SECTION("test 100080 nano sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_current_tick();
        test_subject_nano.StartTimer(std::chrono::duration_cast<std::chrono::nanoseconds>(10080ns).count(), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
            uint64_t expiration_time = test_subject_nano.get_current_tick() - start_time;
        }, nullptr);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(80ns).count());
        REQUIRE(expired == false);
        REQUIRE(80 == test_subject_nano.get_current_tick() - start_time);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(3us).count());
        REQUIRE(expired == false);
        REQUIRE(3080 == test_subject_nano.get_current_tick() - start_time);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(7us).count());
        REQUIRE(expired == true);
        REQUIRE(10080 == test_subject_nano.get_current_tick() - start_time);
    }
    SECTION("test 100000 nano sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_current_tick();
        test_subject_nano.StartTimer(std::chrono::duration_cast<std::chrono::nanoseconds>(10000ns).count(), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
            uint64_t expiration_time = test_subject_nano.get_current_tick() - start_time;
        }, nullptr);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(80ns).count());
        REQUIRE(expired == false);
        REQUIRE(80 == test_subject_nano.get_current_tick() - start_time);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(3us).count());
        REQUIRE(expired == false);
        REQUIRE(3080 == test_subject_nano.get_current_tick() - start_time);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(7us).count());
        REQUIRE(expired == true);
        REQUIRE(10080 == test_subject_nano.get_current_tick() - start_time);
    }
    SECTION("test 100milli + 50usec + 300nano sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_current_tick();
        test_subject_nano.StartTimer(std::chrono::duration_cast<std::chrono::nanoseconds>(100ms + 50us + 300ns).count(), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
            uint64_t expiration_time = test_subject_nano.get_current_tick() - start_time;
        }, nullptr);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(300ns).count());
        REQUIRE(expired == false);
        REQUIRE(300 == test_subject_nano.get_current_tick() - start_time);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(50us).count());
        REQUIRE(expired == false);
        REQUIRE(50300 == test_subject_nano.get_current_tick() - start_time);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(100ms).count());
        REQUIRE(expired == true);
        REQUIRE(100050300 == test_subject_nano.get_current_tick() - start_time);
    }
}

TEST_CASE("mill-sec-no-thread-timer", "mill-sec")
{
    SECTION("test 100 nano sec, not valid")
    {
        test_subject_milli.StartTimer(std::chrono::duration_cast<std::chrono::nanoseconds>(100ns).count(), [&](TIMER_HANDLE timer, void *cookie) {
        }, nullptr);
        REQUIRE(0 == test_subject_milli.number_of_active_timers());
    }
    SECTION("test 1000 milli sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_milli.get_current_tick();
        test_subject_milli.StartTimer(std::chrono::duration_cast<std::chrono::nanoseconds>(100ms).count(), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
            uint64_t expiration_time = test_subject_milli.get_current_tick() - start_time;
        }, nullptr);
        test_subject_milli.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(99ms).count());
        REQUIRE(expired == false);
        REQUIRE(99000000 == test_subject_milli.get_current_tick() - start_time);
        test_subject_milli.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(1ms).count());
        REQUIRE(expired == true);
        REQUIRE(100000000 == test_subject_milli.get_current_tick() - start_time);
    }
}
