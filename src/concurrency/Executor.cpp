#include <chrono>

#include<iostream>

#include <afina/concurrency/Executor.h>
#include "afina/network/Server.h"

namespace Afina {
namespace Concurrency {

void perform(Executor *executor)
{
    bool running = true;
    while(running)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(executor->mutex);
            while(executor->tasks.empty() && executor->state == Executor::State::kRun)
            {
                if (executor->empty_condition.wait_for(lock,
                  std::chrono::milliseconds(executor->_idle_time))  == std :: cv_status :: timeout)
                {
                    if (executor->threads.size() >= executor->_low_watermark)
                    {
                        running = false;
                        auto th_id = std::this_thread::get_id();
                        for (auto it = executor->threads.begin(); it != executor->threads.end(); it++)
                        {
                            if (it->get_id() == th_id)
                            {
                                executor->_free_threads--;
                                if(it->joinable())
                                {
                                    it->detach();
                                }
                                executor->threads.erase(it);
                                break;
                            }
                        }
                    }
                }
            }
            running = executor->tasks.empty() || (executor->state != Executor::State::kStopped);
            if (!running)
            {
                executor->empty_condition.notify_all();
                continue;
            }
            task = executor->tasks.front();
            executor->tasks.pop_front();
            executor->_free_threads--;
        }
        task();
        {
            std::unique_lock<std::mutex> lock(executor->mutex);
            executor->_free_threads++;
            running = (executor->state != Executor::State::kStopped);
            if ((executor->state == Executor::State::kStopped) &&
                (executor->_free_threads == executor->threads.size()))
            {
                executor->join_condition.notify_all();
            }
        }
    }
}

Executor::Executor(uint32_t low_watermark, uint32_t hight_watermark, uint32_t max_queue_size, uint32_t idle_time):
_low_watermark(low_watermark),
_hight_watermark(hight_watermark),
_max_queue_size(max_queue_size),
_idle_time(idle_time),
_free_threads(low_watermark),
state(State::kRun)
{
    for(size_t i = 0; i < _low_watermark; i++)
    {
        threads.emplace_back(&perform, this);
    }
}

Executor::~Executor(){}

void Executor::Stop(bool await)
{
    std::cout << "Executor::Stop" << std::endl;
    {
        std::unique_lock<std::mutex> locker(mutex);
        state = Executor::State::kStopping;
        // std::cout << "lock1" << std::endl;
        while (!tasks.empty())
        {
            empty_condition.wait(locker);
        }
        // std::cout << "after cv_wait" << std::endl;
        state = Executor::State::kStopped;
    }
    {
        std::unique_lock<std::mutex> locker(mutex);
        // std::cout << "lock2" << std::endl;
        while (_free_threads != threads.size())
        {
            join_condition.wait(locker);
        }
    // std::cout << "after j_c" << std::endl;
        if (await)
        {
            for (auto &thread: threads)
            {
                if (thread.joinable())
                {
                    thread.join();
                }
            }
                // std::cout << "after if(await)" << std::endl;
        }
        else
        {
            for (auto &thread: threads)
            {
                if (thread.joinable())
                {
                    thread.detach();
                }
            }
        }
    }
}


}
} // namespace Afina
