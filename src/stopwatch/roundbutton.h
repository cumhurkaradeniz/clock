// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "stopwatch/motion.h"
#include "ui/view.h"

#include <functional>
#include <memory>
#include <string>

namespace cutefish::ui {
class Label;
}

namespace clockapp {

// The round button of the stopwatch and the timers: a tinted disc with a
// hairline ring a little inside its edge, lit from above, its title in the
// tint. Changing the role or title cross-fades rather than jumps.
class RoundButton : public cutefish::ui::View
{
public:
    enum class Role {
        Neutral,
        Start,
        Stop,
        // Pause and resume.
        Pause,
    };

    explicit RoundButton(cutefish::ui::Window* window, double diameter = 76);
    ~RoundButton() override;

    void setTitle(const std::string& title);
    const std::string& title() const { return m_title; }
    // A play triangle in place of the title, which stays its accessible name.
    void setPlayGlyph(bool play);
    void setRole(Role role);
    Role role() const { return m_role; }

    cutefish::ui::SizeF sizeHint() const override { return {m_diameter, m_diameter}; }
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override;
    void paint(cutefish::ui::Painter& painter) override;
    void enabledChanged() override;
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const cutefish::ui::PointerEvent& event) override;
    void pointerRelease(const cutefish::ui::PointerEvent& event) override;
    bool keyPress(const cutefish::ui::KeyEvent& event) override;
    void focusIn(cutefish::ui::FocusReason) override { requestPaint(); }
    void focusOut() override { requestPaint(); }

private:
    void updateState();

    double m_diameter;
    std::string m_title;
    Role m_role = Role::Neutral;
    Role m_previousRole = Role::Neutral;
    cutefish::ui::Label* m_label = nullptr;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_playGlyph = false;
    // 0 at rest, 1 hovered, 2 pressed; and the fade from the previous role.
    std::unique_ptr<Glide> m_state;
    std::unique_ptr<Glide> m_roleFade;
};

} // namespace clockapp
