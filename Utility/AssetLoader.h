#pragma once
#include "stdafx.h"
#include "FbxLoader.h"
#include "../Gameplay/RenderComponent.h"
#include <future>

namespace Utility
{
	class AssetLoader
	{
	public:
		static AssetLoader& GetLoader()
		{
			static AssetLoader l_loader;
			return l_loader;
		}

		__forceinline void LoadFbx(const std::string p_fileName, Gameplay::RenderComponent* p_component)
		{
			m_fbxLoader->LoadSceneFromFile(p_fileName, p_component);
		};

		static bool LoadTextureFromFile(const std::string& p_fileName, Gameplay::Texture& p_texture);

		FbxLoader* m_fbxLoader;

		~AssetLoader();

	private:
		AssetLoader();


	};
}