#include "RCube/Window.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/RCube.h"
#include <iostream>

namespace rcube
{

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                            const GLchar *message, const void *userParam)
{
    // Taken from: https://learnopengl.com/In-Practice/Debugging
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    std::cerr << "---------------" << std::endl;
    std::cerr << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        std::cerr << "Source: API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cerr << "Source: Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cerr << "Source: Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cerr << "Source: Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cerr << "Source: Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cerr << "Source: Other";
        break;
    }
    std::cerr << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        std::cerr << "Type: Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cerr << "Type: Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cerr << "Type: Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cerr << "Type: Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cerr << "Type: Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cerr << "Type: Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cerr << "Type: Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cerr << "Type: Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cerr << "Type: Other";
        break;
    }
    std::cerr << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cerr << "Severity: high";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cerr << "Severity: medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cerr << "Severity: low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cerr << "Severity: notification";
        break;
    }
    std::cerr << std::endl;
    std::cerr << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
// Low-level GLFW callbacks

static void callbackOnError(int, const char *err_str)
{
    std::cerr << "GLFW Error: " << err_str << std::endl;
}

static void callbackOnResize(GLFWwindow *window, int w, int h)
{
    static_cast<Window *>(glfwGetWindowUserPointer(window))->onResize(w, h);
}

static void callbackKeyboard(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        static_cast<Window *>(glfwGetWindowUserPointer(window))->onKeyPress(key, mods);
    }
    else if (action == GLFW_RELEASE)
    {
        static_cast<Window *>(glfwGetWindowUserPointer(window))->onKeyRelease(key, mods);
    }
    else if (action == GLFW_REPEAT)
    {
        static_cast<Window *>(glfwGetWindowUserPointer(window))->onKeyRepeat(key, mods);
    }
}

static void callbackMouse(GLFWwindow *window, int button, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        static_cast<Window *>(glfwGetWindowUserPointer(window))->onMousePress(button, mods);
    }
    else if (action == GLFW_RELEASE)
    {
        static_cast<Window *>(glfwGetWindowUserPointer(window))->onMouseRelease(button, mods);
    }
}

static void callbackMouseMove(GLFWwindow *window, double xpos, double ypos)
{
    static_cast<Window *>(glfwGetWindowUserPointer(window))->onMouseMove(xpos, ypos);
}

static void callbackScroll(GLFWwindow *window, double xoffset, double yoffset)
{
    static_cast<Window *>(glfwGetWindowUserPointer(window))->onScroll(xoffset, yoffset);
}


///////////////////////////////////////////////////////////////////////////////

Window::Window(const std::string &title, glm::ivec2 size)
{
    glfwSetErrorCallback(callbackOnError);
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    window_ = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(window_);

    glfwSwapInterval(1); // Enable vsync
    rcube::initGL();
    rcube::checkGLError();

    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    glfwSetWindowUserPointer(window_, this);
    rcube::checkGLError();
    glfwSetFramebufferSizeCallback(window_, callbackOnResize);
    glfwSetKeyCallback(window_, callbackKeyboard);
    glfwSetMouseButtonCallback(window_, callbackMouse);
    glfwSetCursorPosCallback(window_, callbackMouseMove);
    glfwSetScrollCallback(window_, callbackScroll);
}

void Window::initialize()
{
}

void Window::draw()
{
}

void Window::onResize(int, int)
{
}

void Window::onKeyPress(int, int)
{
}

void Window::onKeyRelease(int, int)
{
}

void Window::onKeyRepeat(int, int)
{
}

void Window::onMousePress(int, int)
{
}

void Window::onMouseRelease(int, int)
{
}

void Window::onMouseMove(double, double)
{
}

void Window::onScroll(double xoffset, double yoffset)
{
}

glm::dvec2 Window::getMousePosition() const
{
    glm::dvec2 xy;
    glfwGetCursorPos(window_, &xy[0], &xy[1]);
    return xy;
}

void Window::execute()
{
    initialize(); // User should override this method
    glfwSwapInterval(0);
    while (!glfwWindowShouldClose(window_))
    {
        draw(); // User should override this method
        glfwPollEvents();
        glfwSwapBuffers(window_);
    }
    beforeTerminate(); // User should override this method
    glfwDestroyWindow(window_);
    glfwTerminate();
}

void Window::beforeTerminate()
{
}

bool Window::fullscreen() const
{
    return fullscreen_;
}

void Window::setFullscreen(bool flag)
{
    if (fullscreen_ == flag)
    {
        return;
    }

    if (flag)
    {
        glfwGetWindowPos(window_, &wndpos_.x, &wndpos_.y);
        glfwGetWindowSize(window_, &wndsz_.x, &wndsz_.y);
        GLFWmonitor *primary = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(primary);
        glfwSetWindowMonitor(window_, primary, 0, 0, mode->width, mode->height, 0);
    }
    else
    {
        // restore last window size and position
        glfwSetWindowMonitor(window_, nullptr, wndpos_.x, wndpos_.y, wndsz_.x, wndsz_.y, 0);
    }
    fullscreen_ = flag;
}

void Window::shouldClose(bool flag)
{
    glfwSetWindowShouldClose(window_, flag);
}

glm::ivec2 Window::size() const
{
    glm::ivec2 sz;
    glfwGetWindowSize(window_, &sz.x, &sz.y);
    return sz;
}

} // namespace rcube
