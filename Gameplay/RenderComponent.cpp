#include "RenderComponent.h"
#include "../Utility/AssetLoader.h"

Gameplay::RenderComponent::RenderComponent():
    m_componentVertexOffset(0),
    m_componentIndexOffset(0),
    m_meshVertexOffset(0),
    m_meshIndexOffset(0)
{
}

Gameplay::RenderComponent::RenderComponent(const std::string& p_fbxFileName):
    m_componentVertexOffset(0),
    m_componentIndexOffset(0),
    m_meshVertexOffset(0),
    m_meshIndexOffset(0)
{
    auto& loader = Utility::AssetLoader::GetLoader();
    loader.LoadFbx(p_fbxFileName.c_str(), this);

}

Gameplay::RenderComponent::RenderComponent(Mesh* p_mesh):
    m_componentVertexOffset(0),
    m_componentIndexOffset(0),
    m_meshVertexOffset(0),
    m_meshIndexOffset(0)
{
    m_meshes.push_back(std::move(p_mesh));
}

Gameplay::RenderComponent::~RenderComponent()
{
    for (int i = 0; i < m_meshes.size(); ++i)
    {
        SAFE_DELETE(m_meshes[i]);
    }
}


Gameplay::Mesh::Mesh()
{
}

Gameplay::Mesh::Mesh(
    std::vector<Vertex>&& p_vertices,
    std::vector<uint32_t>&& p_indices,
    SkeletonAnim* p_anim) :
    m_vertices(p_vertices),
    m_indices(p_indices),
    m_anim(p_anim),
    m_vertexOffset(0),
    m_indexOffset(0),
    m_roughness(0.0f)
{
}

Gameplay::Mesh::Mesh(
    std::vector<Vertex>&& p_vertices,
    std::vector<uint32_t>&& p_indices) :
    m_vertices(p_vertices),
    m_indices(p_indices),
    m_vertexOffset(0),
    m_indexOffset(0),
    m_anim(nullptr),
    m_roughness(0.0f)
{
}

void Gameplay::Mesh::SetRoughness(const float& p_roughness)
{
    m_roughness = p_roughness;
}

Gameplay::Mesh::~Mesh()
{
    SAFE_DELETE(m_anim);
}
 