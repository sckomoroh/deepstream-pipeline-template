#include "RenderBinFactory.h"

namespace yz {

namespace {

constexpr const char* BIN_NAME = "yz-render";

}

std::string RenderBinFactory::binName() const { return BIN_NAME; }

bool RenderBinFactory::createChildren(const YAML::Node& node,
                                      const std::string& binName,
                                      GstBin* bin,
                                      std::vector<GstElement*>& elements) {
    using namespace common;

    // queue -> nvvideoconvert -> nvdsosd -> nveglglessink
    auto elementName = str::format("%s-%s", binName.c_str(), "queue");
    auto queueElement = gst_element_factory_make("queue", elementName.c_str());
    if (queueElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, queueElement);

    elementName = str::format("%s-%s", binName.c_str(), "nvvideoconvert");
    auto nvvideoconvertElement = gst_element_factory_make("nvvideoconvert", elementName.c_str());
    if (nvvideoconvertElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, nvvideoconvertElement);

    elementName = str::format("%s-%s", binName.c_str(), "nvdsosd");
    auto nvdsosd = gst_element_factory_make("nvdsosd", elementName.c_str());
    if (nvdsosd == nullptr) {
        return false;
    }
    gst_bin_add(bin, nvdsosd);

    elementName = str::format("%s-%s", binName.c_str(), "nveglglessink");
    auto nveglglessinkElement = gst_element_factory_make("nveglglessink", elementName.c_str());
    if (nveglglessinkElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, nveglglessinkElement);

    elements = {queueElement, nvvideoconvertElement, nvdsosd, nveglglessinkElement};

    return true;
}

bool RenderBinFactory::connectChildren(const std::vector<GstElement*>& elements) {
    if (gst_element_link_many(elements.at(INDEX_QUEUE), elements.at(INDEX_NV_VIDEO_CONVERT),
                              elements.at(INDEX_NV_DS_OSD), elements.at(INDEX_NV_EGL_GLES_SINK), nullptr) == false) {
        return false;
    }

    return true;
}

bool RenderBinFactory::setupChildren(const render::Config& config, const std::vector<GstElement*>& elements) {
    return true;
}

bool RenderBinFactory::createPads(GstBin* bin, const std::vector<GstElement*>& elements) {
    return createPad(bin, elements.at(INDEX_QUEUE), "sink", "sink");
}

std::optional<render::Config> RenderBinFactory::parseConfig(const YAML::Node& node, int index) {
    render::Config config;

    return config;
}

}  // namespace yz