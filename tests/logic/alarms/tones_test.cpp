// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "alarms/tones.h"
#include "store/store.h"

#include <gtest/gtest.h>

#include <cmath>

using namespace clockapp;

TEST(TonesTest, EveryToneLoopsQuietlyAtItsSeam)
{
    for (const std::string& name : clockstore::soundNames()) {
        const std::vector<float> samples = tones::synthesize(name);
        ASSERT_GT(samples.size(), size_t(tones::SampleRate)) << name;
        float peak = 0;
        float step = 0;
        for (size_t i = 0; i < samples.size(); ++i) {
            ASSERT_TRUE(std::isfinite(samples[i])) << name;
            peak = std::max(peak, std::abs(samples[i]));
            if (i > 0)
                step = std::max(step, std::abs(samples[i] - samples[i - 1]));
        }
        EXPECT_NEAR(peak, 0.9f, 0.001f) << name;
        // The loop's end runs into its start as smoothly as any two samples
        // follow each other inside it: no click.
        EXPECT_LE(std::abs(samples.back() - samples.front()), step) << name;
        EXPECT_FALSE(tones::displayName(name).empty());
    }
}

TEST(TonesTest, UnknownNamesFallBack)
{
    EXPECT_EQ(tones::resolved(""), clockstore::soundNames().front());
    EXPECT_EQ(tones::resolved("siren"), clockstore::soundNames().front());
    EXPECT_EQ(tones::resolved("glow"), "glow");
}
