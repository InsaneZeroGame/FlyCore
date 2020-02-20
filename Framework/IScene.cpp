#include "stdafx.h"
#include "IScene.h"

Renderer::Scene::Scene()
{

}

Renderer::Scene::~Scene()
{

}

Renderer::Actor::Actor():
	m_currentVertexOffset(0),
	m_currentIndexOffset(0)
{
}

Renderer::Actor::~Actor()
{
}

Renderer::Mesh::Mesh()
{
}

Renderer::Mesh::Mesh(std::vector<Vertex>&& p_vertices, std::vector<uint32_t>&& p_indices):
	m_vertices(p_vertices),
	m_indices(p_indices),
	m_vertexOffset(0),
	m_indexOffset(0)
{
}

Renderer::Mesh::~Mesh()
{
}
