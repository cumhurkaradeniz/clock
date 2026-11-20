// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "windowfixture.h"

#include "base/settings.h"
#include "ui/event.h"


using clockapp::Tab;
using namespace cutefish::ui;

static const char* const kPages[] = {"page:world", "page:alarms", "page:stopwatch", "page:timers"};

// Exactly the page of the tab shows, at rest.
static void expectShowing(WindowFixture& fixture, Tab tab)
{
    EXPECT_EQ(fixture.window->tab(), tab);
    for (int i = 0; i < clockapp::TabCount; ++i)
        EXPECT_EQ(fixture.driver.shows(kPages[i]), i == int(tab)) << kPages[i];
    clockapp::Page* page = fixture.window->page(tab);
    EXPECT_EQ(page->opacity(), 1);
    EXPECT_TRUE(page->transform().isIdentity());
    EXPECT_FALSE(page->isLayered());
    EXPECT_EQ(fixture.window->headerBar()->tabSwitch()->currentIndex(), int(tab));
    EXPECT_EQ(fixture.window->headerBar()->tabSwitch()->platePosition(), double(int(tab)));
}

TEST(WindowTest, OpensOnWorldClock)
{
    WindowFixture fixture;
    expectShowing(fixture, Tab::WorldClock);
}

TEST(WindowTest, ClickingATabSwitchesWithATransition)
{
    WindowFixture fixture;
    ASSERT_TRUE(fixture.driver.click("header:tab:alarms"));
    EXPECT_EQ(fixture.window->tab(), Tab::Alarms);
    EXPECT_TRUE(fixture.window->isSwitching());
    // Both pages show while one gives way to the other.
    EXPECT_TRUE(fixture.driver.shows("page:world"));
    EXPECT_TRUE(fixture.driver.shows("page:alarms"));
    ASSERT_TRUE(fixture.settle());
    expectShowing(fixture, Tab::Alarms);
}

TEST(WindowTest, ClickingTheCurrentTabDoesNothing)
{
    WindowFixture fixture;
    ASSERT_TRUE(fixture.driver.click("header:tab:world"));
    EXPECT_FALSE(fixture.window->isSwitching());
    EXPECT_FALSE(fixture.window->headerBar()->tabSwitch()->isGliding());
    expectShowing(fixture, Tab::WorldClock);
}

TEST(WindowTest, ASwitchDuringASwitchEndsOnTheLastTab)
{
    WindowFixture fixture;
    ASSERT_TRUE(fixture.driver.click("header:tab:alarms"));
    fixture.driver.layout();
    ASSERT_TRUE(fixture.driver.click("header:tab:timers"));
    fixture.driver.layout();
    ASSERT_TRUE(fixture.driver.click("header:tab:world"));
    ASSERT_TRUE(fixture.settle());
    expectShowing(fixture, Tab::WorldClock);
}

TEST(WindowTest, CommandDigitsPickTheTabs)
{
    WindowFixture fixture;
    const Tab tabs[] = {Tab::Stopwatch, Tab::Timers, Tab::Alarms, Tab::WorldClock};
    for (Tab tab : tabs) {
        fixture.driver.typeKey(uint32_t('1' + int(tab)), ControlModifier);
        ASSERT_TRUE(fixture.settle());
        expectShowing(fixture, tab);
    }
}

TEST(WindowTest, TheViewMenuPicksTheTabs)
{
    WindowFixture fixture;
    ASSERT_TRUE(fixture.driver.chooseMenuItem("View", "Stopwatch"));
    ASSERT_TRUE(fixture.settle());
    expectShowing(fixture, Tab::Stopwatch);
    ASSERT_TRUE(fixture.driver.chooseMenuItem("View", "World Clock"));
    ASSERT_TRUE(fixture.settle());
    expectShowing(fixture, Tab::WorldClock);
}

TEST(WindowTest, TheTabIsKeptForTheNextStart)
{
    ScratchDir dir;
    {
        WindowFixture fixture(1, {clockapp::style::windowWidth, clockapp::style::windowHeight}, &dir);
        ASSERT_TRUE(fixture.driver.click("header:tab:timers"));
        ASSERT_TRUE(fixture.settle());
    }
    {
        const cutefish::base::Settings settings(dir.file("clock.conf"));
        EXPECT_EQ(settings.stringValue("View/Tab"), "timers");
    }
    WindowFixture fixture(1, {clockapp::style::windowWidth, clockapp::style::windowHeight}, &dir);
    EXPECT_FALSE(fixture.window->isSwitching());
    expectShowing(fixture, Tab::Timers);
}

TEST(WindowTest, AnUnknownSavedTabOpensWorldClock)
{
    ScratchDir dir;
    {
        cutefish::base::Settings settings(dir.file("clock.conf"));
        settings.setValue("View/Tab", "calendar");
    }
    WindowFixture fixture(1, {clockapp::style::windowWidth, clockapp::style::windowHeight}, &dir);
    expectShowing(fixture, Tab::WorldClock);
}

TEST(WindowTest, TheFileMenuClosesTheWindow)
{
    WindowFixture fixture;
    EXPECT_TRUE(fixture.driver.chooseMenuItem("File", "Close Window"));
}
