#include "SourceBinFactory.h"

#include "common/string-tools.h"
#include "common/yaml-tools.h"

namespace yz {

namespace {

constexpr const char* FACTORY_FILESRC = "filesrc";
constexpr const char* FACTORY_QTDEMUX = "qtdemux";
constexpr const char* FACTORY_H264PARSE = "h264parse";
constexpr const char* FACTORY_NVV4L2DECODER = "nvv4l2decoder";

constexpr const char* BIN_NAME = "yz-source";

constexpr const char* PROPERTY_LOCATION = "location";

constexpr const char* PAD_SRC = "src";
constexpr const char* PAD_SINK = "sink";

}  // namespace

std::string SourceBinFactory::binName() const { return BIN_NAME; }

bool SourceBinFactory::createChildren(const YAML::Node& node,
                                      const std::string& binName,
                                      GstBin* bin,
                                      std::vector<GstElement*>& elements) {
    GST_INFO("SourceBinFactory::createChildren:");
    using namespace common;

    auto elementName = str::format("%s-%s", binName.c_str(), FACTORY_FILESRC);
    auto filesrcElement = gst_element_factory_make(FACTORY_FILESRC, elementName.c_str());
    if (filesrcElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, filesrcElement);

    elementName = str::format("%s-%s", binName.c_str(), FACTORY_QTDEMUX);
    auto qtdemuxElement = gst_element_factory_make(FACTORY_QTDEMUX, elementName.c_str());
    if (qtdemuxElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, qtdemuxElement);

    elementName = str::format("%s-%s", binName.c_str(), FACTORY_H264PARSE);
    auto h264parserElement = gst_element_factory_make(FACTORY_H264PARSE, elementName.c_str());
    if (h264parserElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, h264parserElement);

    elementName = str::format("%s-%s", binName.c_str(), FACTORY_NVV4L2DECODER);
    auto nvv512decoderElement = gst_element_factory_make(FACTORY_NVV4L2DECODER, elementName.c_str());
    if (nvv512decoderElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, nvv512decoderElement);

    elements = {filesrcElement, qtdemuxElement, h264parserElement, nvv512decoderElement};

    GST_INFO("SourceBinFactory::createChildren: Done");

    return true;
}

bool SourceBinFactory::connectChildren(const std::vector<GstElement*>& elements) {
    if (gst_element_link_many(elements.at(INDEX_FILESRC), elements.at(INDEX_QTDEMUX), nullptr) == false) {
        return false;
    }

    if (gst_element_link_many(elements.at(INDEX_H264PARSER), elements.at(INDEX_NVV512DECODER), nullptr) == false) {
        return false;
    }

    g_signal_connect(elements.at(INDEX_QTDEMUX), "pad-added", G_CALLBACK(&SourceBinFactory::onQTDemuxPadAdded),
                     elements.at(INDEX_H264PARSER));

    return true;
}

bool SourceBinFactory::setupChildren(const source::Config& config, const std::vector<GstElement*>& elements) {
    g_object_set(G_OBJECT(elements.at(INDEX_FILESRC)), PROPERTY_LOCATION, config.fileSrc.location.c_str(), nullptr);

    return true;
}

bool SourceBinFactory::createPads(GstBin* bin, const std::vector<GstElement*>& elements) {
    if (createPad(bin, elements.at(INDEX_NVV512DECODER), PAD_SRC, PAD_SRC) == false) {
        return false;
    }

    return true;
}

std::optional<source::Config> SourceBinFactory::parseConfig(const YAML::Node& node, int index) {
    using namespace common;

    source::Config config;
    for (const auto& item : node) {
        auto name = item.first.as<std::string>();
        auto binClass = str::removeSuffixIndex(name);

        if (binClass == FACTORY_FILESRC) {
            auto location = yaml::getValue<std::string>(PROPERTY_LOCATION, item.second);
            if (location.has_value() == false) {
                return {};
            }

            config.fileSrc.location = location.value();
        }
    }

    return config;
}

void SourceBinFactory::onQTDemuxPadAdded(GstElement*, GstPad* newPad, gpointer data) {
    GstElement* target = static_cast<GstElement*>(data);

    GstPad* sinkPad = gst_element_get_static_pad(target, PAD_SINK);
    if (gst_pad_is_linked(sinkPad)) {
        gst_object_unref(sinkPad);
        return;
    }

    if (gst_pad_link(newPad, sinkPad) != GST_PAD_LINK_OK) {
        GST_ERROR("Failed to link qtdemux to h264parser\n");
    }

    gst_object_unref(sinkPad);
}

}  // namespace yz