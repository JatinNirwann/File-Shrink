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

class PerformanceAnalyzer {
private:
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
    
    std::unordered_map<std::string, Metric> metrics;
    std::mutex metricsMutex;
    
    // Timer for measuring durations
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
    
    // Records a metric
    void recordMetric(const std::string& name, double duration, size_t originalSize, size_t compressedSize) {
        std::lock_guard<std::mutex> lock(metricsMutex);
        metrics[name] = {duration, originalSize, compressedSize};
    }
    
public:
    // Create a timer for an operation
    std::unique_ptr<Timer> startTimer(const std::string& operationName, 
                                     size_t originalSize, 
                                     size_t compressedSize = 0) {
        return std::make_unique<Timer>(operationName, *this, originalSize, compressedSize);
    }
    
    // Get a specific metric
    Metric getMetric(const std::string& name) {
        std::lock_guard<std::mutex> lock(metricsMutex);
        if (metrics.find(name) != metrics.end()) {
            return metrics[name];
        }
        return {0.0, 0, 0};
    }
    
    // Get all metrics
    std::unordered_map<std::string, Metric> getAllMetrics() {
        std::lock_guard<std::mutex> lock(metricsMutex);
        return metrics;
    }
    
    // Generate performance report
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
    
    // Save report to file
    void saveReport(const std::string& filePath) {
        std::ofstream file(filePath);
        if (!file) {
            throw std::runtime_error("Unable to create report file: " + filePath);
        }
        
        generateReport(file);
    }
};

#endif // PERFORMANCE_ANALYZER_H