clock_add_test(clock-timers-layout-tests layout
    SOURCES timers_layout_test.cpp
    LIBRARIES clock-timers)

# Stills and motion frames of both pages, for looking at them.
add_executable(clock-timers-capture capture.cpp)
target_link_libraries(clock-timers-capture PRIVATE clock-timers)
