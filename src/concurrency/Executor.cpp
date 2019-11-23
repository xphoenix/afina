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
            if (executor->empty_condition.wait_for(lock, std::chrono::milliseconds(executor->_idle_time), [executor] {
                    return !executor->tasks.empty() && (executor->state == Executor::State::kRun);
                })) {
                task = executor->tasks.front();
                executor->tasks.pop_front();
                executor->_free_threads--;
            } else if ((executor->state != Executor::State::kRun) ||
                       (executor->_threads_count > executor->_low_watermark)) {
                break;
            } else {
                continue;
            }
        }
        try {
            task();
        } catch (...) {
            std::cerr << "Failed to execute task" << std::endl;
        }
        {
            std::unique_lock<std::mutex> lock(executor->_mutex);
            executor->_free_threads++;
            running = (executor->state != Executor::State::kStopped);
        }
    }

    {
        std::unique_lock<std::mutex> lock(executor->_mutex);
        executor->_free_threads--;
        executor->_threads_count--;
        if ((executor->_threads_count == 0) && (executor->state == Executor::State::kStopping)) {
            executor->state = Executor::State::kStopped;
            executor->stop_condition.notify_all();
        }
    }
}

Executor::Executor(uint32_t low_watermark, uint32_t hight_watermark, uint32_t max_queue_size, uint32_t idle_time)
    : _low_watermark(low_watermark), _hight_watermark(hight_watermark), _max_queue_size(max_queue_size),
      _idle_time(idle_time), _free_threads(low_watermark), _threads_count(low_watermark), state(State::kRun) {
    for (size_t i = 0; i < _low_watermark; i++) {
        std::thread(&perform, this).detach();
    }
}

Executor::~Executor() {}

void Executor::Stop(bool await) {
    {
        std::unique_lock<std::mutex> locker(_mutex);
        if (state == Executor::State::kRun) {
            state = Executor::State::kStopping;
        }
        if (_threads_count == 0) {
            state = Executor::State::kStopped;
        }
    }

    if (await) {
        std::unique_lock<std::mutex> locker(_mutex);
        while (state != Executor::State::kStopped) {
            stop_condition.wait(locker);
        }
    }
}

} // namespace Concurrency
} // namespace Afina
