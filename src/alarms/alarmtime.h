// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "store/store.h"

#include <cstdint>
#include <optional>
#include <string>

namespace clockapp::alarmtime {

// A snoozed alarm rings again this long after.
inline constexpr int SnoozeMinutes = 9;

inline constexpr uint8_t Weekdays = 0b0111110;
inline constexpr uint8_t Weekends = 0b1000001;
inline constexpr uint8_t EveryDay = 0b1111111;

// The Unix second the wall clock first shows hour:minute on the local day
// `daysAhead` after the one `around` falls on. In an hour skipped by a
// daylight saving change, the moment the clock jumps past it; in an hour
// shown twice, its first showing.
int64_t wallTime(int64_t around, int daysAhead, int hour, int minute);

// When a one-time alarm rings: the first hour:minute after it was armed.
int64_t oneTimeRing(const Alarm& alarm);

// A one-time alarm whose ring time has come: it shows as off from then on.
bool isOver(const Alarm& alarm, int64_t now);
// Switched on and not over.
bool isActive(const Alarm& alarm, int64_t now);

// The first ring strictly after `after`; nothing for an alarm that is off or
// over by then.
std::optional<int64_t> nextRing(const Alarm& alarm, int64_t after);

// "Every Day", "Weekdays", "Weekends", or the days' short names in the
// locale's week order; empty for a one-time alarm.
std::string repeatSummary(uint8_t repeat);

// "in 7 hours 20 minutes", "in 5 minutes", "in less than a minute": how
// long until a ring that many seconds ahead.
std::string untilText(int64_t seconds);

} // namespace clockapp::alarmtime
