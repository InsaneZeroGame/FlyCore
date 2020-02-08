#pragma once
#include "stdafx.h"
#define FBXSDK_SHARED
#include <fbxsdk.h>
#include "../Framework/IScene.h"

namespace Utility
{
	class FbxLoader
	{
	public:
		FbxLoader();

		~FbxLoader();

		void LoadFile(const std::string p_fileName, Renderer::Scene* p_scene);


	private:
		FbxManager* m_sdkManager;

		FbxScene* m_scene;

		Renderer::Scene* m_currentGameScene;

		FbxImporter* m_importer;

		FbxAnimLayer* m_currentAnimLayer;

		FbxArray<FbxString*> m_animStackNameArray;

		FbxArray<FbxNode*> m_cameraArray;

		FbxArray<FbxPose*> m_poseArray;

		FbxTime mFrameTime, mStart, mStop, mCurrentTime;
		FbxTime mCache_Start, mCache_Stop;

		bool m_supportVBO;

		bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);

		void DisplayContent(FbxScene* pScene);
		void DisplayContent(FbxNode* pNode);
		void DisplayMesh(FbxNode* pNode);
		void DisplayControlsPoints(FbxMesh* pMesh,std::vector<float>& p_vertices);
		void DisplayPolygons(FbxMesh* pMesh, std::vector<uint32_t>& p_indices);
		void DisplayTextureNames(FbxProperty& pProperty, FbxString& pConnectionString);
		void DisplayMaterialTextureConnections(FbxSurfaceMaterial* pMaterial, char* header, int pMatId, int l);
		void DisplayMaterialConnections(FbxMesh* pMesh);
		void DisplayMaterialMapping(FbxMesh* pMesh);
		void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);

	};

	
}