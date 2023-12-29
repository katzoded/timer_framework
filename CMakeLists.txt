cmake_minimum_required(VERSION 3.24)
project(timer_framework)

set(CMAKE_CXX_STANDARD 17)

include_directories(AFTER "${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories(AFTER "${CMAKE_CURRENT_SOURCE_DIR}/extern/public/Catch2/single_include/catch2")

add_executable(timer_framework main.cpp)

FILE(GLOB TEST_SRC "test/*.cpp")
add_executable(timer_framework_test ${TEST_SRC})
set_property(TARGET timer_framework_test PROPERTY CXX_STANDARD 17)
target_link_libraries(timer_framework_test)
target_compile_options(timer_framework_test PUBLIC "-fno-omit-frame-pointer" "-fsanitize=address")
target_link_options(timer_framework_test PUBLIC "-fsanitize=address")
add_custom_target(run_timer_framework_test $<TARGET_FILE:timer_framework_test>)
