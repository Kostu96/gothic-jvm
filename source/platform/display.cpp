#include "platform/display.hpp"

#include <SDL3/SDL.h>

#include <stdexcept>
#include <string>

Display::Display(const std::string& title, int width, int height, int scale) :
    width_(width),
    height_(height),
    scale_(scale)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    if (!SDL_CreateWindowAndRenderer(title.c_str(), width * scale, height * scale, 0, &window_, &renderer_)) {
        std::string error = SDL_GetError();
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindowAndRenderer failed: " + error);
    }

    framebuffer_ = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ARGB8888);

    // Cap the frame rate to the display refresh so the render loop does not
    // spin at 100% CPU.
    SDL_SetRenderVSync(renderer_, 1);
}

Display::~Display() {
    if (framebuffer_ != nullptr) {
        SDL_DestroySurface(framebuffer_);
    }
    if (renderer_ != nullptr) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
}

bool Display::process_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            return false;
        default:
            break;
        }
    }
    return true;
}

void Display::clear(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    SDL_SetRenderDrawColor(renderer_, r, g, b, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer_);
}

void Display::present() {
    SDL_RenderPresent(renderer_);
}

void Display::flush() {
    std::lock_guard<std::mutex> lock(fb_mutex_);
    fb_ready_ = true;
    fb_cv_.notify_one();
}

void Display::render() {
    std::unique_lock<std::mutex> lock(fb_mutex_);
    fb_cv_.wait_for(lock, std::chrono::milliseconds(16));
    static SDL_Texture* texture = nullptr;
    if (fb_ready_) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
        texture = SDL_CreateTextureFromSurface(renderer_, framebuffer_);
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        fb_ready_ = false;
    }
    if (texture) {
        SDL_FRect dst_rect{ 0, 0, static_cast<float>(width_ * scale_), static_cast<float>(height_ * scale_) };
        SDL_RenderTexture(renderer_, texture, nullptr, &dst_rect);
    }
}
