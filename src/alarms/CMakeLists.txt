find_package(PkgConfig REQUIRED)
pkg_check_modules(PipeWire REQUIRED IMPORTED_TARGET libpipewire-0.3)

# What the alarm service shares with the page: ring times and the tones.
add_library(clock-alarm-logic STATIC
    alarmtime.cpp
    tones.cpp
    toneplayer.cpp
)
target_link_libraries(clock-alarm-logic PUBLIC clock-store PRIVATE PkgConfig::PipeWire)

add_library(clock-alarms STATIC
    alarmeditor.cpp
    alarmrow.cpp
    alarmspage.cpp
    spring.cpp
    timepicker.cpp
)
target_link_libraries(clock-alarms PUBLIC clock-page clock-alarm-logic clock-wheel)
