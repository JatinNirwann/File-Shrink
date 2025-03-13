cmake_minimum_required(VERSION 3.12)
project(HuffmanCompression VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -pedantic)
endif()

# Find dependencies
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(Doxygen)

# Include directories
include_directories(${PROJECT_SOURCE_DIR})

# Define source files
set(SOURCES
    HuffmanCompress.cpp
)

# Define header files
set(HEADERS
    header/HuffmanCore.h
    header/HuffmanCompressor.h
    header/EncryptionModule.h
    header/BatchProcessor.h
    header/FileProcessor.h
    header/PerformanceAnalyzer.h
)

# Create executable
add_executable(huffman_compress ${SOURCES} ${HEADERS})

# Link with required libraries
target_link_libraries(huffman_compress 
    OpenSSL::SSL 
    OpenSSL::Crypto
    Threads::Threads
)

# Link with filesystem library if necessary (older compilers)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(huffman_compress stdc++fs)
endif()

# Installation rules
install(TARGETS huffman_compress DESTINATION bin)

# Package configuration
set(CPACK_PACKAGE_NAME "huffman-compress")
set(CPACK_PACKAGE_VENDOR "HuffmanCompression")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High-performance Huffman compression with encryption")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "your-email@example.com")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Generator-specific configurations
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_PACKAGE_NAME "Huffman Compression")
    set(CPACK_NSIS_DISPLAY_NAME "Huffman Compression System")
    set(CPACK_NSIS_MODIFY_PATH ON)
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
    set(CPACK_DMG_VOLUME_NAME "Huffman Compression")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "openssl (>= 1.1.0)")
    set(CPACK_RPM_PACKAGE_REQUIRES "openssl >= 1.1.0")
endif()

include(CPack)

# Add testing
enable_testing()

# Add documentation target
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    if(EXISTS ${DOXYGEN_IN})
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        add_custom_target(doc
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    else()
        message(WARNING "Doxygen file missing, skipping documentation generation.")
    endif()
endif()

