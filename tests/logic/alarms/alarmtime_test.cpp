// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "alarms/alarmtime.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <ctime>

using namespace clockapp;

// The wall clock of a zone with daylight saving, whatever the machine's.
class AlarmTimeTest : public testing::Test
{
protected:
    void SetUp() override
    {
        setenv("TZ", "America/New_York", 1);
        tzset();
    }
    void TearDown() override
    {
        unsetenv("TZ");
        tzset();
    }

    // Local wall time; the first of two when the hour is shown twice.
    static int64_t local(int year, int month, int day, int hour, int minute, int isdst = -1)
    {
        std::tm tm{};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_isdst = isdst;
        return int64_t(mktime(&tm));
    }

    static std::tm shown(int64_t t)
    {
        const time_t value = time_t(t);
        std::tm tm{};
        localtime_r(&value, &tm);
        return tm;
    }

    static Alarm alarm(int hour, int minute, uint8_t repeat, int64_t armedAt = 0)
    {
        Alarm alarm;
        alarm.id = "a";
        alarm.hour = hour;
        alarm.minute = minute;
        alarm.repeat = repeat;
        alarm.armedAt = armedAt;
        return alarm;
    }
};

TEST_F(AlarmTimeTest, OneTimeRingsAtTheNextShowing)
{
    // Armed at 21:00 on Friday 2026-11-20: 7:30 is tomorrow, 22:15 tonight.
    const int64_t armed = local(2026, 11, 20, 21, 0);
    EXPECT_EQ(alarmtime::nextRing(alarm(7, 30, 0, armed), armed), local(2026, 11, 21, 7, 30));
    EXPECT_EQ(alarmtime::nextRing(alarm(22, 15, 0, armed), armed), local(2026, 11, 20, 22, 15));
    // The same minute as armed: a day ahead, not at once.
    EXPECT_EQ(alarmtime::nextRing(alarm(21, 0, 0, armed), armed), local(2026, 11, 21, 21, 0));
}

TEST_F(AlarmTimeTest, OneTimeIsOverOnceRung)
{
    const int64_t armed = local(2026, 11, 20, 21, 0);
    const Alarm once = alarm(22, 15, 0, armed);
    const int64_t ring = local(2026, 11, 20, 22, 15);
    EXPECT_FALSE(alarmtime::isOver(once, ring - 1));
    EXPECT_TRUE(alarmtime::isActive(once, ring - 1));
    EXPECT_TRUE(alarmtime::isOver(once, ring));
    EXPECT_FALSE(alarmtime::isActive(once, ring + 60));
    EXPECT_FALSE(alarmtime::nextRing(once, ring));
    // Rearmed later: tomorrow.
    EXPECT_EQ(alarmtime::nextRing(alarm(22, 15, 0, ring + 60), ring + 60), local(2026, 11, 21, 22, 15));
}

TEST_F(AlarmTimeTest, OffNeverRings)
{
    Alarm off = alarm(7, 0, alarmtime::EveryDay);
    off.enabled = false;
    EXPECT_FALSE(alarmtime::nextRing(off, local(2026, 11, 20, 6, 0)));
    EXPECT_FALSE(alarmtime::isActive(off, local(2026, 11, 20, 6, 0)));
}

TEST_F(AlarmTimeTest, RepeatsOnTheChosenDays)
{
    // 2026-11-20 is a Friday.
    const int64_t friday = local(2026, 11, 20, 8, 0);
    EXPECT_EQ(alarmtime::nextRing(alarm(7, 0, alarmtime::Weekdays), friday), local(2026, 11, 23, 7, 0));
    EXPECT_EQ(alarmtime::nextRing(alarm(9, 0, alarmtime::Weekdays), friday), local(2026, 11, 20, 9, 0));
    EXPECT_EQ(alarmtime::nextRing(alarm(7, 0, alarmtime::Weekends), friday), local(2026, 11, 21, 7, 0));
    EXPECT_EQ(alarmtime::nextRing(alarm(7, 0, alarmtime::EveryDay), friday), local(2026, 11, 21, 7, 0));
    // Fridays only, the time just gone: a week ahead.
    EXPECT_EQ(alarmtime::nextRing(alarm(8, 0, 1 << 5), friday), local(2026, 11, 27, 8, 0));
    // Wednesday only.
    EXPECT_EQ(alarmtime::nextRing(alarm(8, 0, 1 << 3), friday), local(2026, 11, 25, 8, 0));
}

TEST_F(AlarmTimeTest, KeepsTheWallClockAcrossDaylightSaving)
{
    // Clocks went back at 2:00 on Sunday 2026-11-01: 7:00 stays 7:00.
    const int64_t saturday = local(2026, 10, 31, 8, 0);
    const int64_t ring = *alarmtime::nextRing(alarm(7, 0, alarmtime::EveryDay), saturday);
    EXPECT_EQ(shown(ring).tm_hour, 7);
    EXPECT_EQ(shown(ring).tm_mday, 1);
    EXPECT_EQ(ring - saturday, 24 * 3600);

    // Forward at 2:00 on Sunday 2026-03-08.
    const int64_t before = local(2026, 3, 7, 8, 0);
    const int64_t spring = *alarmtime::nextRing(alarm(7, 0, alarmtime::EveryDay), before);
    EXPECT_EQ(shown(spring).tm_hour, 7);
    EXPECT_EQ(spring - before, 22 * 3600);
}

TEST_F(AlarmTimeTest, SkippedTimeRingsWhenTheClockJumps)
{
    // 2:30 does not exist on 2026-03-08; the clock goes from 1:59 to 3:00.
    const int64_t before = local(2026, 3, 7, 23, 0);
    const int64_t ring = *alarmtime::nextRing(alarm(2, 30, alarmtime::EveryDay), before);
    EXPECT_EQ(shown(ring).tm_hour, 3);
    EXPECT_EQ(shown(ring).tm_min, 0);
    EXPECT_EQ(shown(ring).tm_mday, 8);
}

TEST_F(AlarmTimeTest, RepeatedHourRingsOnce)
{
    // 1:30 is shown twice on 2026-11-01: the first, in daylight time.
    const int64_t before = local(2026, 10, 31, 23, 0);
    const Alarm night = alarm(1, 30, alarmtime::EveryDay);
    const int64_t first = *alarmtime::nextRing(night, before);
    EXPECT_EQ(first, local(2026, 11, 1, 1, 30, 1));
    const int64_t next = *alarmtime::nextRing(night, first);
    EXPECT_EQ(shown(next).tm_mday, 2);
    EXPECT_EQ(shown(next).tm_hour, 1);
}

TEST_F(AlarmTimeTest, PastTimesLookAhead)
{
    // Asked long after, as after the clock was set forward: the next showing
    // from then, not the missed ones.
    const Alarm daily = alarm(6, 0, alarmtime::EveryDay);
    const int64_t later = local(2027, 1, 5, 12, 0);
    EXPECT_EQ(alarmtime::nextRing(daily, later), local(2027, 1, 6, 6, 0));
}

TEST_F(AlarmTimeTest, Summaries)
{
    EXPECT_EQ(alarmtime::repeatSummary(0), "");
    EXPECT_EQ(alarmtime::repeatSummary(alarmtime::EveryDay), "Every Day");
    EXPECT_EQ(alarmtime::repeatSummary(alarmtime::Weekdays), "Weekdays");
    EXPECT_EQ(alarmtime::repeatSummary(alarmtime::Weekends), "Weekends");
    EXPECT_FALSE(alarmtime::repeatSummary(0b0101010).empty());
}

TEST(SnoozeTest, NineMinutes)
{
    EXPECT_EQ(alarmtime::SnoozeMinutes, 9);
}
