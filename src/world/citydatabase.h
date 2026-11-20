// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <vector>

namespace clockapp::world {

struct City
{
    // The IANA zone, such as "Asia/Singapore".
    std::string zoneId;
    // In the language the database was read in: "Ho Chi Minh City".
    std::string name;
    // ISO 3166, such as "VN".
    std::string countryCode;
    std::string country;
    // Degrees, north and east positive; both 0 for a zone without a place.
    double latitude = 0;
    double longitude = 0;
    bool hasLocation = false;
};

// The cities of the system's time zone database, one per zone of zone.tab,
// named in a language through ICU's exemplar city names.
class CityDatabase
{
public:
    // The zones of the system, named in the session's language.
    static const CityDatabase& system();

    // `zoneTab` in zone.tab's format (zone1970.tab's lists of country codes
    // are taken by their first code); `language` an ICU locale id, such as
    // "de" or "zh_Hans".
    CityDatabase(const std::string& zoneTab, const std::string& language);

    const std::vector<City>& cities() const { return m_cities; }
    // A zone the table does not list but the time zone database knows, such
    // as "Etc/UTC", is named from its id; nullptr only for unknown ones.
    const City* find(std::string_view zoneId) const;
    // Cities whose name, zone id or country matches the text, case and
    // accents ignored: names starting with it first, then ids, then names
    // containing it, then countries; alphabetical within each.
    std::vector<const City*> search(std::string_view text, size_t limit = 50) const;

    // The zone this machine is set to.
    static std::string localZoneId();
    // Lower case, accents and other combining marks removed: "São Paulo"
    // reads "sao paulo".
    static std::string folded(std::string_view text);
    // "America/Argentina/Buenos_Aires" reads "Buenos Aires".
    static std::string nameFromZoneId(std::string_view zoneId);

private:
    struct Entry
    {
        std::string name;
        std::string id;
        std::string country;
    };

    City make(const std::string& zoneId, const std::string& countryCode) const;

public:
    // One ISO 6709 coordinate of zone.tab, "+0117" or "-0583506":
    // `degreeDigits` is 2 for a latitude and 3 for a longitude.
    static double iso6709Degrees(std::string_view text, size_t degreeDigits);

private:

    std::string m_language;
    std::vector<City> m_cities;
    // folded() forms, by index into m_cities.
    std::vector<Entry> m_folded;
    // Zones outside the table that were asked for.
    mutable std::deque<City> m_extra;
};

} // namespace clockapp::world
