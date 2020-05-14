#pragma once
#include "stdafx.h"
#include "Component.h"


namespace Gameplay
{
	struct Vertex
	{
		std::array<float, 4> position;
		std::array<float, 3> normal;
		std::array<float, 2> uv;
		std::array<int, 4> boneIndex = {-1,-1,-1,-1};
		std::array<float, 4> boneWeight;
	};


	struct Texture
	{
		int width;//Max to 4096
		int height;//Max to 4096
		int componentSize;
		unsigned char* data;
		int size;

		~Texture() {
			if (data)
			{
				delete[] data;
				data = nullptr;
			}
		}
	};

	struct SkeletonAnim
	{
		enum {MAX_BONE = 60};
		std::array<glm::mat4x4, MAX_BONE> bones;
	};

	class Mesh
	{
	public:
		Mesh();

		Mesh(
			std::vector<Vertex>&& m_vertices,
			std::vector<uint32_t>&& m_indices,
			SkeletonAnim* p_anim);

		Mesh(
			std::vector<Vertex>&& m_vertices,
			std::vector<uint32_t>&& m_indices);

		~Mesh();

		std::vector<Vertex> m_vertices;

		std::vector<uint32_t> m_indices;

		SkeletonAnim* m_anim;

		uint64_t m_vertexOffset;

		uint64_t m_indexOffset;

	private:

	};

	
	class RenderComponent 
	{
	public:
		RenderComponent();
		RenderComponent(const std::string& p_fbxFileName);
		RenderComponent(Mesh&& p_mesh);
		~RenderComponent();
	 	__forceinline void AddMesh(Mesh&& p_mesh)
	 	{
	 		p_mesh.m_vertexOffset = m_meshVertexOffset;
	 		p_mesh.m_indexOffset = m_meshIndexOffset;
	 		m_meshes.push_back(p_mesh);
			m_meshVertexOffset += static_cast<uint64_t>(p_mesh.m_vertices.size());
			m_meshIndexOffset += static_cast<uint64_t>(p_mesh.m_indices.size());
	 	}

		__forceinline const std::vector<Mesh>& GetMeshes()
		{
			return  m_meshes;
		}
		uint64_t m_componentVertexOffset;

		uint64_t m_componentIndexOffset;
	private:

		std::vector<Mesh> m_meshes;

		uint64_t m_meshVertexOffset;

		uint64_t m_meshIndexOffset;
	};
}