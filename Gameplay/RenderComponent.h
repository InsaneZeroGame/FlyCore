#pragma once
#include "stdafx.h"
#include "Component.h"
#include "Scene.h"


namespace Gameplay
{
	class RenderComponent 
	{
	public:
		RenderComponent();
		RenderComponent(const std::string& p_fbxFileName);
		~RenderComponent();
		Scene* m_scene;

	private:

	};
}