// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "ui/view.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace cutefish::ui {
class Animation;
class Label;
}

namespace clockapp {

// The header's segmented control. Its plate glides to the picked segment on
// the frame clock and settles there with a slight overshoot, stretching a
// little on the way; a pick during the glide starts from where the plate is.
class TabSwitch : public cutefish::ui::View
{
public:
    static constexpr double kHeight = 28;
    static constexpr double kPadding = 2;

    explicit TabSwitch(std::vector<std::string> items);
    ~TabSwitch() override;

    void setItems(std::vector<std::string> items);
    // View names for the segments' labels, for tests.
    void setSegmentNames(const std::vector<std::string>& names);
    size_t count() const { return m_labels.size(); }
    int currentIndex() const { return m_current; }
    // Does not call onCurrentChanged.
    void setCurrentIndex(int index, bool animated);
    // Where the plate is, in segments: the current index at rest.
    double platePosition() const { return m_position; }
    bool isGliding() const;
    // The plate's rect as now drawn, in local coordinates.
    cutefish::ui::RectF plateRect() const;
    cutefish::ui::RectF itemRect(size_t index) const;
    // The widest label's width times the count, and the padding.
    cutefish::ui::SizeF sizeHint() const override;

    bool acceptsPointer() const override { return true; }

    std::function<void(int index)> onCurrentChanged;

protected:
    void layout() override;
    void paint(cutefish::ui::Painter& painter) override;
    void pointerPress(const cutefish::ui::PointerEvent& event) override;
    void pointerMove(const cutefish::ui::PointerEvent& event) override;
    void pointerRelease(const cutefish::ui::PointerEvent& event) override;
    void pointerLeave() override;

private:
    int itemAt(cutefish::ui::PointF point) const;
    void setPosition(double position, double stretch);
    void updateLabels();

    std::vector<cutefish::ui::Label*> m_labels;
    int m_current = 0;
    int m_pressed = -1;
    bool m_pressInside = false;
    double m_position = 0;
    double m_stretch = 0;
    double m_from = 0;
    std::unique_ptr<cutefish::ui::Animation> m_glide;
};

} // namespace clockapp
