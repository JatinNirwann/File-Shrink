# FileProcessor.h - Comprehensive Explanation for Beginners

This document provides a detailed, beginner-friendly explanation of the `FileProcessor.h` header file, which handles file input/output operations for the Huffman compression system.

## Header Guards and Includes

```cpp
#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;
```

As with other header files, this file begins with an include guard to prevent multiple inclusions.

The file includes several standard C++ libraries:
- `<string>`: Provides string handling functionality
- `<vector>`: Provides dynamic array implementation
- `<fstream>`: Provides file input/output operations
- `<stdexcept>`: Provides standard exception classes
- `<filesystem>`: Provides facilities for working with file paths, directories, etc.

The last line creates an alias `fs` for the `std::filesystem` namespace, which makes the code more concise when using filesystem functionality.

## The FileProcessor Class

```cpp
class FileProcessor {
public:
    // Methods...
};
```

This class is designed with a collection of static methods for file handling operations. Being static means these methods can be called directly from the class without creating an instance of it (e.g., `FileProcessor::readFile("example.txt")`).

All methods in this class are public, making them accessible to any code that includes this header.

## File Reading and Writing Methods

### readFile Method

```cpp
static std::vector<unsigned char> readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file: " + filePath);
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read file content
    std::vector<unsigned char> buffer(fileSize);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        throw std::runtime_error("Error reading file: " + filePath);
    }
    
    return buffer;
}
```

This method reads an entire file into memory as a byte array:

1. Opens the specified file in binary mode:
   - `std::ifstream` is used for input file stream
   - `std::ios::binary` ensures the file is read as raw bytes without text transformations
   - This is important for compressed files which contain non-text data

2. Checks if the file opened successfully:
   - If not, throws a descriptive runtime error

3. Determines the file size:
   - `seekg(0, std::ios::end)` moves the file pointer to the end
   - `tellg()` gets the current position, which equals the file size in bytes
   - `seekg(0, std::ios::beg)` resets the pointer to the beginning of the file

4. Creates a buffer and reads the entire file:
   - Initializes a vector of unsigned chars with the exact file size
   - `buffer.data()` returns a pointer to the vector's internal array
   - `reinterpret_cast<char*>` converts the unsigned char pointer to char pointer
     required by the `read` method
   - Reads fileSize bytes into the buffer

5. Checks if the read operation was successful:
   - If not, throws a runtime error

6. Returns the buffer containing the file's contents

**Note for beginners**: Reading the entire file at once is efficient but could cause memory issues with extremely large files. For very large files, streaming or chunked approaches might be better.

### writeFile Method

```cpp
static void writeFile(const std::string& filePath, const std::vector<unsigned char>& data) {
    // Create directory if it doesn't exist
    fs::path path(filePath);
    if (!path.parent_path().empty()) {
        fs::create_directories(path.parent_path());
    }
    
    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to create file: " + filePath);
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    if (!file) {
        throw std::runtime_error("Error writing to file: " + filePath);
    }
}
```

This method writes a byte array to a file:

1. Creates any necessary parent directories:
   - Converts the file path to a `fs::path` object for easier manipulation
   - Checks if there's a parent directory path
   - If so, creates all directories in the path that don't already exist

2. Opens the file for writing in binary mode:
   - `std::ofstream` is used for output file stream
   - `std::ios::binary` ensures data is written as raw bytes

3. Checks if the file opened successfully:
   - If not, throws a descriptive runtime error

4. Writes the entire data buffer to the file:
   - `data.data()` gets a pointer to the vector's internal array
   - `reinterpret_cast<const char*>` converts the unsigned char pointer to char pointer
     required by the `write` method
   - Writes `data.size()` bytes to the file

5. Checks if the write operation was successful:
   - If not, throws a runtime error

This method provides both file writing and directory creation functionality, making it easy to work with files in new or nested directories.

## Utility Methods

### getFileSize Method

```cpp
static size_t getFileSize(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Unable to open file: " + filePath);
    }
    
    return file.tellg();
}
```

This method efficiently gets a file's size without reading its contents:

1. Opens the file with specific flags:
   - `std::ios::binary` for binary mode
   - `std::ios::ate` positions the file pointer at the end immediately after opening
     (this is more efficient than seeking to the end afterward)

2. Checks if the file opened successfully:
   - If not, throws a runtime error

3. Returns the current file position (`tellg()`) which is the file size in bytes

This method is useful for quickly checking file sizes, like when calculating compression ratios.

### fileExists Method

```cpp
static bool fileExists(const std::string& filePath) {
    return fs::exists(filePath);
}
```

A simple wrapper around the filesystem's `exists` function:
- Takes a file path as input
- Returns true if the file exists, false otherwise
- Uses C++17's filesystem library for robust path handling

This method is useful for checking if a file exists before attempting to read it or to avoid overwriting existing files.

## Path Generation Methods

### getCompressedPath Method

```cpp
static std::string getCompressedPath(const std::string& inputPath, const std::string& outputDir = "") {
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
```

This method generates an appropriate path for a compressed file:

1. Converts the input file path to a `fs::path` object for easier manipulation
2. Depending on if an output directory is specified:
   - If no output directory is provided, places the output file in the same directory as the input
   - If an output directory is provided, places the output file in that directory
3. In either case, it uses the original filename plus a ".huf" extension
4. Returns the constructed path as a string

The `/` operator between path components is overloaded by the filesystem library to create proper paths regardless of operating system (e.g., using backslashes on Windows).

### getDecompressedPath Method

```cpp
static std::string getDecompressedPath(const std::string& inputPath, const std::string& outputDir = "") {
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

This method generates an appropriate path for a decompressed file:

1. Extracts the filename from the input path
2. Handles the filename based on its extension:
   - If it has a ".huf" extension, removes it
   - If not, adds a "decompressed_" prefix to avoid name conflicts
3. Depending on if an output directory is specified:
   - If no output directory is provided, places the output file in the same directory as the input
   - If an output directory is provided, places the output file in that directory
4. Returns the constructed path as a string

This method ensures decompressed files have meaningful names and don't overwrite the original files.

## Directory Processing

### listFiles Method

```cpp
static std::vector<std::string> listFiles(const std::string& directory, bool recursive = false) {
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

This method returns a list of all files in a directory:

1. First, it validates the directory:
   - Checks if the path exists and is actually a directory
   - Throws an exception if either condition fails
   
2. Depending on the `recursive` flag:
   - For non-recursive mode, uses `fs::directory_iterator` to iterate through just the immediate contents
   - For recursive mode, uses `fs::recursive_directory_iterator` to walk through the directory tree
   
3. For each entry found:
   - Checks if it's a regular file (not a directory, symbolic link, etc.)
   - If so, adds its full path to the results vector
   
4. Returns the list of file paths

This method is particularly useful for batch processing, allowing the compression of entire directories of files.

## End of File

```cpp
#endif // FILE_PROCESSOR_H
```

This closes the include guard started at the beginning of the file.

## Design Patterns and Principles

The `FileProcessor` class demonstrates several important design principles:

### Static Utility Class Pattern
All methods are static, making this a "utility class" that provides functionality without requiring an instance. This is appropriate since file operations don't need to maintain state between calls.

### Exception-Based Error Handling
The class uses exceptions to handle error conditions, which:
- Separates error handling code from the main logic
- Forces clients to acknowledge and handle potential errors
- Provides specific error messages for debugging

### Filesystem Abstraction
By using the C++17 filesystem library, the code:
- Works across different operating systems
- Handles path manipulation safely
- Provides robust directory creation and traversal

## Practical Applications

The `FileProcessor` class enables several key features for the compression tool:

1. **Input/Output Operations**: Reading and writing files for compression/decompression
2. **Path Management**: Generating appropriate output paths
3. **Batch Processing**: Finding all files in directories for bulk operations
4. **Error Prevention**: Checking for file existence and handling missing files/directories

## Implementation Notes

### Memory Considerations
The file reading implementation loads entire files into memory, which:
- Is efficient for small to medium files
- Could cause issues with very large files
- An alternative approach for large files would be to process them in chunks

### Platform Compatibility
The use of C++17's `std::filesystem` ensures:
- Path separators are handled correctly across platforms
- Unicode filenames are supported
- Directory traversal works consistently on different operating systems

### Best Practices
The code demonstrates several best practices:
- Input validation before operations
- Clear error messages
- Proper resource management (file streams are automatically closed)
- Consistent error handling through exceptions
