# The timers' arithmetic alone, built from its source: no window.
clock_add_test(clock-timers-logic-tests logic
    SOURCES timermath_test.cpp ${PROJECT_SOURCE_DIR}/src/timers/timermath.cpp
    LIBRARIES clock-store)
target_include_directories(clock-timers-logic-tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
