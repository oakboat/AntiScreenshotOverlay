// 开源反截图绘制.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "Overlay.h"


int main()
{
	if(!Overlay::InitOverlay())
	{
		return 1;
	}

	MSG msg{ 0 };
	ImVec4 clear_clr = { 0,0,0,0 };
	bool done = false;
	while (true)
	{
		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::GetForegroundDrawList()->AddText(
			ImVec2(10, 10),
			IM_COL32(255, 255, 0, 255),
			("FPS: " + std::to_string((int)ImGui::GetIO().Framerate)).c_str()
		);

		ImGui::Render();
		Overlay::device_context->OMSetRenderTargets(1, &Overlay::render_target_view, NULL);
		Overlay::device_context->ClearRenderTargetView(Overlay::render_target_view, (float*)&clear_clr); // might break, if so then put cheat color of type ImVec4 { 0,0,0,0 }
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		Overlay::swap_chain->Present(1, 0);
	}
	Overlay::Cleanup();
	return msg.message;

}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
