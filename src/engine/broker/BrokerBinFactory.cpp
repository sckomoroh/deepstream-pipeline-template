#include "BrokerBinFactory.h"

#include "common/yaml-tools.h"

namespace yz {

namespace {

constexpr const char* BIN_NAME = "yz-broker";

constexpr const char* FACTORY_QUEUE = "queue";
constexpr const char* FACTORY_NVMSGCONV = "nvmsgconv";
constexpr const char* FACTORY_NVMSGBROKER = "nvmsgbroker";

constexpr const char* PAD_SINK = "sink";

constexpr const char* PROPERTY_PAYLOAD_TYPE = "payload-type";
constexpr const char* PROPERTY_MSG2P_NEWAPI = "msg2p-newapi";
constexpr const char* PROPERTY_FRAME_INTERVAL = "frame-interval";
constexpr const char* PROPERTY_CONFIG = "config";
constexpr const char* PROPERTY_MULTIPLE_PAYLOADS = "multiple-payloads";
constexpr const char* PROPERTY_CONN_STR = "conn-str";
constexpr const char* PROPERTY_PROTO_LIB = "proto-lib";
constexpr const char* PROPERTY_SYNC = "sync";
constexpr const char* PROPERTY_TOPIC = "topic";
constexpr const char* PROPERTY_MSG2P_LIB = "msg2p-lib";

}  // namespace

std::string BrokerBinFactory::binName() const { return BIN_NAME; }

bool BrokerBinFactory::createChildren(const YAML::Node& node,
                                      const std::string& binName,
                                      GstBin* bin,
                                      std::vector<GstElement*>& elements) {
    using namespace common;

    // queue -> msgconv -> msgbroker
    auto elementName = str::format("%s-%s", binName.c_str(), FACTORY_QUEUE);
    auto queueElement = gst_element_factory_make(FACTORY_QUEUE, elementName.c_str());
    if (queueElement == nullptr) {
        GST_ERROR("Failed to create %s\n", FACTORY_QUEUE);
        return false;
    }
    gst_bin_add(bin, queueElement);

    elementName = str::format("%s-%s", binName.c_str(), FACTORY_NVMSGCONV);
    auto msgconvElement = gst_element_factory_make(FACTORY_NVMSGCONV, elementName.c_str());
    if (msgconvElement == nullptr) {
        GST_ERROR("Failed to create %s\n", FACTORY_NVMSGCONV);
        return false;
    }
    gst_bin_add(bin, msgconvElement);

    elementName = str::format("%s-%s", binName.c_str(), FACTORY_NVMSGBROKER);
    auto msgbrokerElement = gst_element_factory_make(FACTORY_NVMSGBROKER, elementName.c_str());
    if (msgbrokerElement == nullptr) {
        GST_ERROR("Failed to create %s\n", FACTORY_NVMSGBROKER);
        return false;
    }
    gst_bin_add(bin, msgbrokerElement);

    elements = {queueElement, msgconvElement, msgbrokerElement};

    return true;
}

bool BrokerBinFactory::connectChildren(const std::vector<GstElement*>& elements) {
    if (gst_element_link_many(elements.at(INDEX_QUEUE), elements.at(INDEX_MSG_CONV), elements.at(INDEX_MSG_BROKER),
                              nullptr) == false) {
        GST_ERROR("Linkage failed\n");
        return false;
    }

    return true;
}

bool BrokerBinFactory::setupChildren(const broker::Config& config, const std::vector<GstElement*>& elements) {
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_CONV)), PROPERTY_PAYLOAD_TYPE, config.nvmsgconv.payload_type, nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_CONV)), PROPERTY_MSG2P_NEWAPI, config.nvmsgconv.msg2p_newapi, nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_CONV)), PROPERTY_FRAME_INTERVAL, config.nvmsgconv.frame_interval,
                 nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_CONV)), PROPERTY_CONFIG, config.nvmsgconv.config.c_str(), nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_CONV)), PROPERTY_MULTIPLE_PAYLOADS, config.nvmsgconv.multiple_payloads,
                 nullptr);
    if (config.nvmsgconv.msg2p_lib.empty() == false) {
        g_object_set(G_OBJECT(elements.at(INDEX_MSG_CONV)), PROPERTY_MSG2P_LIB, config.nvmsgconv.msg2p_lib.c_str(),
                     nullptr);
    }
    // g_object_set(G_OBJECT(elements.at(INDEX_MSG_CONV)), "debug-payload-dir",
    // "/home/dev/Documents/sources/DS_testApp/debug-payload",
    //              nullptr);

    g_object_set(G_OBJECT(elements.at(INDEX_MSG_BROKER)), PROPERTY_CONN_STR, config.nvmsgbroker.conn_str.c_str(),
                 nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_BROKER)), PROPERTY_PROTO_LIB, config.nvmsgbroker.proto_lib.c_str(),
                 nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_BROKER)), PROPERTY_SYNC, config.nvmsgbroker.sync, nullptr);
    g_object_set(G_OBJECT(elements.at(INDEX_MSG_BROKER)), PROPERTY_TOPIC, config.nvmsgbroker.topic.c_str(), nullptr);

    return true;
}

bool BrokerBinFactory::createPads(GstBin* bin, const std::vector<GstElement*>& elements) {
    return createPad(bin, elements.at(0), PAD_SINK, PAD_SINK);
}

std::optional<broker::Config> BrokerBinFactory::parseConfig(const YAML::Node& node, int index) {
    using namespace common;

    auto config = broker::Config();

    auto nvmsgconvNode = yaml::getChild(FACTORY_NVMSGCONV, node);
    if (nvmsgconvNode.has_value() == false) {
        GST_ERROR("Config node %s not found\n", FACTORY_NVMSGCONV);
        return {};
    }

    if (parseMsgConvConfig(nvmsgconvNode.value(), config) == false) {
        return {};
    }

    auto nvmsgbrokerNode = yaml::getChild(FACTORY_NVMSGBROKER, node);
    if (nvmsgbrokerNode.has_value() == false) {
        GST_ERROR("Config node %s not found\n", FACTORY_NVMSGBROKER);
        return {};
    }

    if (parseMsgBrokerConfig(nvmsgbrokerNode.value(), config) == false) {
        return {};
    }

    return config;
}

bool BrokerBinFactory::parseMsgConvConfig(const YAML::Node& node, broker::Config& config) {
    using namespace common;

    auto payload_type = yaml::getValue<int>(PROPERTY_PAYLOAD_TYPE, node);
    if (payload_type.has_value() == false) {
        GST_ERROR("Property %s not found\n", PROPERTY_PAYLOAD_TYPE);
        return false;
    }

    auto msg2p_newapi = yaml::getValue<int>(PROPERTY_MSG2P_NEWAPI, node);
    if (msg2p_newapi.has_value() == false) {
        GST_ERROR("Property %s not found\n", PROPERTY_MSG2P_NEWAPI);
        return false;
    }

    auto frame_interval = yaml::getValue<int>(PROPERTY_FRAME_INTERVAL, node);
    if (frame_interval.has_value() == false) {
        GST_ERROR("Property %s not found\n", PROPERTY_FRAME_INTERVAL);
        return false;
    }

    auto config_file = yaml::getValue<std::string>(PROPERTY_CONFIG, node);
    if (config_file.has_value() == false) {
        GST_ERROR("Property %s not found\n", PROPERTY_CONFIG);
        return false;
    }

    auto msg2p_lib = yaml::getValue<std::string>(PROPERTY_MSG2P_LIB, node);
    if (msg2p_lib.has_value() == false) {
        GST_WARNING("Property %s not found\n", PROPERTY_MSG2P_LIB);
    }

    auto multiple_payloads = yaml::getValue<int>(PROPERTY_MULTIPLE_PAYLOADS, node);
    if (multiple_payloads.has_value() == false) {
        GST_WARNING("Property %s not found\n", PROPERTY_MULTIPLE_PAYLOADS);
    }

    config.nvmsgconv.payload_type = payload_type.value();
    config.nvmsgconv.msg2p_newapi = msg2p_newapi.value();
    config.nvmsgconv.frame_interval = frame_interval.value();
    config.nvmsgconv.config = config_file.value();
    config.nvmsgconv.msg2p_lib = msg2p_lib.value_or("");
    config.nvmsgconv.multiple_payloads = multiple_payloads.value();

    return true;
}

bool BrokerBinFactory::parseMsgBrokerConfig(const YAML::Node& node, broker::Config& config) {
    using namespace common;

    auto proto_lib = yaml::getValue<std::string>(PROPERTY_PROTO_LIB, node);
    if (proto_lib.has_value() == false) {
        GST_ERROR("Property %s not found\n", PROPERTY_PROTO_LIB);
        return false;
    }

    auto conn_str = yaml::getValue<std::string>(PROPERTY_CONN_STR, node);
    if (conn_str.has_value() == false) {
        GST_ERROR("Property %s not found\n", PROPERTY_CONN_STR);
        return false;
    }

    auto topic = yaml::getValue<std::string>(PROPERTY_TOPIC, node);
    if (topic.has_value() == false) {
        GST_ERROR("Property %s not found\n", PROPERTY_TOPIC);
        return false;
    }

    auto sync = yaml::getValue<int>(PROPERTY_SYNC, node);
    if (sync.has_value() == false) {
        GST_ERROR("Property %s not found\n", PROPERTY_SYNC);
        return false;
    }

    config.nvmsgbroker.conn_str = conn_str.value();
    config.nvmsgbroker.proto_lib = proto_lib.value();
    config.nvmsgbroker.sync = sync.value();
    config.nvmsgbroker.topic = topic.value();

    return true;
}

}  // namespace yz