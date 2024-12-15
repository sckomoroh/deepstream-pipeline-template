#include <gtest/gtest.h>

#include "common/string-tools.h"

using namespace yz::common;

using std::string;

TEST(StringTools, hasPrefix) {
    string source = "my-element-name";
    ASSERT_TRUE(str::hasPrefix(source, "my-element"));
    ASSERT_FALSE(str::hasPrefix(source, "my-element1"));
    ASSERT_FALSE(str::hasPrefix(source, "element"));
}

TEST(StringTools, hasSuffix) {
    string source = "my-element-name";
    ASSERT_TRUE(str::hasSuffix(source, "name"));
    ASSERT_FALSE(str::hasSuffix(source, "name1"));
    ASSERT_FALSE(str::hasSuffix(source, "element"));
}

TEST(StringTools, suffixIndex) {
    auto ind = str::getSuffixIndex("element-1");
    ASSERT_TRUE(ind.has_value());
    ASSERT_EQ(1, ind.value());

    ind = str::getSuffixIndex("element-01");
    ASSERT_TRUE(ind.has_value());
    ASSERT_EQ(1, ind.value());

    ind = str::getSuffixIndex("element-0-23");
    ASSERT_TRUE(ind.has_value());
    ASSERT_EQ(23, ind.value());

    ind = str::getSuffixIndex("element-a");
    ASSERT_FALSE(ind.has_value());
}