#define UNIT_TESTING
#include "with_thread_timer_manager.hpp"
#include "catch.hpp"

using namespace std;


//static WithThreadTimerManager test_subject_milli(milli_sec);

SCENARIO("milli-sec-with-thread-timer")
{
    WithThreadTimerManager test_subject_nano(milli_sec);
    test_subject_nano.advance(CONVERT_2_NANO(nanoseconds, 1));

    SECTION("test 100ms sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_previous_processed_tick();
        test_subject_nano.start_timer(CONVERT_2_NANO(milliseconds, 100), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
        }, nullptr);
        test_subject_nano.advance(CONVERT_2_NANO(milliseconds, 99));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(milliseconds, 99) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(milliseconds, 2));
        REQUIRE(expired == true);
        REQUIRE(CONVERT_2_NANO(milliseconds, 101) == test_subject_nano.get_previous_processed_tick() - start_time);
    }
    SECTION("test 10080 ms sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_previous_processed_tick();
        test_subject_nano.start_timer(CONVERT_2_NANO(milliseconds, 10080), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
        }, nullptr);
        test_subject_nano.advance(CONVERT_2_NANO(milliseconds, 80));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(milliseconds, 80) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(seconds, 3));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(seconds, 3) +
                CONVERT_2_NANO(milliseconds, 80) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(seconds, 7));
        REQUIRE(expired == true);
        REQUIRE(CONVERT_2_NANO(seconds, 10) +
                CONVERT_2_NANO(milliseconds, 80) == test_subject_nano.get_previous_processed_tick() - start_time);
    }
    SECTION("test 10000 ms sec exp")
    {
        bool expired = false;
        uint64_t start_time = test_subject_nano.get_previous_processed_tick();
        test_subject_nano.start_timer(CONVERT_2_NANO(milliseconds, 10000), [&](TIMER_HANDLE timer, void *cookie) {
            expired = true;
        }, nullptr);
        test_subject_nano.advance(CONVERT_2_NANO(milliseconds, 80));
        REQUIRE(CONVERT_2_NANO(milliseconds, 80) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(seconds, 3));
        REQUIRE(expired == false);
        REQUIRE(CONVERT_2_NANO(seconds, 3) +
                CONVERT_2_NANO(milliseconds, 80) == test_subject_nano.get_previous_processed_tick() - start_time);
        test_subject_nano.advance(CONVERT_2_NANO(seconds, 7));
        REQUIRE(expired == true);
        REQUIRE(CONVERT_2_NANO(seconds, 10) +
                CONVERT_2_NANO(milliseconds, 80) == test_subject_nano.get_previous_processed_tick() - start_time);
    }
//    SECTION("test 100milli + 50usec + 300nano sec exp")
//    {
//        bool expired = false;
//        uint64_t start_time = test_subject_nano.get_previous_processed_tick();
//        test_subject_nano.start_timer(std::chrono::duration_cast<std::chrono::nanoseconds>(100ms + 50us + 300ms).count(), [&](TIMER_HANDLE timer, void *cookie) {
//            expired = true;
//        }, nullptr);
//        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(300ns).count());
//        REQUIRE(expired == false);
//        REQUIRE(300 == test_subject_nano.get_previous_processed_tick() - start_time);
//        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(50us).count());
//        REQUIRE(expired == false);
//        REQUIRE(50300 == test_subject_nano.get_previous_processed_tick() - start_time);
//        test_subject_nano.advance(std::chrono::duration_cast<std::chrono::nanoseconds>(100ms).count());
//        REQUIRE(expired == true);
//        REQUIRE(100050300 == test_subject_nano.get_previous_processed_tick() - start_time);
//    }
}
