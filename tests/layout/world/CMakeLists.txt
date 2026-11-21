# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

clock_add_test(clock-world-layout-tests layout
    SOURCES
        palette_test.cpp
        worldclock_layout_test.cpp
    LIBRARIES clock-world)
