#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include <gst/gst.h>

#include <yaml-cpp/yaml.h>

#include "common/yaml-tools.h"
#include "engine/IBinFactory.h"

namespace yz {

class PiplineFactory {
public:
    GstPipeline* createPipeLine(const YAML::Node& rootNode);

private:
    bool createSubBins(const YAML::Node& rootNode, GstPipeline* pipeline);
};

}  // namespace yz