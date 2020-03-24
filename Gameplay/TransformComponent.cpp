#include "TransformComponent.h"

Gameplay::TranformComponent::TranformComponent(const glm::vec3 p_pos, const glm::vec3& p_lookAt):
	m_model(1.0f),
	m_lookAt(p_lookAt),
	m_pos(p_pos)
{
}

Gameplay::TranformComponent::~TranformComponent()
{
}
