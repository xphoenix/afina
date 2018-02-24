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

TEST(StorageTest, PutIfAbsent) {
    MapBasedGlobalLockImpl storage;

    storage.Put("KEY1", "val1");
    storage.PutIfAbsent("KEY1", "val2");

    std::string value;
    EXPECT_TRUE(storage.Get("KEY1", value));
    EXPECT_TRUE(value == "val1");
}

TEST(StorageTest, BigTest) {
	/*
	Specify min key number in storage after insertion of 100000 key-value pairs
	This number N can be found as a maximum solution of the equation:
	[Sum 2 * (NumberOfDigits(k) + 3) where k from N to 100000] <= 100000
	*/
	constexpr long min_value = 93750;

    MapBasedGlobalLockImpl storage(100000);

    std::stringstream ss;

    for(long i=0; i<100000; ++i)
    {
        ss << "Key" << i;
        std::string key = ss.str();
        ss.str("");
        ss << "Val" << i;
        std::string val = ss.str();
        ss.str("");
        storage.Put(key, val);
    }
    
    for(long i=99999; i>= min_value; --i)
    {
        ss << "Key" << i;
        std::string key = ss.str();
        ss.str("");
        ss << "Val" << i;
        std::string val = ss.str();
        ss.str("");
        
        std::string res;
        EXPECT_TRUE(storage.Get(key, res));

        EXPECT_TRUE(val == res);
    }
	
	for (long i = min_value - 1; i >= 0; --i)
	{
		ss << "Key" << i;
		std::string key = ss.str();
		ss.str("");
		ss << "Val" << i;
		std::string val = ss.str();
		ss.str("");

		std::string res;
		EXPECT_FALSE(storage.Get(key, res));

		EXPECT_FALSE(val == res);
	}

}

TEST(StorageTest, MaxTest) {
/*
	Specify min key number in storage after insertion of 1100 key-value pairs
	This number N can be found as a maximum solution of the equation:
		[Sum 2 * (NumberOfDigits(k) + 3) where k from N to 1100] <= 1000
*/
	constexpr long min_value = 1030;
    MapBasedGlobalLockImpl storage(1000);

    std::stringstream ss;

    for(long i=0; i<1100; ++i)
    {
        ss << "Key" << i;
        std::string key = ss.str();
        ss.str("");
        ss << "Val" << i;
        std::string val = ss.str();
        ss.str("");
        storage.Put(key, val);
    }
    
    for(long i = min_value; i<1100; ++i)
    {
        ss << "Key" << i;
        std::string key = ss.str();
        ss.str("");
        ss << "Val" << i;
        std::string val = ss.str();
        ss.str("");
        
        std::string res;
        storage.Get(key, res);

        EXPECT_TRUE(val == res);
    }
    
    for(long i=0; i < min_value - 1; ++i)
    {
        ss << "Key" << i;
        std::string key = ss.str();
        ss.str("");
        
        std::string res;
        EXPECT_FALSE(storage.Get(key, res));
    }
}
