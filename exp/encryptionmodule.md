# EncryptionModule.h - Detailed Explanation for Beginners

This document provides a comprehensive explanation of the `EncryptionModule.h` header file, which implements encryption and decryption capabilities using the AES-256 algorithm with OpenSSL. This explanation is designed for someone new to cryptography and C++ programming.

## What is Encryption?

Before diving into the code, let's understand what encryption does:

Encryption is the process of converting data (plaintext) into a scrambled format (ciphertext) that can only be read by someone who has the correct key. This protects data from unauthorized access.

## Header Guards and Includes

```cpp
#ifndef ENCRYPTION_MODULE_H
#define ENCRYPTION_MODULE_H

#include <vector>
#include <string>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <stdexcept>
#include <cstring>
```

As with other header files, this begins with an include guard to prevent multiple inclusions. The file includes several libraries:

- **Standard C++ Libraries**:
  - `<vector>`: For storing arrays of bytes
  - `<string>`: For working with text strings (like passwords)
  - `<stdexcept>`: For exception handling
  - `<cstring>`: For C-style string manipulation

- **OpenSSL Libraries**:
  - `<openssl/evp.h>`: Provides OpenSSL's high-level cryptographic functions
  - `<openssl/rand.h>`: Provides secure random number generation
  - `<openssl/sha.h>`: Provides SHA hash functions
  
OpenSSL is a widely-used, open-source cryptography library that implements various cryptographic algorithms and protocols. It's considered industry standard and is extensively tested for security.

## The EncryptionModule Class

```cpp
class EncryptionModule {
private:
    // Constants and private methods...
public:
    // Public methods...
};
```

This class encapsulates all functionality related to encrypting and decrypting data with a password.

## Private Constants

```cpp
// AES-256 constants
static constexpr int KEY_LENGTH = 32; // 256 bits
static constexpr int IV_LENGTH = 16;  // 128 bits
static constexpr int SALT_LENGTH = 16;
```

These constants define important parameters for the encryption algorithm:

- `KEY_LENGTH`: Specifies the length of the encryption key in bytes (32 bytes = 256 bits).
  - AES-256 uses a 256-bit key, which is considered highly secure and resistant to brute force attacks.
  - As of 2023, there are no known practical attacks that can break AES-256 when properly implemented.

- `IV_LENGTH`: Specifies the length of the Initialization Vector in bytes (16 bytes = 128 bits).
  - The IV is a random value used to ensure that encrypting the same data with the same key produces different ciphertexts.
  - This prevents attackers from identifying patterns in encrypted data.
  - For AES, the IV must always be 128 bits (16 bytes), regardless of the key size.

- `SALT_LENGTH`: Specifies the length of the salt in bytes (16 bytes).
  - A salt is random data added to the password before hashing.
  - It prevents attackers from using precomputed tables (rainbow tables) to crack passwords.
  - 16 bytes provides sufficient randomness while keeping the overhead reasonable.

The `static constexpr` keywords mean these are compile-time constants that belong to the class rather than to individual objects.

## Private Method: deriveKey

```cpp
std::vector<unsigned char> deriveKey(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> key(KEY_LENGTH);
    
    // Use PBKDF2 with SHA-256 for key derivation
    if (PKCS5_PBKDF2_HMAC(
            password.c_str(), password.length(),
            salt.data(), salt.size(),
            10000,  // Iteration count
            EVP_sha256(),
            KEY_LENGTH,
            key.data()) != 1) {
        throw std::runtime_error("Key derivation failed");
    }
    
    return key;
}
```

This method converts a user's password into a cryptographic key suitable for encryption:

### What is Key Derivation and Why is it Needed?

User passwords are often too short, not random enough, or don't have the right format to be used directly as encryption keys. Key derivation transforms these passwords into strong cryptographic keys.

### Parameters:
- `password`: The user-provided password as a string
- `salt`: Random bytes that make the derived key unique even if the password is the same

### How it Works:

1. Creates an empty key buffer of the required size (KEY_LENGTH = 32 bytes)

2. Uses PBKDF2 (Password-Based Key Derivation Function 2):
   - `PKCS5_PBKDF2_HMAC`: The OpenSSL function that implements PBKDF2
   - `password.c_str(), password.length()`: The input password and its length
   - `salt.data(), salt.size()`: The salt value and its length
   - `10000`: The iteration count - this is a **hard-coded security parameter**
     - Higher values make the function slower (more secure against brute force)
     - 10,000 iterations is a reasonable balance between security and performance
     - Modern recommendations suggest even higher counts (100,000+) for highly sensitive data
   - `EVP_sha256()`: Specifies SHA-256 as the hash function
   - `KEY_LENGTH`: The desired output key length (32 bytes)
   - `key.data()`: Where to store the resulting key

3. Error Handling:
   - If the function returns anything other than 1, it throws an exception
   - This detects any failures in the OpenSSL library

4. Returns the derived key

### Security Considerations:
- Using PBKDF2 with SHA-256 is a standard approach for secure key derivation
- The 10,000 iteration count makes brute force attacks computationally expensive
- The salt ensures that the same password will generate different keys when different salts are used

## Public Method: encrypt

```cpp
std::vector<unsigned char> encrypt(const std::vector<unsigned char>& data, const std::string& password) {
    if (data.empty()) {
        return {};
    }
    
    // Generate random salt
    std::vector<unsigned char> salt(SALT_LENGTH);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }
    
    // Derive key from password
    std::vector<unsigned char> key = deriveKey(password, salt);
    
    // Generate random IV
    std::vector<unsigned char> iv(IV_LENGTH);
    if (RAND_bytes(iv.data(), iv.size()) != 1) {
        throw std::runtime_error("Failed to generate random IV");
    }
    
    // Setup encryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create encryption context");
    }
    
    // Initialize encryption
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }
    
    // Allocate output buffer with extra space for padding
    std::vector<unsigned char> encryptedData(data.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int outLen1 = 0;
    
    // Encrypt data
    if (EVP_EncryptUpdate(ctx, encryptedData.data(), &outLen1, data.data(), data.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption failed");
    }
    
    // Finalize encryption
    int outLen2 = 0;
    if (EVP_EncryptFinal_ex(ctx, encryptedData.data() + outLen1, &outLen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption finalization failed");
    }
    
    // Free context
    EVP_CIPHER_CTX_free(ctx);
    
    // Resize encrypted data to actual size
    encryptedData.resize(outLen1 + outLen2);
    
    // Format result: [salt][IV][encrypted data]
    std::vector<unsigned char> result;
    result.insert(result.end(), salt.begin(), salt.end());
    result.insert(result.end(), iv.begin(), iv.end());
    result.insert(result.end(), encryptedData.begin(), encryptedData.end());
    
    return result;
}
```

This method encrypts data using AES-256 in CBC mode:

### Parameters:
- `data`: The data to be encrypted (as a vector of bytes)
- `password`: The password used to derive the encryption key

### How it Works:

1. **Handle Empty Input:**
   - If the input data is empty, return an empty result

2. **Generate Random Salt:**
   - Creates a vector for the salt (16 bytes)
   - Uses `RAND_bytes()` from OpenSSL to fill it with cryptographically secure random values
   - The salt will be used in key derivation

3. **Derive Encryption Key:**
   - Calls the private `deriveKey()` method to transform the password into a suitable encryption key
   - Uses the freshly generated salt

4. **Generate Random IV (Initialization Vector):**
   - Creates a vector for the IV (16 bytes)
   - Uses `RAND_bytes()` to fill it with random values
   - The IV is used to add randomness to the encryption process

5. **Set Up Encryption Context:**
   - Creates a new OpenSSL cipher context with `EVP_CIPHER_CTX_new()`
   - This context will hold the state of the encryption operation

6. **Initialize Encryption:**
   - Calls `EVP_EncryptInit_ex()` to set up the encryption parameters:
     - `EVP_aes_256_cbc()`: Specifies AES-256 in CBC (Cipher Block Chaining) mode
     - `key.data()`: Provides the encryption key
     - `iv.data()`: Provides the initialization vector

7. **Prepare Output Buffer:**
   - Allocates a vector for the encrypted output
   - Makes it slightly larger than the input size to accommodate padding
   - AES works on 16-byte blocks, so the last block may need padding

8. **Encrypt Data:**
   - Calls `EVP_EncryptUpdate()` to encrypt the input data
   - Stores the encrypted data in the output buffer
   - Keeps track of how many bytes were written (`outLen1`)

9. **Finalize Encryption:**
   - Calls `EVP_EncryptFinal_ex()` to process any remaining data and add padding
   - Adds the final block to the output buffer after `outLen1` bytes
   - Records how many additional bytes were written (`outLen2`)

10. **Clean Up:**
    - Frees the encryption context to prevent memory leaks

11. **Resize Output:**
    - Adjusts the size of the encrypted data vector to match the actual amount of data written

12. **Format Final Output:**
    - Creates a result vector that combines:
      - The salt (16 bytes): Needed for key derivation during decryption
      - The IV (16 bytes): Needed for decryption
      - The encrypted data: The actual ciphertext
    - This format ensures all necessary information is available for decryption

13. **Return Result:**
    - Returns the complete encrypted package

### Security Considerations:
- Fresh random salt and IV values are generated for each encryption operation
- The salt and IV are stored with the encrypted data (this is standard practice)
- AES-256-CBC is a widely trusted encryption algorithm
- Exception handling ensures resources are properly cleaned up even if errors occur

## Public Method: decrypt

```cpp
std::vector<unsigned char> decrypt(const std::vector<unsigned char>& encryptedData, const std::string& password) {
    if (encryptedData.size() <= SALT_LENGTH + IV_LENGTH) {
        throw std::runtime_error("Encrypted data is too small");
    }
    
    // Extract salt and IV
    std::vector<unsigned char> salt(encryptedData.begin(), encryptedData.begin() + SALT_LENGTH);
    std::vector<unsigned char> iv(encryptedData.begin() + SALT_LENGTH, 
                                 encryptedData.begin() + SALT_LENGTH + IV_LENGTH);
    
    // Derive key from password and salt
    std::vector<unsigned char> key = deriveKey(password, salt);
    
    // Extract actual encrypted data
    std::vector<unsigned char> actualData(encryptedData.begin() + SALT_LENGTH + IV_LENGTH, 
                                        encryptedData.end());
    
    // Setup decryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create decryption context");
    }
    
    // Initialize decryption
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }
    
    // Allocate output buffer
    std::vector<unsigned char> decryptedData(actualData.size());
    int outLen1 = 0;
    
    // Decrypt data
    if (EVP_DecryptUpdate(ctx, decryptedData.data(), &outLen1, actualData.data(), actualData.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption failed");
    }
    
    // Finalize decryption
    int outLen2 = 0;
    if (EVP_DecryptFinal_ex(ctx, decryptedData.data() + outLen1, &outLen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption finalization failed: Invalid password or corrupted data");
    }
    
    // Free context
    EVP_CIPHER_CTX_free(ctx);
    
    // Resize decrypted data to actual size
    decryptedData.resize(outLen1 + outLen2);
    
    return decryptedData;
}
```

This method decrypts data that was previously encrypted using the `encrypt` method:

### Parameters:
- `encryptedData`: The encrypted data package containing salt, IV, and ciphertext
- `password`: The password that was used for encryption

### How it Works:

1. **Validate Input:**
   - Checks if the encrypted data is at least large enough to contain the salt and IV
   - Throws an exception if the data is too small

2. **Extract Salt and IV:**
   - Extracts the first `SALT_LENGTH` bytes as the salt
   - Extracts the next `IV_LENGTH` bytes as the initialization vector
   - These were prepended to the actual encrypted data by the `encrypt` method

3. **Derive Key:**
   - Calls the private `deriveKey()` method using the extracted salt and provided password
   - This recreates the same encryption key that was used for encryption

4. **Extract Encrypted Data:**
   - Extracts everything after the salt and IV as the actual encrypted data

5. **Set Up Decryption Context:**
   - Creates a new OpenSSL cipher context with `EVP_CIPHER_CTX_new()`

6. **Initialize Decryption:**
   - Calls `EVP_DecryptInit_ex()` to set up the decryption parameters:
     - `EVP_aes_256_cbc()`: Specifies AES-256 in CBC mode (same as for encryption)
     - `key.data()`: Provides the derived key
     - `iv.data()`: Provides the extracted initialization vector

7. **Prepare Output Buffer:**
   - Allocates a vector for the decrypted output
   - Size is initially set to match the encrypted data (will be adjusted later)

8. **Decrypt Data:**
   - Calls `EVP_DecryptUpdate()` to decrypt the input data
   - Stores the decrypted data in the output buffer
   - Keeps track of how many bytes were written (`outLen1`)

9. **Finalize Decryption:**
   - Calls `EVP_DecryptFinal_ex()` to process any remaining data and handle padding
   - This step will fail if the password is incorrect or the data is corrupted
   - Adds the final block to the output buffer after `outLen1` bytes
   - Records how many additional bytes were written (`outLen2`)

10. **Clean Up:**
    - Frees the decryption context to prevent memory leaks

11. **Resize Output:**
    - Adjusts the size of the decrypted data vector to match the actual amount of data written

12. **Return Result:**
    - Returns the decrypted data

### Security Considerations:
- The decryption process will fail if:
  - The wrong password is provided
  - The encrypted data has been tampered with
  - The data structure doesn't match what's expected
- The error message specifically mentions "Invalid password or corrupted data" to indicate these possibilities

## Public Method: verifyPassword

```cpp
bool verifyPassword(const std::vector<unsigned char>& encryptedData, const std::string& password) {
    try {
        // Try to decrypt the first block to verify password
        if (encryptedData.size() <= SALT_LENGTH + IV_LENGTH) {
            return false;
        }
        
        // Extract salt and IV
        std::vector<unsigned char> salt(encryptedData.begin(), encryptedData.begin() + SALT_LENGTH);
        std::vector<unsigned char> iv(encryptedData.begin() + SALT_LENGTH, 
                                     encryptedData.begin() + SALT_LENGTH + IV_LENGTH);
        
        // Derive key from password and salt
        std::vector<unsigned char> key = deriveKey(password, salt);
        
        // Setup decryption context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            return false;
        }
        
        // Initialize decryption
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        // Just try to decrypt a small portion
        size_t sampleSize = std::min(static_cast<size_t>(64), encryptedData.size() - SALT_LENGTH - IV_LENGTH);
        std::vector<unsigned char> actualData(encryptedData.begin() + SALT_LENGTH + IV_LENGTH, 
                                            encryptedData.begin() + SALT_LENGTH + IV_LENGTH + sampleSize);
        
        std::vector<unsigned char> decryptedData(sampleSize + 16);  // Add buffer for padding
        int outLen = 0;
        
        // Decrypt sample
        if (EVP_DecryptUpdate(ctx, decryptedData.data(), &outLen, actualData.data(), actualData.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        // Try to finalize
        int finalLen = 0;
        bool success = EVP_DecryptFinal_ex(ctx, decryptedData.data() + outLen, &finalLen) == 1;
        
        EVP_CIPHER_CTX_free(ctx);
        return success;
        
    } catch (...) {
        return false;
    }
}
```

This method checks if a given password is correct for a particular encrypted data package, without performing the full decryption:

### Parameters:
- `encryptedData`: The encrypted data package containing salt, IV, and ciphertext
- `password`: The password to verify

### How it Works:

1. **Wrap in Try-Catch:**
   - Puts the entire operation in a try-catch block
   - Returns false for any exception, simplifying error handling

2. **Validate Input:**
   - Checks if the encrypted data is at least large enough to contain the salt and IV
   - Returns false if the data is too small

3. **Extract Salt and IV:**
   - Extracts the first `SALT_LENGTH` bytes as the salt
   - Extracts the next `IV_LENGTH` bytes as the initialization vector

4. **Derive Key:**
   - Calls the private `deriveKey()` method using the extracted salt and provided password

5. **Set Up Decryption Context:**
   - Creates a new OpenSSL cipher context

6. **Initialize Decryption:**
   - Sets up the decryption parameters just like in the `decrypt` method

7. **Extract Sample Data:**
   - Instead of decrypting the entire data, selects only a small portion (up to 64 bytes)
   - This is a performance optimization since we only need to verify the password
   - The value 64 bytes is a **hard-coded optimization parameter**

8. **Prepare Output Buffer:**
   - Allocates a vector for the decrypted output
   - Adds extra 16 bytes for padding (AES block size)

9. **Decrypt Sample:**
   - Decrypts only the small sample of data

10. **Verify Decryption:**
    - Tries to finalize the decryption with `EVP_DecryptFinal_ex()`
    - If this succeeds, the password is correct
    - If this fails, the password is incorrect

11. **Clean Up and Return:**
    - Frees the decryption context
    - Returns true if successful, false otherwise

### Security Considerations:
- This method is more efficient than trying to decrypt the entire file
- It still performs the full key derivation process, maintaining security
- The try-catch block ensures the method always returns a valid boolean
- This method does not reveal any information about why a password failed

## End of File

```cpp
#endif // ENCRYPTION_MODULE_H
```

This closes the include guard started at the beginning of the file.

## The AES-256-CBC Encryption Algorithm

The EncryptionModule uses AES-256 in CBC mode:

### AES (Advanced Encryption Standard)
- A symmetric encryption algorithm (same key for encryption and decryption)
- Established by the U.S. National Institute of Standards and Technology (NIST)
- Widely used worldwide for secure communications
- Operates on fixed-size blocks of data (128 bits / 16 bytes)
- The "256" refers to the key size in bits (stronger than 128 or 192-bit variants)

### CBC Mode (Cipher Block Chaining)
- Each block of plaintext is XORed with the previous ciphertext block before encryption
- This creates a dependency between blocks, improving security
- Requires an IV (Initialization Vector) for the first block
- The IV must be unique but not secret for each encryption operation

## Hard-Coded Security Parameters

The code contains several security-related constants and values:

1. **Key Length: 32 bytes (256 bits)**
   - AES-256 is currently considered secure against all known attacks
   - Even quantum computers would need extremely large resources to break it

2. **IV Length: 16 bytes (128 bits)**
   - This is the standard block size for AES
   - Random IVs of this size ensure each encryption produces different results

3. **Salt Length: 16 bytes**
   - 16 random bytes provide 2^128 possible salts
   - This is sufficient to prevent precomputation attacks

4. **PBKDF2 Iteration Count: 10,000**
   - Higher values increase the computational cost of guessing passwords
   - 10,000 is a reasonable value for most applications
   - Critical applications might use 100,000+ iterations for greater security

5. **Sample Size for Password Verification: 64 bytes**
   - Balances efficiency and security
   - Only needs to decrypt a small portion to verify the password

## Complete Usage Example

Here's how the EncryptionModule would be used in an application:

```cpp
// Create an instance of the encryption module
EncryptionModule encryptor;

// Original data to encrypt
std::vector<unsigned char> originalData = {/* data bytes */};
std::string password = "my-secure-password";

// Encrypt the data
try {
    std::vector<unsigned char> encryptedData = encryptor.encrypt(originalData, password);
    
    // Store or transmit the encrypted data
    
    // Later, when you need to decrypt:
    
    // First, verify the password (optional)
    if (encryptor.verifyPassword(encryptedData, password)) {
        // Password is correct, proceed with decryption
        std::vector<unsigned char> decryptedData = encryptor.decrypt(encryptedData, password);
        
        // Use the decrypted data
    } else {
        // Password is incorrect
        std::cout << "Incorrect password!" << std::endl;
    }
    
} catch (const std::exception& e) {
    std::cerr << "Encryption error: " << e.what() << std::endl;
}
```

## OpenSSL Integration

This implementation relies on OpenSSL, which has some important considerations:

1. **External Dependency**:
   - The application must be linked with the OpenSSL libraries
   - Typically requires `-lcrypto -lssl` flags during compilation

2. **Version Compatibility**:
   - The code was written for OpenSSL 1.1.x
   - Newer or older versions might require adjustments

3. **Initialization**:
   - Modern OpenSSL (1.1.0+) doesn't require explicit initialization
   - Older versions would need calls to `SSL_library_init()` and related functions

## Best Practices Implemented

The code follows several cryptographic best practices:

1. **Secure Key Derivation**:
   - Uses PBKDF2 with many iterations to slow down brute force attacks
   - Applies unique salt for each encryption operation

2. **Unique IVs**:
   - Generates a new random IV for each encryption operation

3. **Padding Verification**:
   - Validates padding during decryption to detect tampering or incorrect passwords

4. **Cleanup**:
   - Properly frees all allocated OpenSSL resources
   - Uses exception handling to ensure cleanup even in error conditions

5. **Error Handling**:
   - Provides meaningful error messages
   - Returns appropriate values for password verification

## Potential Enhancements

For even higher security, the following enhancements could be considered:

1. **Key Stretching**:
   - Increase the iteration count in the PBKDF2 function (e.g., to 100,000+)
   - Consider using Argon2 instead of PBKDF2 for better resistance to GPU attacks

2. **Authenticated Encryption**:
   - Use AES-GCM mode or add an HMAC to detect tampering
   - This would protect against certain types of attacks

3. **Secure Memory Handling**:
   - Zero out sensitive data (like keys) after use
   - Use secure memory allocation for cryptographic materials

4. **Key Rotation**:
   - Add support for changing passwords without full re-encryption
   - Implement key versioning for long-term data storage
