#pragma once

#include "BaseBinFactory.h"
#include "common/yaml-tools.h"

namespace yz {

namespace source {

struct Config {
    struct {
        int index;
        std::string location;
    } fileSrc;
};

}  // namespace source

class SourceBinFactory : public BaseBinFactory<source::Config> {
private:
    std::string binName() const override;

    bool createChildren(const YAML::Node& node, const std::string& binName, GstBin* bin, std::vector<GstElement*>& elements) override;

    bool connectChildren(const std::vector<GstElement*>& elements) override;

    bool setupChildren(const source::Config& config, const std::vector<GstElement*>& elements) override;

    bool createPads(GstBin* bin, const std::vector<GstElement*>& elements) override;

    std::optional<source::Config> parseConfig(const YAML::Node& node, int index) override;

private:
    static void onQTDemuxPadAdded(GstElement*, GstPad* newPad, gpointer data);
};

}  // namespace yz