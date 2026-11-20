// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "alarms/timepicker.h"

#include "timers/wheel.h"

#include <base/locale.h>

#include "ui/window.h"

#include <cctype>
#include <string>
#include <vector>

using namespace cutefish::ui;

namespace clockapp {

static constexpr size_t Hours = 0;
static constexpr size_t Minutes = 1;
static constexpr size_t Half = 2;

static std::string twoDigits(int value)
{
    return (value < 10 ? "0" : "") + std::to_string(value);
}

TimePicker::TimePicker(Window* window, bool twelveHour)
    : m_twelveHour(twelveHour)
{
    WheelColumn hours;
    hours.name = "picker:hours";
    for (int hour = 0; hour < (twelveHour ? 12 : 24); ++hour)
        hours.values.push_back(twelveHour ? std::to_string(hour == 0 ? 12 : hour) : twoDigits(hour));

    WheelColumn minutes;
    minutes.name = "picker:minutes";
    for (int minute = 0; minute < 60; ++minute)
        minutes.values.push_back(twoDigits(minute));

    std::vector<WheelColumn> columns{hours, minutes};
    if (twelveHour) {
        const cutefish::base::Locale& locale = cutefish::base::Locale::system();
        WheelColumn half;
        half.name = "picker:half";
        half.values = {locale.amText(), locale.pmText()};
        half.wraps = false;
        columns.push_back(half);
    }

    m_wheels = addChild<WheelPicker>(window, std::move(columns));
    m_wheels->setName("picker:wheels");
    m_wheels->onChanged = [this](size_t) {
        if (onChanged)
            onChanged();
    };
}

TimePicker::~TimePicker() = default;

int TimePicker::hour() const
{
    const int hour = m_wheels->value(Hours);
    if (!m_twelveHour)
        return hour;
    return hour + (m_wheels->value(Half) == 1 ? 12 : 0);
}

int TimePicker::minute() const
{
    return m_wheels->value(Minutes);
}

void TimePicker::setTime(int hour, int minute, bool animate)
{
    if (m_twelveHour) {
        m_wheels->setValue(Hours, hour % 12, animate);
        m_wheels->setValue(Half, hour >= 12 ? 1 : 0, animate);
    } else {
        m_wheels->setValue(Hours, hour, animate);
    }
    m_wheels->setValue(Minutes, minute, animate);
}

bool TimePicker::keyPress(const KeyEvent& event)
{
    // The drums take digits and arrows; the letters of the half of the day
    // come here from whichever has focus.
    if (!m_twelveHour || event.text.size() != 1 || (event.modifiers & ~ShiftModifier))
        return false;
    const char c = char(std::tolower(static_cast<unsigned char>(event.text[0])));
    if (c != 'a' && c != 'p')
        return false;
    m_wheels->setValue(Half, c == 'p' ? 1 : 0, true);
    if (onChanged)
        onChanged();
    return true;
}

SizeF TimePicker::sizeHint() const
{
    return m_wheels->sizeHint();
}

void TimePicker::layout()
{
    m_wheels->setGeometry(localRect());
}

} // namespace clockapp
