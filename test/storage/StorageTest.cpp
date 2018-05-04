#include "gtest/gtest.h"
#include <iostream>
#include <set>
#include <vector>
#include <utility>

#include <storage/MapBasedGlobalLockImpl.h>
#include <afina/execute/Get.h>
#include <afina/execute/Set.h>
#include <afina/execute/Add.h>
#include <afina/execute/AppendPrepend.h>
#include <afina/execute/Delete.h>

using namespace Afina::Backend;
using namespace Afina::Execute;
using namespace std;

const size_t BigTestSize = 10000;

const size_t LRUTestStringSize = 10;

const size_t OverheadTestSize = 1000;
const size_t OverheadSize = 100;

//exists: is existing of elements assumed
void CheckKeyValuePair(MapBasedGlobalLockImpl& storage, const std::string& key, const std::string& value, bool exists = true) {
    std::string res;
    bool result = storage.Get(key, res);
    if (exists)
    {
        EXPECT_TRUE(value == res);
    }
    else
    {
	EXPECT_FALSE(result || (res != "")); //res != "" : check if no changes
    }
}

TEST(StorageTest, PutGet) {
    MapBasedGlobalLockImpl storage;

    storage.Put("KEY1", "val1");
    storage.Put("KEY2", "val2");

    CheckKeyValuePair(storage, "KEY1", "val1");
    CheckKeyValuePair(storage, "KEY2", "val2");
}

TEST(StorageTest, PutOverwrite) {
    MapBasedGlobalLockImpl storage;

    storage.Put("KEY1", "val1");
    storage.Put("KEY1", "val2");
    
    CheckKeyValuePair(storage, "KEY1", "val2");
}

TEST(StorageTest, PutIfAbsent) {
    MapBasedGlobalLockImpl storage;

    storage.Put("KEY1", "val1");
    storage.PutIfAbsent("KEY1", "val2");

    CheckKeyValuePair(storage, "KEY1", "val1");
}

TEST(StorageTest, DeleteTest) {
    MapBasedGlobalLockImpl storage;

    storage.Put("KEY1", "val1");
    storage.Put("KEY2", "val2");
 
    storage.Delete("KEY1");
    CheckKeyValuePair(storage, "KEY1", "val1", false);
    CheckKeyValuePair(storage, "KEY2", "val2");
}

std::string FormatNumber(int val, int width) {
	std::string result = std::to_string(val);
	result.resize(width, 'x');
	return result;
}

//key = num, value = max_num-num, formatted for width
std::pair<std::string, std::string> GetKeyValuePair (int num, int max_num, int width) {
	return std::make_pair(FormatNumber(num, width), FormatNumber(max_num-num, width));
}

void PutCount(MapBasedGlobalLockImpl& storage, int count, int width) {
	for(long i=0; i<count; ++i)
    	{
		auto key_val = GetKeyValuePair(i, count, width);
	        storage.Put(key_val.first, key_val.second);
	}
	
}

//start - included, end - excluded
//exists: is existing of elements assumed
void CheckRange(MapBasedGlobalLockImpl& storage, int start, int end, int max_size, int width, bool exists = true) {
	for (long i = start; i < end; ++i)
	{
		auto key_val = GetKeyValuePair(i, max_size, width);
		CheckKeyValuePair(storage, key_val.first, key_val.second, exists);	
	}
}

TEST(StorageTest, LRUTest) {
    MapBasedGlobalLockImpl storage(LRUTestStringSize*2*2); //2 pairs (key+value)

    PutCount(storage, 2, LRUTestStringSize); //put two pairs. Keys = 00..0, 01..0 (LRUTestStringSize). So, shoulg be purged 00..0

    auto key_val = GetKeyValuePair(0, 2, LRUTestStringSize); //Create pair for key 00..0
    CheckKeyValuePair(storage, key_val.first, key_val.second); //Now should be purged 01..0 (we've used 00..0) 

    storage.Put("NewKey", "NewVal");

    CheckKeyValuePair(storage, key_val.first, key_val.second); 
    CheckKeyValuePair(storage, "NewKey", "NewVal");

    key_val = GetKeyValuePair(1, 2, LRUTestStringSize); //Should be purged
    CheckKeyValuePair(storage, key_val.first, key_val.second, false);
}


TEST(StorageTest, BigTest) {
    MapBasedGlobalLockImpl storage;
    int len = BigTestSize/10+1;

    PutCount(storage, BigTestSize, len);
    
    CheckRange(storage, 0, BigTestSize, BigTestSize, len);
}

TEST(StorageTest, OverheadTest) {
    int sum_size = OverheadTestSize+OverheadSize;
    int len = sum_size/10+1; //Max number len
    MapBasedGlobalLockImpl storage(OverheadTestSize*len*2); //*2: key+value
    
    PutCount(storage, sum_size, len);
    CheckRange(storage, OverheadSize, sum_size, sum_size, len);
    CheckRange(storage, 0, OverheadSize, sum_size, false, len);
}
