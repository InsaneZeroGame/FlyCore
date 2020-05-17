#include "Director.h"

Gameplay::Director::Director()
{
}

Gameplay::Director::~Director()
{
}

void Gameplay::Director::Scale(const Entity& e, const glm::vec3& p_scale)
{
	m_systemEntites[e]->GetComponent()->m_model = glm::scale(m_systemEntites[e]->GetComponent()->m_model, p_scale);
}

void Gameplay::Director::Translate(const Entity& e, const glm::vec3& p_pos)
{
	m_systemEntites[e]->GetComponent()->m_model = glm::translate(m_systemEntites[e]->GetComponent()->m_model, p_pos);

}