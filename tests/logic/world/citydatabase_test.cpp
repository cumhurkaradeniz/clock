// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "world/citydatabase.h"

using namespace clockapp::world;

static const char* kZoneTab =
    "# comment\n"
    "SG\t+0117+10351\tAsia/Singapore\n"
    "BR\t-2332-04637\tAmerica/Sao_Paulo\tS & SE Brazil\n"
    "AT\t+4813+01620\tEurope/Vienna\n"
    "VN\t+1045+10640\tAsia/Ho_Chi_Minh\n"
    "US\t+404251-0740023\tAmerica/New_York\tEastern (most areas)\n"
    "US\t+340308-1181434\tAmerica/Los_Angeles\tPacific\n"
    "CH,DE,LI\t+4723+00832\tEurope/Zurich\n"
    "XX\t+0000+00000\tNowhere/Atlantis\n";

static std::vector<std::string> ids(const std::vector<const City*>& cities)
{
    std::vector<std::string> result;
    for (const City* city : cities)
        result.push_back(city->zoneId);
    return result;
}

TEST(CityDatabase, ReadsTheTable)
{
    const CityDatabase database(kZoneTab, "en");
    // The unknown zone is left out.
    ASSERT_EQ(database.cities().size(), 7u);

    const City* singapore = database.find("Asia/Singapore");
    ASSERT_NE(singapore, nullptr);
    EXPECT_EQ(singapore->name, "Singapore");
    EXPECT_EQ(singapore->countryCode, "SG");
    EXPECT_EQ(singapore->country, "Singapore");
    EXPECT_TRUE(singapore->hasLocation);
    EXPECT_NEAR(singapore->latitude, 1 + 17 / 60.0, 1e-9);
    EXPECT_NEAR(singapore->longitude, 103 + 51 / 60.0, 1e-9);

    const City* newYork = database.find("America/New_York");
    ASSERT_NE(newYork, nullptr);
    EXPECT_EQ(newYork->name, "New York");
    EXPECT_NEAR(newYork->latitude, 40 + 42 / 60.0 + 51 / 3600.0, 1e-9);
    EXPECT_NEAR(newYork->longitude, -(74 + 0 / 60.0 + 23 / 3600.0), 1e-9);

    EXPECT_EQ(database.find("Europe/Zurich")->countryCode, "CH");
    EXPECT_EQ(database.find("America/Sao_Paulo")->name, "Sao Paulo");
}

TEST(CityDatabase, FindsZonesOutsideTheTable)
{
    const CityDatabase database(kZoneTab, "en");
    const City* utc = database.find("Etc/UTC");
    ASSERT_NE(utc, nullptr);
    EXPECT_EQ(utc->name, "UTC");
    EXPECT_FALSE(utc->hasLocation);
    EXPECT_EQ(database.find("Etc/UTC"), utc);
    EXPECT_EQ(database.find("Nowhere/Atlantis"), nullptr);
    EXPECT_EQ(database.find(""), nullptr);
}

TEST(CityDatabase, NamesInTheLanguage)
{
    const CityDatabase database(kZoneTab, "de");
    EXPECT_EQ(database.find("Europe/Vienna")->name, "Wien");
    EXPECT_EQ(database.find("Europe/Vienna")->country, "Österreich");
}

TEST(CityDatabase, SearchIgnoresCaseAndAccents)
{
    const CityDatabase database(kZoneTab, "en");
    EXPECT_EQ(ids(database.search("sao paulo")), std::vector<std::string>{"America/Sao_Paulo"});
    EXPECT_EQ(ids(database.search("SÃO")), std::vector<std::string>{"America/Sao_Paulo"});
    EXPECT_EQ(ids(database.search("  vienna ")), std::vector<std::string>{"Europe/Vienna"});
    EXPECT_TRUE(database.search("").empty());
    EXPECT_TRUE(database.search("   ").empty());

    const CityDatabase german(kZoneTab, "de");
    EXPECT_EQ(ids(german.search("osterreich")), std::vector<std::string>{"Europe/Vienna"});
    EXPECT_EQ(german.find("Europe/Zurich")->name, "Z\u00fcrich");
    EXPECT_EQ(ids(german.search("zurich")), std::vector<std::string>{"Europe/Zurich"});
    // The zone id still finds it in another language.
    EXPECT_EQ(ids(german.search("vienna")), std::vector<std::string>{"Europe/Vienna"});
}

TEST(CityDatabase, SearchRanksNamesBeforeCountries)
{
    const CityDatabase database(kZoneTab, "en");
    // Both US cities by country, Ho Chi Minh by a word inside its name.
    EXPECT_EQ(ids(database.search("united states")),
              (std::vector<std::string>{"America/Los_Angeles", "America/New_York"}));
    const std::vector<std::string> us = ids(database.search("us"));
    ASSERT_GE(us.size(), 2u);
    EXPECT_EQ(us[0], "America/Los_Angeles");
    EXPECT_EQ(us[1], "America/New_York");
    EXPECT_EQ(ids(database.search("minh")), std::vector<std::string>{"Asia/Ho_Chi_Minh"});
    // Vienna by its name before Ho Chi Minh by its country.
    EXPECT_EQ(ids(database.search("vi")), (std::vector<std::string>{"Europe/Vienna", "Asia/Ho_Chi_Minh"}));
    EXPECT_EQ(database.search("a", 2).size(), 2u);
}

TEST(CityDatabase, Folded)
{
    EXPECT_EQ(CityDatabase::folded("São Paulo"), "sao paulo");
    EXPECT_EQ(CityDatabase::folded("Zürich"), "zurich");
    EXPECT_EQ(CityDatabase::folded("Łódź"), "lodz");
    EXPECT_EQ(CityDatabase::folded("Los_Angeles"), "los angeles");
}

TEST(CityDatabase, NameFromZoneId)
{
    EXPECT_EQ(CityDatabase::nameFromZoneId("America/Argentina/Buenos_Aires"), "Buenos Aires");
    EXPECT_EQ(CityDatabase::nameFromZoneId("UTC"), "UTC");
}

TEST(CityDatabase, SystemDatabase)
{
    const CityDatabase& database = CityDatabase::system();
    EXPECT_GT(database.cities().size(), 300u);
    EXPECT_NE(database.find("Asia/Tokyo"), nullptr);
    EXPECT_NE(database.find(CityDatabase::localZoneId()), nullptr);
}
