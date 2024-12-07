#include "PiplineFactory.h"
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

#include <gst/gst.h>

#include <yaml-cpp/yaml.h>

#include "RegisteredFactories.h"
#include "common/string-tools.h"
#include "common/yaml-tools.h"

namespace yz {

GstPipeline* PiplineFactory::createPipeLine(const YAML::Node& rootNode) {
    auto firstNode = rootNode.begin();
    auto name = firstNode->first.as<std::string>();

    GstPipeline* pipeline = GST_PIPELINE(gst_pipeline_new(name.c_str()));

    if (createSubBins(firstNode->second, pipeline) == false) {
        g_object_unref(G_OBJECT(pipeline));

        pipeline = nullptr;
    }

    return pipeline;
}


bool PiplineFactory::createSubBins(const YAML::Node& rootNode, GstPipeline* pipeline) {
    using namespace common;

    auto factories = getFactories();

    GstBin* previosBin = nullptr;
    for (const auto& child : rootNode) {
        auto name = child.first.as<std::string>();
        auto binClass = str::removeSuffixIndex(name);

        auto factoryIter = factories.find(binClass);
        if (factoryIter == factories.end()) {
            GST_WARNING("PiplineFactory::createSubBins: Factory for %s not found", name.c_str());
            printf("PiplineFactory::createSubBins: Factory for %s not found\n", name.c_str());
            continue;
        }

        GST_INFO("Create %s bin", name.c_str());

        auto index = str::getSuffixIndex(name);

        auto bin = factoryIter->second->createBin(child.second, index.has_value() == true ? index.value() : -1);
        if (bin == nullptr) {
            return false;
        }

        gst_bin_add(GST_BIN(pipeline), GST_ELEMENT(bin));
        if (previosBin != nullptr) {
            if (gst_element_link(GST_ELEMENT(previosBin), GST_ELEMENT(bin)) == false) {
                return false;
            }
        }

        previosBin = bin;
    }

    return true;
}

}  // namespace yz