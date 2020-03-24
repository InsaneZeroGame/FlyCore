#include "BaseCamera.h"

Gameplay::BaseCamera::BaseCamera(const glm::vec3& p_pos, const glm::vec3& p_lookAt):
	TranformComponent(p_pos,p_lookAt),
	m_fov(45.0f),
	m_width(30.0f),
	m_height(30.0f),
	m_near(0.1f),
	m_far(50.0f)
{
	m_proj = glm::perspectiveFovLH(m_fov, m_width, m_height, m_near, m_far);
	m_view = glm::lookAtLH(m_pos, m_lookAt, Y_UP);
}

Gameplay::BaseCamera::~BaseCamera()
{
}

void Gameplay::BaseCamera::UpdateCamera()
{
	m_proj = glm::perspectiveFovLH(m_fov, m_width, m_height, m_near, m_far);
	m_inverseProj = glm::inverse(m_proj);
}

void Gameplay::BaseCamera::OnMouseWheelScroll(double x, double y)
{
	m_fov -= y * 0.03;
}
