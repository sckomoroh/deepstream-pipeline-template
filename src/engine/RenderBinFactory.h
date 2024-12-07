#pragma once

#include "BaseBinFactory.h"

namespace yz {

namespace render {

struct Config {};

}  // namespace render

class RenderBinFactory : public BaseBinFactory<render::Config> {
private:
    enum {
        INDEX_QUEUE = 0,
        INDEX_NV_VIDEO_CONVERT,
        INDEX_NV_DS_OSD,
        INDEX_NV_EGL_GLES_SINK
    };

protected:
    std::string binName() const override;

    bool createChildren(const YAML::Node& node, const std::string& binName, GstBin* bin, std::vector<GstElement*>& elements) override;

    bool connectChildren(const std::vector<GstElement*>& elements) override;

    bool setupChildren(const render::Config& config, const std::vector<GstElement*>& elements) override;

    bool createPads(GstBin* bin, const std::vector<GstElement*>& elements) override;

    std::optional<render::Config> parseConfig(const YAML::Node& node, int index) override;
};

}  // namespace yz