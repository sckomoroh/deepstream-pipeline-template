#pragma once

#include <optional>

#include "IBinFactory.h"
#include "common/string-tools.h"

namespace yz {

template <class TConfig>
class BaseBinFactory : public IBinFactory {
public:
    enum class EPadType { Static, Simple };

public:
    GstBin* createBin(const YAML::Node& node, int index) override {
        using namespace common;

        auto config = parseConfig(node, index);
        if (config.has_value() == false) {
            return nullptr;
        }

        auto fullBinName = str::format("%s-%d", binName().c_str(), index);
        GstBin* bin = GST_BIN(gst_bin_new(fullBinName.c_str()));

        std::vector<GstElement*> elements;
        if (createChildren(node, fullBinName, bin, elements) == false) {
            gst_object_unref(bin);

            return nullptr;
        }

        if (connectChildren(elements) == false) {
            gst_object_unref(bin);

            return nullptr;
        }

        if (setupChildren(config.value(), elements) == false) {
            gst_object_unref(bin);

            return nullptr;
        }

        if (createPads(bin, elements) == false) {
            gst_object_unref(bin);

            return nullptr;
        }

        return bin;
    }

protected:
    virtual std::string binName() const = 0;

    virtual bool createChildren(const YAML::Node& node,
                                const std::string& binName,
                                GstBin* bin,
                                std::vector<GstElement*>& elements) = 0;

    virtual bool connectChildren(const std::vector<GstElement*>& elements) = 0;

    virtual bool setupChildren(const TConfig& config, const std::vector<GstElement*>& elements) = 0;

    virtual bool createPads(GstBin* bin, const std::vector<GstElement*>& elements) = 0;

    virtual std::optional<TConfig> parseConfig(const YAML::Node& node, int index) = 0;

protected:
    bool createPad(GstBin* bin,
                   GstElement* element,
                   const std::string& elementPadName,
                   const std::string& padName,
                   EPadType padType = EPadType::Static) {
        GstPad* pad;
        switch (padType) {
        case EPadType::Static:
            pad = gst_element_get_static_pad(element, elementPadName.c_str());
            break;
        case EPadType::Simple:
            pad = gst_element_request_pad_simple(element, elementPadName.c_str());
            break;
        }

        if (pad == nullptr) {
            return false;
        }

        GstPad* binSrcPad = gst_ghost_pad_new(padName.c_str(), pad);
        if (binSrcPad == nullptr) {
            gst_object_unref(pad);
            return false;
        }

        if (gst_element_add_pad(GST_ELEMENT(bin), binSrcPad) == false) {
            gst_object_unref(pad);
            return false;
        }

        gst_object_unref(pad);
        return true;
    }
};

}  // namespace yz