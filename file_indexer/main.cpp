#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "directory_iterator.hpp"
#include "hasher.hpp"
#include "index.hpp"
#include "server.hpp"

namespace fs = std::filesystem;
namespace po = boost::program_options;

using Clock = std::chrono::steady_clock;
using Indexer = indexer::file_index<indexer::sha1>;
using Protocol = boost::asio::ip::tcp;

constexpr int kDefaultPort = 54321;

int main(int argc, char* argv[])
{
    uint16_t port;
    std::vector<std::string> paths;

    po::options_description desc("Usage");
    // clang-format off
    desc.add_options()
        ("help,h", "produce help message")
        ("port,p", po::value<uint16_t>(&port)->default_value(kDefaultPort),
            "TCP port on which the server will listen")
        ("directories,d", po::value<std::vector<std::string> >(&paths),
            "directories that will be parsed");
    // clang-format on

    po::positional_options_description p;
    p.add("directories", -1);

    po::variables_map vm;
    try {
        auto parsed =
            po::command_line_parser(argc, argv).options(desc).positional(p).run();
        po::store(parsed, vm);
        po::notify(vm);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << desc << std::endl;
        return -1;
    }

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    boost::asio::io_context io;

    auto index = std::make_shared<Indexer>();
    std::vector<std::shared_ptr<indexer::directory_iterator>> dirits;
    std::transform(
        paths.begin(),
        paths.end(),
        std::back_inserter(dirits),
        [&](const std::string& path) {
            auto dirit = std::make_shared<indexer::directory_iterator>(
                io,
                path,
                fs::directory_options::skip_permission_denied,
                [index](const fs::path& p) { index->push(p); },
                [](const fs::path& p) { return fs::is_regular_file(p); });
            dirit->start();
            return dirit;
        });

    Protocol::endpoint ep{boost::asio::ip::make_address("127.0.0.1"), port};
    indexer::server<Protocol, Indexer> server(index, io, ep);

    io.run();

    return 0;
}
