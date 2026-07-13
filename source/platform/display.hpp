#pragma once
#include <cstdint>
#include <string>

struct SDL_Window;
struct SDL_Renderer;

// Owns the SDL window and 2D renderer that the JVM will eventually draw the
// MIDlet screen onto. For now it opens a window, can clear it to a solid
// colour, present a frame, and pump window events (close only).
class Display {
public:
    Display(const std::string& title, int width, int height);
    ~Display();

    // Pump pending SDL events. Returns false once the user has asked to close
    // the window, true otherwise.
    bool process_events();

    // Clear the back buffer to the given RGB colour.
    void clear(std::uint8_t r, std::uint8_t g, std::uint8_t b);

    // Present the current frame to the window.
    void present();

    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }
    SDL_Renderer* renderer() const noexcept { return renderer_; }

    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;
private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    int width_ = 0;
    int height_ = 0;
};
