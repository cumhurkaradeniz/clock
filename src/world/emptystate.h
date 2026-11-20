// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <functional>

#include "ui/view.h"

namespace cutefish::ui {
class DialogButton;
class Label;
}

namespace clockapp::world {

// No cities: a drawn globe, its meridians turning slowly once when the state
// appears, over a line of text and the Add City button.
class EmptyState : public cutefish::ui::View
{
public:
    EmptyState();

    // From 0 to 1 as the state comes in; the globe turns a quarter of its
    // meridians' spacing on the way.
    void setProgress(double progress);

    std::function<void()> onAdd;

protected:
    void layout() override;
    void paint(cutefish::ui::Painter& painter) override;

private:
    cutefish::ui::RectF globeRect() const;

    cutefish::ui::Label* m_title;
    cutefish::ui::Label* m_text;
    cutefish::ui::DialogButton* m_add;
    double m_progress = 1;
};

} // namespace clockapp::world
