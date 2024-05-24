#pragma once

#include <optional>
#include <cstdint>
#include <queue>
#include <stdexcept>

#include <mutex>
#include <condition_variable>

#include <iostream>

template <class T>
class BufferedChannel {
public:
    BufferedChannel(uint32_t size) : is_closed_(false), max_size_(size) {
    }

    void Send(const T& value) {
        std::unique_lock lock{mutex_};
        if (is_closed_) {
            throw std::runtime_error("sending from closing channel");
        }
        wcv_.wait(lock, [this] { return q_.size() < max_size_ || is_closed_; });
        if (is_closed_) {
            throw std::runtime_error("sending from closing channel");
        }
        q_.emplace(value);
        rcv_.notify_one();
    }

    void Send(T&& value) {
        std::unique_lock lock{mutex_};
        if (is_closed_) {
            throw std::runtime_error("sending from closing channel");
        }
        wcv_.wait(lock, [this] { return q_.size() < max_size_ || is_closed_; });
        if (is_closed_) {
            throw std::runtime_error("sending from closing channel");
        }
        q_.emplace(std::move(value));
        rcv_.notify_one();
    }

    std::optional<T> Recv() {
        std::unique_lock lock{mutex_};

        if (is_closed_) {
            if (q_.empty()) {
                return std::nullopt;
            }
        } else {
            rcv_.wait(lock, [this] { return !q_.empty() || is_closed_; });
            if (is_closed_ && q_.empty()) {
                return std::nullopt;
            }
        }

        T res = std::move(q_.front());
        q_.pop();
        wcv_.notify_one();

        return {std::move(res)};
    }

    void Close() {
        std::unique_lock lock{mutex_};
        is_closed_ = true;
        rcv_.notify_all();
        wcv_.notify_all();
    }

private:
    std::queue<T> q_;
    bool is_closed_;
    size_t max_size_;
    std::mutex mutex_;
    std::condition_variable wcv_;
    std::condition_variable rcv_;
};
