#include "BrokerBinFactory.h"

#include "common/yaml-tools.h"

namespace yz {

namespace {

constexpr const char* BIN_NAME = "yz-broker";

}

std::string BrokerBinFactory::binName() const { return BIN_NAME; }

bool BrokerBinFactory::createChildren(const YAML::Node& node,
                                      const std::string& binName,
                                      GstBin* bin,
                                      std::vector<GstElement*>& elements) {
    using namespace common;

    // queue -> msgconv -> msgbroker
    auto elementName = str::format("%s-%s", binName.c_str(), "queue");
    auto queueElement = gst_element_factory_make("queue", elementName.c_str());
    if (queueElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, queueElement);

    elementName = str::format("%s-%s", binName.c_str(), "nvmsgconv");
    auto msgconvElement = gst_element_factory_make("nvmsgconv", elementName.c_str());
    if (msgconvElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, msgconvElement);

    elementName = str::format("%s-%s", binName.c_str(), "nvmsgbroker");
    auto msgbrokerElement = gst_element_factory_make("nvmsgbroker", elementName.c_str());
    if (msgbrokerElement == nullptr) {
        return false;
    }
    gst_bin_add(bin, msgbrokerElement);

    elements = {queueElement, msgconvElement, msgbrokerElement};

    return true;
}

bool BrokerBinFactory::connectChildren(const std::vector<GstElement*>& elements) {
    if (gst_element_link_many(elements.at(INDEX_QUEUE), elements.at(INDEX_MSG_CONV), elements.at(INDEX_MSG_BROKER),
                              nullptr) == false) {
        return false;
    }

    return true;
}

bool BrokerBinFactory::setupChildren(const broker::Config& config, const std::vector<GstElement*>& elements) {
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_CONV)), "payload-type", config.nvmsgconv.payload_type, nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_CONV)), "msg2p-newapi", config.nvmsgconv.msg2p_newapi, nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_CONV)), "frame-interval", config.nvmsgconv.frame_interval, nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_CONV)), "config", config.nvmsgconv.config.c_str(), nullptr);

    g_object_set(G_OBJECT(elements.at(INDEX_MSG_BROKER)), "conn-str", config.nvmsgbroker.conn_str.c_str(), nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_BROKER)), "proto-lib", config.nvmsgbroker.proto_lib.c_str(), nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_BROKER)), "sync", config.nvmsgbroker.sync, nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_BROKER)), "topic", config.nvmsgbroker.topic.c_str(), nullptr);

    return true;
}

bool BrokerBinFactory::createPads(GstBin* bin, const std::vector<GstElement*>& elements) {
    return createPad(bin, elements.at(0), "sink", "yz-broker-src-pad");
}

std::optional<broker::Config> BrokerBinFactory::parseConfig(const YAML::Node& node, int index) {
    using namespace common;

    auto nvmsgconvNode = yaml::getChild("nvmsgconv", node);
    if (nvmsgconvNode.has_value() == false) {
        return {};
    }

    auto payload_type = yaml::getValue<int>("payload-type", nvmsgconvNode.value());
    if (payload_type.has_value() == false) {
        return {};
    }

    auto msg2p_newapi = yaml::getValue<int>("msg2p-newapi", nvmsgconvNode.value());
    if (msg2p_newapi.has_value() == false) {
        return {};
    }

    auto frame_interval = yaml::getValue<int>("frame-interval", nvmsgconvNode.value());
    if (frame_interval.has_value() == false) {
        return {};
    }

    auto config_file = yaml::getValue<std::string>("config", nvmsgconvNode.value());
    if (config_file.has_value() == false) {
        return {};
    }

    auto nvmsgbrokerNode = yaml::getChild("nvmsgbroker", node);
    if (nvmsgbrokerNode.has_value() == false) {
        return {};
    }

    auto proto_lib = yaml::getValue<std::string>("proto-lib", nvmsgbrokerNode.value());
    if (proto_lib.has_value() == false) {
        return {};
    }

    auto conn_str = yaml::getValue<std::string>("conn-str", nvmsgbrokerNode.value());
    if (conn_str.has_value() == false) {
        return {};
    }

    auto topic = yaml::getValue<std::string>("topic", nvmsgbrokerNode.value());
    if (topic.has_value() == false) {
        return {};
    }

    auto sync = yaml::getValue<int>("sync", nvmsgbrokerNode.value());
    if (sync.has_value() == false) {
        return {};
    }

    auto config = broker::Config();
    config.nvmsgconv.payload_type = payload_type.value();
    config.nvmsgconv.msg2p_newapi = msg2p_newapi.value();
    config.nvmsgconv.frame_interval = frame_interval.value();
    config.nvmsgconv.config = config_file.value();

    config.nvmsgbroker.conn_str = conn_str.value();
    config.nvmsgbroker.proto_lib = proto_lib.value();
    config.nvmsgbroker.sync = sync.value();
    config.nvmsgbroker.topic = topic.value();

    return config;
}

}  // namespace yz