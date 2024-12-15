#include "InferBinFactory.h"

#include "engine/BaseBinFactory.h"
#include "common/string-tools.h"
#include "common/tools.h"
#include "common/yaml-tools.h"

namespace yz {

namespace {

constexpr const char* BIN_NAME = "yz-infer";

constexpr const char* FACTORY_NVSTREAMMUX = "nvstreammux";
constexpr const char* FACTORY_NVINFER = "nvinfer";
constexpr const char* FACTORY_NVVIDEOCONVERT = "nvvideoconvert";

constexpr const char* PROPERTY_BATCH_SIZE = "batch-size";
constexpr const char* PROPERTY_BATCHED_PUSH_TIMEOUT = "batched-push-timeout";
constexpr const char* PROPERTY_WIDTH = "width";
constexpr const char* PROPERTY_HEIGHT = "height";
constexpr const char* PROPERTY_GPU_ID = "gpu-id";
constexpr const char* PROPERTY_CONFIG_FILE_PATH = "config-file-path";

constexpr const char* PAD_SRC = "src";
constexpr const char* PAD_SINK = "sink";

}  // namespace

std::string InferBinFactory::binName() const { return BIN_NAME; }

bool InferBinFactory::createChildren(const YAML::Node& node,
                                     const std::string& binName,
                                     GstBin* bin,
                                     std::vector<GstElement*>& elements) {
    using namespace common;

    auto elementName = str::format("%s-%s", binName.c_str(), FACTORY_NVSTREAMMUX);
    auto nvstreammuxElement = gst_element_factory_make(FACTORY_NVSTREAMMUX, elementName.c_str());
    if (nvstreammuxElement == nullptr) {
        GST_ERROR("Failed to create %s\n", FACTORY_NVSTREAMMUX);
        return false;
    }
    gst_bin_add(bin, nvstreammuxElement);

    elementName = str::format("%s-%s", binName.c_str(), FACTORY_NVINFER);
    auto nvinferElement = gst_element_factory_make(FACTORY_NVINFER, elementName.c_str());
    if (nvinferElement == nullptr) {
        GST_ERROR("Failed to create %s\n", FACTORY_NVINFER);
        return false;
    }
    gst_bin_add(bin, nvinferElement);

    elementName = str::format("%s-%s", binName.c_str(), FACTORY_NVVIDEOCONVERT);
    auto nvvideoconvertElement = gst_element_factory_make(FACTORY_NVVIDEOCONVERT, elementName.c_str());
    if (nvvideoconvertElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, nvvideoconvertElement);

    elements = {nvstreammuxElement, nvinferElement, nvvideoconvertElement};

    return true;
}

bool InferBinFactory::connectChildren(const std::vector<GstElement*>& elements) {
    if (gst_element_link_many(elements.at(INDEX_NVSTREAMMUX), elements.at(INDEX_NVINFER),
                              elements.at(INDEX_NV_VIDEO_CONVERT), nullptr) == false) {
        GST_ERROR("Linkage failed\n");
        return false;
    }

    return true;
}

bool InferBinFactory::setupChildren(const infer::Config& config, const std::vector<GstElement*>& elements) {
    g_object_set(G_OBJECT(elements.at(INDEX_NVSTREAMMUX)), PROPERTY_BATCH_SIZE, config.streammux.batch_size, nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_NVSTREAMMUX)), PROPERTY_BATCHED_PUSH_TIMEOUT,
                 config.streammux.batched_push_timeout, nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_NVSTREAMMUX)), PROPERTY_WIDTH, config.streammux.width, nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_NVSTREAMMUX)), PROPERTY_HEIGHT, config.streammux.height, nullptr);

    g_object_set(G_OBJECT(elements.at(INDEX_NVINFER)), PROPERTY_GPU_ID, config.primary_gie.gpu_id, nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_NVINFER)), PROPERTY_CONFIG_FILE_PATH,
                 config.primary_gie.config_file_path.c_str(), nullptr);

    return true;
}

bool InferBinFactory::createPads(GstBin* bin, const std::vector<GstElement*>& elements) {
    using namespace common;

    if (createPad(bin, elements.at(INDEX_NVSTREAMMUX), "sink_0", PAD_SINK, EPadType::Simple) == false) {
        GST_ERROR("Failed to create pad %s\n", PAD_SINK);
        return false;
    }

    if (createPad(bin, elements.at(INDEX_NV_VIDEO_CONVERT), PAD_SRC, PAD_SRC) == false) {
        GST_ERROR("Failed to create pad %s\n", PAD_SRC);
        return false;
    }

    return true;
}

std::optional<infer::Config> InferBinFactory::parseConfig(const YAML::Node& node, int index) {
    using namespace common;

    infer::Config config;
    for (const auto& item : node) {
        auto name = item.first.as<std::string>();
        auto binClass = str::removeSuffixIndex(name);

        if (binClass == FACTORY_NVSTREAMMUX) {
            if (parseStreammuxConfig(item.second, config) == false) {
                return {};
            }
        }

        if (binClass == FACTORY_NVINFER) {
            if (parsePrimaryGieConfig(item.second, config) == false) {
                return {};
            }
        }
    }

    return config;
}

bool InferBinFactory::parseStreammuxConfig(const YAML::Node& node, infer::Config& config) {
    using namespace common;

    auto batch_size = yaml::getValue<int>(PROPERTY_BATCH_SIZE, node);
    auto batched_push_timeout = yaml::getValue<int>(PROPERTY_BATCHED_PUSH_TIMEOUT, node);
    auto width = yaml::getValue<int>(PROPERTY_WIDTH, node);
    auto height = yaml::getValue<int>(PROPERTY_HEIGHT, node);

    if (batch_size.has_value() == false || batched_push_timeout.has_value() == false || width.has_value() == false ||
        height.has_value() == false) {
        return false;
    }

    config.streammux.batch_size = batch_size.value();
    config.streammux.batched_push_timeout = batched_push_timeout.value();
    config.streammux.width = width.value();
    config.streammux.height = height.value();

    return true;
}

bool InferBinFactory::parsePrimaryGieConfig(const YAML::Node& node, infer::Config& config) {
    using namespace common;

    auto gpu_id = yaml::getValue<int>(PROPERTY_GPU_ID, node);
    auto config_file_path = yaml::getValue<std::string>(PROPERTY_CONFIG_FILE_PATH, node);

    if (gpu_id.has_value() == false || config_file_path.has_value() == false) {
        return false;
    }

    config.primary_gie.gpu_id = gpu_id.value();
    config.primary_gie.config_file_path = config_file_path.value();

    return true;
}

}  // namespace yz
