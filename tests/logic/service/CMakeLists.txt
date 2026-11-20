# As clock_add_test makes a logic test, one at a time: each case owns the
# private bus's notification server name.
add_executable(clock-service-logic-tests alarmservice_test.cpp)
target_link_libraries(clock-service-logic-tests PRIVATE clock-service GTest::gtest_main)
gtest_discover_tests(clock-service-logic-tests PROPERTIES LABELS logic RESOURCE_LOCK clock-notifications)
