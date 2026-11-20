// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "world/settle.h"

#include <cmath>

namespace clockapp::world {

// Damping and natural frequency: about a twelfth of a step of overshoot,
// most of the way there in the first 90 ms.
static constexpr double kDamping = 0.62;
static constexpr double kFrequency = 2 * M_PI / 210.0;

double secondHandStep(double elapsedMs)
{
    if (elapsedMs <= 0)
        return 0;
    if (elapsedMs >= kSettleMs)
        return 1;
    const double damped = kFrequency * std::sqrt(1 - kDamping * kDamping);
    const double decay = std::exp(-kDamping * kFrequency * elapsedMs);
    const double response = 1 - decay * (std::cos(damped * elapsedMs)
                                         + kDamping / std::sqrt(1 - kDamping * kDamping) * std::sin(damped * elapsedMs));
    // What is left at kSettleMs is blended away so the hand lands on 1
    // without a jump.
    const double blend = elapsedMs / kSettleMs;
    return response + (1 - response) * blend * blend * blend;
}

void SpringState::advance(double elapsedMs, double settleMs)
{
    // Critically damped: omega from the settle time, about 4.6 time
    // constants to within 1%.
    const double omega = 4.6 / settleMs;
    double remaining = elapsedMs;
    // Small steps keep the explicit integration stable on a long frame.
    while (remaining > 0) {
        const double step = remaining > 4 ? 4 : remaining;
        const double displacement = value - target;
        const double acceleration = -omega * omega * displacement - 2 * omega * velocity;
        velocity += acceleration * step;
        value += velocity * step;
        remaining -= step;
    }
    if (isAtRest())
        jump(target);
}

bool SpringState::isAtRest() const
{
    // A hundredth of a pixel or of a fade: no frame would show the rest.
    return std::abs(value - target) < 0.01 && std::abs(velocity) < 0.0005;
}

} // namespace clockapp::world
