#pragma once
#include "stdafx.h"
#define FBXSDK_SHARED
#include <fbxsdk.h>
#include "../Gameplay/RenderComponent.h"

namespace Utility
{
	class FbxLoader
	{
	public:
		FbxLoader();

		~FbxLoader();

		void LoadSceneFromFile(const std::string p_fileName, Gameplay::RenderComponent*);


	private:
		Gameplay::RenderComponent* m_renderComponent;

		FbxManager* m_sdkManager;

		FbxScene* m_scene;

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
		void DisplayControlsPoints(FbxMesh* pMesh,std::vector<Gameplay::Vertex>& p_vertices,FbxAMatrix* l_transformMatrix);
		void DisplayPolygons(FbxMesh* pMesh, std::vector<uint32_t>& p_indices, std::vector<Gameplay::Vertex>& p_vertices, FbxAMatrix* l_transformMatrix);
		void DisplayTextureNames(FbxProperty& pProperty, FbxString& pConnectionString);
		void DisplayMaterialTextureConnections(FbxSurfaceMaterial* pMaterial, char* header, int pMatId, int l);
		void DisplayMaterialConnections(FbxMesh* pMesh);
		void DisplayMaterialMapping(FbxMesh* pMesh);
		void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
		void DisplayLink(FbxGeometry* pGeometry, std::vector<Gameplay::Vertex>& p_vertices);

	};

	
}