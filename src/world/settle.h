// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

namespace clockapp::world {

// The second hand's step: an underdamped spring's response, from 0 at the
// second's start through a slight overshoot back to 1, at rest by
// kSettleMs. Past that it is 1.
inline constexpr double kSettleMs = 320;
double secondHandStep(double elapsedMs);

// A critically damped spring, for positions and fades that may be retargeted
// mid-flight: advance() moves value and velocity on by the frame's time.
struct SpringState
{
    double value = 0;
    double velocity = 0;
    double target = 0;

    // `settleMs` is the time it takes to cover nearly all of the distance.
    void advance(double elapsedMs, double settleMs);
    bool isAtRest() const;
    void jump(double to)
    {
        value = target = to;
        velocity = 0;
    }
};

} // namespace clockapp::world
