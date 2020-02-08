#pragma once
#include "INoCopy.h"

namespace Renderer
{

	class Mesh
	{
	public:
		Mesh();
		Mesh(std::vector<float>&& m_vertices,std::vector<uint32_t>&& m_indices);
		~Mesh();
		std::vector<float> m_vertices;
		std::vector<uint32_t> m_indices;

	private:
		

	};


	class Actor
	{
	public:
		Actor();
		~Actor();

		__forceinline void AddMesh(Mesh&& p_mesh)
		{
			m_meshes.push_back(p_mesh);
		}
		std::vector<Mesh> m_meshes;


	private:

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

		std::vector<Actor> m_actors;

	private:


	};
}