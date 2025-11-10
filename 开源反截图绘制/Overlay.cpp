#include "Overlay.h"
#include <dcomp.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dcomp.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Overlay {
    HWND window_handle;
    int window_width;
    int window_height;
    ID3D11Device* d3d_device;
    ID3D11DeviceContext* device_context;
    IDXGISwapChain1* swap_chain;
    ID3D11RenderTargetView* render_target_view;
    IDXGIFactory2* dxgi_factory;
    IDCompositionDevice* dcomp_device;
    IDCompositionTarget* dcomp_traget;
    IDCompositionVisual* dcomp_visual;
    VOID init_render_target() {
        ID3D11Texture2D* back_buffer = nullptr;
        swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
        if (back_buffer)
            d3d_device->CreateRenderTargetView(back_buffer, NULL, &render_target_view);
        // todo: error here if it can't get the backbuffer of the render target
        back_buffer->Release();
    }

    bool init_device() {
        UINT createDeviceFlags = 0;// D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

        HRESULT hr = D3D11CreateDevice(
            nullptr,                    // 默认适配器
            D3D_DRIVER_TYPE_HARDWARE,  // 硬件驱动
            nullptr,                   // 无软件渲染
            createDeviceFlags,         // 调试标志
            featureLevels,             // 支持的 Feature Level
            ARRAYSIZE(featureLevels),  // 数量
            D3D11_SDK_VERSION,         // SDK 版本
            &d3d_device,               // 返回的设备
            &featureLevel,             // 返回的 Feature Level
            &device_context            // 返回的设备上下文
        );

        // 3. 如果硬件设备创建失败，尝试 WARP 驱动（软件回退）
        if (FAILED(hr)) {
            hr = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_WARP,
                nullptr,
                createDeviceFlags,
                featureLevels,
                ARRAYSIZE(featureLevels),
                D3D11_SDK_VERSION,
                &d3d_device,
                &featureLevel,
                &device_context
            );
            if (FAILED(hr)) {
                return false;
            }
        }

        // 5. 配置交换链描述
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
        swapChainDesc.Width = window_width;
        swapChainDesc.Height = window_height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;  // 双缓冲
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;  // Flip 模型
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_DISPLAY_ONLY | DXGI_SWAP_CHAIN_FLAG_HW_PROTECTED;  // 关键：DISPLAY_ONLY

        hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgi_factory));
        if (FAILED(hr)) {
            return false;
        }

        hr = dxgi_factory->CreateSwapChainForComposition(
            d3d_device,
            &swapChainDesc,
            nullptr, // 无限制输出
            &swap_chain
        );

        IDXGIDevice* dxgiDevice = nullptr;
        hr = d3d_device->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
        if (FAILED(hr)) {
            return false;
        }
		hr = DCompositionCreateDevice(
			dxgiDevice, // 从 IDXGIDevice 获取
			IID_PPV_ARGS(&dcomp_device)
		);
        dxgiDevice->Release();
        if (FAILED(hr)) {
            return false;
        }

        hr = dcomp_device->CreateTargetForHwnd(
            window_handle,
            TRUE, // Top-most
            &dcomp_traget
        );
        if (FAILED(hr)) {
            return false;
        }

        hr = dcomp_device->CreateVisual(&dcomp_visual);
        if (FAILED(hr)) {
            return false;
        }

        dcomp_visual->SetContent(swap_chain);
        dcomp_traget->SetRoot(dcomp_visual);
        dcomp_device->Commit();

        // 7. 初始化渲染目标
        init_render_target();

        ShowWindow(window_handle, SW_SHOWDEFAULT);
        UpdateWindow(window_handle);
        return true;
    }

    VOID init_imgui() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();
        io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 14.0f, 0, io.Fonts->GetGlyphRangesChineseFull());

        ImGui_ImplWin32_Init(window_handle);
        ImGui_ImplDX11_Init(d3d_device, device_context);
    }

    void input_handler() {
        // added a sleep per user @colbyfortnite's suggestion
        for (; ; Sleep(1)) {
            // for our imgui menu interaction
            ImGuiIO& io = ImGui::GetIO();

            POINT p{};
            GetCursorPos(&p);
            io.MousePos = ImVec2((float)p.x, (float)p.y);

            io.MouseDown[0] = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
            io.MouseDown[1] = GetAsyncKeyState(VK_RBUTTON) & 0x8000;
        }
    }

    LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
            return true;

        switch (message)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_ERASEBKGND:
            return 1; // Prevent background erasure
        }

        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    bool CreateOverlayWindow()
    {
        // Register window class
        WNDCLASSEXA wc = { sizeof(WNDCLASSEXA) };
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = "zzzOverlayClass";

        if (!RegisterClassExA(&wc))
            return false;

        // Create window
        window_handle = CreateWindowExA(
            WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_NOREDIRECTIONBITMAP,
            wc.lpszClassName,
            "zzzOverlay",
            WS_POPUP,
            0, 0,
            window_width,
            window_height,
            nullptr,
            nullptr,
            wc.hInstance,
            nullptr);

        if (!window_handle)
            return false;

        return true;
    }

    std::string WStringToUTF8(const std::wstring& wstr) {
        if (wstr.empty()) return std::string();

        // 获取转换后所需的缓冲区大小
        int size_needed = WideCharToMultiByte(
            CP_UTF8,            // 目标编码：UTF-8
            0,                  // 标志（通常为0）
            wstr.c_str(),       // 输入宽字符串
            (int)wstr.size(),   // 输入字符串长度
            NULL,               // 输出缓冲区（NULL表示计算大小）
            0,                  // 输出缓冲区大小
            NULL, NULL          // 默认字符和是否使用默认字符（通常NULL）
        );

        if (size_needed == 0) {
            return ""; // 转换失败
        }

        // 分配缓冲区并执行转换
        std::string utf8_str(size_needed, 0);
        WideCharToMultiByte(
            CP_UTF8, 0,
            wstr.c_str(), (int)wstr.size(),
            &utf8_str[0], size_needed,
            NULL, NULL
        );

        return utf8_str;
    }

    void DrawTxt(const std::wstring& text, int x, int y, ImU32 color)
    {
        auto utf8Text = WStringToUTF8(text);
        ImGui::GetBackgroundDrawList()->AddText(ImVec2((float)x, (float)y),
            color,
            utf8Text.c_str());
    }

    void DrawLine(int x1, int y1, int x2, int y2, ImU32 color, float thickness)
    {
        ImGui::GetBackgroundDrawList()->AddLine(ImVec2((float)x1, (float)y1),
            ImVec2((float)x2, (float)y2),
            color,
            thickness);
    }

    void DrawCircle(int x, int y, float radius, ImU32 color, float thickness)
    {
        ImGui::GetBackgroundDrawList()->AddCircle(ImVec2((float)x, (float)y),
            radius,
            color,
            0,  // No segments specified (auto)
            thickness);
    }

    bool InitOverlay()
    {
        // 获取桌面分辨率
        window_width = GetSystemMetrics(SM_CXSCREEN);
        window_height = GetSystemMetrics(SM_CYSCREEN);

        if (!CreateOverlayWindow())
            return false;

        if (!init_device())
            return false; // error creating device

        init_imgui();

        std::thread(input_handler).detach();
        return true;
    }

    void Cleanup()
    {
        dxgi_factory->Release();
        d3d_device->Release();
        device_context->Release();
        swap_chain->Release();
        render_target_view->Release();
        dcomp_device->Release();
        dcomp_traget->Release();
        dcomp_visual->Release();
    }
}