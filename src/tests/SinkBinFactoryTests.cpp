#include <gtest/gtest.h>

#include "engine/SinkBinFactory.h"

class SinkBinFactoryTest : public testing::Test {
protected:
    yz::SinkBinFactory factory;

protected:
    void SetUp() override {
        GError* error;
        gst_init_check(0, nullptr, &error);
    }
};

TEST_F(SinkBinFactoryTest, create) {
    auto bin = factory.createBin(YAML::Node(YAML::NodeType::Null));
    ASSERT_NE(nullptr, bin);

    gst_object_unref(bin);
}
