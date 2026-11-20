// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "ui/view.h"

#include <cstdint>
#include <functional>
#include <string>

namespace cutefish::ui {
class Window;
}

namespace clockapp {

class WheelPicker;

// The alarm's time on the shared drums: hours and minutes, and AM/PM where
// the locale uses a 12-hour clock. Digits typed on any drum set the whole
// time ("7", "730", "0730", "1945"); A and P pick the half of the day.
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

private:
    bool typed(const cutefish::ui::KeyEvent& event);

    bool m_twelveHour;
    WheelPicker* m_wheels;
    std::string m_digits;
    uint64_t m_lastDigitNs = 0;
};

} // namespace clockapp
