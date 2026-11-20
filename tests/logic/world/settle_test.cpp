// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <cmath>

#include "world/settle.h"

using namespace clockapp::world;

TEST(Settle, SecondHandStepOvershootsSlightlyAndLands)
{
    EXPECT_DOUBLE_EQ(secondHandStep(0), 0);
    EXPECT_DOUBLE_EQ(secondHandStep(kSettleMs), 1);
    EXPECT_DOUBLE_EQ(secondHandStep(5000), 1);

    double peak = 0;
    double previous = 0;
    for (double t = 1; t < kSettleMs; t += 1) {
        const double value = secondHandStep(t);
        peak = std::max(peak, value);
        // No jump from one millisecond to the next, nor into rest.
        EXPECT_LT(std::abs(value - previous), 0.03) << t;
        previous = value;
    }
    EXPECT_LT(std::abs(previous - 1), 0.01);
    EXPECT_GT(peak, 1.02);
    EXPECT_LT(peak, 1.15);
    EXPECT_GT(secondHandStep(90), 0.8);
}

TEST(Settle, SpringReachesItsTargetAndCanBeRetargeted)
{
    SpringState spring;
    spring.jump(0);
    spring.target = 100;
    double previous = 0;
    for (int frame = 0; frame < 60; ++frame) {
        spring.advance(16.7, 300);
        // Critically damped: never past the target.
        EXPECT_LE(spring.value, 100 + 1e-9);
        EXPECT_GE(spring.value, previous);
        previous = spring.value;
    }
    EXPECT_TRUE(spring.isAtRest());
    EXPECT_DOUBLE_EQ(spring.value, 100);

    // Halfway, back again: the motion turns without a jump.
    spring.target = 0;
    spring.advance(50, 300);
    const double midway = spring.value;
    spring.target = 100;
    spring.advance(1, 300);
    EXPECT_LT(std::abs(spring.value - midway), 2);
}
