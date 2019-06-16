#pragma once

#include <chrono>
#include <memory>

#include <boost/asio.hpp>

#include "hasher.hpp"
#include "index.hpp"
#include <spdlog/spdlog.h>

namespace indexer {

namespace fs = std::filesystem;

template <typename Socket, typename Index>
class session : public std::enable_shared_from_this<session<Socket, Index>> {
public:
    static constexpr char kRequestSep = ';';
    static constexpr char kRequestEnd = '\n';

    using hash_type = typename Index::hash_type;
    using request = std::tuple<std::size_t, hash_type>;

    session(std::shared_ptr<Index> index, Socket sock)
        : index_(std::move(index)), socket_(std::move(sock))
    {
    }

    ~session()
    {
        SPDLOG_INFO("closing connection");
    }

    void start()
    {
        do_read();
    }

private:
    using std::enable_shared_from_this<session>::shared_from_this;
    using Clock = std::chrono::steady_clock;

    std::optional<request> parse() const
    {
        request req;

        std::get<std::size_t>(req) = std::strtol(buffer_.c_str(), nullptr, 10);
        if (std::get<std::size_t>(req) == 0) {
            SPDLOG_WARN("could not parse size");
            return std::nullopt;
        }

        auto sep = buffer_.find(kRequestSep);
        if (sep == std::string::npos) {
            SPDLOG_WARN("could not parse separator");
            return std::nullopt;
        }

        // hash end is 2*sizeof(hash) because we receive a hex representation
        auto hash_start = sep + 1;
        auto hash_end = hash_start + std::get<hash_type>(req).size() * 2;

        if (hash_end >= buffer_.size() || buffer_[hash_end] != kRequestEnd) {
            SPDLOG_WARN("wrong hash size");
            return std::nullopt;
        }

        std::get<hash_type>(req) = hex_to_hash<hash_type>(
            buffer_.substr(hash_start, hash_end - hash_start));
        return req;
    }

    std::optional<fs::path> pull(const request& req)
    {
        return index_->pull(
            std::get<std::size_t>(req), std::get<hash_type>(req));
    }

    void do_read()
    {
        auto self(shared_from_this());
        buffer_.clear();
        boost::asio::async_read_until(
            socket_,
            boost::asio::dynamic_buffer(buffer_),
            kRequestEnd,
            [this, self](boost::system::error_code ec, std::size_t bytes) {
                using namespace std::string_literals;

                start_of_request_ = Clock::now();

                if (ec) {
                    response_ = ec.message() + kRequestEnd;
                    if (ec != boost::asio::error::eof) {
                        SPDLOG_WARN("read failed: {}", ec.message());
                    }
                    return;
                }

                SPDLOG_INFO("request: {}", buffer_.substr(0, bytes - 1));
                (void)bytes;

                auto req = parse();
                if (req) {
                    auto path = pull(*req);
                    if (path) {
                        response_ = path->string() + kRequestEnd;
                    }
                    else {
                        response_ = "not found"s + kRequestEnd;
                    }
                }
                else {
                    response_ = "bad request"s + kRequestEnd;
                }

                // write response
                do_write();
            });
    }

    void do_write()
    {
        std::chrono::nanoseconds request_duration =
            Clock::now() - start_of_request_;
        SPDLOG_INFO(
            "response: {} ({}ms)",
            response_.substr(0, response_.size() - 1),
            request_duration.count() / 1e6);
        (void)request_duration;
        auto self(shared_from_this());

        boost::asio::async_write(
            socket_,
            boost::asio::buffer(response_),
            [this, self](boost::system::error_code, std::size_t) {
                do_read();
            });
    }

    std::shared_ptr<Index> index_;
    Socket socket_;
    std::string buffer_;
    std::string response_;
    Clock::time_point start_of_request_;
}; // namespace indexer

template <typename Protocol, typename Index>
class server {
public:
    using endpoint = typename Protocol::endpoint;
    using acceptor = typename Protocol::acceptor;
    using socket = typename Protocol::socket;

    server(
        std::shared_ptr<Index> index,
        boost::asio::io_context& io_context,
        const endpoint& ep)
        : index_(std::move(index)), acceptor_(io_context, ep)
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept([this](
                                   boost::system::error_code ec, socket sock) {
            if (!ec) {
                SPDLOG_INFO("accepting connection");
                std::make_shared<session<socket, Index>>(
                    index_, std::move(sock))
                    ->start();
            }
            else {
                SPDLOG_WARN("failed to accept connection: {}", ec.message());
            }

            do_accept();
        });
    }

    std::shared_ptr<Index> index_;
    acceptor acceptor_;
};

} // namespace indexer
