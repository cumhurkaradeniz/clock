add_library(clock-service STATIC alarmservice.cpp)
target_link_libraries(clock-service PUBLIC clock-alarm-logic)

add_executable(cutefish-clock-alarms main.cpp)
target_link_libraries(cutefish-clock-alarms PRIVATE clock-service)
install(TARGETS cutefish-clock-alarms RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
