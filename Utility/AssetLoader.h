#pragma once
#include "stdafx.h"
#include "../Framework/IScene.h"
#include "FbxLoader.h"

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

		__forceinline void LoadFbx(const std::string p_fileName, Renderer::Scene* p_scene)
		{
			m_fbxLoader->LoadSceneFromFile(p_fileName, p_scene);
		};

		static bool LoadTextureFromFile(const std::string& p_fileName, Renderer::Texture& p_texture);

		FbxLoader* m_fbxLoader;

		~AssetLoader();

	private:
		AssetLoader();


	};
}