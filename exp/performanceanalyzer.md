# PerformanceAnalyzer.h - Detailed Explanation for Complete Beginners

This document provides a comprehensive explanation of the `PerformanceAnalyzer.h` header file, which measures and reports on the performance of compression and decompression operations. This explanation is designed for someone new to C++ programming.

## Header Guards and Includes

```cpp
#ifndef PERFORMANCE_ANALYZER_H
#define PERFORMANCE_ANALYZER_H

#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <iostream>
#include <fstream>
#include <iomanip>
```

As with other header files, this begins with an include guard to prevent multiple inclusions. The file includes several standard C++ libraries:

- `<chrono>`: Provides time-related functionality for performance measurement
- `<string>`: Provides string handling functionality
- `<vector>`: Provides dynamic array implementation
- `<unordered_map>`: Provides hash map data structure for storing metrics by name
- `<mutex>`: Provides synchronization primitives for thread safety
- `<iostream>`: Provides console input/output operations
- `<fstream>`: Provides file input/output operations
- `<iomanip>`: Provides formatting utilities for output

## The PerformanceAnalyzer Class

```cpp
class PerformanceAnalyzer {
private:
    // Private members and nested classes...
public:
    // Public methods...
};
```

This class measures and reports on the performance of compression and decompression operations. It contains:
- A nested `Metric` structure for storing performance data
- A nested `Timer` class for measuring execution time
- Methods for recording metrics and generating reports

## The Metric Structure

```cpp
struct Metric {
    double duration;      // in milliseconds
    size_t originalSize;
    size_t compressedSize;
    
    double getCompressionRatio() const {
        if (originalSize == 0) return 0.0;
        return 100.0 * (1.0 - static_cast<double>(compressedSize) / originalSize);
    }
    
    double getThroughput() const {
        if (duration == 0) return 0.0;
        // MB per second
        return (originalSize / 1024.0 / 1024.0) / (duration / 1000.0);
    }
};
```

This structure stores performance metrics for a single operation:

### Member Variables

- `duration`: The time taken to complete the operation, measured in milliseconds
- `originalSize`: The size of the original (uncompressed) data in bytes
- `compressedSize`: The size of the compressed data in bytes

### Methods

#### getCompressionRatio Method

```cpp
double getCompressionRatio() const {
    if (originalSize == 0) return 0.0;
    return 100.0 * (1.0 - static_cast<double>(compressedSize) / originalSize);
}
```

This method calculates the compression ratio as a percentage:

1. First checks if original size is zero to avoid division by zero:
   - Returns 0.0 if original size is zero
2. Calculates the compression ratio using the formula:
   - `100.0 * (1.0 - compressedSize / originalSize)`
   - The `static_cast<double>` converts integer values to floating point for accurate division
3. Returns the result as a percentage:
   - A value of 60.0 means the data was compressed by 60% (the compressed version is 40% of the original size)
   - Higher values indicate better compression

For example, if the original file is 1000 bytes and the compressed file is 400 bytes:
- Compression ratio = 100.0 * (1.0 - 400/1000) = 100.0 * 0.6 = 60%

#### getThroughput Method

```cpp
double getThroughput() const {
    if (duration == 0) return 0.0;
    // MB per second
    return (originalSize / 1024.0 / 1024.0) / (duration / 1000.0);
}
```

This method calculates the processing speed in megabytes per second:

1. First checks if duration is zero to avoid division by zero:
   - Returns 0.0 if duration is zero
2. Converts the original size from bytes to megabytes:
   - `originalSize / 1024.0 / 1024.0` (divides by 1024 twice)
   - 1 KB = 1024 bytes, 1 MB = 1024 KB
3. Converts duration from milliseconds to seconds:
   - `duration / 1000.0`
4. Divides the size in MB by the duration in seconds to get MB/s:
   - The result is how many megabytes the program processed per second

For example, if a 10MB file was processed in 500ms:
- Throughput = (10,000,000 / 1024 / 1024) / (500 / 1000) = 9.54 MB/s / 0.5s = 19.07 MB/s

## Private Members

```cpp
std::unordered_map<std::string, Metric> metrics;
std::mutex metricsMutex;
```

- `metrics`: A hash map that stores performance metrics indexed by operation name
  - The key is a string like "Compress file.txt" 
  - The value is a Metric structure containing the performance data
- `metricsMutex`: A mutex (mutual exclusion) object for thread safety
  - Prevents race conditions when multiple threads try to update metrics simultaneously

## The Timer Inner Class

```cpp
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start;
    std::string operationName;
    PerformanceAnalyzer& analyzer;
    size_t originalSize;
    size_t compressedSize;
    
public:
    Timer(const std::string& name, PerformanceAnalyzer& a, 
          size_t origSize, size_t compSize = 0) 
        : operationName(name), analyzer(a), 
          originalSize(origSize), compressedSize(compSize) {
        start = std::chrono::high_resolution_clock::now();
    }
    
    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        analyzer.recordMetric(operationName, duration.count(), originalSize, compressedSize);
    }
    
    void setCompressedSize(size_t size) {
        compressedSize = size;
    }
};
```

This nested class is responsible for timing operations using the RAII (Resource Acquisition Is Initialization) pattern:

### Member Variables

- `start`: The timestamp when timing began
- `operationName`: The name of the operation being timed (e.g., "Compress file.txt")
- `analyzer`: A reference to the parent PerformanceAnalyzer 
- `originalSize`: The size of the original data in bytes
- `compressedSize`: The size of the compressed data in bytes (if applicable)

### Constructor

```cpp
Timer(const std::string& name, PerformanceAnalyzer& a, 
      size_t origSize, size_t compSize = 0) 
    : operationName(name), analyzer(a), 
      originalSize(origSize), compressedSize(compSize) {
    start = std::chrono::high_resolution_clock::now();
}
```

1. Takes parameters for the operation name, analyzer reference, and file sizes
2. Initializes member variables using an initialization list:
   - `operationName(name)`: Sets the operation name
   - `analyzer(a)`: Sets the reference to the parent analyzer
   - `originalSize(origSize)`: Sets the original data size
   - `compressedSize(compSize)`: Sets the compressed size (defaults to 0)
3. Records the current time as the starting point:
   - `std::chrono::high_resolution_clock` is the most precise clock available
   - `now()` gets the current time point
   - This happens after the initialization list, inside the constructor body

### Destructor

```cpp
~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    analyzer.recordMetric(operationName, duration.count(), originalSize, compressedSize);
}
```

The destructor is automatically called when the Timer object goes out of scope:

1. Records the current time as the ending point:
   - `high_resolution_clock::now()` gets the current time
2. Calculates the duration by subtracting the starting time:
   - Creates a `duration` object that represents time in milliseconds
   - `std::milli` specifies milliseconds as the unit
   - `duration = end - start` calculates the time difference
3. Records the metric in the parent analyzer:
   - `analyzer.recordMetric(...)` calls the private recordMetric method
   - `duration.count()` converts the duration to a simple number

This destructor-based approach is powerful because:
- Timing stops automatically when the scope ends
- Timing works correctly even if exceptions occur
- The code is cleaner and less error-prone than manual start/stop calls

### setCompressedSize Method

```cpp
void setCompressedSize(size_t size) {
    compressedSize = size;
}
```

A simple method to update the compressed size during timing:
- Takes a size parameter
- Sets the compressedSize member variable
- Useful when the compressed size is only known after processing begins

## Private Method

### recordMetric Method

```cpp
void recordMetric(const std::string& name, double duration, size_t originalSize, size_t compressedSize) {
    std::lock_guard<std::mutex> lock(metricsMutex);
    metrics[name] = {duration, originalSize, compressedSize};
}
```

This method records a performance metric:

1. Creates a `lock_guard` object to lock the mutex:
   - `std::lock_guard<std::mutex> lock(metricsMutex)` automatically locks the mutex
   - The mutex remains locked until the lock_guard goes out of scope
   - This ensures thread safety when multiple operations run concurrently
2. Stores the metric in the hash map:
   - `metrics[name] = {duration, originalSize, compressedSize}` 
   - Uses C++ aggregate initialization to create a Metric structure
   - If an entry with the same name already exists, it's overwritten

## Public Methods

### startTimer Method

```cpp
std::unique_ptr<Timer> startTimer(const std::string& operationName, 
                                size_t originalSize, 
                                size_t compressedSize = 0) {
    return std::make_unique<Timer>(operationName, *this, originalSize, compressedSize);
}
```

This method creates and starts a new Timer:

1. Takes parameters for the operation name and file sizes:
   - `operationName`: A descriptive name for the operation
   - `originalSize`: Size of the original data in bytes
   - `compressedSize`: Size of the compressed data (optional, defaults to 0)
2. Returns a `std::unique_ptr<Timer>`:
   - `std::make_unique<Timer>` creates a dynamically allocated Timer object
   - `std::unique_ptr` ensures the Timer is automatically deleted when no longer needed
   - The Timer starts timing as soon as it's created

Example usage from application code:
```cpp
size_t fileSize = getFileSize("example.txt");
auto timer = analyzer.startTimer("Compress example.txt", fileSize);
// Perform compression...
timer->setCompressedSize(compressedSize);
// When timer goes out of scope, timing stops and metrics are recorded
```

### getMetric Method

```cpp
Metric getMetric(const std::string& name) {
    std::lock_guard<std::mutex> lock(metricsMutex);
    if (metrics.find(name) != metrics.end()) {
        return metrics[name];
    }
    return {0.0, 0, 0};
}
```

This method retrieves a specific metric by name:

1. Locks the mutex for thread safety:
   - `std::lock_guard<std::mutex> lock(metricsMutex)`
2. Checks if the metric exists:
   - `metrics.find(name) != metrics.end()` searches the hash map
3. Returns the metric if found:
   - `return metrics[name]` gets the stored Metric structure
4. Returns a default metric (all zeros) if not found:
   - `return {0.0, 0, 0}` creates a new Metric with default values

### getAllMetrics Method

```cpp
std::unordered_map<std::string, Metric> getAllMetrics() {
    std::lock_guard<std::mutex> lock(metricsMutex);
    return metrics;
}
```

This method retrieves all recorded metrics:

1. Locks the mutex for thread safety
2. Returns a copy of the entire metrics hash map
3. The caller can then iterate through all metrics

### generateReport Method

```cpp
void generateReport(std::ostream& output = std::cout) {
    std::lock_guard<std::mutex> lock(metricsMutex);
    
    output << "==== Performance Analysis Report ====\n\n";
    
    output << std::setw(30) << std::left << "Operation"
           << std::setw(15) << std::right << "Duration (ms)"
           << std::setw(15) << "Original (B)"
           << std::setw(15) << "Compressed (B)"
           << std::setw(15) << "Ratio (%)"
           << std::setw(15) << "MB/s"
           << "\n";
    
    output << std::string(105, '-') << "\n";
    
    for (const auto& pair : metrics) {
        output << std::setw(30) << std::left << pair.first
               << std::setw(15) << std::fixed << std::setprecision(2) << std::right << pair.second.duration
               << std::setw(15) << pair.second.originalSize
               << std::setw(15) << pair.second.compressedSize
               << std::setw(15) << std::fixed << std::setprecision(2) << pair.second.getCompressionRatio()
               << std::setw(15) << std::fixed << std::setprecision(2) << pair.second.getThroughput()
               << "\n";
    }
    
    output << "\n";
}
```

This method formats and outputs a performance report:

1. Locks the mutex for thread safety
2. Formats a report header:
   - Title banner with "==== Performance Analysis Report ===="
   - Column headers for each metric
   - Separator line with 105 dashes (`std::string(105, '-')`)
3. Iterates through all metrics and formats each row:
   - Uses `std::setw()` to set column widths:
     - 30 characters for operation name
     - 15 characters for other columns
   - Uses `std::left` and `std::right` to control text alignment
   - Uses `std::fixed` and `std::setprecision(2)` to format floating point numbers with 2 decimal places
4. Outputs each metric with:
   - Operation name (left-aligned)
   - Duration in milliseconds (right-aligned)
   - Original size in bytes (right-aligned)
   - Compressed size in bytes (right-aligned)
   - Compression ratio percentage (right-aligned)
   - Throughput in MB/s (right-aligned)
5. Default output is to `std::cout` (console):
   - The parameter `std::ostream& output = std::cout` has a default value of standard output
   - This can be overridden to output to a file or string stream

Example report output:
```
==== Performance Analysis Report ====

Operation                       Duration (ms)  Original (B)  Compressed (B)      Ratio (%)          MB/s
---------------------------------------------------------------------------------------------------------
Compress example.txt                   245.32       1048576         425984          59.38           4.08
Decompress example.txt                 142.65        425984        1048576          -146.15         7.02
```

### saveReport Method

```cpp
void saveReport(const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file) {
        throw std::runtime_error("Unable to create report file: " + filePath);
    }
    
    generateReport(file);
}
```

This method saves the performance report to a file:

1. Opens a file for writing:
   - `std::ofstream file(filePath)` creates or overwrites the specified file
2. Checks if the file opened successfully:
   - If not, throws a runtime_error with a descriptive message
3. Calls `generateReport(file)` to write the report to the file:
   - Passes the file stream as the output parameter
   - The same formatting is used as for console output

## End of File

```cpp
#endif // PERFORMANCE_ANALYZER_H
```

This closes the include guard started at the beginning of the file.

## Hard-Coded Values Explained

The code contains several hard-coded values for formatting and display:

1. **Column Widths**:
   - Operation name: 30 characters (`std::setw(30)`)
   - Other columns: 15 characters (`std::setw(15)`)
   - These values were chosen to accommodate typical data while maintaining a readable format

2. **Separator Line Length**:
   - 105 dashes (`std::string(105, '-')`)
   - This equals the sum of all column widths (30 + 15*5 = 105)

3. **Decimal Precision**:
   - 2 decimal places for percentages and throughput (`std::setprecision(2)`)
   - This provides sufficient accuracy without cluttering the display

4. **Unit Conversions**:
   - Bytes to MB: division by 1024.0 twice
   - Milliseconds to seconds: division by 1000.0
   - These are standard binary and decimal conversions

## Complete Usage Example

Here's a complete example of how the PerformanceAnalyzer would be used in an application:

```cpp
// Create an analyzer
PerformanceAnalyzer analyzer;

// Time a compression operation
{
    size_t fileSize = FileProcessor::getFileSize("example.txt");
    auto timer = analyzer.startTimer("Compress example.txt", fileSize);
    
    // Read the file
    auto data = FileProcessor::readFile("example.txt");
    
    // Compress the data
    HuffmanCompressor compressor;
    auto compressedData = compressor.compress(data);
    
    // Update the compressed size
    timer->setCompressedSize(compressedData.size());
    
    // Write the compressed file
    FileProcessor::writeFile("example.txt.huf", compressedData);
    
    // Timer automatically stops and records metrics when it goes out of scope
}

// Time a decompression operation
{
    size_t compressedSize = FileProcessor::getFileSize("example.txt.huf");
    auto data = FileProcessor::readFile("example.txt.huf");
    
    HuffmanCompressor compressor;
    auto timer = analyzer.startTimer("Decompress example.txt", compressedSize);
    
    // Decompress the data
    auto decompressedData = compressor.decompress(data);
    
    // Update with the decompressed size
    timer->setCompressedSize(decompressedData.size());
    
    // Write the decompressed file
    FileProcessor::writeFile("example_decompressed.txt", decompressedData);
    
    // Timer automatically stops and records metrics when it goes out of scope
}

// Print performance report to console
analyzer.generateReport();

// Save report to file
analyzer.saveReport("performance_report.txt");

// Access specific metrics programmatically
Metric compressionMetric = analyzer.getMetric("Compress example.txt");
double ratio = compressionMetric.getCompressionRatio();
std::cout << "Achieved compression ratio: " << ratio << "%" << std::endl;
```

## Thread Safety Considerations

The PerformanceAnalyzer class is designed to be thread-safe:

1. **Mutex Protection**: 
   - The `metricsMutex` protects access to the metrics map
   - Each public method that accesses the map locks the mutex
   - The `std::lock_guard` ensures the mutex is always released, even if exceptions occur

2. **Copy Return Values**:
   - Methods return copies of metrics rather than references
   - This prevents thread safety issues after the method returns

3. **Timer References**:
   - Each Timer holds a reference to the parent analyzer
   - This is safe because the Timer's lifetime is expected to be shorter than the analyzer's

This thread safety allows the analyzer to be used in multi-threaded applications, such as compressing multiple files in parallel.

## Performance Considerations

The PerformanceAnalyzer introduces minimal overhead:

1. **Time Measurement**:
   - `std::chrono::high_resolution_clock` provides nanosecond-level precision
   - The overhead of starting and stopping timers is negligible compared to file operations

2. **Memory Usage**:
   - Each metric is small (two size_t values and one double)
   - The total memory usage scales linearly with the number of operations measured

3. **Thread Contention**:
   - The mutex is held only briefly during metric recording and retrieval
   - This minimizes thread blocking in multi-threaded applications

## Design Patterns Used

The PerformanceAnalyzer demonstrates several design patterns:

1. **RAII (Resource Acquisition Is Initialization)**:
   - The Timer automatically starts on construction and stops on destruction
   - This ensures proper timing even in the presence of exceptions

2. **Facade Pattern**:
   - Provides a simplified interface for timing operations
   - Hides the complexity of chrono library and thread safety

3. **Observer Pattern**:
   - The Timer notifies the analyzer when an operation completes
   - The analyzer collects and aggregates these observations

4. **Singleton-like Usage**:
   - While not a true singleton, the class is typically used with a single instance
   - This instance collects metrics from throughout the application
