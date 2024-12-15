#include "SinkBinFactory.h"

#include <gstnvdsmeta.h>
#include <nvbufsurface.h>
#include <nvds_obj_encode.h>
#include <nvdsmeta_schema.h>

#include "engine/BaseBinFactory.h"
#include "engine/RegisteredFactories.h"
#include "common/yaml-tools.h"

namespace yz {

namespace {

constexpr const char* BIN_NAME = "yz-sink";

constexpr const char* FACTORY_TEE = "tee";

constexpr const char* PAD_SINK = "sink";

}  // namespace

std::string SinkBinFactory::binName() const { return BIN_NAME; }

bool SinkBinFactory::createChildren(const YAML::Node& node,
                                    const std::string& binName,
                                    GstBin* bin,
                                    std::vector<GstElement*>& elements) {
    // tee -+-> YZRender
    //      +-> YZBroker

    using namespace common;

    auto elementName = str::format("%s-%s", binName.c_str(), FACTORY_TEE);
    auto teeElement = gst_element_factory_make(FACTORY_TEE, elementName.c_str());
    if (teeElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, teeElement);

    elementName = str::format("%s-fakesink-queue", binName.c_str());
    auto queueElement = gst_element_factory_make(FACTORY_TEE, elementName.c_str());
    if (queueElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, queueElement);

    elementName = str::format("%s-fakesink-fakesink", binName.c_str());
    auto fakesinkElement = gst_element_factory_make(FACTORY_TEE, elementName.c_str());
    if (fakesinkElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, fakesinkElement);

    if (gst_element_link_many(teeElement, queueElement, fakesinkElement, nullptr) == false) {
        GST_WARNING("Failed to link fake sink\n");
    }

    elements.push_back(teeElement);

    auto factories = getFactories();

    for (const auto& child : node) {
        auto name = child.first.as<std::string>();
        auto binClass = str::removeSuffixIndex(name);

        auto factoryIter = factories.find(binClass);
        if (factoryIter == factories.end()) {
            GST_WARNING("Factory for %s not found\n", name.c_str());
            continue;
        }

        auto index = str::getSuffixIndex(name);

        auto subBin = factoryIter->second->createBin(child.second, index.has_value() == true ? index.value() : -1);
        if (subBin == nullptr) {
            GST_ERROR("Failed to create bin %s\n", name.c_str());
            return false;
        }

        gst_bin_add(bin, GST_ELEMENT(subBin));
        elements.push_back(GST_ELEMENT(subBin));
    }

    return true;
}

bool SinkBinFactory::connectChildren(const std::vector<GstElement*>& elements) {
    GstElement* queueElement = elements.at(0);

    for (int i = INDEX_TEE + 1; i < elements.size(); i++) {
        if (gst_element_link(queueElement, elements[i]) == false) {
            GST_ERROR("Linkage failed\n");
            return false;
        }
    }

    return true;
}

bool SinkBinFactory::setupChildren(const sink::Config& config, const std::vector<GstElement*>& elements) {
    return true;
}

bool SinkBinFactory::createPads(GstBin* bin, const std::vector<GstElement*>& elements) {
    using namespace common;

    if (createPad(bin, elements.at(0), PAD_SINK, PAD_SINK) == false) {
        GST_ERROR("Failed to create pad %s\n", PAD_SINK);
        return false;
    }

    return true;
}

std::optional<sink::Config> SinkBinFactory::parseConfig(const YAML::Node& node, int index) { return sink::Config(); }

}  // namespace yz