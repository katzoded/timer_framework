#define UNIT_TESTING
#include "with_thread_timer_manager.hpp"
#include "catch.hpp"

using namespace std;


//static WithThreadTimerManager test_subject_milli(milli_sec);

SCENARIO("milli-sec-with-thread-timer")
{
    WithThreadTimerManager test_subject_nano(milli_sec);
    test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(1ms).count());

    SECTION("test 100ms sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_current_tick();
        test_subject_nano.start_timer(std::chrono::duration_cast<std::chrono::nanoseconds>(100ms).count(), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
        }, nullptr);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(99ms).count());
        REQUIRE(expired == false);
        REQUIRE(99000000 == test_subject_nano.get_current_tick() - start_time);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(2ms).count());
        REQUIRE(expired == true);
        REQUIRE(101000000 == test_subject_nano.get_current_tick() - start_time);
    }
    SECTION("test 10080 ms sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_current_tick();
        test_subject_nano.start_timer(std::chrono::duration_cast<std::chrono::nanoseconds>(10080ms).count(), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
            uint64_t expiration_time = test_subject_nano.get_current_tick() - start_time;
        }, nullptr);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(80ms).count());
        REQUIRE(expired == false);
        REQUIRE(80000000 == test_subject_nano.get_current_tick() - start_time);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(3s).count());
        REQUIRE(expired == false);
        REQUIRE(3080000000 == test_subject_nano.get_current_tick() - start_time);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(7s).count());
        REQUIRE(expired == true);
        REQUIRE(10080000000 == test_subject_nano.get_current_tick() - start_time);
    }
    SECTION("test 10000 ms sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_current_tick();
        test_subject_nano.start_timer(std::chrono::duration_cast<std::chrono::nanoseconds>(10000ms).count(), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
            uint64_t expiration_time = test_subject_nano.get_current_tick() - start_time;
        }, nullptr);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(80ms).count());
        REQUIRE(expired == false);
        REQUIRE(80000000 == test_subject_nano.get_current_tick() - start_time);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(3s).count());
        REQUIRE(expired == false);
        REQUIRE(3080000000 == test_subject_nano.get_current_tick() - start_time);
        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(7s).count());
        REQUIRE(expired == true);
        REQUIRE(10080000000 == test_subject_nano.get_current_tick() - start_time);
    }
//    SECTION("test 100milli + 50usec + 300nano sec exp")
//    {
//        bool expired = false;
//        uint64_t start_time = test_subject_nano.get_current_tick();
//        test_subject_nano.start_timer(std::chrono::duration_cast<std::chrono::nanoseconds>(100ms + 50us + 300ms).count(), [&](TIMER_HANDLE timer, void *cookie) {
//            expired = true;
//        }, nullptr);
//        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(300ns).count());
//        REQUIRE(expired == false);
//        REQUIRE(300 == test_subject_nano.get_current_tick() - start_time);
//        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(50us).count());
//        REQUIRE(expired == false);
//        REQUIRE(50300 == test_subject_nano.get_current_tick() - start_time);
//        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(100ms).count());
//        REQUIRE(expired == true);
//        REQUIRE(100050300 == test_subject_nano.get_current_tick() - start_time);
//    }
}
