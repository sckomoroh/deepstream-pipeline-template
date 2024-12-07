#include "SinkBinFactory.h"

#include <gstnvdsmeta.h>
#include <nvbufsurface.h>
#include <nvds_obj_encode.h>
#include <nvdsmeta_schema.h>

#include "BaseBinFactory.h"
#include "RegisteredFactories.h"
#include "common/yaml-tools.h"

namespace yz {

std::string SinkBinFactory::binName() const { return "yz-sink"; }

bool SinkBinFactory::createChildren(const YAML::Node& node,
                                    const std::string& binName,
                                    GstBin* bin,
                                    std::vector<GstElement*>& elements) {
    // tee -+-> YZRender
    //      +-> YZBroker

    using namespace common;

    auto elementName = str::format("%s-%s", binName.c_str(), "tee");
    auto teeElement = gst_element_factory_make("tee", elementName.c_str());
    if (teeElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, teeElement);

    elements.push_back(teeElement);

    auto factories = getFactories();

    for (const auto& child : node) {
        auto name = child.first.as<std::string>();
        auto binClass = str::removeSuffixIndex(name);

        auto factoryIter = factories.find(binClass);
        if (factoryIter == factories.end()) {
            // TODO: WARN
            continue;
        }

        auto index = str::getSuffixIndex(name);

        auto subBin = factoryIter->second->createBin(child.second, index.has_value() == true ? index.value() : -1);
        if (subBin == nullptr) {
            return false;
        }

        gst_bin_add(bin, GST_ELEMENT(subBin));
        elements.push_back(GST_ELEMENT(subBin));
    }

    return true;
}

bool SinkBinFactory::connectChildren(const std::vector<GstElement*>& elements) {
    GstElement* queueElement = elements.at(0);

    for (const auto& element : elements) {
        gst_element_link(queueElement, element);
    }

    return true;
}

bool SinkBinFactory::setupChildren(const sink::Config& config, const std::vector<GstElement*>& elements) {
    return true;
}

bool SinkBinFactory::createPads(GstBin* bin, const std::vector<GstElement*>& elements) {
    using namespace common;

    if (createPad(bin, elements.at(0), "sink", "sink") == false) {
        return false;
    }

    return true;
}

std::optional<sink::Config> SinkBinFactory::parseConfig(const YAML::Node& node, int index) { return sink::Config(); }

}  // namespace yz