#include <afina/concurrency/Executor.h>

namespace Afina {
namespace Concurrency {

Executor::Executor(const std::string &name, int max_queue_size,
                        size_t high_watermark, size_t low_watermark,
                        size_t wait_time) :
                                        name(name),
                                        max_queue_size(max_queue_size),
                                        low_watermark(low_watermark),
                                        high_watermark(high_watermark),
                                        wait_time(wait_time) {

    std::unique_lock<std::mutex> lock(mutex);

    for (size_t i = 0; i < low_watermark; ++i) {
        // std::thread thread = std::thread(perform, this);
        std::thread thread = std::thread([this] { return perform(this); });
        threads.emplace(thread.get_id(), std::move(thread));
    }
    std::cerr << "Threads size: " << threads.size() << std::endl;
    state = State::kRun;
}


Executor::~Executor() {
    Stop(true);
};


void Executor::Stop(bool await = false) {
    if (state == State::kStopped || state == State::kStopping) {
        return;
    }

    {
        std::unique_lock<std::mutex> lock(mutex);
        state = State::kStopping;
    }

    empty_condition.notify_all();

    std::unique_lock<std::mutex> lock(mutex);
    if (await) {
        while (!threads.empty()) {
            stop_condition.wait(lock);
        }
    }
    if (working_threads_count == 0) {
        state = State::kStopped;
    }
}


//template <typename F, typename... Types>
//bool Executor::Execute(F &&func, Types... args) {
//    // Prepare "task"
//    auto exec = std::bind(std::forward<F>(func), std::forward<Types>(args)...);
//
//    std::unique_lock<std::mutex> lock(this->mutex);
//    if (state != State::kRun || tasks.size() >= max_queue_size) {
//        return false;
//    }
//
//    // Enqueue new task
//    if (threads.size() < high_watermark && threads.size() == working_threads_count) {
//        auto thread = std::thread([=] { return perform(this); });
//        // auto thread = std::thread(perform, this);
//        threads.emplace(thread.get_id(), std::move(thread));
//    }
//
//    tasks.push_back(exec);
//    empty_condition.notify_one();
//
//    return true;
//}


void perform(Executor *executor) {

    std::unique_lock<std::mutex> lock(executor->mutex);
    bool thread_is_useless = false;

    while (executor->state == Executor::State::kRun || !executor->tasks.empty()) {
        auto now = std::chrono::system_clock::now();

        while (executor->tasks.empty()) {
            auto wait_status = executor->empty_condition.wait_until(
                lock, now + std::chrono::milliseconds(executor->wait_time));
            if (executor->state != Executor::State::kRun ||
                    executor->tasks.empty() && executor->threads.size() > executor->low_watermark &&
                    wait_status == std::cv_status::timeout) {
                thread_is_useless = true;
                break;
            }
        }

        if (thread_is_useless) {
            break;
        }

        std::function<void()> func = std::move( executor->tasks.front() );
        executor->working_threads_count++;
        executor->tasks.pop_front();

        lock.unlock();

        try {
            func();
        } catch (const std::exception &e) {
            std::cout << "Perform exception : " << e.what() << std::endl;
        }

        lock.lock();
        executor->working_threads_count--;
    }

    auto thread_id = std::this_thread::get_id();

    try {
        executor->threads.at(thread_id).detach();
    } catch(std::out_of_range &e) {
        // executor->Stop();
        throw std::out_of_range("Threads indexing out of range");
    }

    executor->threads.erase(thread_id);

    // Last alive thread
    if (executor->threads.empty() && executor->state != Executor::State::kRun) {
        executor->stop_condition.notify_all();
    }
}

}
} // namespace Afina
