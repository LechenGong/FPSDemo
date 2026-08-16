#include <d3d11.h>

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/ImGuiSystem.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"

ImGuiSystem::ImGuiSystem( Renderer* renderer, Window* window )
	: m_rendererRef( renderer )
	, m_windowRef( window )
{
}

ImGuiSystem::~ImGuiSystem()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiSystem::Startup()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init( m_windowRef->GetHwnd() );
	ImGui_ImplDX11_Init( m_rendererRef->GetD3D11Device(), m_rendererRef->GetD3D11DeviceContext() );
	ImGui::StyleColorsDark();
	UNUSED( io );
}

void ImGuiSystem::BeginFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
}

void ImGuiSystem::Render()
{
	ImGui::Render();
	ImDrawData* data = ImGui::GetDrawData();
	ID3D11RenderTargetView* dx11_rtv = m_rendererRef->GetD3D11RenderTargetView();
	ID3D11DeviceContext* context = m_rendererRef->GetD3D11DeviceContext();
	context->OMSetRenderTargets( 1, &dx11_rtv, NULL );

	ImGui_ImplDX11_RenderDrawData( data );
}

void ImGuiSystem::EndFrame()
{
}
