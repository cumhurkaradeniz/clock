// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "tabswitch.h"
#include "style.h"

#include "ui/animation.h"
#include "ui/color_id.h"
#include "ui/color_provider.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>
#include <numbers>

using namespace cutefish::ui;

namespace clockapp {

static constexpr std::chrono::milliseconds kGlideDuration{360};
// How far past the segment the plate swings before it settles, as a share
// of the travel: about 1.8 percent.
static constexpr double kOvershoot = 0.7;
// How much wider the plate grows at the middle of a glide of a segment or more.
static constexpr double kStretch = 0.12;

// An ease out that passes its end a little and comes back.
static double settle(double t)
{
    const double u = t - 1;
    return 1 + (kOvershoot + 1) * u * u * u + kOvershoot * u * u;
}

TabSwitch::TabSwitch(std::vector<std::string> items)
{
    setItems(std::move(items));
}

TabSwitch::~TabSwitch() = default;

void TabSwitch::setItems(std::vector<std::string> items)
{
    for (Label* label : m_labels)
        takeChild(label);
    m_labels.clear();
    for (std::string& item : items) {
        Label* label = addChild<Label>(std::move(item));
        label->setFontSize(style::points(9.5));
        label->setWeight(500);
        label->setElide(Label::Elide::Right);
        m_labels.push_back(label);
    }
    m_current = std::clamp(m_current, 0, std::max(0, int(m_labels.size()) - 1));
    m_position = m_current;
    updateLabels();
    requestLayout();
}

void TabSwitch::setSegmentNames(const std::vector<std::string>& names)
{
    for (size_t i = 0; i < m_labels.size() && i < names.size(); ++i)
        m_labels[i]->setName(names[i]);
}

void TabSwitch::setCurrentIndex(int index, bool animated)
{
    if (index < 0 || index >= int(m_labels.size()))
        return;
    if (index == m_current) {
        if (!animated && isGliding()) {
            m_glide->stop();
            setPosition(index, 0);
        }
        return;
    }
    m_current = index;
    Window* window = this->window();
    if (!animated || !window || std::abs(m_position - index) < 1e-6) {
        if (m_glide)
            m_glide->stop();
        setPosition(index, 0);
        return;
    }
    if (!m_glide) {
        m_glide = std::make_unique<Animation>(window);
        m_glide->setEasing(Animation::Easing::Linear);
        m_glide->setDuration(kGlideDuration);
    }
    m_glide->stop();
    m_from = m_position;
    const double to = index;
    const double reach = std::min(1.0, std::abs(to - m_from));
    m_glide->onUpdate = [this, to, reach](double t) {
        setPosition(m_from + (to - m_from) * settle(t), kStretch * reach * std::sin(std::numbers::pi * std::min(1.0, t * 1.4)));
    };
    m_glide->onFinished = [this, to] { setPosition(to, 0); };
    m_glide->start();
}

bool TabSwitch::isGliding() const
{
    return m_glide && m_glide->isRunning();
}

void TabSwitch::setPosition(double position, double stretch)
{
    m_position = position;
    m_stretch = std::max(0.0, stretch);
    updateLabels();
    requestPaint();
}

// The label under the plate in full text colour, the others a shade
// lighter, passing from one to the other as the plate goes by.
void TabSwitch::updateLabels()
{
    const Color text = Theme::current().text;
    for (size_t i = 0; i < m_labels.size(); ++i) {
        const double under = std::clamp(1 - std::abs(double(i) - m_position), 0.0, 1.0);
        double alpha = 0.72 + 0.28 * under;
        if (int(i) == m_pressed && m_pressInside && int(i) != m_current)
            alpha = 0.5;
        m_labels[i]->setColor(ColorRecipe(style::withAlpha(text, alpha)));
    }
}

RectF TabSwitch::itemRect(size_t index) const
{
    const RectF inner = localRect().adjusted(kPadding, kPadding, -kPadding, -kPadding);
    const double width = m_labels.empty() ? 0 : inner.width / double(m_labels.size());
    return {inner.x + width * double(index), inner.y, width, inner.height};
}

RectF TabSwitch::plateRect() const
{
    const RectF inner = localRect().adjusted(kPadding, kPadding, -kPadding, -kPadding);
    if (m_labels.empty())
        return {};
    const double width = inner.width / double(m_labels.size());
    const double grown = width * (1 + m_stretch);
    // Kept inside the track while it swings past an end segment.
    const double x = std::clamp(inner.x + width * m_position - (grown - width) / 2, inner.x, inner.right() - grown);
    return {x, inner.y, grown, inner.height};
}

SizeF TabSwitch::sizeHint() const
{
    double widest = 0;
    for (const Label* label : m_labels)
        widest = std::max(widest, label->sizeHint().width);
    return {(widest + 24) * double(m_labels.size()) + 2 * kPadding, kHeight};
}

int TabSwitch::itemAt(PointF point) const
{
    if (!localRect().contains(point))
        return -1;
    for (size_t i = 0; i < m_labels.size(); ++i) {
        const RectF item = itemRect(i);
        if (point.x >= item.x && point.x < item.right())
            return int(i);
    }
    return point.x < kPadding ? 0 : int(m_labels.size()) - 1;
}

void TabSwitch::layout()
{
    const double scale = window() ? window()->scale() : 1;
    for (size_t i = 0; i < m_labels.size(); ++i) {
        const RectF item = itemRect(i);
        const SizeF text = m_labels[i]->sizeHint();
        const double width = std::min(text.width, item.width - 8);
        // The capitals centred on the plate, on whole device pixels.
        const double baseline = style::snap(item.y + item.height / 2 + m_labels[i]->capHeight() / 2, scale);
        m_labels[i]->setGeometry({style::snap(item.x + (item.width - width) / 2, scale),
                                  baseline - m_labels[i]->baselineOffset(), width, text.height});
    }
}

void TabSwitch::paint(Painter& painter)
{
    const RectF bounds = localRect();
    painter.drawRoundedRect(bounds, {bounds.height / 2}, style::controlTint());
    if (m_labels.empty())
        return;
    // Concentric with the track through the padding: a pill again.
    const RectF plate = plateRect();
    painter.drawShadow(plate, plate.height / 2, 1.2, color(ColorId::SegmentedControlPlateShadow), {0, 1});
    painter.drawSmoothRectangle(plate, plate.height / 2, color(ColorId::SegmentedControlPlate));
}

void TabSwitch::pointerPress(const PointerEvent& event)
{
    if (event.button != MouseButton::Left)
        return;
    m_pressed = itemAt(event.position);
    m_pressInside = m_pressed >= 0;
    updateLabels();
}

void TabSwitch::pointerMove(const PointerEvent& event)
{
    if (m_pressed < 0)
        return;
    const bool inside = itemAt(event.position) == m_pressed;
    if (inside != m_pressInside) {
        m_pressInside = inside;
        updateLabels();
    }
}

void TabSwitch::pointerRelease(const PointerEvent& event)
{
    const int pressed = m_pressed;
    m_pressed = -1;
    m_pressInside = false;
    if (pressed >= 0 && itemAt(event.position) == pressed && pressed != m_current) {
        setCurrentIndex(pressed, true);
        if (onCurrentChanged)
            onCurrentChanged(pressed);
    }
    updateLabels();
}

void TabSwitch::pointerLeave()
{
    if (m_pressed >= 0 && m_pressInside) {
        m_pressInside = false;
        updateLabels();
    }
}

} // namespace clockapp
