#ifndef CRYPTAGE_HPP
#define CRYPTAGE_HPP

#include <string>
#include <cstring>
#include <random>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/cryptlib.h>

class Cryptage
{
    private:

        std::string base64_decode(const std::string& input);
        std::string base64_encode(const std::string& input);
        
    public:
    Cryptage() = default;
    Cryptage(const Cryptage &cryptage) = default;
    ~Cryptage() = default;
        
    Cryptage &operator=(const Cryptage &cryptage) = default;

    std::string aes_encrypt(const std::string& plaintext, const std::string& key);
    std::string aes_decrypt(const std::string& ciphertext, const std::string& key);

    std::string crypter(std::string texte, std::string cle);
    std::string decrypter(std::string texte, std::string cle);
};

#endif