# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

# The stopwatch page, and the round buttons, figures and motion the timers
# page shares.
add_library(clock-stopwatch STATIC
    dialview.cpp
    digitdisplay.cpp
    lapslist.cpp
    motion.cpp
    roundbutton.cpp
    stopwatch.cpp
    stopwatchpage.cpp
)
target_link_libraries(clock-stopwatch PUBLIC clock-page)
