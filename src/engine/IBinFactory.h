#pragma once

#include <gst/gst.h>

#include <yaml-cpp/yaml.h>

namespace yz {

class IBinFactory {
public:
    virtual ~IBinFactory() = default;

public:
    virtual GstBin* createBin(const YAML::Node& node, int index) = 0;
};

}  // namespace yz