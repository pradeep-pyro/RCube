#pragma once

#include "RefCntAutoPtr.hpp"
#include "DeviceContext.h"
#include "EngineFactory.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "RenderDevice.h"
#include "SwapChain.h"
#include <vector>

namespace rcube
{

class Graphics
{
    Diligent::RefCntAutoPtr<Diligent::IEngineFactory> m_engineFactory;
    Diligent::RefCntAutoPtr<Diligent::IRenderDevice> m_device;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_immediateContext;
    std::vector<Diligent::RefCntAutoPtr<Diligent::IDeviceContext>> m_deferredContexts;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain> m_swapChain;
    GLFWwindow *m_window = nullptr;

    Graphics()
    {
        initEngine(Diligent::RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_D3D12);
    }
    ~Graphics()
    {
    }

    bool initEngine(Diligent::RENDER_DEVICE_TYPE devType);

  public:
    Graphics(const Graphics &) = delete;
    Graphics(Graphics &&) = delete;
    Graphics &operator=(const Graphics &) = delete;
    Graphics &operator=(Graphics &&) = delete;
    static Graphics &instance()
    {
        static Graphics instance;
        return instance;
    }
    Diligent::IRenderDevice *renderDevice();
    Diligent::IEngineFactory *engineFactory();
    Diligent::IDeviceContext* context();
    Diligent::ISwapChain *swapChain();
    GLFWwindow *window();
};

} // namespace rcube