// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "stopwatch/motion.h"
#include "ui/view.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace cutefish::ui {
class Label;
}

namespace clockapp {

class DigitDisplay;

// The stopwatch drawn as a dial: a 60-second face in half seconds, a
// 30-minute sub-dial above the centre, the seconds hand and, once laps are
// taken, the lap hand. The face is drawn once into a layer of its own; only
// the hands paint on each frame.
class DialView : public cutefish::ui::View
{
public:
    explicit DialView(cutefish::ui::Window* window);
    ~DialView() override;

    // lapNs < 0 hides the lap hand.
    void setTimes(int64_t elapsedNs, int64_t lapNs);
    // The hands sweep back to twelve from where they are.
    void sweepToZero();

    // The face, square and centred in the view.
    cutefish::ui::RectF faceRect() const;
    DigitDisplay* readout() const { return m_readout; }

    cutefish::ui::SizeF sizeHint() const override { return {300, 300}; }

protected:
    void layout() override;

private:
    class Face;
    class Hands;

    void updateHands();

    Face* m_face = nullptr;
    Hands* m_hands = nullptr;
    DigitDisplay* m_readout = nullptr;
    std::unique_ptr<Glide> m_sweep;
    bool m_sweeping = false;
    int64_t m_elapsedNs = 0;
    int64_t m_lapNs = -1;
};

} // namespace clockapp
