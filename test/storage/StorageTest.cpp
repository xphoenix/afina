#include "gtest/gtest.h"
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

#include <afina/execute/Add.h>
#include <afina/execute/Append.h>
#include <afina/execute/Delete.h>
#include <afina/execute/Get.h>
#include <afina/execute/Set.h>

#include "storage/SimpleLRU.h"

using namespace Afina::Backend;
using namespace Afina::Execute;
using namespace std;




TEST(StorageTest, PutGet) {
    SimpleLRU storage;

    EXPECT_TRUE(storage.Put("KEY1", "val1"));
    EXPECT_TRUE(storage.Put("KEY2", "val2"));

    std::string value;
    EXPECT_TRUE(storage.Get("KEY1", value));
    EXPECT_TRUE(value == "val1");

    EXPECT_TRUE(storage.Get("KEY2", value));
    EXPECT_TRUE(value == "val2");
}

TEST(StorageTest, PutOverwrite) {
    SimpleLRU storage;

    EXPECT_TRUE(storage.Put("KEY1", "val1"));
    EXPECT_TRUE(storage.Put("KEY1", "val2"));

    std::string value;
    EXPECT_TRUE(storage.Get("KEY1", value));
    EXPECT_TRUE(value == "val2");
}

TEST(StorageTest, PutIfAbsent) {
    SimpleLRU storage;

    EXPECT_TRUE(storage.PutIfAbsent("KEY1", "val1"));

    EXPECT_FALSE(storage.PutIfAbsent("KEY1", "val2"));

    std::string value;
    EXPECT_TRUE(storage.Get("KEY1", value));
    EXPECT_TRUE(value == "val1");
}

TEST(StorageTest, PutSetGet) {
    SimpleLRU storage;

    EXPECT_TRUE(storage.Put("KEY1", "val1"));
    EXPECT_TRUE(storage.Set("KEY1", "val2"));

    EXPECT_FALSE(storage.Set("KEY2", "val2"));

    std::string value;
    EXPECT_TRUE(storage.Get("KEY1", value));
    EXPECT_TRUE(value == "val2");
}

TEST(StorageTest, SetIfAbsent) {
    SimpleLRU storage;

    EXPECT_TRUE(storage.Put("KEY1", "val1"));

    std::string value;
    EXPECT_FALSE(storage.Set("KEY2", "val2"));
    EXPECT_TRUE(storage.Get("KEY1", value));
    EXPECT_TRUE(value == "val1");
}

TEST(StorageTest, PutDeleteGet) {
    SimpleLRU storage;

    EXPECT_TRUE(storage.Put("KEY1", "val1"));
    EXPECT_TRUE(storage.Put("KEY2", "val2"));

    EXPECT_TRUE(storage.Delete("KEY1"));

    std::string value;
    EXPECT_FALSE(storage.Get("KEY1", value));
    EXPECT_TRUE(storage.Get("KEY2", value));
    EXPECT_TRUE(value == "val2");
}


TEST(StorageTest, GetIfAbsent)
{
    SimpleLRU storage;


    std::string value;
    EXPECT_FALSE(storage.Get("KEY1", value));

    EXPECT_FALSE(storage.Get("KEY2", value));

    EXPECT_FALSE(storage.Get("KEY3", value));
}

TEST(StorageTest, DeleteIfAbsent)
{
    SimpleLRU storage;
    EXPECT_FALSE(storage.Delete("KEY1"));

    EXPECT_FALSE(storage.Delete("KEY2"));

    EXPECT_FALSE(storage.Delete("KEY3"));
}

TEST(StorageTest, DeleteHeadAndTailNode)
{
    SimpleLRU storage;

    EXPECT_TRUE(storage.Put("KEY1", "val1"));
    EXPECT_TRUE(storage.Put("KEY2", "val2"));
    EXPECT_TRUE(storage.Put("KEY3", "val3"));
    EXPECT_TRUE(storage.Put("KEY4", "val4"));


    EXPECT_TRUE(storage.Set("KEY2", "val22"));
    EXPECT_TRUE(storage.Set("KEY3", "val23"));
    EXPECT_TRUE(storage.Set("KEY1", "val21"));
    EXPECT_TRUE(storage.Set("KEY1", "val31"));
    EXPECT_TRUE(storage.Set("KEY1", "val41"));
    // After that, KEY1 should be first in the rating.
    // And KEY4 should be the last.
    EXPECT_TRUE(storage.Delete("KEY4"));
    EXPECT_TRUE(storage.Delete("KEY1"));
}

std::string pad_space(const std::string &s, size_t length) {
    std::string result = s;
    result.resize(length, ' ');
    return result;
}

TEST(StorageTest, BigTest) {
    const size_t length = 20;
    SimpleLRU storage(2 * 100000 * length);

    for (long i = 0; i < 100000; ++i) {
        auto key = pad_space("Key " + std::to_string(i), length);
        auto val = pad_space("Val " + std::to_string(i), length);
        EXPECT_TRUE(storage.Put(key, val));
    }

    for (long i = 99999; i >= 0; --i) {
        auto key = pad_space("Key " + std::to_string(i), length);
        auto val = pad_space("Val " + std::to_string(i), length);

        std::string res;
        EXPECT_TRUE(storage.Get(key, res));

        EXPECT_TRUE(val == res);
    }
}

TEST(StorageTest, MaxTest) {
    const size_t length = 20;
    SimpleLRU storage(2 * 1000 * length);

    std::stringstream ss;

    for (long i = 0; i < 1100; ++i) {
        auto key = pad_space("Key " + std::to_string(i), length);
        auto val = pad_space("Val " + std::to_string(i), length);
        EXPECT_TRUE(storage.Put(key, val));
    }

    for (long i = 100; i < 1100; ++i) {
        auto key = pad_space("Key " + std::to_string(i), length);
        auto val = pad_space("Val " + std::to_string(i), length);

        std::string res;
        EXPECT_TRUE(storage.Get(key, res));

        EXPECT_TRUE(val == res);
    }

    for (long i = 0; i < 100; ++i) {
        auto key = pad_space("Key " + std::to_string(i), length);

        std::string res;
        EXPECT_FALSE(storage.Get(key, res));
    }
}

TEST(StorageTest, OverLimit) {
    SimpleLRU storage(10);

    auto key = pad_space("Key " + std::to_string(0), 5);
    auto val = pad_space("Val " + std::to_string(0), 6);
    EXPECT_FALSE(storage.Put(key, val));

    val.resize(5, ' ');
    EXPECT_TRUE(storage.Put(key, val));
    // It is necessary to verify the correct deletion of a single value in the container during overflow
    EXPECT_TRUE(storage.Set(key, val));

    val.resize(6, ' ');
    EXPECT_FALSE(storage.Set(key, val));

    key = pad_space("Key2" + std::to_string(0), 5);
    EXPECT_FALSE(storage.Put(key, val));
    EXPECT_FALSE(storage.PutIfAbsent(key, val));
}
