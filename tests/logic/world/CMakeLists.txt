# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

clock_add_test(clock-world-logic-tests logic
    SOURCES
        cities_test.cpp
        settle_test.cpp
        citydatabase_test.cpp
        sky_test.cpp
        zonetime_test.cpp
    LIBRARIES clock-world-logic)
