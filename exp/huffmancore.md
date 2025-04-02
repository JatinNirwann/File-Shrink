# HuffmanCore.h - Comprehensive Explanation for Beginners

This document provides a detailed, beginner-friendly explanation of the `HuffmanCore.h` header file, which implements Huffman coding - a popular algorithm for lossless data compression.

## What is Huffman Coding?

Before diving into the code, let's understand what Huffman coding does:

Huffman coding is a technique that assigns variable-length codes to input characters, with shorter codes for more frequent characters. This minimizes the average code length, resulting in efficient data compression.

For example, in English text, the letter 'e' appears more frequently than 'z', so 'e' would get a shorter code (maybe '10') while 'z' might get a longer one (like '000111').

## Header Guards and Includes

```cpp
#ifndef HUFFMAN_CORE_H
#define HUFFMAN_CORE_H
```

These are **include guards**. They prevent the compiler from processing the same header file multiple times, which could lead to redefinition errors.

- When the compiler first encounters this file, `HUFFMAN_CORE_H` is not defined
- The `#define` statement then defines it
- If the file is included again, the `#ifndef` test fails, and the compiler skips everything until the matching `#endif`

Next, the file includes several standard C++ libraries:

```cpp
#include <vector>          // Provides dynamic arrays (std::vector)
#include <queue>           // Provides priority queue
#include <unordered_map>   // Provides hash table implementation
#include <memory>          // Provides smart pointers like shared_ptr
#include <string>          // Provides string handling functionality
#include <fstream>         // Provides file input/output operations
#include <iostream>        // Provides console input/output operations
```

Let's understand why each is needed:

- **vector**: To store arrays of bytes and other data that may change in size
- **queue**: Specifically for priority_queue, which is essential for building the Huffman tree
- **unordered_map**: To create mappings between bytes and their frequencies/codes
- **memory**: For shared_ptr, which helps with memory management of tree nodes
- **string**: To handle strings, particularly for representing binary codes
- **fstream**: For reading from and writing to files
- **iostream**: For basic input/output operations

## The HuffmanNode Structure

```cpp
struct HuffmanNode {
    unsigned char byte;
    uint64_t frequency;
    std::shared_ptr<HuffmanNode> left;
    std::shared_ptr<HuffmanNode> right;
    
    // Constructors and methods...
}
```

This structure represents a single node in our Huffman tree. Let's break down each component:

### Member Variables

- **byte**: An unsigned character (8 bits, values 0-255) that represents the byte value this node corresponds to. This is only meaningful for leaf nodes.
  - Using `unsigned char` ensures we can represent all possible byte values (0-255), while a regular `char` might be signed (-128 to 127).

- **frequency**: A 64-bit unsigned integer that stores how many times this byte appears in the input data.
  - `uint64_t` is used instead of regular `int` to support very large files where frequencies might exceed 32-bit integer limits.

- **left** and **right**: Pointers to the child nodes in the Huffman tree.
  - These use `std::shared_ptr` which automatically manages memory (deletes nodes when they're no longer referenced), preventing memory leaks.
  - For leaf nodes, both pointers are `nullptr`.

### Constructors

The structure has two different constructors for different scenarios:

#### 1. Constructor for Leaf Nodes

```cpp
HuffmanNode(unsigned char b, uint64_t freq) : 
    byte(b), frequency(freq), left(nullptr), right(nullptr) {}
```

This creates a node for an actual byte from our input data:
- Takes a byte value and how often it appears
- The `: byte(b), frequency(freq), left(nullptr), right(nullptr)` part is an initialization list
- It sets the `byte` and `frequency` fields to the provided values
- It explicitly sets `left` and `right` pointers to `nullptr` (no children)

#### 2. Constructor for Internal Nodes

```cpp
HuffmanNode(std::shared_ptr<HuffmanNode> l, std::shared_ptr<HuffmanNode> r) : 
    byte(0), frequency(l->frequency + r->frequency), left(l), right(r) {}
```

This creates an internal (non-leaf) node that combines two other nodes:
- Takes two existing nodes as arguments
- Sets `byte` to 0 (not used for internal nodes)
- Sets `frequency` to the sum of the two child nodes' frequencies
- Points `left` and `right` to the provided nodes

### The isLeaf Method

```cpp
bool isLeaf() const {
    return left == nullptr && right == nullptr;
}
```

This is a helper method that checks if the node is a leaf node:
- Returns `true` if both child pointers are `nullptr`
- Returns `false` otherwise
- The `const` keyword means this method doesn't modify the node
- This method is useful when traversing the tree to decode data

## HuffmanNodeComparator Structure

```cpp
struct HuffmanNodeComparator {
    bool operator()(const std::shared_ptr<HuffmanNode>& a, const std::shared_ptr<HuffmanNode>& b) const {
        return a->frequency > b->frequency; // Min-heap based on frequency
    }
};
```

This structure defines how we compare nodes when building our Huffman tree:

- It overloads the function call operator `operator()`, which makes this struct behave like a function
- It takes two `HuffmanNode` pointers as arguments
- It returns `true` if node `a` has a higher frequency than node `b`
- This creates a min-heap, meaning nodes with lower frequencies come out first
- The `const` keywords ensure the method doesn't modify the nodes or the comparator itself

**Important for beginners**: The priority queue in C++ is usually a max-heap (largest items come out first). By using this comparator that reverses the comparison (`a > b` instead of `a < b`), we convert it to a min-heap (smallest items come out first), which is what we need for Huffman coding.

## Type Definitions (Aliases)

```cpp
using HuffmanTree = std::shared_ptr<HuffmanNode>;
using FrequencyMap = std::unordered_map<unsigned char, uint64_t>;
using CodeMap = std::unordered_map<unsigned char, std::string>;
using PriorityQueue = std::priority_queue<HuffmanTree, std::vector<HuffmanTree>, HuffmanNodeComparator>;
```

These `using` declarations create aliases (alternative names) for complex types, making the code more readable:

- **HuffmanTree**: Represents a tree or subtree as a pointer to its root node
- **FrequencyMap**: Maps each byte to how many times it appears in the input
  - Keys are bytes (0-255)
  - Values are counts (frequencies)
- **CodeMap**: Maps each byte to its Huffman code
  - Keys are bytes (0-255)
  - Values are strings of '0's and '1's representing the binary code
- **PriorityQueue**: A specialized queue that:
  - Contains Huffman trees (nodes)
  - Uses a vector as its internal container
  - Uses our custom comparator to order nodes by frequency

## The BitWriter Class

This class handles writing individual bits to a byte array. Since computers typically work with bytes (8 bits), but Huffman codes can be any number of bits, we need special handling to pack bits efficiently.

```cpp
class BitWriter {
private:
    std::vector<unsigned char> buffer;
    int bitCount = 0;
    unsigned char currentByte = 0;

public:
    // Methods...
};
```

### Member Variables

- **buffer**: A vector that stores the output bytes after they're filled
- **bitCount**: Tracks how many bits have been written to the current byte (0-7)
- **currentByte**: The byte currently being assembled bit by bit

### writeBit Method

```cpp
void writeBit(bool bit) {
    // Set the bit in current byte
    if (bit) {
        currentByte |= (1 << (7 - bitCount));
    }
    
    bitCount++;
    
    // If byte is full, add to buffer and reset
    if (bitCount == 8) {
        buffer.push_back(currentByte);
        currentByte = 0;
        bitCount = 0;
    }
}
```

This method writes a single bit (0 or 1) to the output:

1. If the bit is 1:
   - `1 << (7 - bitCount)` shifts a 1 bit to the correct position
   - The `|=` operator performs a bitwise OR to set that bit without affecting others
   - For example, if bitCount is 3, we set the 4th bit from the left (indexes are 0-7)
2. We increment `bitCount` to track our position
3. If we've written 8 bits:
   - The byte is full, so we add it to our buffer
   - We reset `currentByte` and `bitCount` to start a new byte

### writeBits Method

```cpp
void writeBits(const std::string& bits) {
    for (char bit : bits) {
        writeBit(bit == '1');
    }
}
```

This is a convenience method that writes multiple bits at once:
- Takes a string like "1001" representing bits
- Iterates through each character
- Calls `writeBit` with `true` for '1' characters and `false` for others
- This is useful for writing an entire Huffman code at once

### flush Method

```cpp
void flush() {
    // Add remaining bits if any
    if (bitCount > 0) {
        buffer.push_back(currentByte);
    }
}
```

This method ensures any partially filled byte gets written:
- If there are bits in the current byte (bitCount > 0), adds it to the buffer
- Should be called when done writing to ensure no bits are lost
- The remaining bits in the byte are padded with zeros (which happens automatically since `currentByte` initialized new bits to 0)

### getBuffer Method

```cpp
const std::vector<unsigned char>& getBuffer() const {
    return buffer;
}
```

Returns a reference to the internal buffer containing all written bytes:
- The `const` keyword (used twice) ensures the caller can't modify the buffer
- It returns a reference (`&`) rather than a copy for efficiency

### getBitCount Method

```cpp
size_t getBitCount() const {
    return buffer.size() * 8 - (8 - bitCount) % 8;
}
```

Returns the total number of bits written so far:
- `buffer.size() * 8` is the number of bits in all complete bytes
- `(8 - bitCount) % 8` calculates how many unused bits are in the current byte
- The formula subtracts unused bits from the total
- The `% 8` handles the case when `bitCount` is 0 (fully written byte)

## The BitReader Class

This class handles reading individual bits from a byte array, which is necessary when decompressing Huffman-encoded data.

```cpp
class BitReader {
private:
    const std::vector<unsigned char>& buffer;
    size_t byteIndex = 0;
    int bitIndex = 0;
    size_t totalBits;

public:
    // Methods...
};
```

### Member Variables

- **buffer**: A reference to the input data (stored as bytes)
- **byteIndex**: Current position in the buffer (which byte we're reading)
- **bitIndex**: Current bit position within the current byte (0-7)
- **totalBits**: Total number of valid bits in the buffer

### Constructor

```cpp
BitReader(const std::vector<unsigned char>& buf, size_t bits) 
    : buffer(buf), totalBits(bits) {}
```

Initializes the BitReader:
- Takes a reference to the input buffer
- Takes the total number of bits to read (important since the last byte might not be full)
- The initializer list sets the buffer reference and totalBits
- byteIndex and bitIndex are initialized to 0 by their default values

### readBit Method

```cpp
bool readBit() {
    if (byteIndex >= buffer.size()) {
        throw std::runtime_error("End of buffer reached in BitReader");
    }
    
    bool bit = (buffer[byteIndex] & (1 << (7 - bitIndex))) != 0;
    
    bitIndex++;
    if (bitIndex == 8) {
        byteIndex++;
        bitIndex = 0;
    }
    
    return bit;
}
```

Reads a single bit from the buffer:

1. First, it checks if we've reached the end of the buffer:
   - If so, it throws an exception (a runtime error)
2. To extract the bit:
   - `1 << (7 - bitIndex)` creates a value with a single 1 bit at the current position
   - `buffer[byteIndex] & ...` performs a bitwise AND to isolate that bit
   - `!= 0` converts the result to a boolean (true if bit is 1, false if 0)
3. Updates the position:
   - Increments `bitIndex`
   - If we've read all 8 bits from the current byte, move to the next byte
4. Returns the bit as a boolean

### hasMoreBits Method

```cpp
bool hasMoreBits() const {
    return (byteIndex * 8 + bitIndex) < totalBits;
}
```

Checks if there are more bits to read:
- Calculates the total number of bits we've read: (byteIndex * 8 + bitIndex)
- Compares this to the total number of valid bits
- Returns true if there are still bits to read, false otherwise
- This is important for stopping at the right place, especially for the last byte where some bits might be padding

## End of File

```cpp
#endif // HUFFMAN_CORE_H
```

This closes the include guard started at the beginning of the file.

## The Huffman Coding Process

While the actual algorithms for compression and decompression aren't implemented in this header file (they would be in the source files), here's how these components work together:

### Compression Process:

1. **Frequency Analysis**:
   - Scan the input data and count how often each byte appears
   - Store these counts in a FrequencyMap

2. **Build the Huffman Tree**:
   - Create a leaf node for each byte in the FrequencyMap
   - Add all nodes to a PriorityQueue (ordered by frequency)
   - Repeatedly take the two nodes with lowest frequencies
   - Combine them into a new internal node
   - Add this new node back to the queue
   - Continue until only one node remains (the root of the Huffman tree)

3. **Generate Codes**:
   - Traverse the Huffman tree from root to each leaf
   - Assign '0' when going left, '1' when going right
   - When reaching a leaf, the path becomes the code for that byte
   - Store all codes in a CodeMap

4. **Output Compressed Data**:
   - Write the tree structure or frequency table to the output
   - For each byte in the input, write its code to the output using BitWriter
   - Ensure all bits are written by calling flush()

### Decompression Process:

1. **Rebuild the Huffman Tree**:
   - Read the tree structure or frequency table from the input
   - Reconstruct the same Huffman tree used for compression

2. **Decode the Data**:
   - Use BitReader to read bits one by one
   - Start at the root of the Huffman tree
   - For each bit, go left (if 0) or right (if 1)
   - When a leaf is reached, output its byte value
   - Return to the root and continue
   - Repeat until all bits are processed

## Real-World Applications

Huffman coding is used in many file compression formats, including:
- Parts of JPEG image compression
- Parts of MP3 audio compression
- Parts of ZIP and other archive formats

Its effectiveness comes from adapting to the actual content being compressed, making it ideal for files with uneven character distributions like text, where some characters appear much more frequently than others.

## Technical Details for Advanced Understanding

### Time Complexity

- Building the frequency map: O(n) where n is the input size
- Building the Huffman tree: O(k log k) where k is the number of unique bytes
- Generating codes: O(k)
- Encoding the data: O(n)
- Overall: O(n + k log k)

### Space Complexity

- Frequency map: O(k)
- Priority queue: O(k)
- Huffman tree: O(k)
- Code map: O(k)
- Output buffer: O(n) in worst case
- Overall: O(n + k)

Where n is the input size and k is the number of unique bytes (maximum 256 for byte-level Huffman coding).
