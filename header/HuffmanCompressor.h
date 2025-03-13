#ifndef HUFFMAN_COMPRESSOR_H
#define HUFFMAN_COMPRESSOR_H

#include "HuffmanCore.h"
#include <cstring>
#include <algorithm>
#include <atomic>

class HuffmanCompressor {
private:
    // Frequency counting
    FrequencyMap countFrequencies(const std::vector<unsigned char>& data) {
        FrequencyMap frequencies;
        for (const auto& byte : data) {
            frequencies[byte]++;
        }
        return frequencies;
    }
    
    // Huffman tree building
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
    
    // Generate Huffman codes by traversing the tree
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
    
    // Serialize tree to bitstream for header
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
    
    // Deserialize tree from bitstream
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

public:
    // Compress a byte vector
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
    
    // Decompress a byte vector
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
    
    // Calculate and return compression ratio
    double getCompressionRatio(size_t originalSize, size_t compressedSize) {
        if (originalSize == 0) return 0.0;
        return 100.0 * (1.0 - static_cast<double>(compressedSize) / originalSize);
    }
};

#endif // HUFFMAN_COMPRESSOR_H