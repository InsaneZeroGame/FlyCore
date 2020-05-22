#include <iostream>
#include "../Framework/Window.h"
#include "../Renderer/D3D12Renderer.h"
#include "../Gameplay/BaseCamera.h"
#include "../Gameplay/RenderComponent.h"
#include "../Gameplay/Director.h"
#include <functional>


int main() {

	auto window = new Framework::Window({ 1920,1080,"The Borning Game" });
	window->OnInit();

	auto ui = new UI::D3D12UISystem(window);

	auto mainCamera = new Gameplay::BaseCamera(glm::vec3(10.01, 10.0, 12.0), glm::vec3(0.0f,0.0f,0.0F));

	auto renderer = new Renderer::D3D12Renderer();
	renderer->SetTargetWindow(window);
	renderer->LoadUI(ui);

	auto& director = Gameplay::Director::GetDirector();

	auto& entity_manager = Gameplay::EntityManager::GetManager();
	auto entity0 = entity_manager.SpwanEntity();
	renderer->AddComponent(entity0, "C:\\Dev\\FlyCore\\Assets\\humanoid.fbx");
	director.AddComponent(entity0, glm::vec3(0.0f), glm::vec3(0.0));
	director.Scale(entity0, glm::vec3(0.1f));

	auto entity1 = entity_manager.SpwanEntity();
	renderer->AddComponent(entity1, "C:\\Dev\\FlyCore\\Assets\\scene1.fbx");
	director.AddComponent(entity1, glm::vec3(0.0f), glm::vec3(0.0));
	director.Scale(entity1, glm::vec3(50.0f,1.0f,50.0f));
	director.Translate(entity1, glm::vec3(0.0f, -1.0f, 0.0f));

	auto entity2 = entity_manager.SpwanEntity();
	renderer->AddComponent(entity2, "C:\\Dev\\FlyCore\\Assets\\cube.fbx");
	director.AddComponent(entity2, glm::vec3(0.0f), glm::vec3(0.0));
	director.Scale(entity2, glm::vec3(1.5f, 1.5f, 1.5f));
	director.Translate(entity2, glm::vec3(0.0f, 2.0f, 0.0f));

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

