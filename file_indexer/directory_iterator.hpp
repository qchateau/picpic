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
        std::function<bool(const fs::path&)> filter =
            [](const fs::path&) { return true; })
        : io_{io},
          root_{root},
          handler_{std::move(handler)},
          filter_{std::move(filter)},
          dir_iterator_{root, options}
    {
    }

    directory_iterator(const directory_iterator&) = delete;
    directory_iterator(directory_iterator&&) = delete;
    directory_iterator& operator=(const directory_iterator&) = delete;
    directory_iterator& operator=(directory_iterator&&) = delete;

    void start()
    {
        start_time_ = Clock::now();
        async_iter_one();
    }

private:
    using Clock = std::chrono::steady_clock;

    void async_iter_one()
    {
        auto self = shared_from_this();
        boost::asio::post(io_, [this, self] {
            if (dir_iterator_ != std::filesystem::end(dir_iterator_)) {
                const auto& p = *(dir_iterator_++);
                if (filter_(p)) {
                    handler_(p);
                }

                async_iter_one();
            }
            else {
                SPDLOG_INFO(
                    "parsed {} in {:.3f}s",
                    root_.string(),
                    std::chrono::nanoseconds(Clock::now() - start_time_).count()
                        / 1e9);
            }
        });
    }

    boost::asio::io_context& io_;
    fs::path root_;
    std::function<void(const fs::path&)> handler_;
    std::function<bool(const fs::path&)> filter_;
    fs::recursive_directory_iterator dir_iterator_;
    Clock::time_point start_time_;
};

} // namespace indexer
