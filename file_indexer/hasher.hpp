#pragma once

#include <openssl/sha.h>
#include <filesystem>
#include <fstream>
#include <array>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace indexer {

inline std::string hash_to_hex(const std::string& s) {
    std::ostringstream ret;
    std::for_each(s.begin(), s.end(), [&](char c) {
        int ic = static_cast<int>(static_cast<unsigned char>(c));
        ret << std::hex << std::setfill('0') << std::setw(2) << ic;
    });
    return ret.str();
}

inline std::string hex_to_hash(const std::string& s) {
    std::string hash(SHA_DIGEST_LENGTH, '\0');
    for (std::size_t i=0; i<hash.size(); ++i) {
        hash[i] = std::stoi(s.substr(i*2, 2), nullptr, 16);
    }
    return hash;
}

struct sha1 {
    static constexpr int kChunkSize = 4096;

    std::string operator()(const std::filesystem::path& p) {
        std::array<char, kChunkSize> buffer;
        std::ifstream f(p);
        std::streamsize bytes;
        SHA_CTX ctx;

        SHA1_Init(&ctx);

        do {
            bytes = f.readsome(buffer.data(), buffer.size());
            SHA1_Update(&ctx, buffer.data(), bytes);
        } while (bytes > 0);

        SHA1_Final(reinterpret_cast<unsigned char*>(buffer.data()), &ctx);

        return std::string(buffer.data(), SHA_DIGEST_LENGTH);
    }
};

} // indexer
