#pragma once
#include <cstdint>
#include <string>

struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Window;

class Display {
public:
    Display(const std::string& title, int width, int height, int scale);
    ~Display();

    bool process_events();

    void clear(std::uint8_t r, std::uint8_t g, std::uint8_t b);

    void present();

    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }
    SDL_Renderer* renderer() const noexcept { return renderer_; }
    SDL_Surface* framebuffer() const noexcept { return framebuffer_; }

    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;
private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Surface* framebuffer_ = nullptr;
    int width_ = 0;
    int height_ = 0;
    int scale_ = 1;
};
