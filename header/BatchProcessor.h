#ifndef BATCH_PROCESSOR_H
#define BATCH_PROCESSOR_H

#include <vector>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>
#include <filesystem>

namespace fs = std::filesystem;

// Thread pool for parallel file processing
class BatchProcessor {
private:
    size_t numThreads;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    std::atomic<size_t> activeThreads;
    std::atomic<size_t> completedTasks;
    std::atomic<size_t> totalTasks;
    
public:
    // Constructor with configurable thread count
    BatchProcessor(size_t threads = 0) : 
        numThreads(threads == 0 ? std::thread::hardware_concurrency() : threads),
        stop(false),
        activeThreads(0),
        completedTasks(0),
        totalTasks(0) {
        
        // Create worker threads
        for (size_t i = 0; i < numThreads; i++) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { 
                            return stop || !tasks.empty(); 
                        });
                        
                        if (stop && tasks.empty()) {
                            return;
                        }
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    
                    activeThreads++;
                    task();
                    activeThreads--;
                    completedTasks++;
                }
            });
        }
    }
    
    // Add task to queue
    template<class F>
    auto enqueue(F&& task) -> std::future<typename std::result_of<F()>::type> {
        using ReturnType = typename std::result_of<F()>::type;
        
        auto taskPtr = std::make_shared<std::packaged_task<ReturnType()>>(
            std::forward<F>(task)
        );
        
        totalTasks++;
        
        std::future<ReturnType> result = taskPtr->get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            
            if (stop) {
                throw std::runtime_error("Cannot enqueue on stopped BatchProcessor");
            }
            
            tasks.emplace([taskPtr]() { (*taskPtr)(); });
        }
        
        condition.notify_one();
        return result;
    }
    
    // Process a batch of files with a given function
    template<typename Func>
    void processBatch(const std::vector<std::string>& filePaths, Func processFunc, 
                      std::function<void(const std::string&, int, int)> progressCallback = nullptr) {
        totalTasks = filePaths.size();
        completedTasks = 0;
        
        std::vector<std::future<void>> futures;
        
        for (size_t i = 0; i < filePaths.size(); i++) {
            const auto& path = filePaths[i];
            
            auto future = enqueue([=, &processFunc, &progressCallback]() {
                processFunc(path);
                if (progressCallback) {
                    progressCallback(path, i + 1, filePaths.size());
                }
            });
            
            futures.push_back(std::move(future));
        }
        
        // Wait for all tasks to complete
        for (auto& future : futures) {
            future.wait();
        }
    }
    
    // Get progress information
    double getProgress() const {
        if (totalTasks == 0) return 0.0;
        return static_cast<double>(completedTasks) / totalTasks;
    }
    
    // Get total tasks count
    size_t getTotalTasks() const {
        return totalTasks;
    }
    
    // Get completed tasks count
    size_t getCompletedTasks() const {
        return completedTasks;
    }
    
    // Get active threads count
    size_t getActiveThreads() const {
        return activeThreads;
    }
    
    // Get total threads count
    size_t getThreadCount() const {
        return numThreads;
    }
    
    // Destructor - clean up threads
    ~BatchProcessor() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        
        condition.notify_all();
        
        for (auto& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
};

#endif // BATCH_PROCESSOR_H