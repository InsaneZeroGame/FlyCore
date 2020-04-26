#include "RenderComponent.h"
#include "../Utility/AssetLoader.h"

Gameplay::RenderComponent::RenderComponent()
{
}

Gameplay::RenderComponent::RenderComponent(const std::string& p_fbxFileName)
{
    auto& loader = Utility::AssetLoader::GetLoader();
    m_scene = new Scene;
    loader.LoadFbx("C:\\Dev\\FlyCore\\Assets\\scene1.fbx", m_scene);

}

Gameplay::RenderComponent::~RenderComponent()
{
}
