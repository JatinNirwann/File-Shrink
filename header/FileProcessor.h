#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

class FileProcessor {
public:
    // Read entire file into byte vector
    static std::vector<unsigned char> readFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Unable to open file: " + filePath);
        }
        
        // Get file size
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // Read file content
        std::vector<unsigned char> buffer(fileSize);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
            throw std::runtime_error("Error reading file: " + filePath);
        }
        
        return buffer;
    }
    
    // Write byte vector to file
    static void writeFile(const std::string& filePath, const std::vector<unsigned char>& data) {
        // Create directory if it doesn't exist
        fs::path path(filePath);
        if (!path.parent_path().empty()) {
            fs::create_directories(path.parent_path());
        }
        
        std::ofstream file(filePath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Unable to create file: " + filePath);
        }
        
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        if (!file) {
            throw std::runtime_error("Error writing to file: " + filePath);
        }
    }
    
    // Get file size
    static size_t getFileSize(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file) {
            throw std::runtime_error("Unable to open file: " + filePath);
        }
        
        return file.tellg();
    }
    
    // Check if file exists
    static bool fileExists(const std::string& filePath) {
        return fs::exists(filePath);
    }
    
    // Generate output path for compressed file
    static std::string getCompressedPath(const std::string& inputPath, const std::string& outputDir = "") {
        fs::path path(inputPath);
        fs::path outPath;
        
        if (outputDir.empty()) {
            outPath = path.parent_path() / (path.filename().string() + ".huf");
        } else {
            fs::path outDir(outputDir);
            outPath = outDir / (path.filename().string() + ".huf");
        }
        
        return outPath.string();
    }
    
    // Generate output path for decompressed file
    static std::string getDecompressedPath(const std::string& inputPath, const std::string& outputDir = "") {
        fs::path path(inputPath);
        std::string filename = path.filename().string();
        
        // Remove .huf extension if present
        if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".huf") {
            filename = filename.substr(0, filename.size() - 4);
        } else {
            // Add prefix for disambiguation
            filename = "decompressed_" + filename;
        }
        
        fs::path outPath;
        if (outputDir.empty()) {
            outPath = path.parent_path() / filename;
        } else {
            fs::path outDir(outputDir);
            outPath = outDir / filename;
        }
        
        return outPath.string();
    }
    
    // List files in directory
    static std::vector<std::string> listFiles(const std::string& directory, bool recursive = false) {
        std::vector<std::string> files;
        
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory: " + directory);
        }
        
        if (recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                if (fs::is_regular_file(entry)) {
                    files.push_back(entry.path().string());
                }
            }
        } else {
            for (const auto& entry : fs::directory_iterator(directory)) {
                if (fs::is_regular_file(entry)) {
                    files.push_back(entry.path().string());
                }
            }
        }
        
        return files;
    }
};

#endif // FILE_PROCESSOR_H