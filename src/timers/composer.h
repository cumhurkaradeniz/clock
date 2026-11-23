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
class Label;
class RowComboBox;
class ScrollView;
class TextField;
}

namespace clockapp {

class DurationPicker;
class RoundButton;

// The recent timers in a card of their own at the page's start, each started
// again by a click.
class RecentsPanel : public cutefish::ui::View
{
public:
    explicit RecentsPanel(cutefish::ui::Window* window);
    ~RecentsPanel() override;

    void setRecents(const std::vector<timermath::Recent>& recents);

    std::function<void(size_t index)> onStartRecent;

protected:
    void layout() override;
    void paint(cutefish::ui::Painter& painter) override;

private:
    class RecentList;

    cutefish::ui::RectF cardRect() const;

    cutefish::ui::Label* m_title = nullptr;
    cutefish::ui::ScrollView* m_view = nullptr;
    RecentList* m_list = nullptr;
};

// Where a timer is set: the drums, a card of rows for its label and sound,
// and Start, centred in the area beside Recent (or the whole page when none).
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
    // Cancel goes back to the running timers, when there are any.
    void setCancelVisible(bool visible);
    // The label back to empty, the drums kept as they were.
    void clearLabel();
    // Space left free at the top for the header and at the sides.
    void setMargins(double top, double left, double right);

    std::string label() const;
    std::string sound() const;

    // The picker, its rows and Start, in the composer's coordinates.
    cutefish::ui::RectF groupRect() const;

    std::function<void()> onStart;
    std::function<void()> onCancel;

protected:
    void layout() override;

private:
    class OptionsCard;

    double m_topMargin = 0;
    double m_leftMargin = 0;
    double m_rightMargin = 0;
    DurationPicker* m_picker = nullptr;
    OptionsCard* m_options = nullptr;
    cutefish::ui::TextField* m_label = nullptr;
    cutefish::ui::RowComboBox* m_sound = nullptr;
    RoundButton* m_cancel = nullptr;
    RoundButton* m_start = nullptr;

    void updateStart();
};

} // namespace clockapp
