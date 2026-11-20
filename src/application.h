// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "base/signal.h"

#include <memory>
#include <string>
#include <vector>

namespace cutefish::base::dbus {
class Connection;
class Object;
}

namespace clockapp {

// The running Clock on the session bus: the single instance's object and
// the running-application object the dock follows.
class App
{
public:
    static constexpr const char* kService = "org.cutefish.Clock";
    static constexpr const char* kPath = "/Clock";

    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    // The single instance's object, before its name is asked for.
    void publish(cutefish::base::dbus::Connection* bus);
    // The dock's object, once the name is ours.
    void publishInstance(cutefish::base::dbus::Connection* bus);

    // Another launch asked for the window; Clock opens no files.
    cutefish::base::Signal<const std::string&> activateRequested;
    cutefish::base::Signal<> quitRequested;

private:
    std::unique_ptr<cutefish::base::dbus::Object> m_object;
    std::unique_ptr<cutefish::base::dbus::Object> m_instance;
};

} // namespace clockapp
