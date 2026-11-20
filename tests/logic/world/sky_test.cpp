// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "world/sky.h"

using namespace clockapp::world;

// 2026-06-21 00:00:00 UTC, near the June solstice.
static constexpr int64_t kJune21 = 1782000000000;

static int64_t at(int64_t dayStart, double hours)
{
    return dayStart + int64_t(hours * 3600000);
}

TEST(Sky, SolarElevationAtNoonAndMidnight)
{
    // The sun stands over the Tropic of Cancer at the solstice: at solar
    // noon on the equator it is about 66.5 degrees up.
    EXPECT_NEAR(solarElevation(0, 0, at(kJune21, 12)), 66.5, 0.6);
    EXPECT_NEAR(solarElevation(23.44, 0, at(kJune21, 12)), 90, 0.6);
    // Midnight sun at the North Pole, polar night at the South.
    EXPECT_GT(solarElevation(89, 0, at(kJune21, 0)), 20);
    EXPECT_LT(solarElevation(-89, 0, at(kJune21, 12)), -20);
}

TEST(Sky, PhasesOverADayInSingapore)
{
    // 2026-11-20; Singapore's sun rises about 06:50 local, 22:50 UTC the day
    // before, and sets about 19:00 local, 11:00 UTC.
    const int64_t november20 = 1795132800000;
    const double latitude = 1.283;
    const double longitude = 103.85;

    EXPECT_EQ(Sky::at(latitude, longitude, at(november20, 4)).phase, SkyPhase::Day);
    EXPECT_DOUBLE_EQ(Sky::at(latitude, longitude, at(november20, 4)).daylight, 1);
    EXPECT_EQ(Sky::at(latitude, longitude, at(november20, 11)).phase, SkyPhase::Dusk);
    EXPECT_EQ(Sky::at(latitude, longitude, at(november20, 16)).phase, SkyPhase::Night);
    EXPECT_DOUBLE_EQ(Sky::at(latitude, longitude, at(november20, 16)).daylight, 0);
    EXPECT_EQ(Sky::at(latitude, longitude, at(november20, 22.8)).phase, SkyPhase::Dawn);

    const Sky dusk = Sky::at(latitude, longitude, at(november20, 11));
    EXPECT_GT(dusk.daylight, 0);
    EXPECT_LT(dusk.daylight, 1);
    EXPECT_GT(dusk.twilight, 0);
    EXPECT_LT(dusk.twilight, 1);
}

TEST(Sky, DaylightChangesSmoothly)
{
    const int64_t november20 = 1795132800000;
    double previous = Sky::at(51.5, 0, at(november20, 12)).daylight;
    // London in the evening, a minute at a time: no step larger than a
    // minute of twilight allows.
    for (int minute = 1; minute < 8 * 60; ++minute) {
        const double daylight = Sky::at(51.5, 0, at(november20, 12 + minute / 60.0)).daylight;
        EXPECT_LE(daylight, previous + 1e-9);
        EXPECT_LT(previous - daylight, 0.02);
        previous = daylight;
    }
    EXPECT_DOUBLE_EQ(previous, 0);
}

TEST(Sky, FromHour)
{
    EXPECT_EQ(Sky::fromHour(3, 0).phase, SkyPhase::Night);
    EXPECT_EQ(Sky::fromHour(5, 30).phase, SkyPhase::Dawn);
    EXPECT_EQ(Sky::fromHour(12, 0).phase, SkyPhase::Day);
    EXPECT_EQ(Sky::fromHour(18, 30).phase, SkyPhase::Dusk);
    EXPECT_EQ(Sky::fromHour(23, 0).phase, SkyPhase::Night);
}
