// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "ui/view.h"

#include <functional>

namespace cutefish::ui {
class Window;
}

namespace clockapp {

class WheelPicker;

// The alarm's time on the shared drums: hours and minutes, and AM/PM where
// the locale uses a 12-hour clock. Digits typed on a drum pick its value;
// A and P pick the half of the day from either drum.
class TimePicker : public cutefish::ui::View
{
public:
    TimePicker(cutefish::ui::Window* window, bool twelveHour);
    ~TimePicker() override;

    int hour() const;
    int minute() const;
    void setTime(int hour, int minute, bool animate);

    WheelPicker* wheels() const { return m_wheels; }

    cutefish::ui::SizeF sizeHint() const override;
    double heightForWidth(double) const override { return sizeHint().height; }

    std::function<void()> onChanged;

protected:
    void layout() override;
    bool keyPress(const cutefish::ui::KeyEvent& event) override;

private:
    bool m_twelveHour;
    WheelPicker* m_wheels;
};

} // namespace clockapp
