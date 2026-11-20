// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "world/sky.h"

#include <algorithm>
#include <cmath>

namespace clockapp::world {

static constexpr double kRadians = M_PI / 180.0;
static constexpr double kNightBelow = -12.0;
static constexpr double kDayAbove = 6.0;

double solarElevation(double latitude, double longitude, int64_t utcMilliseconds)
{
    // Julian centuries since J2000.0.
    const double julianDay = double(utcMilliseconds) / 86400000.0 + 2440587.5;
    const double t = (julianDay - 2451545.0) / 36525.0;

    const double meanLongitude = std::fmod(280.46646 + t * (36000.76983 + t * 0.0003032), 360.0);
    const double meanAnomaly = 357.52911 + t * (35999.05029 - 0.0001537 * t);
    const double eccentricity = 0.016708634 - t * (0.000042037 + 0.0000001267 * t);
    const double m = meanAnomaly * kRadians;
    const double center = std::sin(m) * (1.914602 - t * (0.004817 + 0.000014 * t))
        + std::sin(2 * m) * (0.019993 - 0.000101 * t) + std::sin(3 * m) * 0.000289;
    const double omega = 125.04 - 1934.136 * t;
    const double apparentLongitude = meanLongitude + center - 0.00569 - 0.00478 * std::sin(omega * kRadians);
    const double obliquity = 23.0 + (26.0 + (21.448 - t * (46.815 + t * (0.00059 - t * 0.001813))) / 60.0) / 60.0
        + 0.00256 * std::cos(omega * kRadians);
    const double declination = std::asin(std::sin(obliquity * kRadians) * std::sin(apparentLongitude * kRadians));

    const double y = std::pow(std::tan(obliquity * kRadians / 2), 2);
    const double l0 = meanLongitude * kRadians;
    const double equationOfTime = 4.0 / kRadians
        * (y * std::sin(2 * l0) - 2 * eccentricity * std::sin(m) + 4 * eccentricity * y * std::sin(m) * std::cos(2 * l0)
           - 0.5 * y * y * std::sin(4 * l0) - 1.25 * eccentricity * eccentricity * std::sin(2 * m));

    const double minutesOfDay = std::fmod(std::fmod(double(utcMilliseconds) / 60000.0, 1440.0) + 1440.0, 1440.0);
    const double solarTime = minutesOfDay + equationOfTime + 4 * longitude;
    const double hourAngle = (solarTime / 4 - 180) * kRadians;
    const double phi = latitude * kRadians;
    const double cosZenith = std::sin(phi) * std::sin(declination)
        + std::cos(phi) * std::cos(declination) * std::cos(hourAngle);
    return 90.0 - std::acos(std::clamp(cosZenith, -1.0, 1.0)) / kRadians;
}

static double smoothstep(double edge0, double edge1, double x)
{
    const double t = std::clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3 - 2 * t);
}

static Sky fromElevation(double elevation, bool rising)
{
    Sky sky;
    sky.elevation = elevation;
    sky.daylight = smoothstep(kNightBelow, kDayAbove, elevation);
    if (elevation <= kNightBelow) {
        sky.phase = SkyPhase::Night;
    } else if (elevation >= kDayAbove) {
        sky.phase = SkyPhase::Day;
    } else {
        sky.phase = rising ? SkyPhase::Dawn : SkyPhase::Dusk;
        sky.twilight = (elevation - kNightBelow) / (kDayAbove - kNightBelow);
    }
    return sky;
}

Sky Sky::at(double latitude, double longitude, int64_t utcMilliseconds)
{
    const double elevation = solarElevation(latitude, longitude, utcMilliseconds);
    const double later = solarElevation(latitude, longitude, utcMilliseconds + 10 * 60000);
    return fromElevation(elevation, later > elevation);
}

Sky Sky::fromHour(int hour, int minute)
{
    // A sun that rises at 6:00 and sets at 18:00, 18 degrees of twilight
    // an hour.
    const double hours = hour + minute / 60.0;
    const double fromNoon = std::abs(hours - 12.0);
    return fromElevation(6.0 + (6.0 - fromNoon) * 18.0, hours < 12.0);
}

} // namespace clockapp::world
