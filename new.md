# Huffman Compression System

A high-performance file compression system implementing Huffman coding with AES-256 encryption and multi-threaded batch processing capabilities.

> **Note: This project is currently in development. The features and implementation described below represent the planned structure for the project. Work is in progress.**

## Features

- **Universal Format Support**: Compresses text, PDF, images, Word documents, and more
- **High-Efficiency Compression**: Uses Huffman coding for optimal lossless compression
- **Military-Grade Security**: Implements AES-256 encryption on compressed data
- **Batch Processing**: Multi-threaded architecture for processing multiple files simultaneously
- **Metadata Preservation**: Maintains file integrity for complex document types

## Implementation Overview (Planned)

### 1. File Input Processing
- Binary processing of any file type
- Streaming architecture for handling large files efficiently
- Support for various file formats through binary data processing

### 2. Character Frequency Analysis
- Hash map-based byte frequency counter
- Efficient single-pass algorithm for frequency determination
- Foundation for building optimal Huffman trees

### 3. Huffman Tree Construction
- Custom node structure implementation
- Min-heap priority queue for tree building
- Optimal prefix-free code generation

### 4. Code Generation
- Tree traversal algorithms for code assignment
- Hash map for quick character-to-code lookup
- Prefix-free code verification

### 5. Compression Algorithm
- Efficient bit packing (8 bits per byte)
- Header generation for decoding information
- Minimal memory footprint during processing

### 6. Decompression Algorithm
- Header parsing for Huffman tree reconstruction
- Bit-by-bit processing of compressed data
- Original file reconstruction

### 7. Metadata Preservation
- Selective compression for file headers and signatures
- Special handling for complex file formats
- Perfect reconstruction of original files

### 8. AES Encryption
- Industry-standard AES-256 implementation
- Password-based encryption/decryption
- Secure password verification

### 9. Batch Processing
- Thread pool architecture for parallel processing
- Work queue implementation for optimal CPU utilization
- Progress tracking for multiple file operations
- Organized output directory structure

### 10. Performance Analysis Tools
- Compression ratio measurements
- Execution time tracking
- Comparative analysis with other algorithms
- Memory usage monitoring

## Technical Requirements

- C++17 compatible compiler
- OpenSSL library for encryption functionality
- Standard Template Library (STL)
- Minimum 4GB RAM recommended for large files

## Development Status

This project is currently under active development. The following components are being worked on:

- [ ] Core Huffman coding implementation
- [ ] File I/O and binary processing
- [ ] Encryption module integration
- [ ] Multi-threading architecture
- [ ] Command line interface

## Projected Getting Started (Upon Completion)

1. Clone the repository
2. Build using provided Makefile or CMake configuration
3. Run the executable with input files as parameters

```bash
# Basic compression
./huffman_compress input_file.txt

# Compression with encryption
./huffman_compress -e input_file.txt

# Batch processing
./huffman_compress -b directory/*

# Full options
./huffman_compress -e -b -t 8 directory/* -o output_dir/
```

## Planned Command Line Options

- `-e, --encrypt`: Enable AES-256 encryption
- `-b, --batch`: Enable batch processing mode
- `-t, --threads [num]`: Specify number of threads for batch processing
- `-o, --output [dir]`: Specify output directory
- `-v, --verbose`: Enable detailed logging
- `-h, --help`: Display help information

## Target Performance Expectations

| File Type | Targeted Compression Ratio | Expected Processing Speed |
|-----------|----------------------------|---------------------------|
| Text      | 40-60%                     | ~100MB/s                  |
| PDF       | 20-40%                     | ~80MB/s                   |
| Images    | 10-30%                     | ~60MB/s                   |
| Word Docs | 30-50%                     | ~90MB/s                   |

*Projected performance metrics based on design specifications. Actual results may vary.*

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request or open an issue to discuss implementation details.

## License

This project is licensed under the MIT License - see the LICENSE file for details.