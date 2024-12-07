#include "yaml-tools.h"

#include <iostream>

namespace yz::common::yaml {

std::optional<YAML::Node> getChild(const std::string& name, const YAML::Node& node) {
    std::optional<YAML::Node> child;

    try {
        child = node[name];
    } catch (YAML::InvalidNode& ex) {
        return child;
    }

    try {
        if (child.value().IsMap()) {
            return child;
        }
    } catch (YAML::InvalidNode& ex) {
        return {};
    }

    return {};
}

#define DUMP_YAML
void dumpNode(const YAML::Node& node) {
#ifdef DUMP_YAML
    YAML::Emitter out;
    out << node;

    // Print the result
    if (out.good()) {
        std::cout << out.c_str() << std::endl;
    } else {
        std::cerr << "Error emitting YAML: " << out.GetLastError() << std::endl;
    }
#endif  // DUMP_YAML
}

}  // namespace yz::common::yaml