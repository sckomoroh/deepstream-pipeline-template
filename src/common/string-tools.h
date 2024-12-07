#pragma once

#include <cstdarg>

#include <optional>
#include <string>

namespace yz::common::str {

bool hasPrefix(const std::string& str, const std::string& prefix);

bool hasSuffix(const std::string& str, const std::string& prefix);

std::optional<int> getSuffixIndex(const std::string& str);

std::string removeSuffixIndex(const std::string& str);

std::string format(const char* fmt, ...);

}  // namespace yz::common::str
