#ifndef AFINA_CONCURRENCY_EXECUTOR_H
#define AFINA_CONCURRENCY_EXECUTOR_H

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
// #include <string>
#include <thread>

namespace Afina {
namespace Concurrency {

class Executor;

void perform(Executor *executor);

/**
 * # Thread pool
 */
class Executor {
private:
    // No copy/move/assign allowed
    Executor(const Executor &);            // = delete;
    Executor(Executor &&);                 // = delete;
    Executor &operator=(const Executor &); // = delete;
    Executor &operator=(Executor &&);      // = delete;

    /**
     * Main function that all pool threads are running. It polls internal task queue and execute tasks
     */
    friend void perform(Executor *executor);

public:
    enum class State {
        // Threadpool is fully operational, tasks could be added and get executed
        kRun,

        // Threadpool is on the way to be shutdown, no ned task could be added, but existing will be
        // completed as requested
        kStopping,

        // Threadppol is stopped
        kStopped
    };

    // Executor(std::string name, int size);
    Executor(uint32_t low_watermark, uint32_t hight_watermark, uint32_t max_queue_size, uint32_t idle_time);

    ~Executor();

    /**
     * Signal thread pool to stop, it will stop accepting new jobs and close threads just after each become
     * free. All enqueued jobs will be complete.
     *
     * In case if await flag is true, call won't return until all background jobs are done and all threads are stopped
     */
    void Stop(bool await = false);

    /**
     * Add function to be executed on the threadpool. Method returns true in case if task has been placed
     * onto execution queue, i.e scheduled for execution and false otherwise.
     *
     * That function doesn't wait for function result. Function could always be written in a way to notify caller about
     * execution finished by itself
     */
     template <typename F, typename... Types> bool Execute(F &&func, Types... args)
     {
         // Prepare "task"
         auto exec = std::bind(std::forward<F>(func), std::forward<Types>(args)...);

         std::unique_lock<std::mutex> lock(this->mutex);
         if ((state != State::kRun) || (tasks.size() >= _max_queue_size))
         {
             return false;
         }
         if ((_free_threads == 0) && (threads.size() < _hight_watermark))
         {
             threads.emplace_back(&perform, this);
             _free_threads++;
         }

         // Enqueue new task
         tasks.push_back(exec);
         empty_condition.notify_one();
         return true;
     }

private:
    // // No copy/move/assign allowed
    // Executor(const Executor &);            // = delete;
    // Executor(Executor &&);                 // = delete;
    // Executor &operator=(const Executor &); // = delete;
    // Executor &operator=(Executor &&);      // = delete;
    //
    // /**
    //  * Main function that all pool threads are running. It polls internal task queue and execute tasks
    //  */
    // friend void perform(Executor *executor);

    // Minimal number of threads in pool
    uint32_t _low_watermark;

    // Maximal number of threads in pool
    uint32_t _hight_watermark;

    // Maximal size of task queue
    uint32_t _max_queue_size;

    // Waiting time for new task for every thread
    uint32_t _idle_time;

    // Current number of awaiting threads for tasks
    uint32_t _free_threads;

    /**
     * Mutex to protect state below from concurrent modification
     */
    std::mutex mutex;

    /**
     * Conditional variable to await new data in case of empty queue
     */
    std::condition_variable empty_condition;

    /**
     * Conditional variable to await threads to be joined
     */
    std::condition_variable join_condition;

    /**
     * Vector of actual threads that perorm execution
     */
    std::vector<std::thread> threads;

    /**
     * Task queue
     */
    std::deque<std::function<void()>> tasks;

    /**
     * Flag to stop bg threads
     */
    State state;
};

} // namespace Concurrency
} // namespace Afina

#endif // AFINA_CONCURRENCY_EXECUTOR_H
