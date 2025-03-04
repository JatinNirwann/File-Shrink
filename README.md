# File Shrink

## Project Overview
A comprehensive C++ file compression tool using Huffman coding, supporting multiple file formats with AES encryption and batch processing.

## Features
- Lossless file compression
- Multi-format support (PDF, Images, Word documents)
- AES-256 encryption
- Batch file processing
- Metadata preservation

## Tech Stack
- C++
- STL
- OpenSSL
- Multithreading

## Prerequisites
- C++17 Compiler
- OpenSSL Library
- CMake 3.10+


### Options
- `-c`: Compress files
- `-d`: Decompress files
- `-e`: Enable encryption
- `-p`: Set password

## Project Structure
```
project_root/
├── src/
│   ├── compression/
│   ├── encryption/
│   └── batch_processing/
├── include/
├── tests/
└── README.md
```