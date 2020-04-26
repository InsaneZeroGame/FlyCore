#include "stdafx.h"
#include "Scene.h"

Gameplay::Scene::Scene()
{

}

Gameplay::Scene::~Scene()
{

}

Gameplay::Actor::Actor():
	m_currentVertexOffset(0),
	m_currentIndexOffset(0)
{
}

Gameplay::Actor::~Actor()
{
}

Gameplay::Mesh::Mesh()
{
}

Gameplay::Mesh::Mesh(std::vector<Vertex>&& p_vertices, std::vector<uint32_t>&& p_indices):
	m_vertices(p_vertices),
	m_indices(p_indices),
	m_vertexOffset(0),
	m_indexOffset(0)
{
}

Gameplay::Mesh::~Mesh()
{
}
