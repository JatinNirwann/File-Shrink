# File-Shrink Project Explanation

I'll explain how the entire `File-Shrink` project works, covering each header file and the main implementation.

## Overview of Files and Their Functions

### 1. HuffmanCore.h

This file contains the core data structures for Huffman compression:

- **FrequencyMap**: Maps bytes to their frequencies in the input data
- **HuffmanNode**: Tree node structure used to build the Huffman tree
  - Contains byte value, frequency, and pointers to child nodes
  - Supports leaf node detection with `isLeaf()`
- **HuffmanTree**: Shared pointer to a HuffmanNode (root of tree)
- **PriorityQueue**: Minimum heap for Huffman nodes, ordered by frequency
- **CodeMap**: Maps bytes to their Huffman codes (strings of '0's and '1's)
- **BitReader/BitWriter**: Utilities for reading/writing individual bits

These structures form the foundation for Huffman coding, which assigns shorter codes to more frequent symbols.

### 2. HuffmanCompressor.h

This class implements the actual compression/decompression algorithms:

- **Private methods**:
  - `countFrequencies()`: Counts occurrences of each byte in input data
  - `buildHuffmanTree()`: Creates a Huffman tree from frequency data
  - `generateCodes()`: Traverses tree to assign bit codes to each byte
  - `serializeTree()`: Converts Huffman tree to binary for storage in header
  - `deserializeTree()`: Reconstructs Huffman tree from binary header

- **Public methods**:
  - `compress()`: Compresses raw data using Huffman coding
  - `decompress()`: Decompresses Huffman-encoded data
  - `getCompressionRatio()`: Calculates compression percentage

The class recently added image compression with:
  - `compressImage()`: Compresses image data with prediction-based preprocessing
  - `decompressImage()`: Recovers original image from compressed data

### 3. EncryptionModule.h

Provides AES-256 encryption capabilities:

- **Methods**:
  - `encrypt()`: Encrypts data with a password
  - `decrypt()`: Decrypts data with the correct password
  - `generateKey()`: Creates encryption key from password
  - `generateIV()`: Creates initialization vector for AES

This adds security to compressed files for sensitive data.

### 4. FileProcessor.h

Handles file I/O operations:

- **Static methods**:
  - `readFile()`: Reads binary file content into a byte vector
  - `writeFile()`: Writes byte vector to a file
  - `getCompressedPath()`: Generates output path for compressed files
  - `getDecompressedPath()`: Generates output path for decompressed files

Manages all interaction with the filesystem.

### 5. BatchProcessor.h

Implements multi-threaded batch processing:

- **Methods**:
  - `processBatch()`: Processes multiple files concurrently
  - `getThreadCount()`: Returns number of worker threads

Uses a thread pool to parallelize compression/decompression across files.

### 6. PerformanceAnalyzer.h

Measures and reports on compression performance:

- **Classes**:
  - `Timer`: Records time taken, original size, and compressed size
  - `PerformanceAnalyzer`: Manages multiple timers and generates reports

Tracks efficiency metrics like speed and compression ratio.

### 7. HuffmanCompress.cpp (Main file)

The main file ties everything together:

- **Components**:
  - `Options`: Structure for command-line arguments
  - `ProgressBar`: Visual indicator of batch processing progress
  - `parseArguments()`: Converts command-line input to Options
  - `expandPaths()`: Resolves wildcards and directory inputs
  - `compressFile()/decompressFile()`: Core file processing functions
  - `main()`: Entry point that orchestrates the entire process

## How Compression Works (The Flow)

### Compression Process

1. **Input Preparation**:
   - User specifies files via command line
   - Input paths are expanded (wildcards, directories)
   
2. **For Each File**:
   - File data is read into memory as a byte vector
   - Frequencies of each byte are counted

3. **Huffman Tree Construction**:
   - Bytes are sorted by frequency in a priority queue
   - A binary tree is built bottom-up (less frequent characters deeper in tree)
   
4. **Code Generation**:
   - Each byte is assigned a variable-length bit code (shorter for frequent bytes)
   - The tree structure is serialized into the header

5. **Output Creation**:
   - Original data size is stored in header (for decompression)
   - Tree structure is stored in header
   - Data bytes are replaced with their Huffman codes
   - Optional encryption is applied if requested
   - Result is written to output file with `.huf` extension

### Decompression Process

1. **Input Processing**:
   - Compressed file is read into memory
   - Optional decryption is applied if password provided
   
2. **Header Parsing**:
   - Original data size is extracted
   - Huffman tree is deserialized from the header
   
3. **Data Reconstruction**:
   - Compressed data bits are read
   - Bits are traversed through the Huffman tree
   - When a leaf node is reached, the corresponding byte is output
   - Process continues until original data size is reached
   
4. **Output**:
   - Decompressed data is written to output file

## Key Algorithms

### Huffman Coding

1. **Frequency Analysis**: Count occurrences of each byte
2. **Tree Building**: Create a binary tree where:
   - Leaf nodes represent bytes
   - Path to each leaf defines its bit code
   - More frequent bytes have shorter paths
3. **Encoding**: Replace bytes with their variable-length bit codes
4. **Space Savings**: Frequent bytes use fewer bits than rare ones

### Delta Encoding for Images

1. **Prediction**: Predict each pixel based on neighbors
2. **Delta Calculation**: Store difference between actual and predicted value
3. **Benefits**: Produces smaller values that compress better with Huffman

### Multi-threading

1. **Thread Pool**: Creates a set of worker threads
2. **Task Queue**: Files are distributed among worker threads
3. **Progress Tracking**: Completed files are counted for progress display

## Usage Examples

1. **Basic Compression**:
   ```
   huffman_compress file.txt
   ```

2. **Decompression**:
   ```
   huffman_compress -d file.txt.huf
   ```

3. **Encrypted Compression**:
   ```
   huffman_compress -e -p mypassword file.txt
   ```

4. **Batch Processing**:
   ```
   huffman_compress -b *.txt -o compressed_files/
   ```

5. **Image Compression** (with new functionality):
   ```
   huffman_compress --image image.raw -w 1024 -h 768 -c 3
   ```

This project provides a complete, versatile file compression utility with security features, batch processing capabilities, and performance analytics.