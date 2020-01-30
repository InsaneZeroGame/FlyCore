#include <iostream>
#include "../Framework/Window.h"
#include "../Renderer/D3D12Renderer.h"


int main() {
	auto window = new fcf::Window({ 1920,1080,"Ang's Game" });
	window->OnInit();

	auto renderer = new Renderer::D3D12Renderer();
	renderer->SetTargetWindow(window);
	renderer->OnInit();
	while (1)
	{
		renderer->OnUpdate();
		window->OnUpdate();

	}
	

	renderer->OnDestory();
	window->OnDestory();

	return 0;
}

