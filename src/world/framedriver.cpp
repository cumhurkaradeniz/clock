// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "world/framedriver.h"

#include <algorithm>

namespace clockapp::world {

void FrameDriver::wake()
{
    if (isTicking())
        return;
    m_lastFrameNs = 0;
    startTicking();
}

void FrameDriver::tick(uint64_t frameTimeNs)
{
    // A frame that comes late after a stall moves things on by at most a
    // few refreshes, so nothing leaps.
    const double elapsed = m_lastFrameNs == 0 ? 0 : std::min(double(frameTimeNs - m_lastFrameNs) / 1e6, 50.0);
    m_lastFrameNs = frameTimeNs;
    if (!onFrame || !onFrame(elapsed, frameTimeNs))
        stopTicking();
}

} // namespace clockapp::world
