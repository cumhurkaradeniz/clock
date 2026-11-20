// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <cstdint>

namespace clockapp::world {

// The sun's height above the horizon at a place and moment, in degrees:
// negative below it. NOAA's low-precision formulas, within a few tenths of
// a degree, which is plenty for the colour of a sky.
double solarElevation(double latitude, double longitude, int64_t utcMilliseconds);

enum class SkyPhase {
    Night,
    Dawn,
    Day,
    Dusk,
};

// Where a city's sky stands: the phase, and how much daylight there is,
// from 0 below -12 degrees (nautical dusk over) to 1 above +6, eased in
// between, so faces and cards blend from night to day without steps.
struct Sky
{
    SkyPhase phase = SkyPhase::Day;
    double daylight = 1;
    // Within dawn or dusk, from 0 to 1; the warm glow peaks at sunrise and
    // sunset, at the middle.
    double twilight = 0;
    double elevation = 90;

    // Morning or evening is told by whether the sun is rising, from its
    // height a few minutes later.
    static Sky at(double latitude, double longitude, int64_t utcMilliseconds);
    // Without a place: day from 6:00 to 17:59 by the wall clock there, with
    // an hour of dawn and dusk on either side.
    static Sky fromHour(int hour, int minute);
};

} // namespace clockapp::world
