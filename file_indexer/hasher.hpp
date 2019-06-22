#pragma once

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string_view>

#include <openssl/sha.h>
#include <spdlog/spdlog.h>

namespace indexer {

template <typename Hash>
inline std::string hash_to_hex(const Hash& hash)
{
    std::ostringstream ret;
    std::for_each(hash.begin(), hash.end(), [&](char c) {
        int ic = static_cast<int>(static_cast<unsigned char>(c));
        ret << std::hex << std::setfill('0') << std::setw(2) << ic;
    });
    return ret.str();
}

template <typename Hash>
inline Hash hex_to_hash(const std::string& s)
{
    Hash hash;
    for (std::size_t i = 0; i < hash.size(); ++i) {
        hash[i] = std::stoi(s.substr(i * 2, 2), nullptr, 16);
    }
    return hash;
}

struct sha1 {
    static constexpr int kChunkSize = 4096;
    static constexpr int kHashSize = SHA_DIGEST_LENGTH;
    using hash_type = std::array<char, kHashSize>;

    hash_type operator()(const std::filesystem::path& p)
    {
        hash_type hash;
        std::size_t bytes;
        SHA_CTX ctx;
        std::FILE* in = std::fopen(p.c_str(), "rb");
        if (!in) {
            throw std::runtime_error(std::strerror(errno));
        }

        SHA1_Init(&ctx);

        do {
            bytes = std::fread(hash.data(), 1, hash.size(), in);
            SHA1_Update(&ctx, hash.data(), bytes);
        } while (bytes > 0);

        SHA1_Final(reinterpret_cast<unsigned char*>(hash.data()), &ctx);
        std::fclose(in);

        SPDLOG_TRACE("{}: {}", p.string(), hash_to_hex(hash));
        return hash;
    }
};

struct sha256 {
    static constexpr int kChunkSize = 4096;
    static constexpr int kHashSize = SHA256_DIGEST_LENGTH;
    using hash_type = std::array<char, kHashSize>;

    hash_type operator()(const std::filesystem::path& p)
    {
        hash_type hash;
        std::size_t bytes;
        SHA256_CTX ctx;
        std::FILE* in = std::fopen(p.c_str(), "rb");
        if (!in) {
            throw std::runtime_error(std::strerror(errno));
        }

        SHA256_Init(&ctx);

        do {
            bytes = std::fread(hash.data(), 1, hash.size(), in);
            SHA256_Update(&ctx, hash.data(), bytes);
        } while (bytes > 0);

        SHA256_Final(reinterpret_cast<unsigned char*>(hash.data()), &ctx);
        std::fclose(in);

        SPDLOG_TRACE("{}: {}", p.string(), hash_to_hex(hash));
        return hash;
    }
};

struct sha512 {
    static constexpr int kChunkSize = 4096;
    static constexpr int kHashSize = SHA512_DIGEST_LENGTH;
    using hash_type = std::array<char, kHashSize>;

    hash_type operator()(const std::filesystem::path& p)
    {
        hash_type hash;
        std::size_t bytes;
        SHA512_CTX ctx;
        std::FILE* in = std::fopen(p.c_str(), "rb");
        if (!in) {
            throw std::runtime_error(std::strerror(errno));
        }

        SHA512_Init(&ctx);

        do {
            bytes = std::fread(hash.data(), 1, hash.size(), in);
            SHA512_Update(&ctx, hash.data(), bytes);
        } while (bytes > 0);

        SHA512_Final(reinterpret_cast<unsigned char*>(hash.data()), &ctx);
        std::fclose(in);

        SPDLOG_TRACE("{}: {}", p.string(), hash_to_hex(hash));
        return hash;
    }
};

} // namespace indexer

namespace std {

template <>
struct hash<indexer::sha1::hash_type> {
    std::size_t operator()(const indexer::sha1::hash_type& arg) const
    {
        return std::hash<std::string_view>{}(
            std::string_view(arg.data(), arg.size()));
    }
};

template <>
struct hash<indexer::sha256::hash_type> {
    std::size_t operator()(const indexer::sha256::hash_type& arg) const
    {
        return std::hash<std::string_view>{}(
            std::string_view(arg.data(), arg.size()));
    }
};

template <>
struct hash<indexer::sha512::hash_type> {
    std::size_t operator()(const indexer::sha512::hash_type& arg) const
    {
        return std::hash<std::string_view>{}(
            std::string_view(arg.data(), arg.size()));
    }
};

} // namespace std
