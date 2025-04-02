# BatchProcessor.h - Comprehensive Explanation for Beginners

This document provides a detailed explanation of the `BatchProcessor.h` header file, which implements parallel file processing using a thread pool design pattern. This explanation is designed to help newcomers understand both the code and the concepts behind it.

## What is Batch Processing?

Before diving into the code, let's understand what batch processing means:

Batch processing refers to executing multiple similar tasks (like compressing multiple files) without user interaction. Instead of processing files one by one, a batch processor can handle many files simultaneously, taking advantage of multi-core processors to improve performance.

## Header Guards and Includes

```cpp
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
```

These are include guards and library imports:

- **Include Guards**: `#ifndef`, `#define`, and later `#endif` prevent multiple inclusions of the same header

- **Standard Library Includes**:
  - `<vector>`: For dynamic arrays (storing worker threads)
  - `<string>`: For string handling (file paths)
  - `<queue>`: For task queue implementation
  - `<thread>`: For creating and managing threads
  - `<mutex>`: For synchronization primitives to protect shared data
  - `<condition_variable>`: For thread signaling
  - `<functional>`: For `std::function` and related facilities
  - `<atomic>`: For thread-safe counters without locks
  - `<future>`: For getting results from asynchronous operations
  - `<filesystem>`: For file system operations

- **Namespace Alias**: `namespace fs = std::filesystem;` creates a shorter name for the filesystem namespace

## The BatchProcessor Class

```cpp
class BatchProcessor {
private:
    // Private members...
public:
    // Public methods...
};
```

This class implements a thread pool that processes multiple files in parallel. The class follows the object-oriented principle of encapsulation by keeping implementation details private and exposing a clean public interface.

## Private Members

```cpp
size_t numThreads;
std::vector<std::thread> workers;
std::queue<std::function<void()>> tasks;

std::mutex queueMutex;
std::condition_variable condition;
std::atomic<bool> stop;
std::atomic<size_t> activeThreads;
std::atomic<size_t> completedTasks;
std::atomic<size_t> totalTasks;
```

These private members form the core of the thread pool:

- **Thread Management**:
  - `numThreads`: Number of worker threads to create (usually based on CPU cores)
  - `workers`: Vector storing the actual thread objects

- **Task Queue**:
  - `tasks`: Queue containing the functions/tasks to be executed
  - Tasks are wrapped as `std::function<void()>` objects, which are callable objects that take no parameters and return nothing

- **Synchronization Primitives**:
  - `queueMutex`: Protects the task queue from concurrent access
  - `condition`: Allows threads to wait for new tasks or stop signal
  - `stop`: Flag indicating whether the thread pool is shutting down

- **Progress Tracking**:
  - `activeThreads`: Number of threads currently executing tasks
  - `completedTasks`: Number of tasks that have been completed
  - `totalTasks`: Total number of tasks submitted

The `atomic` types are used for counters accessed from multiple threads. They provide thread-safe operations without needing explicit locks, making them more efficient than using a mutex for simple counters.

## Constructor

```cpp
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
```

This constructor initializes the thread pool:

### Parameters:
- `threads`: Number of worker threads to create (default 0)
  - If 0 is provided, it automatically detects the number of CPU cores using `std::thread::hardware_concurrency()`

### Initialization List:
- Sets initial values for member variables:
  - `numThreads`: Number of threads to use
  - `stop`: Initially false (thread pool is active)
  - `activeThreads`, `completedTasks`, `totalTasks`: All start at 0

### Worker Thread Creation:
1. Creates `numThreads` worker threads using a for loop
2. Each worker thread executes the lambda function provided to `emplace_back`
3. The lambda captures `this` to access the BatchProcessor members

### Worker Thread Function:
Each worker thread runs an infinite loop that:

1. **Task Acquisition**:
   ```cpp
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
   ```
   - Creates a scoped lock to protect the task queue
   - Waits on the condition variable until either:
     - The `stop` flag is set (pool is shutting down), or
     - There are tasks in the queue
   - If the pool is stopping AND there are no more tasks, exits the thread
   - Otherwise, takes the next task from the queue using `std::move` for efficiency
   - Releases the lock by ending the scope

2. **Task Execution**:
   ```cpp
   activeThreads++;
   task();
   activeThreads--;
   completedTasks++;
   ```
   - Increments active thread counter
   - Executes the task
   - Decrements active thread counter
   - Increments completed task counter

This busy-wait loop continues until the thread pool is shut down.

## Task Enqueue Method

```cpp
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
```

This method adds a task to the queue and returns a future for getting the result:

### Template Parameters:
- `F`: Type of the callable object (function, lambda, etc.)

### Return Type:
- `std::future<ReturnType>`: A future that will eventually contain the result of the task
- `std::result_of<F()>::type` determines what type the task returns

### Implementation Details:

1. **Type Definition**:
   - `using ReturnType = typename std::result_of<F()>::type;`
   - Defines an alias for the return type of the task

2. **Task Wrapping**:
   ```cpp
   auto taskPtr = std::make_shared<std::packaged_task<ReturnType()>>(
       std::forward<F>(task)
   );
   ```
   - Creates a shared pointer to a packaged_task
   - `std::packaged_task` connects the task to a future
   - `std::forward<F>(task)` preserves the value category (lvalue/rvalue) of the task

3. **Progress Tracking**:
   - `totalTasks++;`
   - Increments the total task count

4. **Future Retrieval**:
   - `std::future<ReturnType> result = taskPtr->get_future();`
   - Gets the future associated with the task for returning to the caller

5. **Queue Insertion**:
   ```cpp
   {
       std::unique_lock<std::mutex> lock(queueMutex);
       
       if (stop) {
           throw std::runtime_error("Cannot enqueue on stopped BatchProcessor");
       }
       
       tasks.emplace([taskPtr]() { (*taskPtr)(); });
   }
   ```
   - Locks the queue mutex
   - Checks if the thread pool is stopping
   - If not, creates a lambda that calls the packaged task and adds it to the queue
   - The lambda captures the shared pointer by value

6. **Thread Notification**:
   - `condition.notify_one();`
   - Wakes up one waiting thread to process the new task

7. **Result Return**:
   - `return result;`
   - Returns the future to the caller

This method is templated to handle any callable object that takes no arguments, while preserving its return type.

## Batch Processing Method

```cpp
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
```

This method processes a batch of files using the provided function:

### Parameters:
- `filePaths`: Vector of file paths to process
- `processFunc`: Function that processes a single file
- `progressCallback`: Optional callback for reporting progress (default: nullptr)

### Implementation:

1. **Reset Counters**:
   ```cpp
   totalTasks = filePaths.size();
   completedTasks = 0;
   ```
   - Sets total tasks to match the number of files
   - Resets completed tasks counter to zero

2. **Future Collection**:
   - `std::vector<std::future<void>> futures;`
   - Creates a vector to store futures for all tasks

3. **Task Submission Loop**:
   ```cpp
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
   ```
   - Iterates through each file path
   - Creates a lambda that:
     - Calls the processing function with the file path
     - Invokes the progress callback if provided
   - Enqueues the lambda to the thread pool
   - Stores the returned future in the vector

4. **Wait for Completion**:
   ```cpp
   for (auto& future : futures) {
       future.wait();
   }
   ```
   - Iterates through all futures
   - Waits for each task to complete
   - This ensures all files are processed before the method returns

This method simplifies batch processing by hiding all the thread management details.

## Progress Tracking Methods

```cpp
double getProgress() const {
    if (totalTasks == 0) return 0.0;
    return static_cast<double>(completedTasks) / totalTasks;
}

size_t getTotalTasks() const {
    return totalTasks;
}

size_t getCompletedTasks() const {
    return completedTasks;
}

size_t getActiveThreads() const {
    return activeThreads;
}

size_t getThreadCount() const {
    return numThreads;
}
```

These methods provide information about the current processing state:

### getProgress Method:
- Returns the completion percentage as a value between 0.0 and 1.0
- Handles the case where there are no tasks to avoid division by zero
- Uses `static_cast<double>` to ensure floating-point division

### getTotalTasks Method:
- Returns the total number of tasks submitted

### getCompletedTasks Method:
- Returns the number of completed tasks

### getActiveThreads Method:
- Returns the number of threads currently executing tasks

### getThreadCount Method:
- Returns the total number of worker threads

These methods are all marked `const` because they don't modify the object's state.

## Destructor

```cpp
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
```

The destructor cleans up resources and shuts down threads:

1. **Set Stop Flag**:
   ```cpp
   {
       std::unique_lock<std::mutex> lock(queueMutex);
       stop = true;
   }
   ```
   - Acquires the queue mutex
   - Sets the stop flag to true
   - Releases the mutex

2. **Notify Waiting Threads**:
   - `condition.notify_all();`
   - Wakes up all waiting worker threads
   - This ensures they check the stop flag and exit

3. **Join Threads**:
   ```cpp
   for (auto& worker : workers) {
       if (worker.joinable()) {
           worker.join();
       }
   }
   ```
   - Loops through all worker threads
   - Checks if each thread is joinable (has not already been joined)
   - Waits for each thread to finish its current task and exit

This proper cleanup is essential to avoid resource leaks or crashes when the BatchProcessor is destroyed.

## End of File

```cpp
#endif // BATCH_PROCESSOR_H
```

This closes the include guard opened at the beginning of the file.

## Key Concepts Explained

### Thread Pool Design Pattern

A thread pool is a software design pattern where a fixed number of threads are created to execute tasks from a queue. This brings several advantages:

1. **Reuse of Threads**: Creating and destroying threads is expensive; reusing them improves performance
2. **Limiting Threads**: Too many threads can degrade performance; a thread pool controls concurrency
3. **Load Balancing**: Tasks are automatically distributed among available threads
4. **Separation of Concerns**: Task creation is separated from task execution

### Concurrency Control Mechanisms

The BatchProcessor uses several mechanisms to ensure thread safety:

#### Mutex (Mutual Exclusion)
A mutex ensures only one thread can access a protected resource at a time:
- `queueMutex` protects the task queue
- `std::unique_lock<std::mutex>` provides RAII-style locking

#### Condition Variables
A condition variable allows threads to efficiently wait for specific conditions:
- `condition` allows worker threads to wait for new tasks
- `wait()` puts threads to sleep until `notify_one()` or `notify_all()` is called
- The predicate `return stop || !tasks.empty()` avoids spurious wakeups

#### Atomic Variables
Atomic variables provide thread-safe operations without locking:
- `stop`, `activeThreads`, `completedTasks`, and `totalTasks` are atomic
- This is more efficient than using a mutex for these simple counters

### Task Submission with Futures

The `enqueue` method uses modern C++ features for task submission:

- **std::future**: Provides a way to retrieve results from asynchronous operations
- **std::packaged_task**: Connects a callable object with a future
- **std::forward**: Preserves the value category (lvalue/rvalue) of forwarded objects

### Fixed vs. Dynamic Thread Pools

This implementation is a fixed-size thread pool, where:
- The number of threads is determined at construction time
- Threads remain active throughout the lifetime of the pool
- Tasks are queued if all threads are busy

A dynamic thread pool would create or destroy threads based on workload, which can be more efficient in some scenarios but also more complex.

## Complete Usage Example

Here's a comprehensive example of how to use the BatchProcessor:

```cpp
#include "BatchProcessor.h"
#include "FileProcessor.h"
#include "HuffmanCompressor.h"
#include <iostream>

int main() {
    try {
        // Create a batch processor with default thread count (use all CPU cores)
        BatchProcessor processor;
        
        // List files to compress
        std::vector<std::string> files = FileProcessor::listFiles("documents/", true);
        
        // Create a progress callback function
        auto progressCallback = [](const std::string& path, int current, int total) {
            std::cout << "Processed: " << path << " (" << current << "/" << total << ")" << std::endl;
        };
        
        // Define the processing function
        auto compressFile = [](const std::string& path) {
            try {
                // Read file
                auto fileData = FileProcessor::readFile(path);
                
                // Compress data
                HuffmanCompressor compressor;
                auto compressedData = compressor.compress(fileData);
                
                // Write compressed file
                std::string outputPath = FileProcessor::getCompressedPath(path);
                FileProcessor::writeFile(outputPath, compressedData);
                
                // Calculate compression ratio
                double ratio = compressor.getCompressionRatio(fileData.size(), compressedData.size());
                std::cout << "Compressed " << path << " - Ratio: " << ratio << "%" << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Error compressing " << path << ": " << e.what() << std::endl;
            }
        };
        
        // Process all files in parallel
        std::cout << "Starting compression with " << processor.getThreadCount() << " threads..." << std::endl;
        processor.processBatch(files, compressFile, progressCallback);
        
        // Report completion
        std::cout << "Compression complete. Processed " << processor.getCompletedTasks() 
                  << " files out of " << processor.getTotalTasks() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

This example:
1. Creates a BatchProcessor using all available CPU cores
2. Gets a list of files to compress
3. Defines callback functions for progress reporting and file processing
4. Processes all files in parallel
5. Reports the final results

## Hard-Coded Values and Performance Considerations

### Thread Count Determination
The code determines thread count as follows:
```cpp
numThreads(threads == 0 ? std::thread::hardware_concurrency() : threads)
```
- If 0 is provided, it uses `std::thread::hardware_concurrency()` to detect CPU cores
- This is a good default for CPU-bound tasks like compression
- For I/O-bound tasks, using more threads than cores can sometimes improve performance

### Progress Reporting
The progress callback function passes three parameters:
```cpp
progressCallback(path, i + 1, filePaths.size());
```
- `path`: The current file path (string)
- `i + 1`: The current file index (starting from 1)
- `filePaths.size()`: The total number of files

The "+1" is added to the index to make it more human-readable (starting from 1 instead of 0).

### Memory and Performance Considerations

1. **Shared Pointers for Tasks**:
   - Tasks are wrapped in `std::shared_ptr` to ensure they remain valid even if the original caller goes out of scope
   - This adds overhead but ensures safety

2. **Move Semantics for Futures**:
   - `std::move(future)` is used when storing futures to avoid unnecessary copying
   - This improves performance when working with large collections of futures

3. **Lambda Captures**:
   - `[=, &processFunc, &progressCallback]` captures most variables by value but references for the callbacks
   - This ensures the callbacks themselves aren't copied, which could be expensive

4. **Task Queue Capacity**:
   - The task queue has no explicit limit on size
   - For very large batches, this could consume significant memory

## Advanced Techniques Used

### Template Functions
The BatchProcessor uses template functions to handle different types of callables:
```cpp
template<class F>
auto enqueue(F&& task) -> std::future<typename std::result_of<F()>::type>
```
- This allows enqueueing any function type, not just a specific signature
- The return type is deduced automatically from the function

### Perfect Forwarding
The code uses perfect forwarding to preserve the value category of the task:
```cpp
std::forward<F>(task)
```
- This ensures efficiency whether the task is passed as an lvalue or rvalue

### RAII (Resource Acquisition Is Initialization)
The code follows the RAII principle for resource management:
- Thread creation happens in the constructor
- Thread cleanup happens in the destructor
- Mutex locking uses `std::unique_lock` which automatically unlocks when it goes out of scope

### Lambda Functions
The code uses lambdas extensively:
- For worker thread bodies
- For task wrappers
- For task processing functions
