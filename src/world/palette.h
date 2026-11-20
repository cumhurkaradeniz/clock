// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "ui/geometry.h"
#include "world/sky.h"

namespace clockapp::world {

// The colours of a city's face and card at its sky, blended continuously
// from night through twilight to day, for the window's dark page.
struct SkyPalette
{
    // The face's dial, top to bottom.
    cutefish::ui::Color dialTop;
    cutefish::ui::Color dialBottom;
    cutefish::ui::Color rim;
    cutefish::ui::Color ticks;
    cutefish::ui::Color numerals;
    cutefish::ui::Color hands;
    cutefish::ui::Color handShadow;
    // Over the card's top, fading into the card.
    cutefish::ui::Color glow;
    // Stars over the dial, 0 by day.
    double stars = 0;

    static SkyPalette of(const Sky& sky);
};

cutefish::ui::Color mix(cutefish::ui::Color a, cutefish::ui::Color b, double t);

// The second hand, the same in every sky.
inline constexpr cutefish::ui::Color kSecondHand = cutefish::ui::Color::rgb(0xFF7A1A);

} // namespace clockapp::world
