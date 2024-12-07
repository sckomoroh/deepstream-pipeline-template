#pragma once

#include <optional>
#include <string>

#include <yaml-cpp/yaml.h>

namespace yz::common::yaml {

template <class TValue>
std::optional<TValue> getValue(const std::string& name, const YAML::Node& node) {
    std::optional<TValue> value;

    auto valueNode = node[name];
    if (valueNode.IsDefined() == false) {
        return value;
    }

    if (valueNode.Type() != YAML::NodeType::Scalar) {
        return value;
    }

    try {
        value = valueNode.as<TValue>();
    } catch (YAML::BadConversion& ex) {
        return value;
    }

    return value;
}

std::optional<YAML::Node> getChild(const std::string& name, const YAML::Node& node);

void dumpNode(const YAML::Node& node);

}  // namespace yz::common::yaml