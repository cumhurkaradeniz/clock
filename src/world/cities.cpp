// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "world/cities.h"

#include <algorithm>

#include "world/citydatabase.h"

namespace clockapp::world {

Cities::Cities(const CityDatabase& database)
    : m_database(database)
{
}

const std::vector<std::string>& Cities::defaultZoneIds()
{
    static const std::vector<std::string> ids = {"America/New_York", "Europe/London", "Europe/Paris", "Asia/Tokyo",
                                                 "Australia/Sydney"};
    return ids;
}

void Cities::load(const std::optional<std::vector<std::string>>& zoneIds, const std::string& localZoneId)
{
    m_zoneIds.clear();
    for (const std::string& id : zoneIds.value_or(std::vector<std::string>())) {
        if (!contains(id) && m_database.find(id))
            m_zoneIds.push_back(id);
    }
    if (!zoneIds) {
        if (m_database.find(localZoneId))
            m_zoneIds.push_back(localZoneId);
        for (const std::string& id : defaultZoneIds()) {
            if (!contains(id) && m_database.find(id))
                m_zoneIds.push_back(id);
        }
    }
    changed.emit();
}

bool Cities::contains(const std::string& zoneId) const
{
    return std::find(m_zoneIds.begin(), m_zoneIds.end(), zoneId) != m_zoneIds.end();
}

bool Cities::add(const std::string& zoneId)
{
    if (contains(zoneId) || !m_database.find(zoneId))
        return false;
    m_zoneIds.push_back(zoneId);
    changed.emit();
    return true;
}

void Cities::remove(size_t index)
{
    if (index >= m_zoneIds.size())
        return;
    m_zoneIds.erase(m_zoneIds.begin() + ptrdiff_t(index));
    changed.emit();
}

void Cities::move(size_t from, size_t to)
{
    if (from >= m_zoneIds.size() || to >= m_zoneIds.size() || from == to)
        return;
    std::string id = std::move(m_zoneIds[from]);
    m_zoneIds.erase(m_zoneIds.begin() + ptrdiff_t(from));
    m_zoneIds.insert(m_zoneIds.begin() + ptrdiff_t(to), std::move(id));
    changed.emit();
}

} // namespace clockapp::world
