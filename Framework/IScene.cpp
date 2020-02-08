#include "IScene.h"

Renderer::Scene::Scene()
{

}

Renderer::Scene::~Scene()
{

}

Renderer::Actor::Actor()
{
}

Renderer::Actor::~Actor()
{
}

Renderer::Mesh::Mesh()
{
}

Renderer::Mesh::Mesh(std::vector<float>&& p_vertices, std::vector<uint32_t>&& p_indices):
	m_vertices(p_vertices),
	m_indices(p_indices)
{
}

Renderer::Mesh::~Mesh()
{
}
