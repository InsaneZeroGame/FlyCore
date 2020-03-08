#pragma once
#include "INoCopy.h"
#include <vector>
#include <array>
namespace Renderer
{

	struct Vertex
	{
		std::array<float,4> position;
		std::array<float,3> normal;
		std::array<float,2> uv;
	};

	class Mesh
	{
	public:
		Mesh();

		Mesh(std::vector<Vertex>&& m_vertices,std::vector<uint32_t>&& m_indices);
		
		~Mesh();

		std::vector<Vertex> m_vertices;

		std::vector<uint32_t> m_indices;

		uint32_t m_vertexOffset;

		uint32_t m_indexOffset;

	private:
		
	};


	class Actor
	{
	public:
		Actor();
		
		~Actor();

		__forceinline void AddMesh(Mesh& p_mesh)
		{
			p_mesh.m_vertexOffset = m_currentVertexOffset;
			p_mesh.m_indexOffset = m_currentIndexOffset;
			m_meshes.push_back(p_mesh);
			m_currentVertexOffset += static_cast<uint32_t>(p_mesh.m_vertices.size());
			m_currentIndexOffset += static_cast<uint32_t>(p_mesh.m_indices.size());
		}

		std::vector<Mesh> m_meshes;


	private:
		//Vertex offset of meshes when loading the assets.
		uint32_t m_currentVertexOffset;
		uint32_t m_currentIndexOffset;

	};


	class Scene : public Interface::INoCopy
	{
	public:
		Scene();

		virtual ~Scene();

		__forceinline void AddActor(Actor&& p_mesh)
		{
			m_actors.push_back(p_mesh);
		}

		virtual void UploadToGPU() {};

		std::vector<Actor> m_actors;


	private:
		
	};
}