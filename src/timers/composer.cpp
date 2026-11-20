// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "timers/composer.h"

#include "alarms/tones.h"
#include "stopwatch/roundbutton.h"
#include "style.h"
#include "timers/wheelpicker.h"

#include <base/i18n.h>

#include "ui/combo_box.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/scroll_view.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>

using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace clockapp {

// The rows of a card, as the alarm editor and Settings have them.
static constexpr double RowHeight = 44;
static constexpr double RowInset = 14;
static constexpr double RecentStartDiameter = 36;

static Corners themeCorners()
{
    return Theme::current().continuousCorners ? Corners::Continuous : Corners::Circular;
}

static ColorRecipe secondaryColor()
{
    return ColorRecipe(std::function<Color(const ColorProvider&)>(
        [](const ColorProvider&) { return style::secondaryText(); }));
}

// A card of rows, a label on the left and its control on the right, a
// hairline between them.
class TimerComposer::OptionsCard : public View
{
public:
    View* addRow(const std::string& text, std::unique_ptr<View> control)
    {
        View* row = addChild<View>();
        Label* label = row->addChild<Label>(text);
        label->setVerticallyCentered(true);
        row->addChild(std::move(control));
        return row->children()[1].get();
    }

    double heightForWidth(double) const override { return RowHeight * double(children().size()); }
    SizeF sizeHint() const override { return {0, heightForWidth(0)}; }

protected:
    void layout() override
    {
        const double width = geometry().width;
        double y = 0;
        for (const auto& row : children()) {
            row->setGeometry({0, y, width, RowHeight});
            y += RowHeight;
            auto* label = static_cast<Label*>(row->children()[0].get());
            View* control = row->children()[1].get();
            const SizeF size = control->sizeHint();
            const double controlWidth = std::ceil(control->layoutHints().preferredWidth > 0
                                                      ? control->layoutHints().preferredWidth
                                                      : size.width);
            const double controlHeight = std::ceil(size.height);
            control->setGeometry({width - RowInset - controlWidth, style::centred(RowHeight, controlHeight),
                                  controlWidth, controlHeight});
            const double labelHeight = std::ceil(label->sizeHint().height);
            label->setGeometry({RowInset, style::centred(RowHeight, labelHeight),
                                std::max(0.0, width - 3 * RowInset - controlWidth), labelHeight});
        }
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        painter.fillRoundedRect(localRect(), theme.bigRadius, style::card(), themeCorners());
        const double hairline = 1 / painter.scale();
        for (size_t i = 0; i + 1 < children().size(); ++i) {
            const RectF row = children()[i]->geometry();
            painter.fillRoundedRect({RowInset, row.bottom() - hairline, geometry().width - 2 * RowInset, hairline}, 0,
                                    style::separator());
        }
    }
};

// The recent timers' rows: the duration, the label and sound under it, and
// a play button that keeps one gap to the row's end, top and bottom.
class TimerComposer::RecentList : public View
{
public:
    explicit RecentList(Window* window)
        : m_window(window)
    {
    }

    void setRecents(const std::vector<timermath::Recent>& recents)
    {
        while (m_rows.size() > recents.size()) {
            takeChild(m_rows.back().view);
            m_rows.pop_back();
        }
        while (m_rows.size() < recents.size()) {
            Row row;
            row.view = addChild<View>();
            row.view->setName("timers:recent");
            row.duration = row.view->addChild<Label>();
            row.duration->setName("timers:recent-duration");
            row.duration->setFontSize(20);
            row.duration->setWeight(400);
            row.duration->setFontFeatures({{"tnum", 1}});
            row.detail = row.view->addChild<Label>();
            row.detail->setFontSize(13);
            row.detail->setColor(secondaryColor());
            row.start = row.view->addChild<RoundButton>(m_window, RecentStartDiameter);
            row.start->setName("timers:recent-start");
            row.start->setTitle(tr("Start"));
            row.start->setPlayGlyph(true);
            row.start->setRole(RoundButton::Role::Start);
            const size_t index = m_rows.size();
            row.start->onClicked = [this, index] {
                if (onStart)
                    onStart(index);
            };
            m_rows.push_back(row);
        }
        for (size_t i = 0; i < recents.size(); ++i) {
            const timermath::Recent& recent = recents[i];
            m_rows[i].duration->setText(timermath::formatRemaining(recent.durationMs));
            std::string detail = recent.label.empty() ? timermath::formatDuration(recent.durationMs) : recent.label;
            detail += " · " + tones::displayName(recent.sound);
            m_rows[i].detail->setText(detail);
        }
        requestLayout();
    }

    SizeF sizeHint() const override { return {0, heightForWidth(0)}; }
    double heightForWidth(double) const override { return TimerComposer::RecentRowHeight * double(m_rows.size()); }

    std::function<void(size_t index)> onStart;

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double rowHeight = TimerComposer::RecentRowHeight;
        const double gap = (rowHeight - RecentStartDiameter) / 2;
        for (size_t i = 0; i < m_rows.size(); ++i) {
            Row& row = m_rows[i];
            row.view->setGeometry({0, rowHeight * double(i), width, rowHeight});
            const double durationHeight = std::ceil(row.duration->sizeHint().height);
            const double detailHeight = std::ceil(row.detail->sizeHint().height);
            const double textTop = style::centred(rowHeight, durationHeight + detailHeight);
            const double textWidth = std::max(0.0, width - RowInset - gap - RecentStartDiameter - 8);
            row.duration->setGeometry({RowInset, textTop, textWidth, durationHeight});
            row.detail->setGeometry({RowInset, textTop + durationHeight, textWidth, detailHeight});
            row.start->setGeometry({width - gap - RecentStartDiameter, gap, RecentStartDiameter, RecentStartDiameter});
        }
    }

    void paint(Painter& painter) override
    {
        const double hairline = 1 / painter.scale();
        for (size_t i = 1; i < m_rows.size(); ++i) {
            const double y = TimerComposer::RecentRowHeight * double(i);
            painter.fillRoundedRect({RowInset, y - hairline, geometry().width - 2 * RowInset, hairline}, 0,
                                    style::separator());
        }
    }

private:
    struct Row
    {
        View* view = nullptr;
        Label* duration = nullptr;
        Label* detail = nullptr;
        RoundButton* start = nullptr;
    };

    Window* m_window;
    std::vector<Row> m_rows;
};

TimerComposer::TimerComposer(Window* window)
    : m_window(window)
    , m_recentsShown(std::make_unique<Glide>(window, 380, 0))
{
    setName("timers:composer");
    m_picker = addChild<DurationPicker>(window);
    m_picker->onChanged = [this](size_t) { updateStart(); };

    m_options = addChild<OptionsCard>();
    auto label = std::make_unique<TextField>();
    m_label = label.get();
    m_label->setName("timers:label");
    m_label->setFrameVisible(false);
    m_label->setRightAligned(true);
    m_label->setPlaceholder(tr("Timer"));
    m_label->setMaximumLength(60);
    m_label->setLayoutHints({.preferredWidth = 200});
    m_options->addRow(tr("Label"), std::move(label));

    std::vector<std::string> sounds;
    for (const std::string& name : clockstore::soundNames())
        sounds.push_back(tones::displayName(name));
    auto sound = std::make_unique<ComboBox>(sounds);
    m_sound = sound.get();
    m_sound->setName("timers:sound");
    m_sound->setCurrentIndex(0);
    m_sound->setLayoutHints({.preferredWidth = 140});
    m_options->addRow(tr("Sound"), std::move(sound));

    m_cancel = addChild<RoundButton>(window, StartDiameter);
    m_cancel->setName("timers:composer-cancel");
    m_cancel->setTitle(tr("Cancel"));
    m_cancel->setVisible(false);
    m_cancel->onClicked = [this] {
        if (onCancel)
            onCancel();
    };
    m_start = addChild<RoundButton>(window, StartDiameter);
    m_start->setName("timers:start");
    m_start->setTitle(tr("Start"));
    m_start->setRole(RoundButton::Role::Start);
    m_start->onClicked = [this] {
        if (m_picker->durationMs() > 0 && onStart)
            onStart();
    };
    m_label->onAccepted = [this] { m_start->onClicked(); };

    m_recentsTitle = addChild<Label>(tr("Recent"));
    m_recentsTitle->setName("timers:recents-title");
    m_recentsTitle->setFontSize(15);
    m_recentsTitle->setWeight(600);
    m_recentsView = addChild<ScrollView>();
    m_recentsView->setName("timers:recents");
    m_recentsView->setScrollDirections(false, true);
    m_recentsView->setCornerRadius(Theme::current().bigRadius);
    m_recents = m_recentsView->setContent<RecentList>(window);
    m_recents->onStart = [this](size_t index) {
        if (onStartRecent)
            onStartRecent(index);
    };
    m_recentsTitle->setVisible(false);
    m_recentsView->setVisible(false);
    m_recentsShown->onChanged = [this] {
        requestLayout();
        requestPaint();
    };

    m_picker->setDurationMs(5 * 60 * 1000);
    updateStart();
}

TimerComposer::~TimerComposer() = default;

std::string TimerComposer::label() const
{
    return m_label->text();
}

std::string TimerComposer::sound() const
{
    const std::vector<std::string>& names = clockstore::soundNames();
    const int index = m_sound->currentIndex();
    return index >= 0 && size_t(index) < names.size() ? names[size_t(index)] : names.front();
}

void TimerComposer::clearLabel()
{
    m_label->setText({});
}

void TimerComposer::setTopMargin(double margin)
{
    if (margin == m_topMargin)
        return;
    m_topMargin = margin;
    requestLayout();
}

void TimerComposer::setCancelVisible(bool visible)
{
    m_cancel->setVisible(visible);
}

void TimerComposer::updateStart()
{
    m_start->setEnabled(m_picker->durationMs() > 0);
}

void TimerComposer::setRecents(const std::vector<timermath::Recent>& recents)
{
    if (!recents.empty())
        m_recents->setRecents(recents);
    const double shown = recents.empty() ? 0 : 1;
    // The list keeps its rows while it slides away, and drops them after.
    if (recents.empty() && !m_recentsShown->isMoving() && m_recentsShown->value() == 0)
        m_recents->setRecents(recents);
    if (window() && isVisible() && geometry().width > 0)
        m_recentsShown->setTarget(shown);
    else
        m_recentsShown->jumpTo(shown);
    requestLayout();
}

RectF TimerComposer::groupRect() const
{
    const double width = geometry().width;
    const double shown = m_recentsShown->value();
    const double left = style::pagePadding + shown * (RecentsWidth + RecentsGap);
    const double right = width - style::pagePadding;
    const double column = std::max(0.0, std::min(ColumnWidth, right - left));
    const double x = std::round(left + (right - left - column) / 2);
    const double height = m_picker->sizeHint().height + 12 + m_options->sizeHint().height + 16 + StartDiameter;
    return {x, m_topMargin + 16, column, height};
}

RectF TimerComposer::recentsCardRect() const
{
    const double shown = m_recentsShown->value();
    const double x = std::round(style::pagePadding - (1 - shown) * (RecentsWidth + style::pagePadding));
    const double titleHeight = std::ceil(m_recentsTitle->sizeHint().height);
    const double top = m_topMargin + 16 + titleHeight + 8;
    const double room = std::max(0.0, geometry().height - top - style::pagePadding);
    const double height = std::min(room, m_recents->heightForWidth(RecentsWidth));
    return {x, top, RecentsWidth, height};
}

void TimerComposer::layout()
{
    const RectF group = groupRect();
    const double pickerHeight = m_picker->sizeHint().height;
    m_picker->setGeometry({group.x, group.y, group.width, pickerHeight});
    const double optionsTop = group.y + pickerHeight + 12;
    const double optionsHeight = m_options->sizeHint().height;
    m_options->setGeometry({group.x, optionsTop, group.width, optionsHeight});
    const double buttonsTop = optionsTop + optionsHeight + 16;
    m_cancel->setGeometry({group.x, buttonsTop, StartDiameter, StartDiameter});
    // Start sits on the drums' centre line; Cancel, when there is one, at
    // the group's start.
    const double scale = window() ? window()->scale() : 1;
    m_start->setGeometry({style::snap(group.x + (group.width - StartDiameter) / 2, scale), buttonsTop, StartDiameter,
                          StartDiameter});

    const double shown = m_recentsShown->value();
    const bool visible = shown > 0;
    m_recentsTitle->setVisible(visible);
    m_recentsView->setVisible(visible);
    if (!visible && !m_recentsShown->isMoving())
        m_recents->setRecents({});
    const RectF card = recentsCardRect();
    const double titleHeight = std::ceil(m_recentsTitle->sizeHint().height);
    m_recentsTitle->setGeometry({card.x + 4, m_topMargin + 16, RecentsWidth - 4, titleHeight});
    m_recentsTitle->setOpacity(shown);
    m_recentsView->setGeometry(card);
    m_recentsView->setOpacity(shown);
}

void TimerComposer::paint(Painter& painter)
{
    const double shown = m_recentsShown->value();
    if (shown <= 0)
        return;
    const Theme& theme = Theme::current();
    painter.save();
    painter.multiplyOpacity(shown);
    painter.fillRoundedRect(recentsCardRect(), theme.bigRadius, style::card(), themeCorners());
    painter.restore();
}

} // namespace clockapp
