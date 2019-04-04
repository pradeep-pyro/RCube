#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include <string>

namespace rcube {

class Window {
public:
    Window(const std::string &title, glm::ivec2 size=glm::ivec2(1280, 720));
    virtual ~Window() = default;
    void execute();
    bool fullscreen() const;
    void setFullscreen(bool flag);
    glm::ivec2 size() const;
    virtual void onResize(int width, int height);
    virtual void onKeyPress(int key, int mods);
    virtual void onKeyRelease(int key, int mods);
    virtual void onKeyRepeat(int key, int mods);
    virtual void onMousePress(int key, int mods);
    virtual void onMouseRelease(int key, int mods);
    virtual void onMouseMove(double xpos, double ypos);
    glm::dvec2 getMousePosition() const;
    void shouldClose(bool flag);
protected:
    virtual void initialize();
    virtual void draw();
    virtual void beforeTerminate();

    GLFWwindow* window_;
    bool fullscreen_ = false;
    glm::ivec2 wndpos_ = glm::ivec2(0, 0);
    glm::ivec2 wndsz_ = glm::ivec2(1280, 720);
};

} // namespace rcube

#endif // APPLICATION_H
