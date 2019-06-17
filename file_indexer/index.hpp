#pragma once

#include <filesystem>
#include <optional>
#include <unordered_map>
#include <variant>

#include <spdlog/spdlog.h>

namespace indexer {

namespace fs = std::filesystem;

template <typename Hasher>
class file_index {
public:
    using hash_type = typename Hasher::hash_type;
    using path_hash_map = std::unordered_map<hash_type, fs::path>;
    using path_variant = std::variant<fs::path, path_hash_map>;
    using path_size_map = std::unordered_map<std::uintmax_t, path_variant>;

    std::optional<fs::path> pull(std::uintmax_t size, const hash_type& hash) const
    {
        auto it = file_map_.find(size);
        if (it == file_map_.end()) {
            return std::nullopt;
        }

        if (std::holds_alternative<fs::path>(it->second)) {
            return std::get<fs::path>(it->second);
        }
        else {
            const auto& map = std::get<path_hash_map>(it->second);
            auto hash_it = map.find(hash);
            if (hash_it == map.end()) {
                return std::nullopt;
            }
            return hash_it->second;
        }
    }

    void push(const fs::path& p)
    {
        std::uintmax_t size = fs::file_size(p);
        auto it = file_map_.find(size);
        if (it == file_map_.end()) {
            SPDLOG_TRACE("indexing {} by size", p.string());
            file_map_.emplace_hint(it, size, p);
        }
        else if (std::holds_alternative<fs::path>(it->second)) {
            SPDLOG_TRACE("indexing {} by hash, rehashing", p.string());
            const auto& existing_path = std::get<fs::path>(it->second);
            if (p != existing_path) {
                it->second = path_hash_map{
                    {Hasher{}(existing_path), existing_path}, {Hasher{}(p), p}};
            }
        }
        else {
            SPDLOG_TRACE("indexing {} by hash", p.string());
            std::get<path_hash_map>(it->second).insert({Hasher{}(p), p});
        }
    }

private:
    path_size_map file_map_;
};

} // namespace indexer
