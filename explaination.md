# File-Shrink: Comprehensive Technical Explanation

This document provides a detailed explanation of the File-Shrink compression system, covering the core compression algorithm, file operations, encryption, batch processing, and performance analysis components.

## Table of Contents
- [Introduction](#introduction)
- [Core Algorithm: Huffman Coding](#core-algorithm-huffman-coding)
- [Compression and Decompression Process](#compression-and-decompression-process)
- [File Management](#file-management)
- [Security: AES-256 Encryption](#security-aes-256-encryption)
- [Parallel Processing](#parallel-processing)
- [Performance Analysis](#performance-analysis)
- [Main Application Structure](#main-application-structure)
- [Technical Design Decisions](#technical-design-decisions)
- [Algorithms Used](#algorithms-used)

## Introduction

File-Shrink is a comprehensive file compression utility that implements Huffman coding with advanced features including encryption, batch processing, and performance analytics. The system consists of modular components working together to provide efficient lossless compression.

## Core Algorithm: Huffman Coding

### What is Huffman Coding?

Huffman coding is an entropy-based compression technique that assigns variable-length codes to input characters, with shorter codes for more frequent characters. This minimizes the average code length, resulting in efficient data compression.

For example, in English text, 'e' appears more frequently than 'z', so 'e' might get a shorter code (like '10') while 'z' might get a longer one (like '000111').

### HuffmanNode Structure

The core data structure is the HuffmanNode:

```cpp
struct HuffmanNode {
    unsigned char byte;
    uint64_t frequency;
    std::shared_ptr<HuffmanNode> left;
    std::shared_ptr<HuffmanNode> right;
    
    // Constructors and methods...
    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }
};
```

Each node is either:
- A leaf node representing an actual byte from the input
- An internal node combining two other nodes (with frequency equal to the sum of its children)

### Bit Manipulation Utilities

Since Huffman codes are variable-length bit sequences, special classes handle bit-level operations:

#### BitWriter
```cpp
class BitWriter {
    // Writes individual bits or bit sequences
    // Packs bits into bytes
    // Maintains internal buffer of completed bytes
};
```

#### BitReader
```cpp
class BitReader {
    // Reads individual bits from a byte array
    // Keeps track of current bit and byte position
    // Checks if more bits are available
};
```

## Compression and Decompression Process

### The HuffmanCompressor Class

This class implements the actual compression and decompression algorithms:

```cpp
class HuffmanCompressor {
    // Compresses and decompresses data using Huffman coding
};
```

### Compression Steps

1. **Frequency Analysis**:
```cpp
FrequencyMap countFrequencies(const std::vector<unsigned char>& data) {
    // Count how often each byte appears in the input
}
```

2. **Build the Huffman Tree**:
```cpp
HuffmanTree buildHuffmanTree(const FrequencyMap& frequencies) {
    // Create leaf nodes for each byte
    // Use priority queue to combine nodes with lowest frequencies
    // Create the optimal prefix code tree
}
```

3. **Generate Codes**:
```cpp
void generateCodes(const HuffmanTree& tree, CodeMap& codes, std::string code = "") {
    // Traverse the tree to generate codes for each byte
    // '0' for left branches, '1' for right branches
}
```

4. **Serialize the Tree and Compress Data**:
```cpp
std::vector<unsigned char> compress(const std::vector<unsigned char>& data) {
    // Count frequencies
    // Build Huffman tree
    // Generate codes
    // Write header (original size + serialized tree)
    // Encode each byte using its Huffman code
    // Return compressed data
}
```

### Decompression Steps

1. **Parse Header and Rebuild Tree**:
```cpp
HuffmanTree deserializeTree(BitReader& reader) {
    // Reconstruct the Huffman tree from the bit sequence
}
```

2. **Decode Data**:
```cpp
std::vector<unsigned char> decompress(const std::vector<unsigned char>& compressedData) {
    // Extract header size, header, and data sections
    // Read original size
    // Deserialize Huffman tree
    // Decode bits by traversing the tree
    // Return decompressed data
}
```

## File Management

The FileProcessor class handles all file input/output operations:

```cpp
class FileProcessor {
    // Methods for reading, writing, and managing files
};
```

### Key File Operations

```cpp
// Read entire file into memory
static std::vector<unsigned char> readFile(const std::string& filePath);

// Write data buffer to file
static void writeFile(const std::string& filePath, const std::vector<unsigned char>& data);

// Generate paths for compressed/decompressed files
static std::string getCompressedPath(const std::string& inputPath, const std::string& outputDir = "");
static std::string getDecompressedPath(const std::string& inputPath, const std::string& outputDir = "");

// List files in directory
static std::vector<std::string> listFiles(const std::string& directory, bool recursive = false);
```

The class handles directory creation, path construction, and file existence checks, providing a robust interface for file operations.

## Security: AES-256 Encryption

The EncryptionModule provides security features using the AES-256 algorithm:

```cpp
class EncryptionModule {
    // Methods for encrypting and decrypting data
};
```

### Encryption Process

```cpp
std::vector<unsigned char> encrypt(const std::vector<unsigned char>& data, const std::string& password) {
    // Generate random salt
    // Derive encryption key using PBKDF2
    // Generate random IV (Initialization Vector)
    // Encrypt data using AES-256-CBC
    // Return [salt][IV][encrypted data]
}
```

### Decryption Process

```cpp
std::vector<unsigned char> decrypt(const std::vector<unsigned char>& encryptedData, const std::string& password) {
    // Extract salt and IV
    // Derive key from password and salt
    // Decrypt data using AES-256-CBC
    // Return original data
}
```

Security features include:
- AES-256 encryption (considered quantum-resistant)
- PBKDF2 key derivation with 10,000 iterations
- Random salt and IV for each encryption
- Proper padding using PKCS#7

## Parallel Processing

The BatchProcessor implements multi-threaded processing for improved performance:

```cpp
class BatchProcessor {
    // Thread pool implementation for parallel file processing
};
```

### Thread Pool Design

```cpp
// Thread pool with fixed number of worker threads
BatchProcessor(size_t threads = 0);

// Process multiple files in parallel
template<typename Func>
void processBatch(const std::vector<std::string>& filePaths, Func processFunc,
                  std::function<void(const std::string&, int, int)> progressCallback = nullptr);
```

The implementation uses:
- Worker threads that wait for tasks
- Thread-safe task queue
- Condition variables for efficient thread waiting
- Atomic counters for progress tracking

### Processing Flow

1. Create thread pool with specified number of threads (default: CPU core count)
2. Submit each file as a task to the queue
3. Workers process files in parallel
4. Main thread waits for all tasks to complete
5. Progress is reported via callback function

## Performance Analysis

The PerformanceAnalyzer tracks and reports compression metrics:

```cpp
class PerformanceAnalyzer {
    // Methods for performance measurement and reporting
};
```

### Timer Implementation

```cpp
class Timer {
    // Nested class for timing operations
    // Uses RAII pattern - timing starts on construction and ends on destruction
};
```

### Performance Metrics

```cpp
struct Metric {
    double duration;      // in milliseconds
    size_t originalSize;
    size_t compressedSize;
    
    double getCompressionRatio() const;
    double getThroughput() const;
};
```

The analyzer provides:
- Per-file timing and size information
- Compression ratios
- Throughput in MB/s
- Detailed performance reports

## Main Application Structure

The main program (HuffmanCompress.cpp) ties everything together:

### Command Line Options

```cpp
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
```

### Main Processing Flow

1. Parse command line arguments
2. Expand paths (handle wildcards and directories)
3. Select operation mode (single/batch, compress/decompress)
4. Process files with appropriate methods
5. Generate performance reports

### Compression Function

```cpp
void compressFile(const std::string& inputPath, const std::string& outputPath, 
                 const std::string& password, bool verbose, PerformanceAnalyzer& analyzer) {
    // Read input file
    // Compress data
    // Encrypt if password provided
    // Write output file
}
```

### Decompression Function

```cpp
void decompressFile(const std::string& inputPath, const std::string& outputPath, 
                   const std::string& password, bool verbose, PerformanceAnalyzer& analyzer) {
    // Read input file
    // Decrypt if password provided
    // Decompress data
    // Write output file
}
```

## Technical Design Decisions

### Memory Management

The system uses modern C++ memory management:
- `std::shared_ptr` for tree nodes and shared resources
- RAII pattern for resource acquisition and release
- Automatic cleanup even when exceptions occur

### Error Handling

The code implements comprehensive error handling:
- Input validation before operations
- Descriptive exception messages
- Graceful recovery from errors in batch processing
- Proper cleanup of resources

### Efficiency Considerations

Several optimizations enhance performance:
- Pre-allocation of buffers with reserve()
- Efficient bit packing for minimal storage overhead
- Thread pool for parallel processing
- Move semantics for efficient data transfer

## Algorithms Used

### Core Compression Algorithms
- **Huffman Coding Algorithm**: Primary lossless data compression technique
- **Shannon-Fano-Elias Coding**: Variant implementation in the Huffman module

### Cryptographic Algorithms
- **AES-256**: Block cipher encryption algorithm
- **PKCS#7 Padding**: Block padding scheme for cryptographic functions
- **PBKDF2**: Password-based key derivation for secure key generation

### Data Processing Algorithms
- **Producer-Consumer Pattern**: For efficient batch file processing
- **Work Stealing Algorithm**: Dynamic load balancing in thread pool

### Search and File System Algorithms
- **Glob Pattern Matching**: For wildcard expansion in batch operations
- **Breadth-First Directory Traversal**: For filesystem operations

### Performance Analysis Algorithms
- **Moving Average Algorithm**: For performance metrics calculation
- **Amdahl's Law Analysis**: For parallelization efficiency estimation

### Utility Algorithms
- **Binary Bit Packing**: For efficient bit-level data storage
- **Prefix-Free Code Generation**: Within the Huffman implementation
- **Command Line Argument Parsing**: For option processing
- **Exponential Backoff Algorithm**: For thread synchronization

## Conclusion

File-Shrink demonstrates a comprehensive approach to file compression, combining the efficient Huffman coding algorithm with modern features like encryption, parallel processing, and detailed analytics. The modular design allows for easy maintenance and future extensions.

The system balances efficiency, security, and usability, providing a versatile tool for reducing file sizes while maintaining data integrity and offering protection through strong encryption.
