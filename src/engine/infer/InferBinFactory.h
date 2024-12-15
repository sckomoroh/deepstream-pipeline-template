#pragma once

#include "engine/BaseBinFactory.h"
#include "common/tools.h"
#include "common/yaml-tools.h"
namespace yz {

namespace infer {

struct Config {
    struct {
        int batch_size;
        int batched_push_timeout;
        int width;
        int height;
    } streammux;

    struct {
        int gpu_id;
        std::string config_file_path;
    } primary_gie;
};

}  // namespace infer

class InferBinFactory : public BaseBinFactory<infer::Config> {
private:
    enum {
        INDEX_NVSTREAMMUX = 0,
        INDEX_NVINFER,
        INDEX_NV_VIDEO_CONVERT,
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

    void dumpProperties(GstElement* element);

    bool setupChildren(const infer::Config& config, const std::vector<GstElement*>& elements) override;

    bool createPads(GstBin* bin, const std::vector<GstElement*>& elements) override;

    std::optional<infer::Config> parseConfig(const YAML::Node& node, int index) override;

    bool parseStreammuxConfig(const YAML::Node& node, infer::Config& config);

    bool parsePrimaryGieConfig(const YAML::Node& node, infer::Config& config);
};

}  // namespace yz
