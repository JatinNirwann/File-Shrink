# HuffmanCompressor.h - Comprehensive Explanation for Beginners

This document provides a detailed, beginner-friendly explanation of the `HuffmanCompressor.h` header file, which implements the actual compression and decompression operations using the Huffman coding algorithm.

## Header Guards and Includes

```cpp
#ifndef HUFFMAN_COMPRESSOR_H
#define HUFFMAN_COMPRESSOR_H

#include "HuffmanCore.h"
#include <cstring>
#include <algorithm>
#include <atomic>
```

Like in HuffmanCore.h, the file starts with include guards to prevent multiple inclusions.

The includes bring in:
- `HuffmanCore.h`: Our custom header that contains the fundamental data structures and utilities
- `cstring`: Provides C-style string manipulation functions
- `algorithm`: Provides various algorithms like sorting and searching
- `atomic`: Provides atomic operations (though not used in the current implementation)

## The HuffmanCompressor Class

This class encapsulates all the functionality needed to compress and decompress data using Huffman coding.

```cpp
class HuffmanCompressor {
private:
    // Private methods...
public:
    // Public methods...
};
```

The class uses a public/private division to separate the interface (what users can access) from the implementation details.

## Private Methods

These methods are hidden implementation details, only accessible from within the class itself.

### countFrequencies Method

```cpp
FrequencyMap countFrequencies(const std::vector<unsigned char>& data) {
    FrequencyMap frequencies;
    for (const auto& byte : data) {
        frequencies[byte]++;
    }
    return frequencies;
}
```

This method counts how often each byte appears in the input data:

1. Creates an empty `FrequencyMap` (unordered_map from bytes to counts)
2. Iterates through each byte in the input data
3. Increments the count for that byte in the map
   - If a byte hasn't been seen before, it's automatically initialized to 0 then incremented
4. Returns the completed frequency map

**Time Complexity**: O(n) where n is the size of the input data
**Space Complexity**: O(k) where k is the number of unique bytes (at most 256)

### buildHuffmanTree Method

```cpp
HuffmanTree buildHuffmanTree(const FrequencyMap& frequencies) {
    if (frequencies.empty()) {
        return nullptr;
    }
    
    // Create a priority queue for all nodes
    PriorityQueue pq;
    
    // Create a leaf node for each byte and add it to the priority queue
    for (const auto& pair : frequencies) {
        pq.push(std::make_shared<HuffmanNode>(pair.first, pair.second));
    }
    
    // Handle edge case: single byte in input
    if (pq.size() == 1) {
        auto node = pq.top();
        return std::make_shared<HuffmanNode>(nullptr, node);
    }
    
    // Build the Huffman tree bottom-up
    while (pq.size() > 1) {
        auto left = pq.top(); pq.pop();
        auto right = pq.top(); pq.pop();
        
        auto parent = std::make_shared<HuffmanNode>(left, right);
        pq.push(parent);
    }
    
    return pq.top();
}
```

This method builds the Huffman tree from the frequency map:

1. First, it handles the edge case of empty input by returning nullptr
2. Creates a priority queue that will order nodes by frequency (smallest first)
3. For each byte in the frequency map:
   - Creates a leaf node with that byte and its frequency
   - Adds the node to the priority queue
4. Handles a special case for files with only one unique byte:
   - Creates a parent node with the single byte as a child
   - This is necessary because Huffman codes require at least two different paths
5. Builds the tree through an iterative process:
   - Takes the two nodes with lowest frequencies from the queue
   - Creates a new internal node with these two as children
   - The new node's frequency is the sum of its children
   - Adds this new node back to the queue
   - Repeats until only one node remains (the root)
6. Returns the root of the Huffman tree

The process ensures that less frequent bytes are deeper in the tree (longer codes) and more frequent bytes are closer to the root (shorter codes).

**Time Complexity**: O(k log k) where k is the number of unique bytes
**Space Complexity**: O(k) for the priority queue and tree

### generateCodes Method

```cpp
void generateCodes(const HuffmanTree& tree, CodeMap& codes, std::string code = "") {
    if (!tree) return;
    
    // Leaf node found - assign code
    if (tree->isLeaf()) {
        if (code.empty()) {
            code = "0"; // Special case for single byte files
        }
        codes[tree->byte] = code;
        return;
    }
    
    // Traverse left (0) and right (1)
    generateCodes(tree->left, codes, code + "0");
    generateCodes(tree->right, codes, code + "1");
}
```

This method traverses the Huffman tree to generate codes for each byte:

1. It uses recursive traversal of the tree
2. For each node:
   - If it's a leaf node, assigns the current code to its byte
   - If it's an internal node, continues traversing:
     - Adds '0' to the code when going left
     - Adds '1' to the code when going right
3. Handles a special case for files with only one unique byte:
   - Assigns code "0" if code is empty

The result is stored in the `codes` map, with each byte mapping to its unique code.

**Time Complexity**: O(k) where k is the number of unique bytes
**Space Complexity**: O(k) for the code map and recursion stack

### serializeTree Method

```cpp
void serializeTree(const HuffmanTree& tree, BitWriter& writer) {
    if (!tree) return;
    
    // Write a bit to indicate if this is a leaf node
    writer.writeBit(tree->isLeaf());
    
    if (tree->isLeaf()) {
        // For leaf nodes, write the byte value
        for (int i = 0; i < 8; i++) {
            writer.writeBit((tree->byte >> (7 - i)) & 1);
        }
    } else {
        // Recursively serialize left and right subtrees
        serializeTree(tree->left, writer);
        serializeTree(tree->right, writer);
    }
}
```

This method converts the Huffman tree into a bit sequence so it can be stored in the compressed file:

1. For each node in the tree (traversed recursively):
   - Writes a bit indicating if it's a leaf (1) or internal node (0)
   - For leaf nodes:
     - Writes the byte value as 8 bits
   - For internal nodes:
     - Recursively writes the left subtree
     - Recursively writes the right subtree

This pre-order traversal (node, left, right) ensures the tree can be perfectly reconstructed during decompression.

**Time Complexity**: O(k) where k is the number of unique bytes
**Space Complexity**: O(h) for the recursion stack, where h is the height of the tree

### deserializeTree Method

```cpp
HuffmanTree deserializeTree(BitReader& reader) {
    if (!reader.hasMoreBits()) {
        return nullptr;
    }
    
    // Read the node type bit
    bool isLeaf = reader.readBit();
    
    if (isLeaf) {
        // Leaf node - read the byte value
        unsigned char byte = 0;
        for (int i = 0; i < 8; i++) {
            if (reader.readBit()) {
                byte |= (1 << (7 - i));
            }
        }
        return std::make_shared<HuffmanNode>(byte, 0);
    } else {
        // Internal node - read left and right subtrees
        auto left = deserializeTree(reader);
        auto right = deserializeTree(reader);
        return std::make_shared<HuffmanNode>(left, right);
    }
}
```

This method reconstructs the Huffman tree from the bit sequence stored in the compressed file:

1. First, checks if there are more bits to read
2. Reads a bit to determine if the current node is a leaf:
   - If it's a leaf node:
     - Reads 8 bits to get the byte value
     - Creates and returns a leaf node with that byte
     - The frequency is set to 0 since it's not needed for decompression
   - If it's an internal node:
     - Recursively deserializes the left subtree
     - Recursively deserializes the right subtree
     - Creates and returns an internal node with these children

This mirrors the serialization process, reading the tree in the same pre-order traversal.

**Time Complexity**: O(k) where k is the number of unique bytes
**Space Complexity**: O(h) for the recursion stack, where h is the height of the tree

## Public Methods

These methods form the public interface of the class, accessible to users.

### compress Method

```cpp
std::vector<unsigned char> compress(const std::vector<unsigned char>& data) {
    if (data.empty()) {
        return {};
    }
    
    // Count byte frequencies
    FrequencyMap frequencies = countFrequencies(data);
    
    // Build Huffman tree
    HuffmanTree tree = buildHuffmanTree(frequencies);
    
    // Generate codes for each byte
    CodeMap codes;
    generateCodes(tree, codes);
    
    // Prepare the header
    BitWriter headerWriter;
    
    // Write the original data size (64 bits)
    uint64_t originalSize = data.size();
    for (int i = 0; i < 64; i++) {
        headerWriter.writeBit((originalSize >> (63 - i)) & 1);
    }
    
    // Serialize the tree structure
    serializeTree(tree, headerWriter);
    headerWriter.flush();
    
    // Prepare the compressed data
    BitWriter dataWriter;
    for (unsigned char byte : data) {
        dataWriter.writeBits(codes[byte]);
    }
    dataWriter.flush();
    
    // Combine header and data
    std::vector<unsigned char> headerBuffer = headerWriter.getBuffer();
    std::vector<unsigned char> dataBuffer = dataWriter.getBuffer();
    
    // Calculate total size for header, tree, and data
    uint32_t headerSize = headerBuffer.size();
    
    // Result buffer: [header size (4 bytes)][header][data]
    std::vector<unsigned char> result;
    
    // Write header size
    for (int i = 0; i < 4; i++) {
        result.push_back((headerSize >> (24 - i * 8)) & 0xFF);
    }
    
    // Append header and data
    result.insert(result.end(), headerBuffer.begin(), headerBuffer.end());
    result.insert(result.end(), dataBuffer.begin(), dataBuffer.end());
    
    return result;
}
```

This is the main compression method that takes raw data and returns compressed data:

1. First, handles the edge case of empty input
2. Counts the frequency of each byte in the input data
3. Builds the Huffman tree based on those frequencies
4. Generates Huffman codes for each byte by traversing the tree
5. Prepares the header section:
   - Writes the original data size as a 64-bit integer
     - This is needed for decompression to know when to stop
   - Writes the serialized Huffman tree
   - Flushes any remaining bits
6. Prepares the data section:
   - For each byte in the input data, writes its Huffman code
   - Flushes any remaining bits
7. Combines everything into a single output:
   - First 4 bytes: Header size (in bytes)
   - Next headerSize bytes: Header (original size + tree)
   - Remaining bytes: Compressed data
8. Returns the complete compressed data

**Time Complexity**: O(n + k log k) where n is the input size and k is unique bytes
**Space Complexity**: O(n + k) for the input, output, and supporting structures

### decompress Method

```cpp
std::vector<unsigned char> decompress(const std::vector<unsigned char>& compressedData) {
    if (compressedData.size() < 4) {
        throw std::runtime_error("Invalid compressed data format");
    }
    
    // Read header size
    uint32_t headerSize = 0;
    for (int i = 0; i < 4; i++) {
        headerSize = (headerSize << 8) | compressedData[i];
    }
    
    if (compressedData.size() < 4 + headerSize) {
        throw std::runtime_error("Corrupted compressed data");
    }
    
    // Extract header and data sections
    std::vector<unsigned char> headerBytes(compressedData.begin() + 4, 
                                          compressedData.begin() + 4 + headerSize);
    std::vector<unsigned char> dataBytes(compressedData.begin() + 4 + headerSize, 
                                        compressedData.end());
    
    // Parse the header
    BitReader headerReader(headerBytes, headerBytes.size() * 8);
    
    // Read original size
    uint64_t originalSize = 0;
    for (int i = 0; i < 64; i++) {
        if (headerReader.readBit()) {
            originalSize |= (1ULL << (63 - i));
        }
    }
    
    // Deserialize the Huffman tree
    HuffmanTree tree = deserializeTree(headerReader);
    
    // Prepare decompressed buffer
    std::vector<unsigned char> result;
    result.reserve(originalSize);
    
    // Calculate total bits in data section
    size_t totalBits = dataBytes.size() * 8;
    
    // Decompress data using the tree
    BitReader dataReader(dataBytes, totalBits);
    HuffmanTree current = tree;
    
    while (result.size() < originalSize && dataReader.hasMoreBits()) {
        if (!current) {
            throw std::runtime_error("Corrupted Huffman tree");
            
        }
        
        if (current->isLeaf()) {
            result.push_back(current->byte);
            current = tree;
        } else {
            // Traverse tree based on bits
            bool bit = dataReader.readBit();
            current = bit ? current->right : current->left;
        }
    }
    
    // Ensure we decompressed the correct amount
    if (result.size() != originalSize) {
        throw std::runtime_error("Decompression failed: size mismatch");
    }
    
    return result;
}
```

This method decompresses data that was previously compressed with the `compress` method:

1. First, validates the input has at least 4 bytes (for header size)
2. Reads the header size from the first 4 bytes
3. Validates that the input contains the full header
4. Splits the input into header and data sections
5. Parses the header:
   - Reads the original data size (64 bits)
   - Deserializes the Huffman tree
6. Prepares for decompression:
   - Creates a buffer for the result
   - Reserves space for the expected size (optimization)
7. Decompresses the data through tree traversal:
   - Start at the root of the tree
   - Read bits one by one from the compressed data
   - For each bit:
     - If 0, go left in the tree
     - If 1, go right in the tree
   - When reaching a leaf node:
     - Output the byte value stored in that leaf
     - Return to the root for the next code
   - Continue until the original size is reached
8. Validates that the decompression produced exactly the expected amount of data
9. Returns the decompressed data

**Time Complexity**: O(n + k) where n is the original size and k is unique bytes
**Space Complexity**: O(n + k) for the output, tree, and supporting structures

### getCompressionRatio Method

```cpp
double getCompressionRatio(size_t originalSize, size_t compressedSize) {
    if (originalSize == 0) return 0.0;
    return 100.0 * (1.0 - static_cast<double>(compressedSize) / originalSize);
}
```

This utility method calculates the compression ratio as a percentage:

1. If the original size is 0, returns 0 (avoids division by zero)
2. Otherwise calculates the percentage of size reduction:
   - The formula is 100% * (1 - compressedSize/originalSize)
   - For example, if the compressed size is 60% of the original, the ratio is 40%
3. Returns the ratio as a double (decimal) value

**Time Complexity**: O(1)
**Space Complexity**: O(1)

## End of File

```cpp
#endif // HUFFMAN_COMPRESSOR_H
```

This closes the include guard started at the beginning of the file.

## The Compression Format

Let's examine the format of the compressed data produced by this implementation:

```
[4 bytes: Header Size]
[Header: 
   [8 bytes: Original Size]
   [Variable: Serialized Huffman Tree]
]
[Data: Encoded bits]
```

### Header Size (4 bytes)
- Stores the size of the header section as a 32-bit integer
- Allows the decoder to know where the header ends and the data begins

### Original Size (8 bytes)
- Stores the size of the original uncompressed data as a 64-bit integer
- Allows decompression to know when to stop and validate the result

### Serialized Huffman Tree (Variable size)
- A bit-by-bit representation of the Huffman tree
- For each node:
  - 1 bit: Leaf flag (1 for leaf, 0 for internal)
  - For leaf nodes: 8 bits for the byte value
  - For internal nodes: Bits for left and right subtrees

### Encoded Data (Variable size)
- The actual compressed data
- Each byte from the original data is replaced by its variable-length Huffman code
- Bits are packed efficiently into bytes

## Implementation Notes

### Handling Edge Cases

The implementation has careful handling of several edge cases:

1. **Empty input**: Returns an empty vector
2. **Single unique byte**: Creates a special tree with one leaf (code "0")
3. **Corrupted input**: Throws descriptive exceptions for various error conditions

### Memory Management

The implementation uses smart pointers (`std::shared_ptr`) for tree nodes, which provides several benefits:
- Automatic memory management (no manual deletion needed)
- Safe handling of node ownership (multiple parts of code can share nodes)
- Exception safety (memory is properly released if an error occurs)

### Error Handling

The decompression process includes several validation steps:
- Checking input size is adequate
- Validating the Huffman tree is not corrupted
- Ensuring the decompressed size matches the expected size

### Performance Considerations

The implementation focuses on correctness and readability, with some performance optimizations:
- Using reserve() to pre-allocate result vectors
- Efficiently writing and reading bits (8 at a time when possible)
- Smart pointer usage for memory management

## Applications and Extensions

This Huffman compressor can be used for:

1. **Text compression**: Particularly effective for natural language text
2. **General file compression**: Works on any file type, though efficiency varies
3. **Network transmission**: Reducing data size for transmission

Possible extensions include:
- **Adaptive Huffman coding**: Updating the tree as data is processed
- **Combining with other algorithms**: Using Huffman as part of a multi-step process
- **Parallelization**: Processing different chunks of data in parallel
- **Dictionary techniques**: Adding dictionary-based pre-processing
