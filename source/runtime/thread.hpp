#pragma once
#include "runtime/frame.hpp"

#include <span>

class Thread {
public:
    Thread() = default;

    Frame& current_frame() noexcept { return frames_.back(); }
    void push_frame(const Method& method, std::span<const Value> args);
    void pop_frame() noexcept { frames_.pop_back(); }
    bool is_terminated() const noexcept { return frames_.empty(); }
private:
    std::vector<Frame> frames_;
};
