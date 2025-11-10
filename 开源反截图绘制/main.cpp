#include "Overlay.h"
#include "Cheat.h"
#include "driver.h"

int main() {
	if (!Overlay::InitOverlay())
	{
		return 1;
	}

	if (!CheatInit())
	{
		return 1;
	}

	g_driver.hide_window(Overlay::window_handle, WDA_EXCLUDEFROMCAPTURE);

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

		CheatDraw(Overlay::window_width, Overlay::window_height);
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