#ifndef ENCRYPTION_MODULE_H
#define ENCRYPTION_MODULE_H

#include <vector>
#include <string>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <stdexcept>
#include <cstring>

class EncryptionModule {
private:
    // AES-256 constants
    static constexpr int KEY_LENGTH = 32; // 256 bits
    static constexpr int IV_LENGTH = 16;  // 128 bits
    static constexpr int SALT_LENGTH = 16;
    
    // Derive encryption key from password
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

public:
    // Encrypt data using AES-256-CBC
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
    
    // Decrypt data using AES-256-CBC
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
    
    // Verify if the provided password is correct for encrypted data
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
};

#endif // ENCRYPTION_MODULE_H