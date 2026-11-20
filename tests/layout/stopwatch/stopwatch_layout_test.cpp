// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pagefixture.h"

#include "stopwatch/dialview.h"
#include "stopwatch/digitdisplay.h"
#include "stopwatch/stopwatchpage.h"

#include "ui/bitmap.h"
#include "ui/scroll_bar.h"
#include "ui/scroll_view.h"
#include "ui/theme.h"

#include <gtest/gtest.h>

#include <chrono>

#include <cmath>

using namespace clockapp;
using namespace cutefish::ui;

// Where the vertical bar's handle reaches in the bar, top and bottom, in
// logical pixels, with the bar always shown: the track's ends at the top
// and the bottom of the content.
static std::pair<double, double> handleEnds(PageFixture& fixture, ScrollView* view, bool atBottom)
{
    view->setScrollPosition({0, atBottom ? 1e9 : 0});
    fixture.driver.layout();
    // Shown once the content is known to overflow, then faded in on frames.
    view->setVerticalBarPolicy(ScrollView::BarPolicy::AsNeeded);
    view->setVerticalBarPolicy(ScrollView::BarPolicy::AlwaysOn);
    for (int i = 0; i < 25; ++i) {
        fixture.driver.layout();
        TestDriver::waitFor([] { return false; }, std::chrono::milliseconds(20));
    }
    ScrollBar* bar = nullptr;
    for (const auto& child : view->children()) {
        auto* candidate = dynamic_cast<ScrollBar*>(child.get());
        if (candidate && candidate->geometry().height > candidate->geometry().width)
            bar = candidate;
    }
    if (!bar)
        return {-1, -1};
    const double scale = fixture.window.scale();
    Bitmap image = bar->grab(scale);
    const int x = image.width() / 2;
    int first = -1;
    int last = -1;
    for (int y = 0; y < image.height(); ++y) {
        const uint8_t alpha = image.constBits()[size_t(y) * image.stride() + size_t(x) * 4 + 3];
        if (alpha > 24) {
            if (first < 0)
                first = y;
            last = y + 1;
        }
    }
    return {first / scale, last / scale};
}

class StopwatchLayout : public testing::TestWithParam<double>
{
protected:
    StopwatchLayout()
        : fixture(GetParam())
    {
    }

    StopwatchPage& open()
    {
        auto page = std::make_unique<StopwatchPage>(fixture.context);
        StopwatchPage* made = page.get();
        fixture.show(std::move(page));
        return *made;
    }

    // Within half a device pixel.
    double tolerance() const { return 0.5 / GetParam(); }

    PageFixture fixture;
};

TEST_P(StopwatchLayout, FiguresAreCentred)
{
    open();
    const RectF frame = fixture.driver.frame("stopwatch:digits");
    DigitDisplay* digits = fixture.driver.find<DigitDisplay>("stopwatch:digits");
    ASSERT_TRUE(digits);
    // The line of figures on the window's middle, and its cells one width
    // each, so the middle stays put as they change.
    const double middle = frame.x + digits->lineStart() + digits->lineWidth() / 2;
    EXPECT_NEAR(middle, clockapp::style::windowWidth / 2, 1);
    EXPECT_GT(digits->lineWidth(), 0);
    const double width = digits->lineWidth();
    digits->setText("59:59.99");
    EXPECT_DOUBLE_EQ(digits->lineWidth(), width);

    // The figures are centred in their band too.
    double top = 1e9;
    double bottom = -1e9;
    for (const auto& child : digits->children()) {
        if (!child->isVisible())
            continue;
        top = std::min(top, child->geometry().y);
        bottom = std::max(bottom, child->geometry().bottom());
    }
    EXPECT_NEAR((top + bottom) / 2, frame.height / 2, 1);
}

TEST_P(StopwatchLayout, ButtonsLineUpWithTheLaps)
{
    StopwatchPage& page = open();
    page.startOrStop();
    page.lapOrReset();
    fixture.driver.layout();
    const RectF lap = fixture.driver.frame("stopwatch:lap");
    const RectF start = fixture.driver.frame("stopwatch:start");
    const std::vector<View*> rows = fixture.driver.findAll<View>("laps:row");
    ASSERT_FALSE(rows.empty());
    const PointF rowStart = rows.front()->mapToWindow({0, 0});
    const double rowEnd = rowStart.x + rows.front()->geometry().width;
    EXPECT_NEAR(lap.x, rowStart.x, tolerance());
    EXPECT_NEAR(start.right(), rowEnd, tolerance());
    EXPECT_NEAR(lap.y, start.y, tolerance());
    EXPECT_NEAR(lap.width, lap.height, tolerance());
    // The column sits on the window's middle; the list itself spans the
    // window, so its bar sits at the window's edge.
    EXPECT_NEAR((rowStart.x + rowEnd) / 2, clockapp::style::windowWidth / 2, 1);
    const RectF laps = fixture.driver.frame("stopwatch:laps");
    EXPECT_NEAR(laps.x, 0, tolerance());
    EXPECT_NEAR(laps.right(), clockapp::style::windowWidth, tolerance());
    EXPECT_GE(lap.y, fixture.driver.frame("stopwatch:digits").bottom() - 1);
}

TEST_P(StopwatchLayout, DialIsRoundAndCentred)
{
    StopwatchPage& page = open();
    page.setFace(StopwatchPage::Face::Dial);
    TestDriver::waitFor([&] {
        fixture.driver.layout();
        return !fixture.driver.find("stopwatch:digits")->isVisible();
    });
    DialView* dial = fixture.driver.find<DialView>("stopwatch:dial");
    ASSERT_TRUE(dial);
    const RectF face = dial->faceRect();
    EXPECT_EQ(face.width, face.height);
    EXPECT_NEAR(face.x + face.width / 2, dial->geometry().width / 2, tolerance());
    EXPECT_NEAR(face.y + face.height / 2, dial->geometry().height / 2, tolerance());
    // The readout's figures on the dial's vertical axis.
    DigitDisplay* readout = dial->readout();
    const double middle = readout->geometry().x + readout->lineStart() + readout->lineWidth() / 2;
    EXPECT_NEAR(middle, face.x + face.width / 2, 1);
    // The buttons stay clear of the dial.
    EXPECT_GE(fixture.driver.frame("stopwatch:lap").y, fixture.driver.frame("stopwatch:dial").bottom() - 1);
}

TEST_P(StopwatchLayout, WideWindowShowsBoth)
{
    fixture.window.resize({1000, 600});
    StopwatchPage& page = open();
    EXPECT_TRUE(page.showsBoth());
    EXPECT_TRUE(fixture.driver.shows("stopwatch:dial"));
    EXPECT_TRUE(fixture.driver.shows("stopwatch:digits"));
    const RectF dial = fixture.driver.frame("stopwatch:dial");
    const RectF digits = fixture.driver.frame("stopwatch:digits");
    EXPECT_LE(dial.right(), digits.x + tolerance());
}

TEST_P(StopwatchLayout, LapsBarClearsTheWindowCorner)
{
    StopwatchPage& page = open();
    page.startOrStop();
    for (int i = 0; i < 30; ++i)
        page.lapOrReset();
    fixture.driver.layout();
    auto* laps = fixture.driver.find<ScrollView>("stopwatch:laps");
    ASSERT_TRUE(laps);
    // The list's bottom edge is the window's: the track stops the corner's
    // padding short of it.
    const double height = laps->geometry().height;
    const double top = handleEnds(fixture, laps, false).first;
    EXPECT_GE(top, 0);
    EXPECT_LE(top, ScrollBar::inset + tolerance() * 2);
    EXPECT_NEAR(height - handleEnds(fixture, laps, true).second,
                ScrollBar::cornerPadding(Theme::current().windowRadius), tolerance() * 2);
}

INSTANTIATE_TEST_SUITE_P(Scales, StopwatchLayout, testing::Values(1.0, 1.75, 2.0));
