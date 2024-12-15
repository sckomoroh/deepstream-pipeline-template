#include <gtest/gtest.h>

#include "engine/PiplineFactory.h"

class ContextFactoryTests : public testing::Test {
protected:
    yz::PiplineFactory factory;

protected:
    void SetUp() override {
        GError* error;
        gst_init_check(0, nullptr, &error);
    }
};

TEST_F(ContextFactoryTests, creation) {
    YAML::Node root = YAML::LoadFile("../config/config.yaml");
    auto bin = factory.createPipeLine(root);
}