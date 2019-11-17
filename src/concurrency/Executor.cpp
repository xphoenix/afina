#include <chrono>

#include <iostream>

#include "afina/network/Server.h"
#include <afina/concurrency/Executor.h>

namespace Afina {
namespace Concurrency {

void perform(Executor *executor) {
    bool running = true;
    while (running) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(executor->_mutex);
            // std::cv_status::timeout
            if (executor->empty_condition.wait_for(lock, std::chrono::milliseconds(executor->_idle_time),
                                                   [executor] { return !executor->tasks.empty(); })) {
                task = executor->tasks.front();
                executor->tasks.pop_front();
                executor->_free_threads--;
            } else if ((executor->state != Executor::State::kRun) ||
                       (executor->threads.size() > executor->_low_watermark)) {
                break;
            } else {
                continue;
            }
        }
        task();
        {
            std::unique_lock<std::mutex> lock(executor->_mutex);
            executor->_free_threads++;
            running = (executor->state != Executor::State::kStopped);
        }
    }

    {
        std::unique_lock<std::mutex> lock(executor->_mutex);
        executor->_free_threads--;
        auto th_id = std::this_thread::get_id();
        for (auto it = executor->threads.begin(); it != executor->threads.end(); it++) {
            if (it->get_id() == th_id) {
                if (it->joinable()) {
                    it->detach();
                }
                executor->threads.erase(it);
                break;
            }
        }
        if (executor->threads.empty() && (executor->state == Executor::State::kStopping)) {
            executor->state = Executor::State::kStopped;
            executor->empty_condition.notify_all();
        }
    }
}

Executor::Executor(uint32_t low_watermark, uint32_t hight_watermark, uint32_t max_queue_size, uint32_t idle_time)
    : _low_watermark(low_watermark), _hight_watermark(hight_watermark), _max_queue_size(max_queue_size),
      _idle_time(idle_time), _free_threads(low_watermark), state(State::kRun) {
    for (size_t i = 0; i < _low_watermark; i++) {
        threads.emplace_back(&perform, this);
    }
}

Executor::~Executor() {}

void Executor::Stop(bool await) {
    {
        std::unique_lock<std::mutex> locker(_mutex);
        if (state == Executor::State::kRun) {
            state = Executor::State::kStopping;
        }
    }
    // empty_condition.notify_all();
    {
        std::unique_lock<std::mutex> locker(_mutex);
        if (await) {
            while (state != Executor::State::kStopped) {
                empty_condition.wait(locker);
            }
        } else if (threads.empty()) {
            state = Executor::State::kStopped;
        }
    }
}

} // namespace Concurrency
} // namespace Afina
