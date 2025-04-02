> **Scheduled Work:**
> - Integrate better image compression functionality using run length encoding
> - Implement simpler navigation for better user experience.


# File-Shrink

This is a Huffman compression tool with advanced features:

- **Core Functionality**: Lossless file compression/decompression using Huffman coding
- **Security**: Optional AES-256 encryption for compressed files
- **Performance**: Batch processing with multi-threading support
- **Analysis**: Built-in performance metrics and reporting

### Key Components

1. **HuffmanCompress.cpp**: Main entry point handling CLI arguments and workflow
2. **HuffmanCore.h**: Core data structures for Huffman algorithm
3. **HuffmanCompressor.h**: Compression/decompression implementation
4. **FileProcessor.h**: File I/O operations
5. **EncryptionModule.h**: AES-256 encryption integration
6. **BatchProcessor.h**: Thread pool for parallel processing
7. **PerformanceAnalyzer.h**: Metrics collection and reporting


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

### Usage Examples

- Basic: `huffman_compress file.txt` → Creates compressed file.txt.huf
- Encrypted: `huffman_compress -e -p mypassword file.txt` → Encrypts while compressing
- Batch: `huffman_compress -d -b *.huf` → Decompresses multiple files in parallel

The tool provides error handling, progress feedback, and comprehensive performance statistics.