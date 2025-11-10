#pragma once

#include <windows.h>
#include <dwmapi.h>
#include <string>
#include <sstream>
#include <chrono>
#include <d3d11.h>
#include <dxgi1_2.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include <thread>

namespace Overlay
{
	extern HWND window_handle;
	extern int window_width;
	extern int window_height;
	extern ID3D11Device* d3d_device;
	extern ID3D11DeviceContext* device_context;
	extern IDXGISwapChain1* swap_chain;
	extern ID3D11RenderTargetView* render_target_view;

	void DrawTxt(const std::wstring& text, int x, int y, ImU32 color = IM_COL32_WHITE);
	void DrawLine(int x1, int y1, int x2, int y2, ImU32 color = IM_COL32_WHITE, float thickness = 1.0f);
	void DrawCircle(int x, int y, float radius, ImU32 color = IM_COL32_WHITE, float thickness = 1.0f);
	bool InitOverlay();
	void Cleanup();
}