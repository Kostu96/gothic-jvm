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

    void set_pending_exception(Object* e) noexcept { pending_exception_ = e; }
    Object* pending_exception() const noexcept { return pending_exception_; }
    bool has_pending_exception() const noexcept { return pending_exception_ != nullptr; }
    void clear_pending_exception() noexcept { pending_exception_ = nullptr; }
private:
    std::vector<Frame> frames_;
    Object* pending_exception_ = nullptr;
};
