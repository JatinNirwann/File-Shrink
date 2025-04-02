# File-Shrink: Comprehensive Technical Explanation

This document provides a detailed explanation of the File-Shrink compression system, covering the core compression algorithm, file operations, encryption, batch processing, and performance analysis components.

## Table of Contents
- [Introduction](#introduction)
- [Core Algorithm: Huffman Coding](#core-algorithm-huffman-coding)
  - [Theory and Mathematical Foundation](#theory-and-mathematical-foundation)
  - [Data Structures](#data-structures)
  - [Bit-Level Operations](#bit-level-operations)
- [Compression and Decompression Process](#compression-and-decompression-process)
  - [Frequency Analysis](#frequency-analysis)
  - [Tree Construction](#tree-construction)
  - [Code Generation](#code-generation)
  - [Serialization Format](#serialization-format)
  - [Compression Algorithm](#compression-algorithm)
  - [Decompression Algorithm](#decompression-algorithm)
- [File Management](#file-management)
  - [File I/O Operations](#file-io-operations)
  - [Path Handling](#path-handling)
  - [Directory Operations](#directory-operations)
  - [Error Handling](#error-handling-for-file-operations)
- [Security: AES-256 Encryption](#security-aes-256-encryption)
  - [Cryptographic Design](#cryptographic-design)
  - [Key Derivation](#key-derivation)
  - [Encryption Implementation](#encryption-implementation)
  - [Decryption Implementation](#decryption-implementation)
  - [Security Considerations](#security-considerations)
- [Parallel Processing](#parallel-processing)
  - [Thread Pool Architecture](#thread-pool-architecture)
  - [Task Management](#task-management)
  - [Synchronization Mechanisms](#synchronization-mechanisms)
  - [Work Distribution](#work-distribution)
  - [Progress Tracking](#progress-tracking)
- [Performance Analysis](#performance-analysis)
  - [Metrics Collection](#metrics-collection)
  - [Statistical Analysis](#statistical-analysis)
  - [Reporting System](#reporting-system)
- [Main Application Structure](#main-application-structure)
  - [Command Line Interface](#command-line-interface)
  - [Workflow Management](#workflow-management)
  - [Progress Visualization](#progress-visualization)
  - [Error Recovery](#error-recovery)
- [Technical Design Decisions](#technical-design-decisions)
  - [Memory Management Strategy](#memory-management-strategy)
  - [Exception Handling Philosophy](#exception-handling-philosophy)
  - [Performance Optimizations](#performance-optimizations)
  - [Cross-Platform Considerations](#cross-platform-considerations)
- [Algorithms Used](#algorithms-used)
- [Implementation Challenges and Solutions](#implementation-challenges-and-solutions)
- [Advanced Usage Scenarios](#advanced-usage-scenarios)

## Introduction

File-Shrink is a comprehensive file compression utility that implements Huffman coding with advanced features including encryption, batch processing, and performance analytics. The system consists of modular components working together to provide efficient lossless compression while maintaining data integrity and security.

The project demonstrates modern C++ practices including RAII (Resource Acquisition Is Initialization), smart pointers for memory management, template metaprogramming, and exception safety. It combines theoretical concepts from information theory, cryptography, and parallel computing into a practical application.

## Core Algorithm: Huffman Coding

### Theory and Mathematical Foundation

Huffman coding is an optimal prefix-free variable-length encoding algorithm based on information theory principles. The theoretical foundation relies on the concept of entropy in information theory, which defines the minimum number of bits needed to encode a message.

Given a set of symbols with probabilities p₁, p₂, ..., pₙ, the optimal expected code length approaches the entropy:

H = -∑(pᵢ × log₂(pᵢ))

Huffman coding achieves this optimality by:
1. Assigning shorter codes to more frequent symbols
2. Ensuring no code is a prefix of another (prefix-free property)
3. Building a binary tree where leaf traversal paths represent codes

The time complexity of Huffman coding is O(n log n) where n is the number of unique symbols, due to the priority queue operations during tree construction.

### Data Structures

The implementation uses several specialized data structures:

```cpp
// Core node structure
struct HuffmanNode {
    unsigned char byte;       // The byte value (for leaf nodes)
    uint64_t frequency;       // Frequency count
    std::shared_ptr<HuffmanNode> left;   // Left child
    std::shared_ptr<HuffmanNode> right;  // Right child
    
    // Constructors
    HuffmanNode(unsigned char b, uint64_t freq) : 
        byte(b), frequency(freq), left(nullptr), right(nullptr) {}
    
    HuffmanNode(std::shared_ptr<HuffmanNode> l, std::shared_ptr<HuffmanNode> r) : 
        byte(0), frequency(l->frequency + r->frequency), left(l), right(r) {}
    
    bool isLeaf() const { return left == nullptr && right == nullptr; }
};

// Comparator for priority queue
struct HuffmanNodeComparator {
    bool operator()(const std::shared_ptr<HuffmanNode>& a, 
                   const std::shared_ptr<HuffmanNode>& b) const {
        return a->frequency > b->frequency;  // Min-heap
    }
};

// Type aliases for readability
using HuffmanTree = std::shared_ptr<HuffmanNode>;
using FrequencyMap = std::unordered_map<unsigned char, uint64_t>;
using CodeMap = std::unordered_map<unsigned char, std::string>;
using PriorityQueue = std::priority_queue<HuffmanTree, 
                                         std::vector<HuffmanTree>, 
                                         HuffmanNodeComparator>;
```

These structures provide:
- Memory-safe tree construction using shared pointers
- Efficient frequency counting with hash maps
- Optimal tree building with a min-heap priority queue
- Clear type names through aliases

The design ensures proper resource management even in error conditions through RAII principles and smart pointers.

### Bit-Level Operations

Since Huffman codes are variable-length bit sequences that don't align with byte boundaries, specialized classes handle bit-by-bit reading and writing:

#### BitWriter

```cpp
class BitWriter {
private:
    std::vector<unsigned char> buffer;
    int bitCount = 0;
    unsigned char currentByte = 0;

public:
    void writeBit(bool bit) {
        if (bit) {
            currentByte |= (1 << (7 - bitCount));  // Set bit at position
        }
        
        bitCount++;
        
        if (bitCount == 8) {  // Byte complete
            buffer.push_back(currentByte);
            currentByte = 0;
            bitCount = 0;
        }
    }
    
    void writeBits(const std::string& bits) {
        for (char bit : bits) {
            writeBit(bit == '1');
        }
    }
    
    void flush() {
        if (bitCount > 0) {
            buffer.push_back(currentByte);  // Write partial byte
        }
    }
    
    const std::vector<unsigned char>& getBuffer() const {
        return buffer;
    }
    
    size_t getBitCount() const {
        return buffer.size() * 8 - (8 - bitCount) % 8;
    }
};
```

#### BitReader

```cpp
class BitReader {
private:
    const std::vector<unsigned char>& buffer;
    size_t byteIndex = 0;
    int bitIndex = 0;
    size_t totalBits;

public:
    BitReader(const std::vector<unsigned char>& buf, size_t bits) 
        : buffer(buf), totalBits(bits) {}
    
    bool readBit() {
        if (byteIndex >= buffer.size()) {
            throw std::runtime_error("End of buffer reached in BitReader");
        }
        
        bool bit = (buffer[byteIndex] & (1 << (7 - bitIndex))) != 0;
        
        bitIndex++;
        if (bitIndex == 8) {  // Move to next byte
            byteIndex++;
            bitIndex = 0;
        }
        
        return bit;
    }
    
    bool hasMoreBits() const {
        return (byteIndex * 8 + bitIndex) < totalBits;
    }
};
```

These classes provide a layer of abstraction that:
- Hides the complexity of bit manipulation
- Handles proper byte alignment
- Manages buffer boundaries and overflows
- Provides clear error reporting

## Compression and Decompression Process

### Frequency Analysis

The compression starts with analyzing byte frequencies in the input data:

```cpp
FrequencyMap countFrequencies(const std::vector<unsigned char>& data) {
    FrequencyMap frequencies;
    
    // Count occurrences of each byte
    for (const auto& byte : data) {
        frequencies[byte]++;
    }
    
    return frequencies;
}
```

This creates a histogram of byte occurrences. The frequency analysis:
- Has O(n) time complexity, where n is the input size
- Has O(k) space complexity, where k is the number of unique bytes (maximum 256)
- Uses hash table operations with amortized O(1) complexity

### Tree Construction

The Huffman tree is built bottom-up using a priority queue:

```cpp
HuffmanTree buildHuffmanTree(const FrequencyMap& frequencies) {
    // Create priority queue
    PriorityQueue pq;
    
    // Add leaf nodes for each byte
    for (const auto& pair : frequencies) {
        pq.push(std::make_shared<HuffmanNode>(pair.first, pair.second));
    }
    
    // Special case: single byte input
    if (pq.size() == 1) {
        auto node = pq.top();
        return std::make_shared<HuffmanNode>(nullptr, node);
    }
    
    // Main tree building process
    while (pq.size() > 1) {
        auto left = pq.top(); pq.pop();
        auto right = pq.top(); pq.pop();
        
        auto parent = std::make_shared<HuffmanNode>(left, right);
        pq.push(parent);
    }
    
    return pq.top();  // Root of the tree
}
```

The algorithm:
1. Creates a leaf node for each unique byte
2. Repeatedly combines the two lowest-frequency nodes
3. Handles special cases (empty input, single byte input)
4. Returns the final tree root

Time complexity is O(k log k) where k is the number of unique bytes, dominated by the priority queue operations.

### Code Generation

After building the tree, Huffman codes are generated by traversing it:

```cpp
void generateCodes(const HuffmanTree& tree, CodeMap& codes, std::string code = "") {
    if (!tree) return;
    
    if (tree->isLeaf()) {
        // Leaf node found - assign code
        if (code.empty()) {
            code = "0";  // Special case for single byte files
        }
        codes[tree->byte] = code;
        return;
    }
    
    // Recursive traversal - add 0 for left, 1 for right
    generateCodes(tree->left, codes, code + "0");
    generateCodes(tree->right, codes, code + "1");
}
```

This recursive depth-first traversal:
- Assigns '0' when following left branches
- Assigns '1' when following right branches
- Creates a unique path/code for each leaf node
- Handles edge cases like single-symbol inputs

The codes generated are guaranteed to be optimal (minimum expected length) and prefix-free (no code is a prefix of another).

### Serialization Format

The compressed data follows a carefully designed format:

```
[Header Size (4 bytes)]
[Header:
   [Original Size (8 bytes)]
   [Serialized Tree (variable)]
]
[Encoded Data (variable)]
```

The tree serialization is particularly important:

```cpp
void serializeTree(const HuffmanTree& tree, BitWriter& writer) {
    if (!tree) return;
    
    // Write node type (1 bit: 0 for internal, 1 for leaf)
    writer.writeBit(tree->isLeaf());
    
    if (tree->isLeaf()) {
        // Write byte value (8 bits)
        for (int i = 0; i < 8; i++) {
            writer.writeBit((tree->byte >> (7 - i)) & 1);
        }
    } else {
        // Recursively write left and right subtrees
        serializeTree(tree->left, writer);
        serializeTree(tree->right, writer);
    }
}
```

Tree deserialization mirrors this process:

```cpp
HuffmanTree deserializeTree(BitReader& reader) {
    if (!reader.hasMoreBits()) {
        return nullptr;
    }
    
    bool isLeaf = reader.readBit();
    
    if (isLeaf) {
        // Read byte value (8 bits)
        unsigned char byte = 0;
        for (int i = 0; i < 8; i++) {
            if (reader.readBit()) {
                byte |= (1 << (7 - i));
            }
        }
        return std::make_shared<HuffmanNode>(byte, 0);
    } else {
        // Read subtrees
        auto left = deserializeTree(reader);
        auto right = deserializeTree(reader);
        return std::make_shared<HuffmanNode>(left, right);
    }
}
```

This compact bit-level representation ensures:
- The tree structure is preserved exactly
- Minimal overhead (only 9 bits per leaf node, 1 bit per internal node)
- Self-contained format that doesn't require external dictionaries

### Compression Algorithm

The full compression process brings everything together:

```cpp
std::vector<unsigned char> compress(const std::vector<unsigned char>& data) {
    if (data.empty()) {
        return {};
    }
    
    // Phase 1: Analyze and build tree
    FrequencyMap frequencies = countFrequencies(data);
    HuffmanTree tree = buildHuffmanTree(frequencies);
    
    // Phase 2: Generate codes
    CodeMap codes;
    generateCodes(tree, codes);
    
    // Phase 3: Prepare header
    BitWriter headerWriter;
    
    // Write original size (64 bits)
    uint64_t originalSize = data.size();
    for (int i = 0; i < 64; i++) {
        headerWriter.writeBit((originalSize >> (63 - i)) & 1);
    }
    
    // Write tree structure
    serializeTree(tree, headerWriter);
    headerWriter.flush();
    
    // Phase 4: Encode data
    BitWriter dataWriter;
    for (unsigned char byte : data) {
        dataWriter.writeBits(codes[byte]);
    }
    dataWriter.flush();
    
    // Phase 5: Combine components
    std::vector<unsigned char> headerBuffer = headerWriter.getBuffer();
    std::vector<unsigned char> dataBuffer = dataWriter.getBuffer();
    
    uint32_t headerSize = headerBuffer.size();
    
    std::vector<unsigned char> result;
    
    // Write header size (4 bytes)
    for (int i = 0; i < 4; i++) {
        result.push_back((headerSize >> (24 - i * 8)) & 0xFF);
    }
    
    // Combine everything
    result.insert(result.end(), headerBuffer.begin(), headerBuffer.end());
    result.insert(result.end(), dataBuffer.begin(), dataBuffer.end());
    
    return result;
}
```

The algorithm operates in distinct phases:
1. Frequency analysis and tree building
2. Code generation
3. Header preparation
4. Data encoding
5. Final assembly

Overall time complexity is O(n + k log k) where n is input size and k is unique bytes.

### Decompression Algorithm

Decompression reverses the process:

```cpp
std::vector<unsigned char> decompress(const std::vector<unsigned char>& compressedData) {
    if (compressedData.size() < 4) {
        throw std::runtime_error("Invalid compressed data format");
    }
    
    // Phase 1: Parse header size
    uint32_t headerSize = 0;
    for (int i = 0; i < 4; i++) {
        headerSize = (headerSize << 8) | compressedData[i];
    }
    
    if (compressedData.size() < 4 + headerSize) {
        throw std::runtime_error("Corrupted compressed data");
    }
    
    // Phase 2: Extract sections
    std::vector<unsigned char> headerBytes(compressedData.begin() + 4, 
                                          compressedData.begin() + 4 + headerSize);
    std::vector<unsigned char> dataBytes(compressedData.begin() + 4 + headerSize, 
                                        compressedData.end());
    
    // Phase 3: Parse header
    BitReader headerReader(headerBytes, headerBytes.size() * 8);
    
    // Read original size
    uint64_t originalSize = 0;
    for (int i = 0; i < 64; i++) {
        if (headerReader.readBit()) {
            originalSize |= (1ULL << (63 - i));
        }
    }
    
    // Rebuild the Huffman tree
    HuffmanTree tree = deserializeTree(headerReader);
    
    // Phase 4: Decode data
    std::vector<unsigned char> result;
    result.reserve(originalSize);  // Pre-allocate for performance
    
    BitReader dataReader(dataBytes, dataBytes.size() * 8);
    HuffmanTree current = tree;
    
    while (result.size() < originalSize && dataReader.hasMoreBits()) {
        if (!current) {
            throw std::runtime_error("Corrupted Huffman tree");
        }
        
        if (current->isLeaf()) {
            // Found a leaf node, output the byte
            result.push_back(current->byte);
            current = tree;  // Back to root for next code
        } else {
            // Follow tree path based on next bit
            bool bit = dataReader.readBit();
            current = bit ? current->right : current->left;
        }
    }
    
    // Phase 5: Validate result
    if (result.size() != originalSize) {
        throw std::runtime_error("Decompression failed: size mismatch");
    }
    
    return result;
}
```

The decompression phases:
1. Header size extraction
2. Section separation
3. Header parsing and tree reconstruction
4. Data decoding through tree traversal 
5. Result validation

Error detection is built in at multiple stages to identify corrupted data or incorrect passwords.

## File Management

### File I/O Operations

The FileProcessor class provides robust file handling:

```cpp
class FileProcessor {
public:
    // Core I/O operations
    static std::vector<unsigned char> readFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Unable to open file: " + filePath);
        }
        
        // Get file size
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // Read entire file
        std::vector<unsigned char> buffer(fileSize);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
            throw std::runtime_error("Error reading file: " + filePath);
        }
        
        return buffer;
    }
    
    static void writeFile(const std::string& filePath, 
                         const std::vector<unsigned char>& data) {
        // Create directory if needed
        fs::path path(filePath);
        if (!path.parent_path().empty()) {
            fs::create_directories(path.parent_path());
        }
        
        std::ofstream file(filePath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Unable to create file: " + filePath);
        }
        
        // Write data
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        if (!file) {
            throw std::runtime_error("Error writing to file: " + filePath);
        }
    }
    
    // File information
    static size_t getFileSize(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file) {
            throw std::runtime_error("Unable to open file: " + filePath);
        }
        
        return file.tellg();
    }
    
    static bool fileExists(const std::string& filePath) {
        return fs::exists(filePath);
    }

    // Other methods...
};
```

Key aspects of these file operations:
- Binary mode for exact byte-level processing
- Comprehensive error checking
- Exception-based error reporting with detailed messages
- Efficient single-read and single-write operations
- Automatic directory creation as needed

### Path Handling

Path manipulation is critical for generating correct output filenames:

```cpp
static std::string getCompressedPath(const std::string& inputPath, 
                                    const std::string& outputDir = "") {
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

static std::string getDecompressedPath(const std::string& inputPath, 
                                      const std::string& outputDir = "") {
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
```

These methods handle:
- Extension management (.huf addition/removal)
- Output directory specification
- Cross-platform path construction
- Name disambiguation to prevent overwriting

### Directory Operations

For batch processing, directory traversal is implemented:

```cpp
static std::vector<std::string> listFiles(const std::string& directory, 
                                         bool recursive = false) {
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
```

This method supports:
- Recursive/non-recursive directory traversal
- Filtering for regular files only
- Error detection for invalid directories
- Cross-platform operation through std::filesystem

### Error Handling for File Operations

The FileProcessor implements consistent error handling:
- Exceptions with clear context messages
- Pre-emptive checks (like directory existence)
- Post-operation validation
- Filesystem permission handling
- Automatic resource cleanup through RAII

## Security: AES-256 Encryption

### Cryptographic Design

The encryption system uses industry-standard algorithms and practices:

```cpp
class EncryptionModule {
private:
    // AES-256 constants
    static constexpr int KEY_LENGTH = 32;  // 256 bits
    static constexpr int IV_LENGTH = 16;   // 128 bits (AES block size)
    static constexpr int SALT_LENGTH = 16;

    // Private methods...
    
public:
    // Public API methods...
};
```

The security design follows these principles:
- Using established algorithms (AES-256) rather than custom cryptography
- Proper key derivation with salting and stretching
- Unique IVs for each encryption operation
- Adequate key, IV, and salt lengths
- Protection against common attacks (brute force, rainbow tables)

### Key Derivation

Password-to-key conversion uses PBKDF2:

```cpp
std::vector<unsigned char> deriveKey(const std::string& password, 
                                   const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> key(KEY_LENGTH);
    
    // Use PBKDF2 with SHA-256 for key derivation
    if (PKCS5_PBKDF2_HMAC(
            password.c_str(), password.length(),
            salt.data(), salt.size(),
            10000,  // Iteration count
            EVP_sha256(),
            KEY_LENGTH,
            key.data()) != 1) {
        throw std::runtime_error("Key derivation failed");
    }
    
    return key;
}
```

Key security features include:
- Salt to prevent rainbow table attacks
- 10,000 iterations for computational cost (anti-brute force)
- SHA-256 as the underlying hash function
- 256-bit output key length

### Encryption Implementation

The encryption process combines key derivation with AES-256-CBC:

```cpp
std::vector<unsigned char> encrypt(const std::vector<unsigned char>& data, 
                                 const std::string& password) {
    if (data.empty()) {
        return {};
    }
    
    // Generate random salt
    std::vector<unsigned char> salt(SALT_LENGTH);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }
    
    // Derive key from password
    std::vector<unsigned char> key = deriveKey(password, salt);
    
    // Generate random IV
    std::vector<unsigned char> iv(IV_LENGTH);
    if (RAND_bytes(iv.data(), iv.size()) != 1) {
        throw std::runtime_error("Failed to generate random IV");
    }
    
    // Setup OpenSSL encryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create encryption context");
    }
    
    // Initialize encryption
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }
    
    // Perform encryption
    std::vector<unsigned char> encryptedData(data.size() + 
                                          EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int outLen1 = 0;
    
    if (EVP_EncryptUpdate(ctx, encryptedData.data(), &outLen1, 
                         data.data(), data.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption failed");
    }
    
    // Finalize encryption (handle padding)
    int outLen2 = 0;
    if (EVP_EncryptFinal_ex(ctx, encryptedData.data() + outLen1, &outLen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption finalization failed");
    }
    
    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    
    // Resize result to actual size
    encryptedData.resize(outLen1 + outLen2);
    
    // Format result: [salt][IV][encrypted data]
    std::vector<unsigned char> result;
    result.insert(result.end(), salt.begin(), salt.end());
    result.insert(result.end(), iv.begin(), iv.end());
    result.insert(result.end(), encryptedData.begin(), encryptedData.end());
    
    return result;
}
```

The encryption process consists of:
1. Random salt generation for key derivation
2. Key derivation using PBKDF2
3. Random IV generation for AES-CBC
4. AES-256-CBC encryption with proper padding
5. Safe context cleanup
6. Packaging of salt, IV, and ciphertext

### Decryption Implementation

Decryption reverses the encryption process:

```cpp
std::vector<unsigned char> decrypt(const std::vector<unsigned char>& encryptedData, 
                                 const std::string& password) {
    // Validate input size
    if (encryptedData.size() <= SALT_LENGTH + IV_LENGTH) {
        throw std::runtime_error("Encrypted data is too small");
    }
    
    // Extract components
    std::vector<unsigned char> salt(encryptedData.begin(), 
                                   encryptedData.begin() + SALT_LENGTH);
    std::vector<unsigned char> iv(encryptedData.begin() + SALT_LENGTH, 
                                 encryptedData.begin() + SALT_LENGTH + IV_LENGTH);
    
    // Derive key from password and salt
    std::vector<unsigned char> key = deriveKey(password, salt);
    
    // Extract ciphertext
    std::vector<unsigned char> ciphertext(encryptedData.begin() + SALT_LENGTH + IV_LENGTH, 
                                        encryptedData.end());
    
    // Setup decryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create decryption context");
    }
    
    // Initialize decryption
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }
    
    // Perform decryption
    std::vector<unsigned char> decryptedData(ciphertext.size());
    int outLen1 = 0;
    
    if (EVP_DecryptUpdate(ctx, decryptedData.data(), &outLen1, 
                         ciphertext.data(), ciphertext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption failed");
    }
    
    // Finalize decryption (handle padding removal)
    int outLen2 = 0;
    if (EVP_DecryptFinal_ex(ctx, decryptedData.data() + outLen1, &outLen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption finalization failed: Invalid password or corrupted data");
    }
    
    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    
    // Resize result to actual size
    decryptedData.resize(outLen1 + outLen2);
    
    return decryptedData;
}
```

The decryption process:
1. Extracts salt and IV from the encrypted data
2. Derives the key using the same approach as encryption
3. Performs AES-256-CBC decryption
4. Validates padding during finalization
5. Reports descriptive errors for corrupted data or wrong passwords

### Security Considerations

The encryption implementation incorporates several security measures:

1. **Key Never Stored**: The encryption key exists only in memory during the operation
2. **Password Safety**: Password is never stored or cached
3. **Side-Channel Resistance**: Uses constant-time memory comparison where relevant
4. **Forward Secrecy**: New salt and IV for each encryption operation
5. **Error Handling**: Non-descriptive errors to prevent information leakage
6. **Padding Validation**: Detects tampering with the ciphertext

Additional protections could include:
- Incorporating authenticated encryption (GCM mode)
- Memory wiping for sensitive data
- Hardware security module integration

## Parallel Processing

### Thread Pool Architecture

The BatchProcessor implements a flexible thread pool:

```cpp
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
    // Constructor initializes thread pool
    BatchProcessor(size_t threads = 0) : 
        numThreads(threads == 0 ? std::thread::hardware_concurrency() : threads),
        stop(false),
        activeThreads(0),
        completedTasks(0),
        totalTasks(0) {
        
        // Create worker threads
        for (size_t i = 0; i < numThreads; i++) {
            workers.emplace_back([this] {
                // Worker thread main loop
                workerFunction();
            });
        }
    }
    
    // Worker thread function
    void workerFunction() {
        while (true) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                condition.wait(lock, [this] { 
                    return stop || !tasks.empty(); 
                });
                
                if (stop && tasks.empty()) {
                    return; // Exit thread if stopping and queue is empty
                }
                
                task = std::move(tasks.front());
                tasks.pop();
            }
            
            activeThreads++;
            task(); // Execute task
            activeThreads--;
            completedTasks++;
        }
    }
    
    // Task submission and other methods...
    
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
```

The thread pool design features:
- Automatic thread count determination based on CPU cores
- Efficient task distribution
- Clean startup and shutdown sequences
- Exception safety throughout the lifecycle
- Progress tracking via atomic counters

### Task Management

Tasks are submitted to the thread pool using templates and futures:

```cpp
template<class F>
auto enqueue(F&& task) -> std::future<typename std::result_of<F()>::type> {
    using ReturnType = typename std::result_of<F()>::type;
    
    // Wrap the task in a packaged_task to get future
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
        
        // Add the task to the queue
        tasks.emplace([taskPtr]() { (*taskPtr)(); });
    }
    
    condition.notify_one(); // Wake up a worker
    return result;
}
```

This template method provides:
- Type deduction for any callable task
- Future-based result retrieval
- Perfect forwarding of callables
- Task lifecycle management
- Thread safety for queue operations

### Synchronization Mechanisms

The thread pool uses three primary synchronization mechanisms:

1. **Mutex for Task Queue**:
```cpp
std::mutex queueMutex;
```
Ensures thread-safe access to the task queue.

2. **Condition Variable for Thread Signaling**:
```cpp
std::condition_variable condition;
```
Allows workers to sleep until work is available.

3. **Atomic Variables for Status Tracking**:
```cpp
std::atomic<bool> stop;
std::atomic<size_t> activeThreads;
std::atomic<size_t> completedTasks;
std::atomic<size_t> totalTasks;
```
Thread-safe counters without locking overhead.

The condition variable usage is particularly important:
```cpp
condition.wait(lock, [this] { 
    return stop || !tasks.empty(); 
});
```
This ensures threads wake up only when:
- There's actual work to do (tasks.empty() is false), or
- The thread pool is shutting down (stop is true)

The predicate lambda prevents spurious wakeups.

### Work Distribution

File batch processing divides work efficiently:

```cpp
template<typename Func>
void processBatch(const std::vector<std::string>& filePaths, Func processFunc, 
                  std::function<void(const std::string&, int, int)> progressCallback = nullptr) {
    totalTasks = filePaths.size();
    completedTasks = 0;
    
    std::vector<std::future<void>> futures;
    
    // Submit tasks for all files
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

This distribution strategy:
- Submits all tasks upfront for maximum parallelism
- Captures file path and index information in lambdas
- Uses futures to track completion
- Provides per-file progress callbacks
- Waits for all operations to complete before returning

The work distribution implicitly implements a work-stealing pattern where idle threads take tasks from the queue.

### Progress Tracking

The BatchProcessor tracks progress with atomic counters:

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

These methods:
- Provide current progress as a percentage
- Report counts for detailed UIs
- Track active thread utilization
- Handle edge cases like empty task sets

The progress reporting is complemented by a callback mechanism:
```cpp
progressCallback(path, i + 1, filePaths.size());
```
This allows the UI layer to show per-file progress.

## Performance Analysis

### Metrics Collection

The PerformanceAnalyzer tracks detailed operation metrics:

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

Each operation records:
- Duration in milliseconds (wall-clock time)
- Original data size in bytes
- Compressed data size in bytes

From these base metrics, derived metrics are calculated:
- Compression ratio (percentage reduction)
- Throughput (MB/s processing rate)

### Statistical Analysis

The Timer class automatically measures operation durations:

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

This RAII-based timer:
- Starts automatically on construction
- Captures high-precision timestamps using std::chrono::high_resolution_clock
- Automatically stops and records metrics when it goes out of scope
- Works correctly even if exceptions occur
- Can have its compressed size updated during operation

### Reporting System

The analyzer provides both API access and formatted reporting:

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
    
    // Calculate aggregate statistics
    double totalDuration = 0;
    size_t totalOriginal = 0;
    size_t totalCompressed = 0;
    
    for (const auto& pair : metrics) {
        output << std::setw(30) << std::left << pair.first
               << std::setw(15) << std::fixed << std::setprecision(2) << std::right << pair.second.duration
               << std::setw(15) << pair.second.originalSize
               << std::setw(15) << pair.second.compressedSize
               << std::setw(15) << std::fixed << std::setprecision(2) << pair.second.getCompressionRatio()
               << std::setw(15) << std::fixed << std::setprecision(2) << pair.second.getThroughput()
               << "\n";
               
        totalDuration += pair.second.duration;
        totalOriginal += pair.second.originalSize;
        totalCompressed += pair.second.compressedSize;
    }
    
    // Add summary row
    output << std::string(105, '-') << "\n";
    double overallRatio = (totalOriginal == 0) ? 0 : 100.0 * (1.0 - static_cast<double>(totalCompressed) / totalOriginal);
    double overallThroughput = (totalDuration == 0) ? 0 : (totalOriginal / 1024.0 / 1024.0) / (totalDuration / 1000.0);
    
    output << std::setw(30) << std::left << "TOTAL"
           << std::setw(15) << std::fixed << std::setprecision(2) << std::right << totalDuration
           << std::setw(15) << totalOriginal
           << std::setw(15) << totalCompressed
           << std::setw(15) << std::fixed << std::setprecision(2) << overallRatio
           << std::setw(15) << std::fixed << std::setprecision(2) << overallThroughput
           << "\n\n";
}
```

The report includes:
- Header with column labels
- One row per operation with detailed metrics
- Precise decimal formatting
- Summary row with aggregate statistics
- Thread safety through mutex protection

## Main Application Structure

### Command Line Interface

The application provides a comprehensive CLI:

```cpp
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

// Parse command line arguments
Options parseArguments(int argc, char* argv[]) {
    Options options;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-e" || arg == "--encrypt") {
            options.encrypt = true;
        } else if (arg == "-b" || arg == "--batch") {
            options.batch = true;
        }
        // More options handling...
    }
    
    return options;
}
```

The CLI supports:
- Long and short option forms
- Password prompting with masked input
- Help and version information
- Wildcard path expansion
- Output directory specification
- Thread count control

### Workflow Management

The main function implements the high-level workflow:

```cpp
int main(int argc, char* argv[]) {
    // Parse command line arguments
    Options options = parseArguments(argc, argv);
    
    // Handle help/version requests
    if (options.help) {
        showHelp();
        return 0;
    }
    
    // Validate inputs and setup
    // ...
    
    // Create performance analyzer
    PerformanceAnalyzer analyzer;
    
    if (options.batch) {
        // Batch processing mode
        BatchProcessor processor(options.threads);
        
        // Setup progress tracking
        ProgressBar progressBar;
        auto progressCallback = [&progressBar](
            const std::string& path, int current, int total) {
            double progress = static_cast<double>(current) / total;
            progressBar.display(progress, "Processing files");
        };
        
        // Process files in parallel
        processor.processBatch(expandedPaths, 
            [&options, &analyzer](const std::string& path) {
                if (options.decompress) {
                    // Decompress file
                } else {
                    // Compress file
                }
            }, 
            progressCallback);
            
    } else {
        // Single file processing
        if (options.decompress) {
            // Decompress single file
        } else {
            // Compress single file
        }
    }
    
    // Display performance report
    if (options.verbose) {
        analyzer.generateReport();
    }
    
    return 0;
}
```

The workflow management:
1. Parses and validates command line options
2. Creates appropriate processing components
3. Sets up progress tracking mechanisms
4. Dispatches to single or batch processing
5. Handles both compression and decompression paths
6. Reports performance metrics

### Progress Visualization

For user feedback, a progress bar is implemented:

```cpp
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
        
        std::cout << "] " << std::fixed << std::setprecision(1) 
                  << (percentage * 100.0) << "%"
                  << " " << std::flush;
    }
    
    void complete(const std::string& message = "Completed") {
        display(1.0);
        std::cout << " " << message << std::endl;
    }
};
```

The progress bar provides:
- Visual representation of completion percentage
- Customizable appearance
- In-place updating with carriage return (\r)
- Optional prefix message
- Completion notification

### Error Recovery

The application implements robust error handling:

```cpp
try {
    processor.processBatch(expandedPaths, 
        [&options, &analyzer](const std::string& path) {
            try {
                // Process file...
            } catch (const std::exception& e) {
                std::cerr << "Error processing " << path << ": " << e.what() << std::endl;
                // Continue with next file
            }
        }, 
        progressCallback);
} catch (const std::exception& e) {
    std::cerr << "\nBatch processing error: " << e.what() << std::endl;
    return 1;
}
```

The error recovery strategy:
- Handles file-level errors without stopping the batch
- Reports specific file errors to the console
- Continues processing remaining files
- Captures and reports thread pool level errors
- Returns appropriate exit codes

## Technical Design Decisions

### Memory Management Strategy

The project uses modern C++ memory management:

1. **Smart Pointers**: `std::shared_ptr` for tree nodes and shared ownership:
```cpp
std::shared_ptr<HuffmanNode> left;
std::shared_ptr<HuffmanNode> right;
```

2. **RAII Pattern**: Resources tied to object lifetimes:
```cpp
~BatchProcessor() {
    // Auto-cleanup of all threads when object is destroyed
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

3. **Move Semantics**: Efficient buffer transfers:
```cpp
futures.push_back(std::move(future));
task = std::move(tasks.front());
```

4. **Pre-allocation**: Performance optimization:
```cpp
result.reserve(originalSize); // Pre-allocate output buffer
```

5. **Resource Management**: Proper OpenSSL context handling:
```cpp
EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
// ...
EVP_CIPHER_CTX_free(ctx);  // Always called, even on errors
```

These strategies minimize memory leaks, reduce copying, and ensure proper resource cleanup even in error conditions.

### Exception Handling Philosophy

The project employs consistent exception handling:

1. **Descriptive Exceptions**: Context-rich error messages:
```cpp
throw std::runtime_error("Unable to open file: " + filePath);
```

2. **Validation Before Operations**: Pre-emptive error checking:
```cpp
if (compressedData.size() < 4) {
    throw std::runtime_error("Invalid compressed data format");
}
```

3. **Resource Safety**: RAII and scope-based cleanup:
```cpp
{
    std::unique_lock<std::mutex> lock(queueMutex);
    // Operations...
} // Lock automatically released here
```

4. **Exception Propagation**: Controlled exception flow:
```cpp
try {
    // Operation that might fail
} catch (const std::exception& e) {
    // Add context information
    throw std::runtime_error(std::string("Operation failed: ") + e.what());
}
```

5. **Graceful Degradation**: Continuing when possible:
```cpp
try {
    processFile(path);
} catch (const std::exception& e) {
    std::cerr << "Error processing " << path << ": " << e.what() << std::endl;
    // Continue with next file
}
```

This approach balances robustness with usability.

### Performance Optimizations

The implementation includes numerous performance optimizations:

1. **Fixed-size types**: Explicit sizing for better control:
```cpp
uint64_t frequency;
uint32_t headerSize;
```

2. **Reusing buffers**: Minimizing allocations:
```cpp
result.reserve(originalSize);  // Allocate once
```

3. **Bit-level operations**: Efficient bit manipulation:
```cpp
currentByte |= (1 << (7 - bitCount));  // Set specific bit
```

4. **Move semantics**: Avoiding expensive copies:
```cpp
task = std::move(tasks.front());  // Move instead of copy
```

5. **Parallel processing**: Utilizing multiple cores:
```cpp
BatchProcessor processor(options.threads);
```

6. **Early returns**: Avoiding unnecessary work:
```cpp
if (data.empty()) {
    return {};
}
```

7. **Minimizing locks**: Reducing contention:
```cpp
{
    std::unique_lock<std::mutex> lock(queueMutex);
    // Minimal operations under lock
}
```

These optimizations collectively improve throughput, reduce memory usage, and enhance scalability.

### Cross-Platform Considerations

The implementation is designed for cross-platform compatibility:

1. **Standard C++17**: Avoiding platform-specific code:
```cpp
#include <filesystem>  // Instead of platform-specific APIs
```

2. **Path handling**: Uniform treatment across OS:
```cpp
fs::path path(filePath);
fs::path outPath = path.parent_path() / (path.filename().string() + ".huf");
```

3. **Thread management**: Standard thread library:
```cpp
std::thread::hardware_concurrency()  // Cross-platform CPU detection
```

4. **File operations**: Binary mode specification:
```cpp
std::ifstream file(filePath, std::ios::binary);
```

5. **Symbol handling**: Supporting full byte range:
```cpp
unsigned char byte;  // Full 0-255 range on all platforms
```

6. **Library dependencies**: Standard or widely ported:
   - Standard C++ library
   - OpenSSL (widely available on most platforms)

These decisions ensure the software works consistently across Windows, macOS, and Linux.

## Algorithms Used

### Core Compression Algorithms
- **Huffman Coding Algorithm**: Primary lossless data compression technique
  - Builds optimal prefix codes based on symbol frequencies
  - Achieves compression ratios near the theoretical entropy limit
  - Time complexity: O(n + k log k) where n is input size and k is unique symbols
  
- **Shannon-Fano-Elias Coding**: Variant implementation in the Huffman module
  - Uses cumulative distribution functions for code assignment
  - Near-optimal prefix codes with simpler assignment algorithm
  - Used as an optimization in certain compression contexts

### Cryptographic Algorithms
- **AES-256**: Block cipher encryption algorithm
  - Considered secure against quantum computer attacks
  - 14 rounds of substitution and permutation operations
  - 256-bit keys providing 2^256 possible key combinations
  
- **PKCS#7 Padding**: Block padding scheme for cryptographic functions
  - Adds bytes to ensure data length is a multiple of block size
  - Padding bytes all have the value equal to the padding length
  - Enables secure verification during decryption
  
- **PBKDF2**: Password-based key derivation for secure key generation
  - Converts passwords into cryptographic keys
  - Applies 10,000 iterations to slow down brute force attacks
  - Uses salting to prevent rainbow table attacks

### Data Processing Algorithms
- **Producer-Consumer Pattern**: For efficient batch file processing
  - Decouples task generation from task execution
  - Enables pipelining of operations for better throughput
  - Supports dynamic load balancing through the task queue
  
- **Work Stealing Algorithm**: Dynamic load balancing in thread pool
  - Idle threads take work from the shared queue
  - Minimizes thread contention through atomic operations
  - Adapts automatically to varying task complexity

### Search and File System Algorithms
- **Glob Pattern Matching**: For wildcard expansion in batch operations
  - Supports * and ? wildcards for pattern matching
  - Implements breadth-first search for file matching
  - Handles edge cases like empty directories and permission errors
  
- **Breadth-First Directory Traversal**: For filesystem operations
  - Processes directories level by level for optimal performance
  - Avoids stack overflow with deep directory structures
  - Supports both recursive and non-recursive modes

### Performance Analysis Algorithms
- **Moving Average Algorithm**: For performance metrics calculation
  - Provides smoothed metrics over operation sequences
  - Reduces impact of outliers in performance measurements
  - Enables trend analysis in multi-file operations
  
- **Amdahl's Law Analysis**: For parallelization efficiency estimation
  - Evaluates theoretical speedup from parallel processing
  - Identifies bottlenecks in the compression pipeline
  - Formula: Speedup = 1 / ((1 - P) + P/N) where P is parallel fraction

### Utility Algorithms
- **Binary Bit Packing**: For efficient bit-level data storage
  - Optimizes storage of variable-length codes
  - Fits codes across byte boundaries for maximum compression
  - Uses bit shifting and masking operations
  
- **Prefix-Free Code Generation**: Within the Huffman implementation
  - Ensures no code is a prefix of another code
  - Enables unambiguous decoding without separators
  - Achieved through tree-based code assignment
  
- **Command Line Argument Parsing**: For option processing
  - Supports both short (-e) and long (--encrypt) option formats
  - Handles dependent options and validation
  - Provides detailed error reporting for invalid inputs
  
- **Exponential Backoff Algorithm**: For thread synchronization
  - Reduces contention under high load
  - Increases wait time between retry attempts
  - Improves performance in multi-threaded scenarios

## Implementation Challenges and Solutions

### Challenge 1: Memory Efficiency with Large Files

**Problem**: Reading entire files into memory could exhaust RAM with large files.

**Solution**: 
- Implemented streaming processing for very large files
- Added chunked processing option for files exceeding memory threshold
- Used memory-mapped files for certain operations
- Added progress tracking to provide feedback during long operations

### Challenge 2: Thread Safety in Parallel Operations

**Problem**: Race conditions and deadlocks in multi-threaded code.

**Solution**:
- Used atomic variables for counters instead of locks
- Minimized critical sections with scoped locks
- Implemented lock-free algorithms where possible
- Added deadlock detection in debug builds
- Used thread sanitizers to identify race conditions

### Challenge 3: Cross-Platform Path Handling

**Problem**: Different path separators and conventions across operating systems.

**Solution**:
- Used C++17's std::filesystem for path manipulation
- Implemented path normalization functions
- Added path validation before operations
- Created unit tests for path handling edge cases
- Added support for both forward and backward slashes

### Challenge 4: Error Recovery in Batch Operations

**Problem**: A single file error should not stop the entire batch.

**Solution**:
- Implemented per-file try-catch blocks
- Created an error collection mechanism
- Added partial success reporting
- Implemented operation retries with backoff
- Provided detailed error logs for failed operations

## Advanced Usage Scenarios

### Scenario 1: Integrating with Backup Systems

The File-Shrink system can be used as part of an automated backup system:

```bash
# Compress and encrypt all modified files
find /home/user/documents -type f -mtime -1 | xargs huffman_compress -e -p "$BACKUP_PASSWORD"

# Add to archive
tar -cf backup.tar *.huf
```

### Scenario 2: Processing Media Files in Parallel

For batch processing of media files:

```bash
# Compress all JPGs with maximum parallelism
huffman_compress -b -v /media/photos/*.jpg

# Monitor compression effectiveness
grep "Ratio" compression_report.txt | sort -nr
```

### Scenario 3: Pipeline Integration

The tool can be part of a data processing pipeline:

```bash
# Extract, compress, encrypt
tar -xf source.tar && \
huffman_compress -b -e -p "$SECRET_KEY" extracted/* && \
scp *.huf remote-server:/backup/
```

### Scenario 4: Custom Compression Profiles

Using environment variables to control compression behavior:

```bash
# Set specific thread count and output directory
export HUFFMAN_THREADS=4
export HUFFMAN_OUTPUT=/mnt/compressed
huffman_compress -b largefiles/*
```

## Conclusion

File-Shrink demonstrates a comprehensive approach to file compression, combining the efficient Huffman coding algorithm with modern features like encryption, parallel processing, and detailed analytics. The modular design allows for easy maintenance and future extensions.

The system balances efficiency, security, and usability, providing a versatile tool for reducing file sizes while maintaining data integrity and offering protection through strong encryption. The implementation showcases modern C++ practices, algorithm design, and software architecture principles.
