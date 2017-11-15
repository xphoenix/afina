#include "gtest/gtest.h"
#include <afina/Executor.h>
#include <mutex>
#include <condition_variable>

using namespace Afina;
using namespace std;

TEST(ExecutorTest, ConstructDestroy) {
	{
		Executor executor{"",1};
		ASSERT_TRUE(1);
	}
	ASSERT_TRUE(1);
}

TEST(ExecutorTest, SimpleTask) {
	Executor ex{"", 3};
	uint32_t c = 0x55555555;
	std::mutex m;
	std::condition_variable cv;
	for (int i = 0; i < 5; i++)
		ex.Execute([&c,&m,&cv](){
			std::unique_lock<std::mutex> lock{m};
			c += 0x11111111;
			if (c == 0xAAAAAAAA) {
				lock.unlock();
				cv.notify_one();
			}
		});
	std::unique_lock<std::mutex> lock{m};
	cv.wait(lock,[&c](){ return c == 0xAAAAAAAA; });
	ex.Stop(true);
	ASSERT_EQ(c, 0xAAAAAAAA);
}
