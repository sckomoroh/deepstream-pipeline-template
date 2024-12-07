#include "string-tools.h"

#include <stdexcept>
#include <vector>

namespace yz::common::str {

bool hasPrefix(const std::string& str, const std::string& prefix) { return str.find(prefix) == 0; }

bool hasSuffix(const std::string& str, const std::string& prefix) {
    return str.find(prefix) == str.length() - prefix.length();
}

std::optional<int> getSuffixIndex(const std::string& str) {
    std::string indexStr;

    auto iter = str.rbegin();
    while (iter != str.rend() && std::isdigit(*iter)) {
        indexStr.push_back(*iter);
        ++iter;
    }

    if (indexStr.empty()) {
        return std::nullopt;
    }

    std::reverse(indexStr.begin(), indexStr.end());

    return std::atoi(indexStr.c_str());
}

std::string format(const char* fmt, ...) {
    // Initialize a variable argument list
    va_list args;

    // First, determine the required size for the buffer
    va_start(args, fmt);
    int size = std::vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    if (size <= 0) {
        throw std::runtime_error("Error during formatting.");
    }

    // Create a buffer of the required size
    std::vector<char> buffer(size + 1);

    // Perform the actual formatting
    va_start(args, fmt);
    std::vsnprintf(buffer.data(), buffer.size(), fmt, args);
    va_end(args);

    // Return the formatted string
    return std::string(buffer.data());
}

std::string removeSuffixIndex(const std::string& str) {
    auto index = getSuffixIndex(str);
    if (index.has_value() == false) {
        return str;
    }

    auto suffix = "-" + std::to_string(index.value());
    return str.substr(0, str.length() - suffix.length());
}

}  // namespace yz::common::str