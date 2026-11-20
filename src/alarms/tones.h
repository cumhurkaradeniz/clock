// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>
#include <vector>

namespace clockapp::tones {

inline constexpr int SampleRate = 48000;

// The tone a sound name stands for; an unknown or empty name is the first.
std::string resolved(const std::string& name);

// "Dawn", "Chime", ..., translated.
std::string displayName(const std::string& name);

// One period of the tone, mono, which loops without a seam: the tail of
// its last note is folded back over its start.
std::vector<float> synthesize(const std::string& name);

} // namespace clockapp::tones
