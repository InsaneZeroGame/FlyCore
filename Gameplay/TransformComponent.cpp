#include "TransformComponent.h"


Gameplay::TranformComponent::TranformComponent(const glm::vec3 p_pos, const glm::vec3& p_lookAt):
	m_model(1.0f),
	m_lookAt(p_lookAt),
	m_pos(p_pos),
	m_quaternion(glm::quat()),
	m_yaw(0.0),
	m_pitch(0.0),
	m_roll(0.0)
{
}

void Gameplay::TranformComponent::UpdateTransform()
{
	auto yaw = glm::angleAxis(m_yaw,glm::vec3(0.0f,1.0f,0.0f));
	auto pitch = glm::angleAxis(m_pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	auto rotation = glm::mat4_cast(glm::normalize(yaw * pitch));
	m_model = rotation;
	m_yaw = 0;
	m_pitch = 0;
}

Gameplay::TranformComponent::~TranformComponent()
{

}
