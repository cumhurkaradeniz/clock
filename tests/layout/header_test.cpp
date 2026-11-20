// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "windowfixture.h"

#include "ui/header_button_group.h"
#include "ui/label.h"
#include "ui/window_controls.h"
#include "ui/window_header.h"

#include <cmath>

using namespace cutefish::ui;
using clockapp::HeaderAction;
using clockapp::TabSwitch;

static RectF windowRect(const View* view)
{
    const PointF topLeft = view->mapToWindow({});
    return {topLeft.x, topLeft.y, view->geometry().width, view->geometry().height};
}

static bool onDevicePixels(double value, double scale)
{
    return std::abs(value * scale - std::round(value * scale)) < 1e-6;
}

static const double kScales[] = {1.0, 1.75, 2.0};

static std::vector<HeaderAction> someActions()
{
    return {
        {.id = "edit", .text = "Edit", .triggered = [] {}},
        {.id = "add", .glyph = "add", .text = "Add", .triggered = [] {}},
    };
}

// The window buttons' middle, from the window's top.
static double buttonsCentre(WindowFixture& fixture)
{
    auto* header = dynamic_cast<WindowHeader*>(fixture.window->headerBar()->parent());
    EXPECT_NE(header, nullptr);
    return header ? header->buttonsTop() + window_controls::kButton / 2 : 0;
}

TEST(HeaderTest, TheSwitchIsCentredOnTheWindowAndTheButtons)
{
    for (const double scale : kScales) {
        SCOPED_TRACE(scale);
        WindowFixture fixture(scale);
        TabSwitch* tabs = fixture.window->headerBar()->tabSwitch();
        const RectF rect = windowRect(tabs);
        EXPECT_NEAR(rect.x + rect.width / 2, clockapp::style::windowWidth / 2, 1 / scale);
        EXPECT_NEAR(rect.y + rect.height / 2, buttonsCentre(fixture), 0.5 / scale);
        EXPECT_EQ(rect.height, TabSwitch::kHeight);
        EXPECT_TRUE(onDevicePixels(rect.x, scale));
        EXPECT_TRUE(onDevicePixels(rect.y, scale));
        EXPECT_LE(rect.bottom(), clockapp::style::headerHeight);
    }
}

TEST(HeaderTest, ThePlateIsConcentricWithTheTrack)
{
    for (const double scale : kScales) {
        WindowFixture fixture(scale);
        TabSwitch* tabs = fixture.window->headerBar()->tabSwitch();
        for (size_t i = 0; i < tabs->count(); ++i) {
            tabs->setCurrentIndex(int(i), false);
            const RectF plate = tabs->plateRect();
            const RectF track = tabs->localRect();
            // One gap to the top, the bottom and the end it meets.
            EXPECT_DOUBLE_EQ(plate.y - track.y, TabSwitch::kPadding);
            EXPECT_DOUBLE_EQ(track.bottom() - plate.bottom(), TabSwitch::kPadding);
            if (i == 0)
                EXPECT_DOUBLE_EQ(plate.x - track.x, TabSwitch::kPadding);
            if (i + 1 == tabs->count())
                EXPECT_NEAR(track.right() - plate.right(), TabSwitch::kPadding, 1e-9);
            const RectF item = tabs->itemRect(i);
            EXPECT_NEAR(plate.x, item.x, 1e-9);
            EXPECT_NEAR(plate.width, item.width, 1e-9);
        }
    }
}

TEST(HeaderTest, TheLabelsAreCentredInTheirSegments)
{
    for (const double scale : kScales) {
        SCOPED_TRACE(scale);
        WindowFixture fixture(scale);
        TabSwitch* tabs = fixture.window->headerBar()->tabSwitch();
        const char* names[] = {"header:tab:world", "header:tab:alarms", "header:tab:stopwatch", "header:tab:timers"};
        for (size_t i = 0; i < tabs->count(); ++i) {
            auto* label = fixture.driver.find<Label>(names[i]);
            ASSERT_NE(label, nullptr);
            const RectF item = tabs->itemRect(i);
            const RectF text = label->geometry();
            EXPECT_NEAR(text.x + text.width / 2, item.x + item.width / 2, 0.5 / scale + 1e-9) << names[i];
            // The capitals' middle on the plate's middle.
            const double capsMiddle = text.y + label->baselineOffset() - label->capHeight() / 2;
            EXPECT_NEAR(capsMiddle, item.y + item.height / 2, 0.5 / scale + 1e-9) << names[i];
            EXPECT_TRUE(onDevicePixels(text.x, scale));
        }
    }
}

TEST(HeaderTest, ThePageButtonsKeepTheButtonsGapToTheEnd)
{
    for (const double scale : kScales) {
        SCOPED_TRACE(scale);
        WindowFixture fixture(scale);
        fixture.window->headerBar()->setActions(someActions(), false);
        fixture.driver.layout();
        const RectF edit = fixture.driver.frame("header:edit");
        const RectF add = fixture.driver.frame("header:add");
        ASSERT_GT(edit.width, 0);
        ASSERT_GT(add.width, 0);
        const RectF actions = windowRect(fixture.driver.find("header:actions"));
        // As far from the end as from the top.
        EXPECT_NEAR(clockapp::style::windowWidth - actions.right(), actions.y, 1 / scale);
        EXPECT_NEAR(actions.y + actions.height / 2, buttonsCentre(fixture), 0.5 / scale);
        EXPECT_EQ(actions.height, HeaderButtonGroup::kSegment);
        EXPECT_LT(edit.right(), add.x);
        EXPECT_TRUE(onDevicePixels(actions.x, scale));
        EXPECT_TRUE(onDevicePixels(actions.y, scale));
    }
}

TEST(HeaderTest, AtTheNarrowestTheSwitchKeepsClearOfBothEnds)
{
    for (const double scale : kScales) {
        SCOPED_TRACE(scale);
        WindowFixture fixture(scale, {clockapp::style::windowMinimumWidth, clockapp::style::windowMinimumHeight});
        fixture.window->headerBar()->setActions(someActions(), false);
        fixture.driver.layout();
        auto* header = dynamic_cast<WindowHeader*>(fixture.window->headerBar()->parent());
        ASSERT_NE(header, nullptr);
        const RectF tabs = windowRect(fixture.window->headerBar()->tabSwitch());
        const RectF actions = windowRect(fixture.driver.find("header:actions"));
        EXPECT_GE(tabs.x, header->leadingInset() + 12 - 1e-9);
        EXPECT_LE(tabs.right(), actions.x - 12 + 1 / scale);
        EXPECT_GE(tabs.width, 280);
    }
}
