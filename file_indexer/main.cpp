#include <iostream>
#include <string>
#include <filesystem>
#include <chrono>

#include "hasher.hpp"
#include "index.hpp"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "give at least one path" << std::endl;
        return 0;
    }

    std::cout << "parsing " << argv[1] << " ..." << std::endl;
    auto start_index_time = std::chrono::steady_clock::now();

    indexer::file_index<indexer::sha1> index;
    int cnt = 0;
    auto dir_it = fs::recursive_directory_iterator(
        argv[1], fs::directory_options::skip_permission_denied);
    for (auto& p: dir_it) {
        if (fs::is_regular_file(p)) {
            index.push(p);
            ++cnt;
        }
    }

    auto index_d= std::chrono::steady_clock::now() - start_index_time;
    std::cout << index.serialize(2) << std::endl;
    std::cout << "parsed " << cnt << " files in "
        << std::chrono::nanoseconds(index_d).count() / 1e6 << "ms" << std::endl;

    while (true) {
        int size;
        std::string hash;
        std::cout << "Enter size: " << std::endl;
        std::cin >> size;
        std::cout << "Enter hash:" << std::endl;
        std::cin >> hash;
        hash = indexer::hex_to_hash(hash);

        auto start_time = std::chrono::steady_clock::now();
        auto path = index.pull(size, hash);
        auto d = std::chrono::steady_clock::now() - start_time;
        if (path) {
            std::cout << *path << std::endl;
        }
        else {
            std::cout << "not found" << std::endl;
        }
        std::cout << "  " << std::chrono::nanoseconds(d).count() / 1e6
            << "ms" << std::endl;
    }

    return 0;
}