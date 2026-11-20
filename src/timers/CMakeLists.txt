# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

# The turning drums, shared with the alarm editor's time picker.
add_library(clock-wheel STATIC wheel.cpp)
target_link_libraries(clock-wheel PUBLIC clock-page clock-stopwatch)

add_library(clock-timers STATIC
    composer.cpp
    timercard.cpp
    timermath.cpp
    timerring.cpp
    timerspage.cpp
    wheelpicker.cpp
)
target_link_libraries(clock-timers PUBLIC clock-page clock-stopwatch clock-wheel clock-store clock-alarm-logic)
