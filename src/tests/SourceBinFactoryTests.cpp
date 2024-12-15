#include <gtest/gtest.h>

#include <gst/gst.h>

#include "engine/SourceBinFactory.h"

using namespace yz::common;

class SourceBinFactoryTests : public testing::Test {
protected:
    yz::SourceBinFactory factory;

public:
    SourceBinFactoryTests() {}

    virtual ~SourceBinFactoryTests() {}

protected:
    static void SetUpTestSuite() {
        GError* error;
        gst_init_check(0, nullptr, &error);
    }

    // static void TearDownTestSuite() { gst_deinit(); }
};

TEST_F(SourceBinFactoryTests, noSource) {
    using namespace YAML;

    auto root = Node(NodeType::Map);
    root["file-source"] = Node(NodeType::Null);

    yaml::dumpNode(root);

    auto bin = factory.createBin(root);
    ASSERT_EQ(nullptr, bin);
}

TEST_F(SourceBinFactoryTests, sourceSequence) {
    using namespace YAML;

    auto root = Node(NodeType::Map);
    root["yz-source"] = Node(NodeType::Sequence);

    yaml::dumpNode(root);

    auto bin = factory.createBin(root);
    ASSERT_EQ(nullptr, bin);
}

TEST_F(SourceBinFactoryTests, sourceScalar) {
    using namespace YAML;

    auto root = Node(NodeType::Map);
    root["yz-source"] = 10;

    yaml::dumpNode(root);

    auto bin = factory.createBin(root);
    ASSERT_EQ(nullptr, bin);
}

TEST_F(SourceBinFactoryTests, sourceNull) {
    using namespace YAML;

    auto root = Node(NodeType::Map);
    root["yz-source"] = Node(NodeType::Null);

    yaml::dumpNode(root);

    auto bin = factory.createBin(root);
    ASSERT_EQ(nullptr, bin);
}

TEST_F(SourceBinFactoryTests, fileSrcNull) {
    using namespace YAML;

    auto root = Node(NodeType::Map);
    root["yz-source"] = Node(NodeType::Map);
    root["yz-source"]["filesrc"] = Node(NodeType::Null);

    yaml::dumpNode(root);

    auto bin = factory.createBin(root);
    ASSERT_EQ(nullptr, bin);
}

TEST_F(SourceBinFactoryTests, fileSrcScalar) {
    using namespace YAML;

    auto root = Node(NodeType::Map);
    root["yz-source"] = Node(NodeType::Map);
    root["yz-source"]["filesrc"] = 10;

    yaml::dumpNode(root);

    auto bin = factory.createBin(root);
    ASSERT_EQ(nullptr, bin);
}

TEST_F(SourceBinFactoryTests, fileSrcSequence) {
    using namespace YAML;

    auto root = Node(NodeType::Map);
    root["yz-source"] = Node(NodeType::Map);
    root["yz-source"]["filesrc"] = Node(NodeType::Sequence);

    yaml::dumpNode(root);

    auto bin = factory.createBin(root);
    ASSERT_EQ(nullptr, bin);
}

TEST_F(SourceBinFactoryTests, fileSrcNoLocation) {
    using namespace YAML;

    auto root = Node(NodeType::Map);
    root["yz-source"] = Node(NodeType::Map);
    root["yz-source"]["filesrc"] = Node(NodeType::Map);
    root["yz-source"]["filesrc"]["loc"] = Node(NodeType::Null);

    yaml::dumpNode(root);

    auto bin = factory.createBin(root);
    ASSERT_EQ(nullptr, bin);
}

TEST_F(SourceBinFactoryTests, fileSrcLocationMap) {
    using namespace YAML;

    auto root = Node(NodeType::Map);
    root["yz-source"] = Node(NodeType::Map);
    root["yz-source"]["filesrc"] = Node(NodeType::Null);
    root["yz-source"]["filesrc"]["location"] = Node(NodeType::Map);

    yaml::dumpNode(root);

    auto bin = factory.createBin(root);
    ASSERT_EQ(nullptr, bin);
}

TEST_F(SourceBinFactoryTests, fileSrcLocationSequence) {
    using namespace YAML;

    auto root = Node(NodeType::Map);
    root["yz-source"] = Node(NodeType::Map);
    root["yz-source"]["filesrc"] = Node(NodeType::Null);
    root["yz-source"]["filesrc"]["location"] = Node(NodeType::Sequence);

    yaml::dumpNode(root);

    auto bin = factory.createBin(root);
    ASSERT_EQ(nullptr, bin);
}

TEST_F(SourceBinFactoryTests, validConfig) {
    using namespace YAML;

    auto root = Node(NodeType::Map);
    root["yz-source"] = Node(NodeType::Map);
    root["filesrc"] = Node(NodeType::Null);
    root["filesrc"]["location"] = "/path";

    yaml::dumpNode(root);

    auto bin = factory.createBin(root);
    ASSERT_NE(nullptr, bin);

    gst_object_unref(bin);
}
