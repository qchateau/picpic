#pragma once

#include <filesystem>
#include <unordered_map>
#include <variant>
#include <optional>

#include <json.hpp>

namespace indexer {

namespace fs = std::filesystem;

template<typename Hasher>
class file_index {
public:
    using path_hash_map = std::unordered_map<std::string, fs::path>;
    using path_variant = std::variant<fs::path, path_hash_map>;
    using path_size_map = std::unordered_map<std::uintmax_t, path_variant>;

    std::optional<fs::path> pull(std::uintmax_t size, const std::string& hash) const {
        auto it = file_map_.find(size);
        if (it != file_map_.end()) {
            if (std::holds_alternative<fs::path>(it->second)) {
                return std::get<fs::path>(it->second);
            }
            else {
                const auto& map = std::get<path_hash_map>(it->second);
                auto hash_it = map.find(hash);
                if (hash_it != map.end()) {
                    return hash_it->second;
                }
            }
        }

        return std::nullopt;
    }

    void push(const fs::path& p) {
        std::uintmax_t size = fs::file_size(p);
        auto it = file_map_.find(size);
        if (it == file_map_.end()) {
            file_map_.emplace_hint(it, size, p);
        }
        else if (std::holds_alternative<fs::path>(it->second)) {
            const auto& existing_path = std::get<fs::path>(it->second);
            if (p != existing_path) {
                it->second = path_hash_map{
                    {Hasher{}(existing_path), existing_path},
                    {Hasher{}(p), p}
                };
            }
        }
        else {
            std::get<path_hash_map>(it->second).insert({Hasher{}(p), p});
        }
    }

    std::string serialize(int indent=0) const {
        auto jsonize_node = [](const path_variant& node) -> nlohmann::json {
            if (std::holds_alternative<fs::path>(node)) {
                return std::get<fs::path>(node).string();
            }
            else {
                nlohmann::json json;
                for (const auto& hash_pair: std::get<path_hash_map>(node)) {
                    json[hash_to_hex(hash_pair.first)] = hash_pair.second.string();
                }
                return json;
            }
        };

        nlohmann::json json;
        for (const auto& file_pair: file_map_) {
            json[std::to_string(file_pair.first)] = jsonize_node(file_pair.second);
        }
        return json.dump(indent);
    }

private:
    path_size_map file_map_;
};

} // indexer
