#include <chrono>

#include <iostream>

#include "afina/network/Server.h"
#include <afina/concurrency/Executor.h>

namespace Afina {
namespace Concurrency {

void perform(Executor *executor) {
    bool running = true;
    bool kill = false;
    while (running) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(executor->mutex);
            if (executor->empty_condition.wait_for(lock, std::chrono::milliseconds(executor->_idle_time)) ==
                std::cv_status::timeout) {
                if (executor->threads.size() > executor->_low_watermark) {
                    kill = true;
                    break;
                }
            }
            if (executor->tasks.empty() && executor->state == Executor::State::kRun) {
                continue;
            } else if (executor->tasks.empty() && executor->state == Executor::State::kStopping) {
                executor->empty_condition.notify_all();
                break;
            } else if (executor->state == Executor::State::kStopped) {
                break;
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
        }
    }

    {
        std::unique_lock<std::mutex> lock(executor->mutex);
        executor->_free_threads--;
        executor->_threads_to_stop++;
        if (kill) {
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
        }

        if ((executor->state == Executor::State::kStopped) &&
            (executor->_threads_to_stop == executor->threads.size())) {
            executor->join_condition.notify_all();
        }
    }
}

Executor::Executor(uint32_t low_watermark, uint32_t hight_watermark, uint32_t max_queue_size, uint32_t idle_time)
    : _low_watermark(low_watermark), _hight_watermark(hight_watermark), _max_queue_size(max_queue_size),
      _idle_time(idle_time), _free_threads(low_watermark), _threads_to_stop(0), state(State::kRun) {
    for (size_t i = 0; i < _low_watermark; i++) {
        threads.emplace_back(&perform, this);
    }
}

Executor::~Executor() {}

void Executor::Stop(bool await) {
    {
        std::unique_lock<std::mutex> locker(mutex);
        state = Executor::State::kStopping;
        empty_condition.notify_all();
    }

    {
        std::unique_lock<std::mutex> locker(mutex);
        while (!tasks.empty()) {
            empty_condition.wait(locker);
        }
        state = Executor::State::kStopped;
        empty_condition.notify_all();
    }

    {
        std::unique_lock<std::mutex> locker(mutex);
        while (_threads_to_stop != threads.size()) {
            join_condition.wait(locker);
        }
        if (await) {
            for (auto &thread : threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
        } else {
            for (auto &thread : threads) {
                if (thread.joinable()) {
                    thread.detach();
                }
            }
        }
    }
}

} // namespace Concurrency
} // namespace Afina
