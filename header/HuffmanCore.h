#ifndef HUFFMAN_CORE_H
#define HUFFMAN_CORE_H

#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>

// Node structure for Huffman Tree
struct HuffmanNode {
    unsigned char byte;
    uint64_t frequency;
    std::shared_ptr<HuffmanNode> left;
    std::shared_ptr<HuffmanNode> right;
    
    // Constructor for leaf nodes
    HuffmanNode(unsigned char b, uint64_t freq) : 
        byte(b), frequency(freq), left(nullptr), right(nullptr) {}
    
    // Constructor for internal nodes
    HuffmanNode(std::shared_ptr<HuffmanNode> l, std::shared_ptr<HuffmanNode> r) : 
        byte(0), frequency(l->frequency + r->frequency), left(l), right(r) {}
    
    // Check if node is a leaf
    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }
};

// Custom comparator for the priority queue
struct HuffmanNodeComparator {
    bool operator()(const std::shared_ptr<HuffmanNode>& a, const std::shared_ptr<HuffmanNode>& b) const {
        return a->frequency > b->frequency; // Min-heap based on frequency
    }
};

// Type definitions for clarity
using HuffmanTree = std::shared_ptr<HuffmanNode>;
using FrequencyMap = std::unordered_map<unsigned char, uint64_t>;
using CodeMap = std::unordered_map<unsigned char, std::string>;
using PriorityQueue = std::priority_queue<HuffmanTree, std::vector<HuffmanTree>, HuffmanNodeComparator>;

// Bit writer utility class
class BitWriter {
private:
    std::vector<unsigned char> buffer;
    int bitCount = 0;
    unsigned char currentByte = 0;

public:
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
    
    void writeBits(const std::string& bits) {
        for (char bit : bits) {
            writeBit(bit == '1');
        }
    }
    
    void flush() {
        // Add remaining bits if any
        if (bitCount > 0) {
            buffer.push_back(currentByte);
        }
    }
    
    const std::vector<unsigned char>& getBuffer() const {
        return buffer;
    }
    
    size_t getBitCount() const {
        return buffer.size() * 8 - (8 - bitCount) % 8;
    }
};

// Bit reader utility class
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
        if (bitIndex == 8) {
            byteIndex++;
            bitIndex = 0;
        }
        
        return bit;
    }
    
    bool hasMoreBits() const {
        return (byteIndex * 8 + bitIndex) < totalBits;
    }
};

#endif // HUFFMAN_CORE_H