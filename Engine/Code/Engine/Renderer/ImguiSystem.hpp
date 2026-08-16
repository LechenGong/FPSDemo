#pragma once

#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_impl_dx11.h"
#include "ThirdParty/imgui/imgui_impl_win32.h"

class Renderer;
class Window;

class ImGuiSystem
{
public:
	ImGuiSystem( Renderer* renderer, Window* window );
	~ImGuiSystem();

	virtual void Startup();
	virtual void BeginFrame();
	virtual void Render();
	virtual void EndFrame();

private:
	Window* m_windowRef;
	Renderer* m_rendererRef;
};