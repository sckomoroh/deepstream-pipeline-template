#include "InferBinFactory.h"

#include "BaseBinFactory.h"
#include "common/string-tools.h"
#include "common/tools.h"
#include "common/yaml-tools.h"

namespace yz {

namespace {

constexpr const char* ELEMENT_NAME_STREAMMUX = "streammux";
constexpr const char* ELEMENT_NAME_PRIMARY_GIE = "primary-gie";

}  // namespace

std::string InferBinFactory::binName() const { return "yz-infer"; }

bool InferBinFactory::createChildren(const YAML::Node& node, const std::string& binName, GstBin* bin, std::vector<GstElement*>& elements) {
    using namespace common;

    auto elementName = str::format("%s-%s", binName.c_str(), "nvstreammux");
    auto nvstreammuxElement = gst_element_factory_make("nvstreammux", elementName.c_str());
    if (nvstreammuxElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, nvstreammuxElement);

    elementName = str::format("%s-%s", binName.c_str(), "nvinfer");
    auto nvinferElement = gst_element_factory_make("nvinfer", elementName.c_str());
    if (nvinferElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, nvinferElement);

    elements = {nvstreammuxElement, nvinferElement};

    return true;
}

bool InferBinFactory::connectChildren(const std::vector<GstElement*>& elements) {
    if (gst_element_link_many(elements.at(0), elements.at(1), nullptr) == false) {
        return false;
    }

    return true;
}

void InferBinFactory::dumpProperties(GstElement* element) {
    GObjectClass* objectClass = G_OBJECT_GET_CLASS(element);
    guint propertiesCount;
    auto properties = g_object_class_list_properties(objectClass, &propertiesCount);
    while (*properties != nullptr) {
        auto property = *properties;
        printf("[DEBUG]     Property: '%s'\n", property->name);
        properties++;
    }
}

bool InferBinFactory::setupChildren(const infer::Config& config, const std::vector<GstElement*>& elements) {
    g_object_set(G_OBJECT(elements.at(0)), "batch-size", config.streammux.batch_size, nullptr);
    g_object_set(G_OBJECT(elements.at(0)), "batched-push-timeout", config.streammux.batched_push_timeout, nullptr);
    g_object_set(G_OBJECT(elements.at(0)), "width", config.streammux.width, nullptr);
    g_object_set(G_OBJECT(elements.at(0)), "height", config.streammux.height, nullptr);

    g_object_set(G_OBJECT(elements.at(1)), "gpu-id", config.primary_gie.gpu_id, nullptr);
    g_object_set(G_OBJECT(elements.at(1)), "config-file-path", config.primary_gie.config_file_path.c_str(), nullptr);

    return true;
}

bool InferBinFactory::createPads(GstBin* bin, const std::vector<GstElement*>& elements) {
    using namespace common;

    /*    GstPad* nvstreammuxSinkPad = gst_element_request_pad_simple(elements.at(0), "sink_0");
        if (nvstreammuxSinkPad == nullptr) {
            return false;
        }

        GstPad* binSinkPad = gst_ghost_pad_new("yz-infer-sink-pad", nvstreammuxSinkPad);
        if (binSinkPad == nullptr) {
            gst_object_unref(nvstreammuxSinkPad);
            return false;
        }

        if (gst_element_add_pad(GST_ELEMENT(bin), binSinkPad) == false) {
            gst_object_unref(binSinkPad);
            gst_object_unref(nvstreammuxSinkPad);
            return false;
        }

        gst_object_unref(nvstreammuxSinkPad);

        // Src
        GstPad* nvinferSrcPad = gst_element_get_static_pad(elements.at(1), "src");
        if (nvinferSrcPad == nullptr) {
            return false;
        }

        GstPad* binSrckPad = gst_ghost_pad_new("yz-infer-src-pad", nvinferSrcPad);
        if (binSrckPad == nullptr) {
            gst_object_unref(nvinferSrcPad);
            return false;
        }

        if (gst_element_add_pad(GST_ELEMENT(bin), binSrckPad) == false) {
            gst_object_unref(binSrckPad);
            gst_object_unref(nvinferSrcPad);
            return false;
        }

        gst_object_unref(nvinferSrcPad);

        return true;*/
    // auto padName = str::format("%s-%s", binName.c_str(), "sink-pad");
    if (createPad(bin, elements.at(0), "sink_0", "sink", EPadType::Simple) == false) {
        return false;
    }

    // padName = str::format("%s-%s", binName.c_str(), "src-pad");
    if (createPad(bin, elements.at(1), "src", "src") == false) {
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

        if (binClass == ELEMENT_NAME_STREAMMUX) {
            if (parseStreammuxConfig(item.second, config) == false) {
                return {};
            }
        }

        if (binClass == ELEMENT_NAME_PRIMARY_GIE) {
            if (parsePrimaryGieConfig(item.second, config) == false) {
                return {};
            }
        }
    }

    return config;
}

bool InferBinFactory::parseStreammuxConfig(const YAML::Node& node, infer::Config& config) {
    using namespace common;

    auto batch_size = yaml::getValue<int>("batch-size", node);
    auto batched_push_timeout = yaml::getValue<int>("batched-push-timeout", node);
    auto width = yaml::getValue<int>("width", node);
    auto height = yaml::getValue<int>("height", node);

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

    auto gpu_id = yaml::getValue<int>("gpu-id", node);
    auto config_file_path = yaml::getValue<std::string>("config-file-path", node);

    if (gpu_id.has_value() == false || config_file_path.has_value() == false) {
        return false;
    }

    config.primary_gie.gpu_id = gpu_id.value();
    config.primary_gie.config_file_path = config_file_path.value();

    return true;
}

}  // namespace yz
