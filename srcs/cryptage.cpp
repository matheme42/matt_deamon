#include "cryptage.hpp"
#include <iostream>

std::string Cryptage::crypter(std::string texte, std::string cle)
{
    if (cle.size() == 0) return texte;
    try {
        std::string msg = aes_encrypt(texte, cle);
        return base64_encode(msg);
    } catch (std::exception& e) {
        return texte;
    }
}

std::string Cryptage::decrypter(std::string texte, std::string cle)
{
    if (cle.size() == 0) return texte;
    try {
         std::string msg = base64_decode(texte);
    return aes_decrypt(msg, cle);   
    } catch (std::exception& e) {
        return texte;
    }
}


std::string generate_unique_key(const std::string& key)
{
    EVP_MD_CTX* mdctx;
    const EVP_MD* md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    
    md = EVP_sha256();
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, key.c_str(), key.size());
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_free(mdctx);
    
    std::string unique_key;
    for (int i = 0; i < md_len; i++) {
        unique_key += md_value[i];
    }
    return unique_key;
}


std::string Cryptage::aes_encrypt(const std::string& plaintext, const std::string& key) {
    // La clé doit avoir une longueur de 16, 24 ou 32 octets pour AES-128, AES-192 ou AES-256 respectivement
    std::string key32 = generate_unique_key(key);
    CryptoPP::SecByteBlock aesKey((const unsigned char*)key32.data(), key32.size());
    if (aesKey.size() != 16 && aesKey.size() != 24 && aesKey.size() != 32) {
        throw std::runtime_error("La longueur de la clé doit être 16, 24 ou 32 octets pour AES");
    }

    // Génère un vecteur d'initialisation (IV) aléatoire
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    unsigned char iv[CryptoPP::AES::BLOCKSIZE];
    for (int i = 0; i < CryptoPP::AES::BLOCKSIZE; i++) {
        iv[i] = (unsigned char)distrib(gen);
    }

    // Chiffrement AES-CBC
    std::string ciphertext;
    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption aesEncryption(aesKey, aesKey.size(), iv);
    CryptoPP::StringSource plaintextSource(plaintext, true,
        new CryptoPP::StreamTransformationFilter(aesEncryption,
            new CryptoPP::StringSink(ciphertext)));

    // Concatène le IV et le texte chiffré en un seul message
    std::string result;
    result.reserve(sizeof(iv) + ciphertext.size());
    result.append((const char*)iv, sizeof(iv));
    result.append(ciphertext);
    return result;
}

std::string Cryptage::aes_decrypt(const std::string& ciphertext, const std::string& key) {
 // La clé doit avoir une longueur de 16, 24 ou 32 octets pour AES-128, AES-192 ou AES-256 respectivement
    std::string key32 = generate_unique_key(key);
    CryptoPP::SecByteBlock aesKey((const unsigned char*)key32.data(), key32.size());
    if (aesKey.size() != 16 && aesKey.size() != 24 && aesKey.size() != 32) {
        throw std::runtime_error("La longueur de la clé doit être 16, 24 ou 32 octets pour AES");
    }

    // Récupère le vecteur d'initialisation (IV) à partir du message chiffré
    unsigned char iv[CryptoPP::AES::BLOCKSIZE];
    memcpy(iv, ciphertext.data(), CryptoPP::AES::BLOCKSIZE);

    // Récupère le texte chiffré à partir du message chiffré
    std::string encryptedData(ciphertext, CryptoPP::AES::BLOCKSIZE);

    // Déchiffrement AES-CBC
    std::string decryptedData;
    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption aesDecryption(aesKey, aesKey.size(), iv);
    CryptoPP::StringSource encryptedDataSource(encryptedData, true,
        new CryptoPP::StreamTransformationFilter(aesDecryption,
            new CryptoPP::StringSink(decryptedData)));

    return decryptedData;
}



std::string Cryptage::base64_encode(const std::string& input) {
    // Initialize the Base64 BIO filter
    BIO* b64 = BIO_new(BIO_f_base64());
    if (b64 == nullptr) {
       exit(1);
    }
    
    // Set the BIO to use a newline after every 64 characters
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    
    // Allocate a buffer for the encoded data
    BIO* bmem = BIO_new(BIO_s_mem());
    if (bmem == nullptr) {
        BIO_free_all(b64);
       exit(1);
    }
    
    // Chain the BIO filters together
    BIO_push(b64, bmem);
    
    // Write the input data to the filter chain
    if (BIO_write(b64, input.c_str(), input.length()) <= 0) {
        BIO_free_all(b64);
       exit(1);
    }
    
    // Flush the filter chain and get the encoded data
    if (BIO_flush(b64) != 1) {
        BIO_free_all(b64);
       exit(1);
    }
    
    // Get the encoded data from the memory buffer
    BUF_MEM* bptr;
    BIO_get_mem_ptr(bmem, &bptr);
    if (bptr == nullptr) {
        BIO_free_all(b64);
       exit(1);
    }
    std::string output(bptr->data, bptr->length);
    
    // Clean up the filter chain
    BIO_free_all(b64);
    
    return output;
}


std::string Cryptage::base64_decode(const std::string& input) {
    // Initialize the Base64 BIO filter
    BIO* b64 = BIO_new(BIO_f_base64());
    if (b64 == nullptr) {
       exit(1);
    }
    
    // Set the BIO to ignore whitespace
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    
    // Allocate a buffer for the decoded data
    BIO* bmem = BIO_new_mem_buf(input.c_str(), input.length());
    if (bmem == nullptr) {
        BIO_free_all(b64);
       exit(1);
    }
    
    // Chain the BIO filters together
    BIO_push(b64, bmem);
    
    // Allocate a buffer for the decoded data
    std::string output(input.length(), '\0');
    
    // Read the decoded data from the filter chain
    int len = BIO_read(b64, &output[0], output.length());
    if (len < 0) {
        BIO_free_all(b64);
       exit(1);
    }
    output.resize(len);
    
    // Clean up the filter chain
    BIO_free_all(b64);
    
    return output;
}
