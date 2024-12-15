#include <gtest/gtest.h>

#include <gst/gst.h>

#include "engine/InferBinFactory.h"

using namespace yz::common;

class InferBinFactoryTests : public testing::Test {
protected:
    yz::InferBinFactory factory;

protected:
    void SetUp() override {
        GError* error;
        gst_init_check(0, nullptr, &error);
    }
};

TEST_F(InferBinFactoryTests, validConfig) {
    using namespace YAML;

    auto root = Node(NodeType::Map);
    root["yz-infer"] = Node(NodeType::Map);
    root["streammux"] = Node(NodeType::Map);
    root["streammux"]["batch-size"] = 0;
    root["streammux"]["batched-push-timeout"] = 0;
    root["streammux"]["width"] = 0;
    root["streammux"]["height"] = 0;

    root["primary-gie"] = Node(NodeType::Map);
    root["primary-gie"]["enable"] = 1;
    root["primary-gie"]["gpu-id"] = 0;
    root["primary-gie"]["gie-unique-id"] = 1;
    root["primary-gie"]["nvbuf-memory-type"] = 0;
    root["primary-gie"]["config-file-path"] = "../config/pgie-config.yml";
    
    yaml::dumpNode(root);

    auto bin = factory.createBin(root);
    ASSERT_NE(nullptr, bin);

    gst_object_unref(bin);
}
