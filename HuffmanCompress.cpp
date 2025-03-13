#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <filesystem>
#include <thread>
#include <iomanip>
#include <cstdlib>
#include <cstring>

#include "header/HuffmanCore.h"
#include "header/HuffmanCompressor.h"
#include "header/EncryptionModule.h"
#include "header/BatchProcessor.h"
#include "header/FileProcessor.h"
#include "header/PerformanceAnalyzer.h"

namespace fs = std::filesystem;

// Application version
const char* VERSION = "1.0.0";

// Command line options structure
struct Options {
    bool encrypt = false;
    bool batch = false;
    bool verbose = false;
    bool decompress = false;
    bool help = false;
    bool version = false;
    int threads = 0;  // 0 means use all available threads
    std::string outputDir;
    std::vector<std::string> inputPaths;
    std::string password;
};

// Progress bar implementation
class ProgressBar {
private:
    size_t width;
    char completeChar;
    char incompleteChar;

public:
    ProgressBar(size_t w = 50, char complete = '=', char incomplete = ' ')
        : width(w), completeChar(complete), incompleteChar(incomplete) {}

    void display(double percentage, const std::string& prefix = "") {
        int pos = width * percentage;
        
        std::cout << "\r" << prefix << " [";
        for (size_t i = 0; i < width; ++i) {
            if (i < pos) std::cout << completeChar;
            else std::cout << incompleteChar;
        }
        
        std::cout << "] " << std::fixed << std::setprecision(1) << (percentage * 100.0) << "%"
                  << " " << std::flush;
    }
    
    void complete(const std::string& message = "Completed") {
        display(1.0);
        std::cout << " " << message << std::endl;
    }
};

// Parse command line arguments
Options parseArguments(int argc, char* argv[]) {
    Options options;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-e" || arg == "--encrypt") {
            options.encrypt = true;
        } else if (arg == "-b" || arg == "--batch") {
            options.batch = true;
        } else if (arg == "-v" || arg == "--verbose") {
            options.verbose = true;
        } else if (arg == "-d" || arg == "--decompress") {
            options.decompress = true;
        } else if (arg == "-h" || arg == "--help") {
            options.help = true;
        } else if (arg == "--version") {
            options.version = true;
        } else if (arg == "-t" || arg == "--threads") {
            if (i + 1 < argc) {
                options.threads = std::atoi(argv[++i]);
                if (options.threads <= 0) {
                    std::cerr << "Invalid thread count: " << options.threads << std::endl;
                    options.threads = 0;
                }
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                options.outputDir = argv[++i];
            }
        } else if (arg == "-p" || arg == "--password") {
            if (i + 1 < argc) {
                options.password = argv[++i];
            }
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << std::endl;
        } else {
            options.inputPaths.push_back(arg);
        }
    }
    
    return options;
}

// Expand wildcards in input paths
std::vector<std::string> expandPaths(const std::vector<std::string>& paths) {
    std::vector<std::string> expandedPaths;
    
    for (const auto& path : paths) {
        if (path.find('*') != std::string::npos || path.find('?') != std::string::npos) {
            // Path contains wildcards
            fs::path dirPath = fs::path(path).parent_path();
            std::string pattern = fs::path(path).filename().string();
            
            if (dirPath.empty()) dirPath = ".";
            
            try {
                for (const auto& entry : fs::directory_iterator(dirPath)) {
                    // Simple pattern matching
                    std::string filename = entry.path().filename().string();
                    
                    bool matches = true;
                    size_t patternIdx = 0;
                    size_t filenameIdx = 0;
                    
                    while (patternIdx < pattern.size() && filenameIdx < filename.size()) {
                        if (pattern[patternIdx] == '*') {
                            // Skip multiple characters
                            patternIdx++;
                            
                            if (patternIdx >= pattern.size()) {
                                // * at the end matches everything
                                matches = true;
                                break;
                            }
                            
                            // Find the next char after * in filename
                            while (filenameIdx < filename.size() && 
                                   filename[filenameIdx] != pattern[patternIdx] &&
                                   pattern[patternIdx] != '?') {
                                filenameIdx++;
                            }
                            
                        } else if (pattern[patternIdx] == '?' || 
                                  pattern[patternIdx] == filename[filenameIdx]) {
                            // Match single character
                            patternIdx++;
                            filenameIdx++;
                        } else {
                            matches = false;
                            break;
                        }
                    }
                    
                    // Check if we matched the entire pattern
                    if (matches && (patternIdx >= pattern.size() || 
                                   (patternIdx == pattern.size() - 1 && pattern[patternIdx] == '*'))) {
                        expandedPaths.push_back(entry.path().string());
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error expanding wildcard: " << e.what() << std::endl;
            }
        } else if (fs::is_directory(path)) {
            // Add all files in directory
            try {
                for (const auto& entry : fs::directory_iterator(path)) {
                    if (fs::is_regular_file(entry)) {
                        expandedPaths.push_back(entry.path().string());
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error reading directory: " << e.what() << std::endl;
            }
        } else {
            // Regular file path
            expandedPaths.push_back(path);
        }
    }
    
    return expandedPaths;
}

// Display help information
void showHelp() {
    std::cout << "Huffman Compression System v" << VERSION << "\n\n"
              << "Usage: huffman_compress [options] input_file(s)\n\n"
              << "Options:\n"
              << "  -e, --encrypt       Enable AES-256 encryption\n"
              << "  -d, --decompress    Decompress the input file(s)\n"
              << "  -b, --batch         Enable batch processing mode\n"
              << "  -t, --threads N     Specify number of threads for batch processing\n"
              << "  -o, --output DIR    Specify output directory\n"
              << "  -p, --password PWD  Specify encryption password\n"
              << "  -v, --verbose       Enable detailed logging\n"
              << "  -h, --help          Display this help information\n"
              << "  --version           Display version information\n\n"
              << "Examples:\n"
              << "  huffman_compress file.txt             # Compress file.txt\n"
              << "  huffman_compress -d file.txt.huf      # Decompress file.txt.huf\n"
              << "  huffman_compress -e -p mypass file.txt # Compress with encryption\n"
              << "  huffman_compress -b -t 4 *.txt        # Batch compress all .txt files\n"
              << "  huffman_compress -d -b *.huf          # Batch decompress all .huf files\n"
              << std::endl;
}

// Display version
void showVersion() {
    std::cout << "Huffman Compression System v" << VERSION << std::endl;
}

// Prompt for password if encryption is enabled but no password was provided
std::string promptPassword() {
    std::string password;
    std::cout << "Enter encryption password: ";
    std::getline(std::cin, password);
    return password;
}

// Compress a single file
void compressFile(const std::string& inputPath, const std::string& outputPath, 
                 const std::string& password, bool verbose, PerformanceAnalyzer& analyzer) {
    try {
        // Read input file
        std::vector<unsigned char> data = FileProcessor::readFile(inputPath);
        size_t originalSize = data.size();
        
        if (verbose) {
            std::cout << "Compressing " << inputPath << " (" << originalSize << " bytes)..." << std::endl;
        }
        
        // Create timer for compression
        auto timer = analyzer.startTimer("Compress: " + inputPath, originalSize);
        
        // Compress data
        HuffmanCompressor compressor;
        std::vector<unsigned char> compressed = compressor.compress(data);
        
        // Encrypt if password provided
        if (!password.empty()) {
            if (verbose) {
                std::cout << "Encrypting data..." << std::endl;
            }
            
            EncryptionModule encryptor;
            compressed = encryptor.encrypt(compressed, password);
        }
        
        // Update timer with compressed size
        timer->setCompressedSize(compressed.size());
        
        // Write output file
        FileProcessor::writeFile(outputPath, compressed);
        
        if (verbose) {
            double ratio = 100.0 * (1.0 - static_cast<double>(compressed.size()) / originalSize);
            std::cout << "Compressed to " << outputPath << " (" << compressed.size() 
                      << " bytes, " << std::fixed << std::setprecision(2) << ratio 
                      << "% reduction)" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error compressing " << inputPath << ": " << e.what() << std::endl;
        throw;
    }
}

// Decompress a single file
void decompressFile(const std::string& inputPath, const std::string& outputPath, 
                   const std::string& password, bool verbose, PerformanceAnalyzer& analyzer) {
    try {
        // Read input file
        std::vector<unsigned char> data = FileProcessor::readFile(inputPath);
        size_t compressedSize = data.size();
        
        if (verbose) {
            std::cout << "Decompressing " << inputPath << " (" << compressedSize << " bytes)..." << std::endl;
        }
        
        // Decrypt if password provided
        if (!password.empty()) {
            if (verbose) {
                std::cout << "Decrypting data..." << std::endl;
            }
            
            EncryptionModule decryptor;
            try {
                data = decryptor.decrypt(data, password);
            } catch (const std::exception& e) {
                throw std::runtime_error("Decryption failed: Invalid password or corrupted file");
            }
        }
        
        // Create timer for decompression
        auto timer = analyzer.startTimer("Decompress: " + inputPath, 0, compressedSize);
        
        // Decompress data
        HuffmanCompressor compressor;
        std::vector<unsigned char> decompressed = compressor.decompress(data);
        
        // Update timer with decompressed size
        timer->setCompressedSize(decompressed.size());
        
        // Write output file
        FileProcessor::writeFile(outputPath, decompressed);
        
        if (verbose) {
            std::cout << "Decompressed to " << outputPath << " (" << decompressed.size() 
                      << " bytes)" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error decompressing " << inputPath << ": " << e.what() << std::endl;
        throw;
    }
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    Options options = parseArguments(argc, argv);
    
    // Show help or version if requested
    if (options.help) {
        showHelp();
        return 0;
    }
    
    if (options.version) {
        showVersion();
        return 0;
    }
    
    // Check for input paths
    if (options.inputPaths.empty()) {
        std::cerr << "Error: No input files specified." << std::endl;
        std::cerr << "Use --help for usage information." << std::endl;
        return 1;
    }
    
    // Prompt for password if encryption is enabled but no password was provided
    if (options.encrypt && options.password.empty()) {
        options.password = promptPassword();
    }
    
    // Create output directory if specified
    if (!options.outputDir.empty() && !fs::exists(options.outputDir)) {
        try {
            fs::create_directories(options.outputDir);
        } catch (const std::exception& e) {
            std::cerr << "Error creating output directory: " << e.what() << std::endl;
            return 1;
        }
    }
    
    // Expand input paths (handle wildcards and directories)
    std::vector<std::string> expandedPaths = expandPaths(options.inputPaths);
    
    if (expandedPaths.empty()) {
        std::cerr << "Error: No files match the specified pattern." << std::endl;
        return 1;
    }
    
    // Create performance analyzer
    PerformanceAnalyzer analyzer;
    
    if (options.batch) {
        // Batch processing mode
        BatchProcessor processor(options.threads);
        
        if (options.verbose) {
            std::cout << "Batch processing " << expandedPaths.size() << " files using " 
                      << processor.getThreadCount() << " threads..." << std::endl;
        }
        
        ProgressBar progressBar;
        
        // Define progress callback
        auto progressCallback = [&progressBar, &expandedPaths](
            const std::string& path, int current, int total) {
            double progress = static_cast<double>(current) / total;
            progressBar.display(progress, "Processing files");
        };
        
        // Process each file
        try {
            processor.processBatch(expandedPaths, 
                [&options, &analyzer](const std::string& path) {
                    if (options.decompress) {
                        // Decompress
                        std::string outputPath = FileProcessor::getDecompressedPath(path, options.outputDir);
                        decompressFile(path, outputPath, options.password, options.verbose, analyzer);
                    } else {
                        // Compress
                        std::string outputPath = FileProcessor::getCompressedPath(path, options.outputDir);
                        compressFile(path, outputPath, options.password, options.verbose, analyzer);
                    }
                }, 
                progressCallback);
            
            progressBar.complete();
            
        } catch (const std::exception& e) {
            std::cerr << "\nBatch processing error: " << e.what() << std::endl;
            return 1;
        }
    } else {
        // Single file mode
        try {
            if (options.decompress) {
                // Decompress single file
                std::string outputPath = FileProcessor::getDecompressedPath(expandedPaths[0], options.outputDir);
                decompressFile(expandedPaths[0], outputPath, options.password, options.verbose, analyzer);
            } else {
                // Compress single file
                std::string outputPath = FileProcessor::getCompressedPath(expandedPaths[0], options.outputDir);
                compressFile(expandedPaths[0], outputPath, options.password, options.verbose, analyzer);
            }
        } catch (const std::exception& e) {
            std::cerr << "Operation failed: " << e.what() << std::endl;
            return 1;
        }
    }
    
    // Display performance report
    if (options.verbose) {
        std::cout << "\nPerformance Analysis:" << std::endl;
        analyzer.generateReport();
    }
    
    return 0;
}