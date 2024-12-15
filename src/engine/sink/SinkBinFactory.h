#pragma once

#include <gstnvdsmeta.h>
#include <nvbufsurface.h>
#include <nvds_obj_encode.h>
#include <nvdsmeta_schema.h>

#include "engine/BaseBinFactory.h"
#include "common/yaml-tools.h"

namespace yz {

namespace sink {

struct Config {};

}  // namespace sink

class SinkBinFactory : public BaseBinFactory<sink::Config> {
private:
    enum {
        INDEX_TEE = 0,
    };

public:
    using BaseBinFactory::BaseBinFactory;

protected:
    std::string binName() const override;

    bool createChildren(const YAML::Node& node,
                        const std::string& binName,
                        GstBin* bin,
                        std::vector<GstElement*>& elements) override;

    bool connectChildren(const std::vector<GstElement*>& elements) override;

    bool setupChildren(const sink::Config& config, const std::vector<GstElement*>& elements) override;

    bool createPads(GstBin* bin, const std::vector<GstElement*>& elements) override;

    std::optional<sink::Config> parseConfig(const YAML::Node& node, int index) override;
};

}  // namespace yz