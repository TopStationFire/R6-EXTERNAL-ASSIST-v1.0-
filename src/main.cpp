#include <Windows.h>
#include <TlHelp32.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include "core/memory.hpp"
#include "core/render.hpp"
#include "features/visuals.hpp"
#include "features/aim.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

const char* window_class = "OverlayClass";
const char* window_title = "External Overlay";

HWND hwnd_target = nullptr;
HWND hwnd_overlay = nullptr;
ID3D11Device* d3d_device = nullptr;
ID3D11DeviceContext* d3d_context = nullptr;
ID3D11RenderTargetView* main_target_view = nullptr;

bool running = true;
bool show_menu = true;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    if (msg == WM_KEYDOWN && wParam == VK_INSERT)
        show_menu = !show_menu;

    if (msg == WM_DESTROY) {
        running = false;
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT create_flags = 0;
    
    D3D_FEATURE_LEVEL feature_level;
    const D3D_FEATURE_LEVEL feature_level_array[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        create_flags, feature_level_array, 2, D3D11_SDK_VERSION, &sd,
        &g_swapchain, &d3d_device, &feature_level, &d3d_context) != S_OK) {
        return false;
    }

    CreateRenderTarget();
    return true;
}

void CreateRenderTarget() {
    ID3D11Texture2D* back_buffer;
    g_swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    d3d_device->CreateRenderTargetView(back_buffer, nullptr, &main_target_view);
    back_buffer->Release();
}

void CleanupDeviceD3D() {
    if (main_target_view) { main_target_view->Release(); main_target_view = nullptr; }
    if (d3d_context) { d3d_context->Release(); d3d_context = nullptr; }
    if (d3d_device) { d3d_device->Release(); d3d_device = nullptr; }
}

void RenderLoop() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (show_menu) {
        ImGui::Begin("Control Panel", &show_menu);
        
        ImGui::Checkbox("Box Overlay", &visuals::enable_boxes);
        ImGui::Checkbox("Skeleton", &visuals::enable_skeleton);
        ImGui::Checkbox("Health Bar", &visuals::enable_health);
        ImGui::Checkbox("Name Tag", &visuals::enable_names);
        ImGui::Separator();
        ImGui::Checkbox("Aim Correction", &aim::enable_correction);
        ImGui::SliderFloat("Smoothness", &aim::smoothness, 1.0f, 10.0f);
        ImGui::SliderFloat("FOV", &aim::field_of_view, 1.0f, 180.0f);
        
        ImGui::End();
    }

    if (memory::attached) {
        visuals::RenderESP(d3d_context, main_target_view);
    }

    ImGui::Render();
    const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    d3d_context->OMSetRenderTargets(1, &main_target_view, nullptr);
    d3d_context->ClearRenderTargetView(main_target_view, clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_swapchain->Present(1, 0);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), 0, WndProc, 0, 0, hInstance, nullptr, nullptr, nullptr, nullptr, window_class, nullptr };
    RegisterClassEx(&wc);

    hwnd_target = FindWindowA("RainbowSix", nullptr);
    if (!hwnd_target) {
        MessageBox(nullptr, "Game not found", "Error", MB_ICONERROR);
        return 1;
    }

    RECT target_rect;
    GetWindowRect(hwnd_target, &target_rect);

    hwnd_overlay = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        window_class, window_title,
        WS_POPUP,
        target_rect.left, target_rect.top,
        target_rect.right - target_rect.left,
        target_rect.bottom - target_rect.top,
        nullptr, nullptr, wc.hInstance, nullptr
    );

    SetLayeredWindowAttributes(hwnd_overlay, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd_overlay, &margins);

    if (!CreateDeviceD3D(hwnd_overlay)) {
        CleanupDeviceD3D();
        UnregisterClass(window_class, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd_overlay, SW_SHOWDEFAULT);
    UpdateWindow(hwnd_overlay);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd_overlay);
    ImGui_ImplDX11_Init(d3d_device, d3d_context);

    memory::Initialize(hwnd_target);

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (running && msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        RECT rect;
        GetWindowRect(hwnd_target, &rect);
        SetWindowPos(hwnd_overlay, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);

        RenderLoop();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(hwnd_overlay);
    UnregisterClass(window_class, wc.hInstance);

    return 0;
}
