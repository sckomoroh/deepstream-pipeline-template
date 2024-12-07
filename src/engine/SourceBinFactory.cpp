#include "SourceBinFactory.h"

#include "common/string-tools.h"
#include "common/yaml-tools.h"

namespace yz {

namespace {

constexpr const char* ELEMENT_NAME_FILE_SRC = "filesrc";

}

std::string SourceBinFactory::binName() const { return "yz-source"; }

bool SourceBinFactory::createChildren(const YAML::Node& node,
                                      const std::string& binName,
                                      GstBin* bin,
                                      std::vector<GstElement*>& elements) {
    GST_INFO("SourceBinFactory::createChildren:");
    using namespace common;

    auto elementName = str::format("%s-%s", binName.c_str(), "filesrc");
    auto filesrcElement = gst_element_factory_make("filesrc", elementName.c_str());
    if (filesrcElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, filesrcElement);

    elementName = str::format("%s-%s", binName.c_str(), "qtdemux");
    auto qtdemuxElement = gst_element_factory_make("qtdemux", elementName.c_str());
    if (qtdemuxElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, qtdemuxElement);

    elementName = str::format("%s-%s", binName.c_str(), "h264parser");
    auto h264parserElement = gst_element_factory_make("h264parse", elementName.c_str());
    if (h264parserElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, h264parserElement);

    elementName = str::format("%s-%s", binName.c_str(), "nvv512decoder");
    auto nvv512decoderElement = gst_element_factory_make("nvv4l2decoder", elementName.c_str());
    if (nvv512decoderElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, nvv512decoderElement);

    elements = {filesrcElement, qtdemuxElement, h264parserElement, nvv512decoderElement};

    GST_INFO("SourceBinFactory::createChildren: Done");

    return true;
}

bool SourceBinFactory::connectChildren(const std::vector<GstElement*>& elements) {
    GST_INFO("SourceBinFactory::connectChildren: ");

    if (gst_element_link_many(elements.at(0), elements.at(1), nullptr) == false) {
        return false;
    }

    if (gst_element_link_many(elements.at(2), elements.at(3), nullptr) == false) {
        return false;
    }

    g_signal_connect(elements.at(1), "pad-added", G_CALLBACK(&SourceBinFactory::onQTDemuxPadAdded), elements.at(2));

    GST_INFO("SourceBinFactory::connectChildren: Done");

    return true;
}

bool SourceBinFactory::setupChildren(const source::Config& config, const std::vector<GstElement*>& elements) {
    GST_INFO("SourceBinFactory::setupChildren: ");

    g_object_set(G_OBJECT(elements.at(0)), "location", config.fileSrc.location.c_str(), nullptr);

    GST_INFO("SourceBinFactory::setupChildren: Done");

    return true;
}

bool SourceBinFactory::createPads(GstBin* bin, const std::vector<GstElement*>& elements) {
    using namespace common;

    GST_INFO("SourceBinFactory::createPads: ");

    if (createPad(bin, elements.at(3), "src", "src") == false) {
        return false;
    }

    GST_INFO("SourceBinFactory::createPads: Done");

    return true;
}

std::optional<source::Config> SourceBinFactory::parseConfig(const YAML::Node& node, int index) {
    using namespace common;

    GST_INFO("SourceBinFactory::parseConfig: ");

    source::Config config;
    for (const auto& item : node) {
        auto name = item.first.as<std::string>();
        auto binClass = str::removeSuffixIndex(name);

        if (binClass == ELEMENT_NAME_FILE_SRC) {
            auto location = yaml::getValue<std::string>("location", item.second);
            if (location.has_value() == false) {
                return {};
            }

            config.fileSrc.location = location.value();
        }
    }

    GST_INFO("SourceBinFactory::parseConfig: Done");

    return config;
}

void SourceBinFactory::onQTDemuxPadAdded(GstElement*, GstPad* newPad, gpointer data) {
    GST_INFO("SourceBinFactory::onQTDemuxPadAdded");
    GstElement* target = static_cast<GstElement*>(data);

    GstPad* sinkPad = gst_element_get_static_pad(target, "sink");
    if (gst_pad_is_linked(sinkPad)) {
        gst_object_unref(sinkPad);
        return;
    }

    // Attempt to link the new pad to the h264parser
    if (gst_pad_link(newPad, sinkPad) != GST_PAD_LINK_OK) {
        GST_ERROR("Failed to link qtdemux to h264parser\n");
        g_printerr("Failed to link qtdemux to h264parser\n");
    }

    gst_object_unref(sinkPad);
}

}  // namespace yz