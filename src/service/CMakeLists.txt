add_library(clock-service STATIC alarmservice.cpp)
target_link_libraries(clock-service PUBLIC clock-alarm-logic)

add_executable(cutefish-clock-alarms main.cpp)
target_link_libraries(cutefish-clock-alarms PRIVATE clock-service)
install(TARGETS cutefish-clock-alarms RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# The translated copy, which the top level's cutefish_add_translations makes.
install(FILES ${CMAKE_BINARY_DIR}/cutefish-clock-alarms.desktop DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/xdg/autostart)
configure_file(org.cutefish.ClockAlarms.service.in ${CMAKE_CURRENT_BINARY_DIR}/org.cutefish.ClockAlarms.service @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.cutefish.ClockAlarms.service
        DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
