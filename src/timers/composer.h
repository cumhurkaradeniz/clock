// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "stopwatch/motion.h"
#include "timers/timermath.h"
#include "ui/view.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace cutefish::ui {
class ComboBox;
class Label;
class ScrollView;
class TextField;
}

namespace clockapp {

class DurationPicker;
class RoundButton;

// Where a timer is set: the drums, a card of rows for its label and sound,
// and Start, centred in the page. Recent timers, when there are any, stand
// in a column of their own at the page's start, each started again by a
// click; the column slides in and out on a spring as they come and go.
class TimerComposer : public cutefish::ui::View
{
public:
    static constexpr double ColumnWidth = 420;
    static constexpr double RecentsWidth = 250;
    static constexpr double RecentsGap = 20;
    static constexpr double RecentRowHeight = 56;
    static constexpr double StartDiameter = 76;

    explicit TimerComposer(cutefish::ui::Window* window);
    ~TimerComposer() override;

    DurationPicker* picker() const { return m_picker; }
    void setRecents(const std::vector<timermath::Recent>& recents);
    // Cancel goes back to the running timers, when there are any.
    void setCancelVisible(bool visible);
    // The label back to empty, the drums kept as they were.
    void clearLabel();
    // Space left free at the top for the header.
    void setTopMargin(double margin);

    std::string label() const;
    std::string sound() const;

    // The recent timers' list, which scrolls on its own.
    cutefish::ui::ScrollView* recentsView() const { return m_recentsView; }
    // The picker, its rows and Start, in the composer's coordinates.
    cutefish::ui::RectF groupRect() const;

    std::function<void()> onStart;
    std::function<void()> onCancel;
    std::function<void(size_t index)> onStartRecent;

protected:
    void layout() override;
    void paint(cutefish::ui::Painter& painter) override;

private:
    class OptionsCard;
    class RecentList;

    cutefish::ui::RectF recentsCardRect() const;
    void updateStart();

    cutefish::ui::Window* m_window;
    double m_topMargin = 0;
    DurationPicker* m_picker = nullptr;
    OptionsCard* m_options = nullptr;
    cutefish::ui::TextField* m_label = nullptr;
    cutefish::ui::ComboBox* m_sound = nullptr;
    RoundButton* m_cancel = nullptr;
    RoundButton* m_start = nullptr;
    cutefish::ui::Label* m_recentsTitle = nullptr;
    cutefish::ui::ScrollView* m_recentsView = nullptr;
    RecentList* m_recents = nullptr;
    // From 0, no column, to 1, the column in place.
    std::unique_ptr<Glide> m_recentsShown;
};

} // namespace clockapp
