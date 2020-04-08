#include <iostream>
#include "../Framework/Window.h"
#include "../Renderer/D3D12Renderer.h"
#include "../Gameplay/BaseCamera.h"
#include <functional>


int main() {

	auto window = new Framework::Window({ 1920,1080,"The Borning Game" });
	window->OnInit();

	auto mainCamera = new Gameplay::BaseCamera(glm::vec3(-15.1f, 25.1f, -15.1f), glm::vec3(0.0f));

	auto renderer = new Renderer::D3D12Renderer();
	renderer->SetTargetWindow(window); 
	renderer->OnInit();
	renderer->SetCamera(mainCamera);
	window->SetRendererCallback(std::bind(&Renderer::D3D12Renderer::OnUpdate,renderer));
	window->SetScrollCallback(std::bind(&Gameplay::BaseCamera::OnMouseWheelScroll, mainCamera, std::placeholders::_1, std::placeholders::_2));
	window->SetMouseMoveCallback(std::bind(&Gameplay::BaseCamera::OnMouseMove, mainCamera, std::placeholders::_1, std::placeholders::_2));
	window->SetKeyCallback(std::bind(&Gameplay::BaseCamera::OnKeyPress, mainCamera, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	
	window->OnUpdate();
	window->OnDestory();
	renderer->OnDestory();

	return 0;
}

