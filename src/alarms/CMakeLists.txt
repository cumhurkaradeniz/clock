# What the alarm service shares with the page: ring times and the tones.
add_library(clock-alarm-logic STATIC
    alarmtime.cpp
)
target_link_libraries(clock-alarm-logic PUBLIC clock-store)
