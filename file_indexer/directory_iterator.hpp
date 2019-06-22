#pragma once

#include <filesystem>

#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

namespace indexer {

namespace fs = std::filesystem;

class directory_iterator
    : public std::enable_shared_from_this<directory_iterator> {
public:
    directory_iterator(
        boost::asio::io_context& io,
        const fs::path& root,
        fs::directory_options options,
        std::function<void(const fs::path&)> handler,
        std::function<void(std::size_t)> completion_handler = [](std::size_t) {},
        std::function<bool(const fs::path&)> filter =
            [](const fs::path&) { return true; })
        : io_{io},
          root_{root},
          handler_{std::move(handler)},
          completion_handler_{std::move(completion_handler)},
          filter_{std::move(filter)},
          dir_iterator_{root, options}
    {
    }

    directory_iterator(const directory_iterator&) = delete;
    directory_iterator(directory_iterator&&) = delete;
    directory_iterator& operator=(const directory_iterator&) = delete;
    directory_iterator& operator=(directory_iterator&&) = delete;

    void start() { async_iter_one(); }

private:
    void async_iter_one(int handled = 0)
    {
        auto self = shared_from_this();
        boost::asio::post(io_, [this, self, handled] {
            if (dir_iterator_ != std::filesystem::end(dir_iterator_)) {
                const auto& p = *(dir_iterator_++);
                int new_handled = handled;

                if (filter_(p)) {
                    handler_(p);
                    ++new_handled;
                }

                async_iter_one(new_handled);
            }
            else {
                completion_handler_(handled);
            }
        });
    }

    boost::asio::io_context& io_;
    fs::path root_;
    std::function<void(const fs::path&)> handler_;
    std::function<void(std::size_t)> completion_handler_;
    std::function<bool(const fs::path&)> filter_;
    fs::recursive_directory_iterator dir_iterator_;
};

} // namespace indexer
