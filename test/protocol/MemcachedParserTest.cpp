#include <gtest/gtest.h>

#include <memory>
#include <string>

#include <afina/execute/Add.h>
#include <afina/execute/Get.h>
#include <afina/execute/Set.h>
#include <afina/execute/Stats.h>

#include <protocol/Parser.h>

using namespace Afina;

// TODO: Negative test on errors
// TODO: Separate tests for integers overflow
// TODO: Special test that consumed only increased

// Verify simple set command passed in a single string
TEST(MemcachedParserTest, SimpleSet) {
    Protocol::Parser parser;

    size_t consumed = 0;
    bool cmd_avail = parser.Parse("set foo 0 0 6\r\nfooval\r\n", consumed);
    ASSERT_TRUE(cmd_avail);
    ASSERT_EQ(15, consumed);
    ASSERT_EQ("set", parser.Name());

    uint32_t value_size;
    std::unique_ptr<Execute::Command> cmd = parser.Build(value_size);
    ASSERT_FALSE(cmd == nullptr);
    ASSERT_EQ(6, value_size);

    Execute::Set *tmp = reinterpret_cast<Execute::Set *>(cmd.get());
    ASSERT_EQ("foo", tmp->key());
    ASSERT_EQ(0, tmp->flags());
    ASSERT_EQ(0, tmp->expire());
}

// Verify simple add command passed in a single string
TEST(MemcachedParserTest, SimpleAdd) {
    Protocol::Parser parser;

    size_t consumed = 0;
    bool cmd_avail = parser.Parse("add bar 10 -1 60\r\nbarval\r\n", consumed);
    ASSERT_TRUE(cmd_avail);
    ASSERT_EQ(18, consumed);
    ASSERT_EQ("add", parser.Name());

    uint32_t value_size;
    std::unique_ptr<Execute::Command> cmd = parser.Build(value_size);
    ASSERT_FALSE(cmd == nullptr);
    ASSERT_EQ(60, value_size);

    Execute::Add *tmp = reinterpret_cast<Execute::Add *>(cmd.get());
    ASSERT_EQ("bar", tmp->key());
    ASSERT_EQ(10, tmp->flags());
    ASSERT_EQ(-1, tmp->expire());
}

// Verify simple get command passed in a single string
TEST(MemcachedParserTest, SimpleGet) {
    Protocol::Parser parser;

    size_t consumed = 0;
    bool cmd_avail = parser.Parse("get ke key2 super_long_key\r\n", consumed);
    ASSERT_TRUE(cmd_avail);
    ASSERT_EQ(28, consumed);
    ASSERT_EQ("get", parser.Name());

    uint32_t value_size;
    std::unique_ptr<Execute::Command> cmd = parser.Build(value_size);
    ASSERT_FALSE(cmd == nullptr);
    ASSERT_EQ(0, value_size);

    Execute::Get *tmp = reinterpret_cast<Execute::Get *>(cmd.get());
    std::vector<std::string> keys = tmp->keys();
    ASSERT_EQ(3, keys.size());
    ASSERT_EQ("ke", keys[0]);
    ASSERT_EQ("key2", keys[1]);
    ASSERT_EQ("super_long_key", keys[2]);
}

TEST(MemcachedParserTest, Stats) {
    Protocol::Parser parser;

    size_t consumed = 0;
    bool cmd_avail = parser.Parse("stats\r\n", consumed);
    ASSERT_TRUE(cmd_avail);
    ASSERT_EQ(7, consumed);
    ASSERT_EQ("stats", parser.Name());

    uint32_t value_size;
    std::unique_ptr<Execute::Command> cmd = parser.Build(value_size);
    ASSERT_FALSE(cmd == nullptr);
    ASSERT_EQ(0, value_size);

    Execute::Stats *tmp = reinterpret_cast<Execute::Stats *>(cmd.get());
	ASSERT_FALSE(tmp == nullptr);
}
