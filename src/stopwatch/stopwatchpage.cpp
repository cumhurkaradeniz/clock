// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "stopwatch/stopwatchpage.h"

#include "pagecontext.h"
#include "stopwatch/dialview.h"
#include "stopwatch/digitdisplay.h"
#include "stopwatch/lapslist.h"
#include "stopwatch/roundbutton.h"
#include "style.h"

#include <base/i18n.h>

#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/scroll_view.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>

using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace clockapp {

static constexpr double ButtonDiameter = 76;
static constexpr double ColumnWidth = 560;
static constexpr double DigitsHeight = 150;
static constexpr double DigitsFontSize = 84;
// Wide enough for the dial and the figures side by side.
static constexpr double BothFacesWidth = 900;
static constexpr double BothFacesColumn = 860;

StopwatchPage::StopwatchPage(PageContext& context, const TimeSource* time)
    : m_context(context)
    , m_time(time ? *time : TimeSource::system())
    , m_faceFade(std::make_unique<Glide>(&context.window(), 340, 0))
    , m_ticker(std::make_unique<FrameTicker>(&context.window()))
{
    setName("page:stopwatch");
    Window* window = &context.window();

    m_digits = addChild<DigitDisplay>(window, DigitsFontSize, 200);
    m_digits->setName("stopwatch:digits");
    m_digits->setSteadyCells(2);

    m_dial = addChild<DialView>(window);
    m_dial->setName("stopwatch:dial");

    m_lapsView = addChild<ScrollView>();
    m_lapsView->setName("stopwatch:laps");
    m_lapsView->setScrollDirections(false, true);
    m_lapsView->setBottomCornerRadius(Theme::current().windowRadius);
    m_laps = m_lapsView->setContent<LapsList>(window);

    m_emptyHint = addChild<Label>(tr("Laps you take show here, the fastest and the slowest marked."));
    m_emptyHint->setName("stopwatch:empty");
    m_emptyHint->setTextAlignment(Label::TextAlignment::Center);
    m_emptyHint->setWrap(true);
    m_emptyHint->setColor(ColorRecipe(std::function<Color(const ColorProvider&)>(
        [](const ColorProvider&) { return style::secondaryText(); })));

    m_lapButton = addChild<RoundButton>(window, ButtonDiameter);
    m_lapButton->setName("stopwatch:lap");
    m_lapButton->onClicked = [this] { lapOrReset(); };
    m_startButton = addChild<RoundButton>(window, ButtonDiameter);
    m_startButton->setName("stopwatch:start");
    m_startButton->onClicked = [this] { startOrStop(); };

    // The faces cross-fade as layers, so the fade costs no repaint; the
    // page is painted again once the fade is over.
    m_digits->setLayered(true);
    m_dial->setLayered(true);
    m_faceFade->setRestDistance(0.01);
    m_faceFade->onChanged = [this] {
        requestLayout();
        if (!m_faceFade->isMoving())
            requestPaint();
    };
    m_ticker->onFrame = [this](uint64_t) { refresh(); };

    cutefish::base::Settings& settings = context.settings();
    m_stopwatch.load(settings, m_time.now());
    m_face = settings.stringValue("Stopwatch/Face") == "dial" ? Face::Dial : Face::Digits;
    m_faceFade->jumpTo(m_face == Face::Dial ? 1 : 0);

    m_shortcuts.push_back(context.addShortcut(*this, NoModifier, key::Space, [this] { startOrStop(); }));
    m_shortcuts.push_back(context.addShortcut(*this, NoModifier, key::letter('l'), [this] {
        if (m_stopwatch.isRunning())
            lapOrReset();
    }));
    m_shortcuts.push_back(context.addShortcut(*this, NoModifier, key::letter('r'), [this] {
        if (!m_stopwatch.isRunning())
            lapOrReset();
    }));

    changed(false);
}

StopwatchPage::~StopwatchPage()
{
    for (int id : m_shortcuts)
        m_context.removeShortcut(id);
}

void StopwatchPage::startOrStop()
{
    const Reading now = m_time.now();
    if (m_stopwatch.isRunning())
        m_stopwatch.stop(now);
    else
        m_stopwatch.start(now);
    save();
    changed(false);
}

void StopwatchPage::lapOrReset()
{
    if (m_stopwatch.isRunning()) {
        m_stopwatch.lap(m_time.now());
        save();
        changed(true);
        return;
    }
    if (!m_stopwatch.hasTime())
        return;
    m_stopwatch.reset();
    save();
    m_dial->sweepToZero();
    changed(false);
}

void StopwatchPage::setFace(Face face)
{
    if (face == m_face)
        return;
    m_face = face;
    m_context.settings().setValue("Stopwatch/Face", face == Face::Dial ? "dial" : "digits");
    m_context.saveSettings();
    if (isVisible() && m_current)
        m_faceFade->setTarget(face == Face::Dial ? 1 : 0);
    else
        m_faceFade->jumpTo(face == Face::Dial ? 1 : 0);
    m_context.actionsChanged(*this);
}

std::vector<MenuAction> StopwatchPage::editActions()
{
    MenuAction face;
    face.text = m_face == Face::Dial ? tr("Show Figures") : tr("Show Dial");
    face.enabled = !m_both;
    face.triggered = [this] { setFace(m_face == Face::Dial ? Face::Digits : Face::Dial); };
    return {face};
}

void StopwatchPage::activated()
{
    m_current = true;
    m_digits->skipRoll();
    refresh();
    updateTicking();
}

void StopwatchPage::deactivated()
{
    m_current = false;
    updateTicking();
}

void StopwatchPage::entering(double progress)
{
    // The buttons and laps settle in a beat after the figures.
    const double later = std::clamp((progress - 0.3) / 0.7, 0.0, 1.0);
    m_lapButton->setOpacity(later);
    m_startButton->setOpacity(later);
    m_lapsView->setOpacity(later);
    m_emptyHint->setOpacity(later);
}

void StopwatchPage::save()
{
    m_stopwatch.save(m_context.settings());
    m_context.saveSettings();
}

void StopwatchPage::updateTicking()
{
    m_ticker->setRunning(m_current && m_stopwatch.isRunning());
}

void StopwatchPage::changed(bool lapAdded)
{
    const bool running = m_stopwatch.isRunning();
    m_startButton->setTitle(running ? tr("Stop") : tr("Start"));
    m_startButton->setRole(running ? RoundButton::Role::Stop : RoundButton::Role::Start);
    m_lapButton->setTitle(running || !m_stopwatch.hasTime() ? tr("Lap") : tr("Reset"));
    m_lapButton->setEnabled(m_stopwatch.hasTime());

    const Reading now = m_time.now();
    const bool hasLaps = m_stopwatch.hasTime();
    m_laps->setLaps(m_stopwatch.lapDurations(), m_stopwatch.lapExtremes(),
                    hasLaps ? m_stopwatch.currentLapNs(now) : -1, lapAdded);
    m_emptyHint->setVisible(!hasLaps);
    if (lapAdded)
        m_lapsView->setScrollPosition({0, 0});
    refresh();
    updateTicking();
    requestLayout();
}

void StopwatchPage::refresh()
{
    const Reading now = m_time.now();
    const int64_t elapsed = m_stopwatch.elapsedNs(now);
    m_digits->setText(formatElapsed(elapsed));
    m_dial->readout()->setText(formatElapsed(elapsed));
    if (m_stopwatch.hasTime()) {
        const int64_t lap = m_stopwatch.currentLapNs(now);
        m_laps->setCurrent(lap);
        m_dial->setTimes(elapsed, m_stopwatch.lapCount() > 0 ? lap : -1);
    } else {
        m_dial->setTimes(0, -1);
    }
}

void StopwatchPage::layout()
{
    const double width = geometry().width;
    const double height = geometry().height;
    const double scale = window() ? window()->scale() : 1;
    const double column = std::min(ColumnWidth, width - 2 * style::pagePadding);
    const double left = style::centred(width, column);
    const double top = headerHeight() + 12;

    const bool both = width >= BothFacesWidth;
    if (both != m_both) {
        m_both = both;
        m_context.actionsChanged(*this);
    }

    // Leave the laps at least three rows.
    const double minimumLaps = LapsList::RowHeight * 3;
    const double dialSide = std::clamp(height - top - ButtonDiameter - 28 - minimumLaps, 180.0, 320.0);
    double faceHeight;
    if (m_both) {
        faceHeight = dialSide;
        const double faces = std::min(BothFacesColumn, width - 2 * style::pagePadding);
        const double facesLeft = style::centred(width, faces);
        const double half = std::floor(faces / 2);
        m_dial->setOpacity(1);
        m_dial->setVisible(true);
        m_digits->setOpacity(1);
        m_digits->setVisible(true);
        m_dial->setGeometry({facesLeft, top, half, dialSide});
        m_digits->setGeometry({facesLeft + half, top + style::centred(dialSide, DigitsHeight), faces - half,
                               DigitsHeight});
    } else {
        const double fade = m_faceFade->value();
        faceHeight = DigitsHeight + (dialSide - DigitsHeight) * fade;
        // The two faces cross: each drifts a little as it fades.
        const double drift = 10;
        m_digits->setOpacity(1 - fade);
        m_digits->setVisible(fade < 1);
        m_digits->setGeometry({left, style::snap(top + style::centred(faceHeight, DigitsHeight) - drift * fade, scale),
                               column, DigitsHeight});
        m_dial->setOpacity(fade);
        m_dial->setVisible(fade > 0);
        m_dial->setGeometry(
            {left, style::snap(top + style::centred(faceHeight, dialSide) + drift * (1 - fade), scale), column,
             dialSide});
    }

    // The dial keeps a wider gap above the buttons than the figures do.
    const double dialShare = m_both ? 1 : m_faceFade->value();
    const double buttonsTop = std::round(top + faceHeight + 8 + 10 * dialShare);
    // Side by side, the figures already show the time the dial would read.
    m_dial->readout()->setVisible(!m_both);
    m_lapButton->setGeometry({left, buttonsTop, ButtonDiameter, ButtonDiameter});
    m_startButton->setGeometry({left + column - ButtonDiameter, buttonsTop, ButtonDiameter, ButtonDiameter});

    const double lapsTop = buttonsTop + ButtonDiameter + 20;
    // The list spans the window, its rows on the column, so its bar sits at
    // the window's edge.
    m_lapsView->setGeometry({0, lapsTop, width, std::max(0.0, height - lapsTop)});
    m_laps->setInsets(left, width - left - column);
    const SizeF hint{column - 80, m_emptyHint->heightForWidth(column - 80)};
    m_emptyHint->setGeometry({style::centred(width, hint.width), lapsTop + 24, hint.width, std::ceil(hint.height)});
}

void StopwatchPage::paint(Painter& painter)
{
    painter.fillRect(localRect(), style::page());
}

std::unique_ptr<Page> makeStopwatchPage(PageContext& context)
{
    return std::make_unique<StopwatchPage>(context);
}

} // namespace clockapp
