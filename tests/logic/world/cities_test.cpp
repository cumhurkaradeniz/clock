// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "world/cities.h"
#include "world/citydatabase.h"

using namespace clockapp::world;

static const char* kZoneTab =
    "SG\t+0117+10351\tAsia/Singapore\n"
    "GB\t+513030-0000731\tEurope/London\n"
    "JP\t+353916+1394441\tAsia/Tokyo\n"
    "US\t+404251-0740023\tAmerica/New_York\n"
    "FR\t+4852+00220\tEurope/Paris\n"
    "AU\t-3352+15113\tAustralia/Sydney\n";

using List = std::vector<std::string>;

TEST(Cities, FirstRunIsTheLocalCityThenTheDefaults)
{
    const CityDatabase database(kZoneTab, "en");
    Cities cities(database);
    cities.load(std::nullopt, "Asia/Singapore");
    EXPECT_EQ(cities.zoneIds(), (List{"Asia/Singapore", "America/New_York", "Europe/London", "Europe/Paris",
                                      "Asia/Tokyo", "Australia/Sydney"}));
}

TEST(Cities, FirstRunHasNoSecondCopyOfTheLocalZone)
{
    const CityDatabase database(kZoneTab, "en");
    Cities cities(database);
    cities.load(std::nullopt, "Europe/London");
    EXPECT_EQ(cities.zoneIds(),
              (List{"Europe/London", "America/New_York", "Europe/Paris", "Asia/Tokyo", "Australia/Sydney"}));
}

TEST(Cities, ARemovedDefaultStaysGoneAfterARestart)
{
    const CityDatabase database(kZoneTab, "en");
    Cities first(database);
    first.load(std::nullopt, "Asia/Singapore");
    first.remove(2);
    const List saved = first.zoneIds();
    EXPECT_FALSE(first.contains("Europe/London"));

    Cities second(database);
    second.load(saved, "Asia/Singapore");
    EXPECT_EQ(second.zoneIds(), saved);
    EXPECT_FALSE(second.contains("Europe/London"));

    // An emptied list stays empty.
    second.load(List{}, "Asia/Singapore");
    EXPECT_TRUE(second.zoneIds().empty());
}

TEST(Cities, LoadDropsUnknownAndRepeatedZones)
{
    const CityDatabase database(kZoneTab, "en");
    Cities cities(database);
    cities.load(List{"Asia/Tokyo", "Nowhere/Atlantis", "Asia/Tokyo", "Etc/UTC"}, "Europe/London");
    EXPECT_EQ(cities.zoneIds(), (List{"Asia/Tokyo", "Etc/UTC"}));
}

TEST(Cities, AddRemoveMove)
{
    const CityDatabase database(kZoneTab, "en");
    Cities cities(database);
    cities.load(List{}, {});
    int changes = 0;
    auto subscription = cities.changed.connect([&] { ++changes; });

    EXPECT_TRUE(cities.add("Asia/Singapore"));
    EXPECT_TRUE(cities.add("Europe/London"));
    EXPECT_TRUE(cities.add("Asia/Tokyo"));
    EXPECT_FALSE(cities.add("Asia/Tokyo"));
    EXPECT_FALSE(cities.add("Nowhere/Atlantis"));
    EXPECT_EQ(changes, 3);

    cities.move(0, 2);
    EXPECT_EQ(cities.zoneIds(), (List{"Europe/London", "Asia/Tokyo", "Asia/Singapore"}));
    cities.move(2, 0);
    EXPECT_EQ(cities.zoneIds(), (List{"Asia/Singapore", "Europe/London", "Asia/Tokyo"}));
    cities.move(1, 5);
    EXPECT_EQ(changes, 5);

    cities.remove(1);
    EXPECT_EQ(cities.zoneIds(), (List{"Asia/Singapore", "Asia/Tokyo"}));
    cities.remove(9);
    EXPECT_EQ(changes, 6);
    EXPECT_TRUE(cities.contains("Asia/Tokyo"));
    EXPECT_FALSE(cities.contains("Europe/London"));
}
