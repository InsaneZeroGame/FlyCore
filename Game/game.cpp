#include <iostream>
#include "../Framework/Window.h"
#include "../Renderer/D3D12Renderer.h"
#include <functional>


int main() {

	auto window = new Framework::Window({ 1920,1080,"Ang's Game" });
	window->OnInit();

	auto renderer = new Renderer::D3D12Renderer();
	renderer->SetTargetWindow(window);
	renderer->OnInit();
	window->SetRendererCallback(std::bind(&Renderer::D3D12Renderer::OnUpdate,renderer));
	window->SetScrollCallback(std::bind(&Renderer::D3D12Renderer::OnMouseWheelScroll, renderer, std::placeholders::_1, std::placeholders::_2));
	while (1)
	{
		window->OnUpdate();
	}

	renderer->OnDestory();
	window->OnDestory();

	return 0;
}

