// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "stopwatch/dialview.h"

#include "stopwatch/digitdisplay.h"
#include "stopwatch/palette.h"
#include "style.h"

#include "ui/label.h"
#include "ui/painter.h"
#include "ui/path.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>
#include <string>

using namespace cutefish::ui;

namespace clockapp {

static constexpr double Pi = 3.14159265358979323846;

static Color handColor()
{
    return palette::orange;
}

static Color lapHandColor()
{
    return style::accent();
}

// The point at a fraction of a turn from twelve, clockwise.
static PointF onCircle(PointF centre, double radius, double turn)
{
    const double angle = turn * 2 * Pi;
    return {centre.x + radius * std::sin(angle), centre.y - radius * std::cos(angle)};
}

// The sub-dial's centre and radius on a face of the radius.
static PointF subDialCentre(PointF centre, double radius)
{
    return {centre.x, centre.y - radius * 0.36};
}

static double subDialRadius(double radius)
{
    return radius * 0.25;
}

class DialView::Face : public View
{
public:
    Face()
    {
        for (int i = 1; i <= 12; ++i)
            m_numerals.push_back(numeral(std::to_string(i * 5)));
        for (int i = 1; i <= 3; ++i)
            m_subNumerals.push_back(numeral(std::to_string(i * 10)));
        setLayered(true);
    }

protected:
    void layout() override
    {
        const double radius = geometry().width / 2;
        const PointF centre{radius, radius};
        const double size = std::max(9.0, std::round(radius * 0.105));
        const double subSize = std::max(8.0, std::round(radius * 0.07));
        const double scale = window() ? window()->scale() : 1;
        for (size_t i = 0; i < m_numerals.size(); ++i)
            place(m_numerals[i], size, onCircle(centre, radius * 0.74, double(i + 1) / 12), scale);
        const PointF sub = subDialCentre(centre, radius);
        for (size_t i = 0; i < m_subNumerals.size(); ++i)
            place(m_subNumerals[i], subSize, onCircle(sub, subDialRadius(radius) * 0.62, double(i + 1) / 3),
                  scale);
    }

    void paint(Painter& painter) override
    {
        const RectF disc = localRect();
        const double radius = disc.width / 2;
        const PointF centre{radius, radius};

        // Lit from above: the face a shade lighter at the top than below.
        Path face;
        face.addEllipse(disc);
        const Color card = style::card();
        painter.fillPath(face, LinearGradient::vertical(disc, card.lighter(1.14), card.darker(1.06)));
        Path rim;
        rim.addEllipse(disc.adjusted(0.5, 0.5, -0.5, -0.5));
        painter.strokePath(rim, style::withAlpha(style::text(), 0.14), 1);

        const Color major = style::text();
        const Color minor = style::withAlpha(style::secondaryText(), 0.7);
        const double edge = radius * 0.94;
        for (int i = 0; i < 120; ++i) {
            const bool five = i % 10 == 0;
            const bool second = i % 2 == 0;
            const double length = radius * (five ? 0.085 : second ? 0.055 : 0.035);
            const double width = five ? std::max(1.5, radius * 0.012) : 1;
            painter.drawLine(onCircle(centre, edge, i / 120.0), onCircle(centre, edge - length, i / 120.0),
                             five ? major : minor, width * painter.scale());
        }

        const PointF sub = subDialCentre(centre, radius);
        const double subRadius = subDialRadius(radius);
        for (int i = 0; i < 30; ++i) {
            const bool ten = i % 5 == 0;
            const double length = subRadius * (ten ? 0.2 : 0.12);
            painter.drawLine(onCircle(sub, subRadius, i / 30.0), onCircle(sub, subRadius - length, i / 30.0),
                             ten ? major : minor, (ten ? 1.5 : 1) * painter.scale());
        }
    }

private:
    Label* numeral(const std::string& text)
    {
        Label* label = addChild<Label>(text);
        label->setFontFeatures({{"tnum", 1}});
        label->setTextAlignment(Label::TextAlignment::Center);
        label->setElide(Label::Elide::None);
        return label;
    }

    static void place(Label* label, double size, PointF at, double scale)
    {
        label->setFontSize(size);
        label->setWeight(size >= 16 ? 400 : 500);
        const SizeF hint = label->sizeHint();
        const double width = std::ceil(hint.width) + 2;
        // Centred on its capitals, not its line box, so the figures sit on
        // the circle.
        const double top = at.y - label->baselineOffset() + label->capHeight() / 2;
        label->setGeometry(
            {style::snap(at.x - width / 2, scale), style::snap(top, scale), width, std::ceil(hint.height)});
    }

    std::vector<Label*> m_numerals;
    std::vector<Label*> m_subNumerals;
};

class DialView::Hands : public View
{
public:
    // Turns from twelve: the seconds hand, the minutes hand, the lap hand.
    double seconds = 0;
    double minutes = 0;
    double lap = -1;
    double fade = 1;

protected:
    void paint(Painter& painter) override
    {
        const double radius = geometry().width / 2;
        const PointF centre{radius, radius};
        const Color hand = handColor();

        const PointF sub = subDialCentre(centre, radius);
        const double subRadius = subDialRadius(radius);
        painter.drawLine(sub, onCircle(sub, subRadius * 0.86, minutes), hand,
                         std::max(1.5, radius * 0.014) * painter.scale());
        painter.fillEllipse({sub.x - 2.5, sub.y - 2.5, 5, 5}, hand);

        if (lap >= 0) {
            drawHand(painter, centre, radius, lap, style::withAlpha(lapHandColor(), fade));
        }
        drawHand(painter, centre, radius, seconds, hand);

        const double cap = std::max(6.0, std::round(radius * 0.045));
        painter.fillEllipse({centre.x - cap / 2, centre.y - cap / 2, cap, cap}, hand);
        const double pin = std::max(2.0, std::round(cap * 0.4));
        painter.fillEllipse({centre.x - pin / 2, centre.y - pin / 2, pin, pin}, style::card());
    }

private:
    static void drawHand(Painter& painter, PointF centre, double radius, double turn, Color color)
    {
        const PointF tip = onCircle(centre, radius * 0.92, turn);
        const PointF tail = onCircle(centre, radius * 0.16, turn + 0.5);
        const double width = std::max(1.5, radius * 0.011);
        // A soft shadow under the hand, as if it stood a little off the face.
        Path shadow;
        shadow.moveTo(PointF{tail.x, tail.y + 1.5});
        shadow.lineTo(PointF{tip.x, tip.y + 1.5});
        painter.strokePath(shadow, Color{0, 0, 0, 80}, width + 1.5);
        Path line;
        line.moveTo(tail);
        line.lineTo(tip);
        painter.strokePath(line, color, width);
    }
};

DialView::DialView(Window* window)
    : m_sweep(std::make_unique<Glide>(window, 520, 0))
{
    m_face = addChild<Face>();
    m_readout = addChild<DigitDisplay>(window, 20, 400);
    m_readout->setSteadyCells(2);
    m_hands = addChild<Hands>();
    m_sweep->onChanged = [this] {
        if (m_sweeping && !m_sweep->isMoving() && m_sweep->value() == 0) {
            m_sweeping = false;
            m_elapsedNs = 0;
            m_lapNs = -1;
        }
        updateHands();
    };
}

DialView::~DialView() = default;

RectF DialView::faceRect() const
{
    const double scale = window() ? window()->scale() : 1;
    const double side = std::floor(std::min(geometry().width, geometry().height) / 2) * 2;
    return {style::snap(style::centred(geometry().width, side), scale),
            style::snap(style::centred(geometry().height, side), scale), side, side};
}

void DialView::layout()
{
    const RectF face = faceRect();
    m_face->setGeometry(face);
    m_hands->setGeometry(face);
    const double radius = face.width / 2;
    const double height = std::ceil(m_readout->lineHeight());
    m_readout->setGeometry({face.x, std::round(face.y + radius * 1.4 - height / 2), face.width, height});
}

void DialView::setTimes(int64_t elapsedNs, int64_t lapNs)
{
    if (m_sweeping) {
        // A start during the sweep takes the hands from where it left them.
        if (elapsedNs == 0)
            return;
        m_sweeping = false;
        m_sweep->jumpTo(0);
    }
    m_elapsedNs = elapsedNs;
    m_lapNs = lapNs;
    updateHands();
}

void DialView::updateHands()
{
    const double sweep = m_sweeping ? m_sweep->value() : 1;
    m_hands->seconds = double(m_elapsedNs % 60000000000LL) / 60e9 * sweep;
    m_hands->minutes = double(m_elapsedNs % 1800000000000LL) / 1800e9 * sweep;
    m_hands->lap = m_lapNs < 0 ? -1 : double(m_lapNs % 60000000000LL) / 60e9 * sweep;
    m_hands->fade = sweep;
    m_hands->requestPaint();
}

void DialView::sweepToZero()
{
    if (m_elapsedNs == 0 || !isVisible() || !window()) {
        m_sweeping = false;
        m_elapsedNs = 0;
        m_lapNs = -1;
        updateHands();
        return;
    }
    m_sweeping = true;
    m_sweep->jumpTo(1);
    m_sweep->setTarget(0);
}

} // namespace clockapp
