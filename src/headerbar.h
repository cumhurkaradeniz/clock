// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "page.h"

#include "ui/view.h"

#include <functional>
#include <memory>
#include <vector>

namespace cutefish::ui {
class Animation;
}

namespace clockapp {

class TabSwitch;

// The header's tools over the window buttons' line: the tab switch centred
// on the window, the current page's buttons at the end. It lies in a
// ui::WindowHeader, under its buttons.
class HeaderBar : public cutefish::ui::View
{
public:
    // The gap between two plates at the end.
    static constexpr double kPlateGap = 8;

    HeaderBar();
    ~HeaderBar() override;

    TabSwitch* tabSwitch() const { return m_tabs; }
    // The page's buttons, faded in when the tab changed.
    void setActions(std::vector<HeaderAction> actions, bool animated);
    // Where the plates and the switch are centred, from the header's top.
    double controlCentre() const;
    // The end buttons' area, in local coordinates, as of the last layout.
    cutefish::ui::RectF actionsRect() const;

protected:
    void layout() override;

private:
    class Actions;

    TabSwitch* m_tabs;
    Actions* m_actions = nullptr;
    std::unique_ptr<cutefish::ui::Animation> m_fade;
};

} // namespace clockapp
