#include "gtest/gtest.h"
#include <iostream>
#include <set>
#include <vector>

#include <storage/MapBasedGlobalLockImpl.h>
#include <afina/execute/Get.h>
#include <afina/execute/Set.h>
#include <afina/execute/Add.h>
#include <afina/execute/Append.h>
#include <afina/execute/Delete.h>

using namespace Afina::Backend;
using namespace Afina::Execute;
using namespace std;

TEST(StorageTest, PutGet) {
    MapBasedGlobalLockImpl storage;

    storage.Put("KEY1", "val1");
    storage.Put("KEY2", "val2");

    std::string value;
    EXPECT_TRUE(storage.Get("KEY1", value));
    EXPECT_TRUE(value == "val1");

    EXPECT_TRUE(storage.Get("KEY2", value));
    EXPECT_TRUE(value == "val2");
}

TEST(StorageTest, PutOverwrite) {
    MapBasedGlobalLockImpl storage;

    storage.Put("KEY1", "val1");
    storage.Put("KEY1", "val2");

    std::string value;
    EXPECT_TRUE(storage.Get("KEY1", value));
    EXPECT_TRUE(value == "val2");
}

TEST(StorageTest, Set) {
    MapBasedGlobalLockImpl storage;

    storage.Put("KEY1", "val1");

    std::string value;
    EXPECT_TRUE(storage.Set("KEY1", "new_val1"));
    EXPECT_TRUE(storage.Get("KEY1", value));
    EXPECT_TRUE(value == "new_val1");

    EXPECT_FALSE(storage.Set("KEY3", "val3"));
}

TEST(StorageTest, PutIfAbsent) {
    MapBasedGlobalLockImpl storage;

    storage.Put("KEY1", "val1");
    storage.PutIfAbsent("KEY1", "val2");

    std::string value;
    EXPECT_TRUE(storage.Get("KEY1", value));
    EXPECT_TRUE(value == "val1");
}

TEST(StorageTest, SmallMemory_no_1) {
    MapBasedGlobalLockImpl storage(2);
    std::string value;

    storage.Put("k", "v");

    EXPECT_TRUE(storage.Get("k", value));
    EXPECT_TRUE(value == "v");

    storage.Put("q", "v");
    EXPECT_FALSE(storage.Get("k", value));
    EXPECT_TRUE(storage.Get("q", value));
    EXPECT_TRUE(value == "v");
}

TEST(StorageTest, SmallMemory_no_2) {
    MapBasedGlobalLockImpl storage(10);
    std::string value;

    EXPECT_TRUE(storage.Put("ke1", "va1"));

    EXPECT_TRUE(storage.Get("ke1", value));
    EXPECT_TRUE(value == "va1");

    EXPECT_TRUE(storage.Put("k2", "v2"));

    EXPECT_TRUE(storage.Get("ke1", value));
    EXPECT_TRUE(value == "va1");

    EXPECT_TRUE(storage.Get("k2", value));
    EXPECT_TRUE(value == "v2");

    // Can not put, over size
    EXPECT_FALSE(storage.Put("new_long_key", "long_value"));

    EXPECT_TRUE(storage.Get("ke1", value));
    EXPECT_TRUE(value == "va1");

    EXPECT_TRUE(storage.Get("k2", value));
    EXPECT_TRUE(value == "v2");

    EXPECT_TRUE(storage.Put("k3", "v3"));

    EXPECT_FALSE(storage.Get("ke1", value));

    EXPECT_TRUE(storage.Get("k2", value));
    EXPECT_TRUE(value == "v2");

    EXPECT_TRUE(storage.Get("k3", value));
    EXPECT_TRUE(value == "v3");

}

//TEST(StorageTest, BigTest) {
//    MapBasedGlobalLockImpl storage(100000);
//
//    std::stringstream ss;
//
//    for(long i = 0; i < 100000; ++i)
//    {
//        ss << "Key" << i;
//        std::string key = ss.str();
//        ss.str("");
//        ss << "Val" << i;
//        std::string val = ss.str();
//        ss.str("");
//        storage.Put(key, val);
//    }
//
//    for(long i=99999; i>=0; --i)
//    {
//        ss << "Key" << i;
//        std::string key = ss.str();
//        ss.str("");
//        ss << "Val" << i;
//        std::string val = ss.str();
//        ss.str("");
//
//        std::string res;
//        storage.Get(key, res);
//
//        EXPECT_TRUE(val == res);
//    }
//
//}



//TEST(StorageTest, MaxTest) {
//    MapBasedGlobalLockImpl storage(1000);
//
//    std::stringstream ss;
//
//    for(long i=0; i<1100; ++i)
//    {
//        ss << "Key" << i;
//        std::string key = ss.str();
//        ss.str("");
//        ss << "Val" << i;
//        std::string val = ss.str();
//        ss.str("");
//        storage.Put(key, val);
//    }
//
//    for(long i=100; i<1100; ++i)
//    {
//        ss << "Key" << i;
//        std::string key = ss.str();
//        ss.str("");
//        ss << "Val" << i;
//        std::string val = ss.str();
//        ss.str("");
//
//        std::string res;
//        storage.Get(key, res);
//
//        EXPECT_TRUE(val == res);
//    }
//
//    for(long i=0; i<100; ++i)
//    {
//        ss << "Key" << i;
//        std::string key = ss.str();
//        ss.str("");
//
//        std::string res;
//        EXPECT_FALSE(storage.Get(key, res));
//    }
//}
