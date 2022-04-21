// Based on the GLFWDemo sample from DiligentEngine


#ifndef ENGINE_DLL
#define ENGINE_DLL 1
#endif

#ifndef D3D11_SUPPORTED
#define D3D11_SUPPORTED 0
#endif

#ifndef D3D12_SUPPORTED
#define D3D12_SUPPORTED 0
#endif

#ifndef GL_SUPPORTED
#define GL_SUPPORTED 0
#endif

#ifndef VULKAN_SUPPORTED
#define VULKAN_SUPPORTED 0
#endif

#ifndef METAL_SUPPORTED
#define METAL_SUPPORTED 0
#endif

#if PLATFORM_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32 1
#endif

#if PLATFORM_LINUX
#define GLFW_EXPOSE_NATIVE_X11 1
#endif

#if PLATFORM_MACOS
#define GLFW_EXPOSE_NATIVE_COCOA 1
#endif

#if D3D11_SUPPORTED
#include "EngineFactoryD3D11.h"
#endif
#if D3D12_SUPPORTED
#include "EngineFactoryD3D12.h"
#endif
#if GL_SUPPORTED
#include "EngineFactoryOpenGL.h"
#endif
#if VULKAN_SUPPORTED
#include "EngineFactoryVk.h"
#endif
#if METAL_SUPPORTED
#include "Graphics/GraphicsEngineMetal/interface/EngineFactoryMtl.h"
#endif

#if PLATFORM_WIN32
#undef GetObject
#undef CreateWindow
#endif

using namespace Diligent;

#include "RCube/Core/Graphics/Graphics.h"

namespace rcube
{

bool Graphics::initEngine(RENDER_DEVICE_TYPE devType)
{
#if PLATFORM_WIN32
    Win32NativeWindow Window{glfwGetWin32Window(m_window)};
#endif
#if PLATFORM_LINUX
    LinuxNativeWindow Window;
    Window.WindowId = glfwGetX11Window(m_window);
    Window.pDisplay = glfwGetX11Display();
    if (devType == RENDER_DEVICE_TYPE_GL)
        glfwMakeContextCurrent(m_window);
#endif
#if PLATFORM_MACOS
    MacOSNativeWindow Window;
    if (devType == RENDER_DEVICE_TYPE_GL)
        glfwMakeContextCurrent(m_window);
    else
        Window.pNSView = GetNSWindowView(m_window);
#endif

    SwapChainDesc SCDesc;
    switch (devType)
    {
#if D3D11_SUPPORTED
    case RENDER_DEVICE_TYPE_D3D11:
    {
#if ENGINE_DLL
        // Load the dll and import GetEngineFactoryD3D11() function
        auto *GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
#endif
        auto *pFactoryD3D11 = GetEngineFactoryD3D11();

        EngineD3D11CreateInfo EngineCI;
        pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &m_device, &m_immediateContext);
        pFactoryD3D11->CreateSwapChainD3D11(m_device, m_immediateContext, SCDesc,
                                            FullScreenModeDesc{}, Window, &m_swapChain);
    }
    break;
#endif // D3D11_SUPPORTED

#if D3D12_SUPPORTED
    case RENDER_DEVICE_TYPE_D3D12:
    {
#if ENGINE_DLL
        // Load the dll and import GetEngineFactoryD3D12() function
        auto *GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#endif
        auto *pFactoryD3D12 = GetEngineFactoryD3D12();

        EngineD3D12CreateInfo EngineCI;
        pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &m_device, &m_immediateContext);
        pFactoryD3D12->CreateSwapChainD3D12(m_device, m_immediateContext, SCDesc,
                                            FullScreenModeDesc{}, Window, &m_swapChain);
    }
    break;
#endif // D3D12_SUPPORTED

#if GL_SUPPORTED
    case RENDER_DEVICE_TYPE_GL:
    {
#if EXPLICITLY_LOAD_ENGINE_GL_DLL
        // Load the dll and import GetEngineFactoryOpenGL() function
        auto GetEngineFactoryOpenGL = LoadGraphicsEngineOpenGL();
#endif
        auto *pFactoryOpenGL = GetEngineFactoryOpenGL();

        EngineGLCreateInfo EngineCI;
        EngineCI.Window = Window;
        pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &m_device, &m_immediateContext, SCDesc,
                                                   &m_swapChain);
    }
    break;
#endif // GL_SUPPORTED

#if VULKAN_SUPPORTED
    case RENDER_DEVICE_TYPE_VULKAN:
    {
#if EXPLICITLY_LOAD_ENGINE_VK_DLL
        // Load the dll and import GetEngineFactoryVk() function
        auto *GetEngineFactoryVk = LoadGraphicsEngineVk();
#endif
        auto *pFactoryVk = GetEngineFactoryVk();

        EngineVkCreateInfo EngineCI;
        pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &m_device, &m_immediateContext);
        pFactoryVk->CreateSwapChainVk(m_device, m_immediateContext, SCDesc, Window, &m_swapChain);
    }
    break;
#endif // VULKAN_SUPPORTED

#if METAL_SUPPORTED
    case RENDER_DEVICE_TYPE_METAL:
    {
        auto *pFactoryMtl = GetEngineFactoryMtl();

        EngineMtlCreateInfo EngineCI;
        pFactoryMtl->CreateDeviceAndContextsMtl(EngineCI, &m_device, &m_immediateContext);
        pFactoryMtl->CreateSwapChainMtl(m_device, m_immediateContext, SCDesc, Window, &m_swapChain);
    }
    break;
#endif // METAL_SUPPORTED

    default:
        std::cerr << "Unknown/unsupported device type";
        return false;
        break;
    }

    if (m_device == nullptr || m_immediateContext == nullptr || m_swapChain == nullptr)
    {
        return false;
    }

    return true;
}

Diligent::IRenderDevice *Graphics::renderDevice()
{
    return m_device;
}

Diligent::IEngineFactory *Graphics::engineFactory()
{
    return m_engineFactory;
}

Diligent::IDeviceContext *Graphics::context()
{
    return m_immediateContext;
}

Diligent::ISwapChain *Graphics::swapChain()
{
    return m_swapChain;
}

GLFWwindow *Graphics::window()
{
    return m_window;
}

} // namespace rcube