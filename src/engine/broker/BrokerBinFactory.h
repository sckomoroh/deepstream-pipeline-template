#pragma

#include "engine/BaseBinFactory.h"

namespace yz {

namespace broker {

struct Config {
    struct {
        int payload_type;
        int msg2p_newapi;
        int frame_interval;
        std::string config;
        int multiple_payloads;
        std::string msg2p_lib;
    } nvmsgconv;

    struct {
        std::string proto_lib;
        std::string conn_str;
        std::string topic;
        int sync;
    } nvmsgbroker;
};

}  // namespace broker

class BrokerBinFactory : public BaseBinFactory<broker::Config> {
private:
    enum { INDEX_QUEUE = 0, INDEX_MSG_CONV, INDEX_MSG_BROKER };

public:
    using BaseBinFactory::BaseBinFactory;

protected:
    std::string binName() const override;

    bool createChildren(const YAML::Node& node,
                        const std::string& binName,
                        GstBin* bin,
                        std::vector<GstElement*>& elements) override;

    bool connectChildren(const std::vector<GstElement*>& elements) override;

    bool setupChildren(const broker::Config& config, const std::vector<GstElement*>& elements) override;

    bool createPads(GstBin* bin, const std::vector<GstElement*>& elements) override;

    std::optional<broker::Config> parseConfig(const YAML::Node& node, int index) override;

private:
    bool parseMsgConvConfig(const YAML::Node& node, broker::Config& config);

    bool parseMsgBrokerConfig(const YAML::Node& node, broker::Config& config);
};

}  // namespace yz