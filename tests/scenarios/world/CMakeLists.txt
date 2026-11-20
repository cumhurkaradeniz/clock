# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

clock_add_test(clock-world-scenario-tests scenario
    SOURCES worldclock_test.cpp
    LIBRARIES clock-world)
