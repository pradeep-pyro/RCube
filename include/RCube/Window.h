#pragma once

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace rcube
{

class InputState
{
    InputState();
    ~InputState() = default;

  public:
    InputState(const InputState &) = delete;
    InputState &operator=(const InputState &) = delete;
    static InputState &instance();

    enum class Key
    {
        A = GLFW_KEY_A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z
    };

    enum class Mouse
    {
        Left,
        Right,
        Middle,
    };

    enum class ButtonState
    {
        Down,
        JustDown,
        Released,
        JustReleased
    };

    bool isKeyDown(Key aKey) const;
    bool isKeyJustDown(Key aKey) const;
    ButtonState keyState(Key aKey) const;
    bool isMouseDown(Mouse aMouseButton) const;
    bool isMouseJustDown(Mouse aMouseButton) const;
    ButtonState mouseState(Mouse aMouseButton) const;
    const glm::dvec2 &mousePos() const;
    const glm::dvec2 scrollAmount();
    void setScrollAmount(double xscroll, double yscroll);
    const glm::ivec2 &windowSize() const;
    bool isMouseInside() const;
    void setMouseInside(bool flag);
    void update(GLFWwindow *window);

  private:
    std::vector<ButtonState> keystate_;
    std::vector<ButtonState> mousestate_;
    glm::dvec2 mousepos_ = glm::dvec2(0.0, 0.0);
    glm::dvec2 scroll_ = glm::vec2(0.f, 0.f);
    glm::ivec2 window_size_ = glm::ivec2(1280, 720);
    bool mouse_inside_ = false;
};

class Window
{
  public:
    Window(const std::string &title, glm::ivec2 size = glm::ivec2(1280, 720));
    virtual ~Window() = default;
    void execute();
    bool fullscreen() const;
    void setFullscreen(bool flag);
    glm::ivec2 size() const;
    virtual void onResize(int width, int height);
    virtual void onKeyPress(int key, int mods);
    virtual void onKeyRelease(int key, int mods);
    virtual void onKeyRepeat(int key, int mods);
    virtual void onMousePress(int button, int mods);
    virtual void onMouseRelease(int button, int mods);
    virtual void onMouseMove(double xpos, double ypos);
    virtual void onScroll(double xoffset, double yoffset);
    double time();
    glm::dvec2 getMousePosition() const;
    void shouldClose(bool flag);

  protected:
    friend class InputState;

    virtual void initialize();
    virtual void draw();
    virtual void beforeTerminate();

    GLFWwindow *window_;
    bool fullscreen_ = false;
    glm::ivec2 wndpos_ = glm::ivec2(0, 0);
    glm::ivec2 wndsz_ = glm::ivec2(1280, 720);

    double time_ = 0.0;
};

} // namespace rcube
