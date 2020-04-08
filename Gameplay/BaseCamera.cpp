#include "BaseCamera.h"

Gameplay::BaseCamera::BaseCamera(const glm::vec3& p_pos, const glm::vec3& p_lookAt):
	TranformComponent(p_pos,p_lookAt),
	m_fov(45.0f),
	m_width(30.0f),
	m_height(30.0f),
	m_near(5.0f),
	m_far(50.0f),
	m_needToUpdate(false)
{
	m_proj = glm::perspectiveFovLH(m_fov, m_width, m_height, m_near, m_far);
	m_shadowProj = glm::perspectiveFovLH(m_fov, m_width, m_height, m_near, m_far);
	m_view = glm::lookAtLH(m_pos, m_lookAt, Y_UP);
}

Gameplay::BaseCamera::~BaseCamera()
{
}

void Gameplay::BaseCamera::UpdateCamera()
{
	UpdateTransform();
	m_proj = glm::perspectiveFovLH(m_fov, m_width, m_height, m_near, m_far);
	m_inverseProj = glm::inverse(m_proj);
	m_view = m_model * m_view;
}

void Gameplay::BaseCamera::OnMouseWheelScroll(double x, double y)
{
	m_fov -= float(y) * 0.03f;
	m_needToUpdate = true;
}

void Gameplay::BaseCamera::OnMouseMove(double x, double y)
{
	//if (abs(m_mouseX - x) < abs(m_mouseY - y))
	//{
	//	m_pitch = m_mouseY < y ? 0.016f : -0.016f;
	//}
	//else
	//{
	//	m_yaw = m_mouseX < x ? 0.01f : -0.01f;
	//}
	//
	//m_mouseX = x;
	//m_mouseY = y;
	//m_needToUpdate = true;
}

#define GLFW_KEY_A                  65
#define GLFW_KEY_B                  66
#define GLFW_KEY_C                  67
#define GLFW_KEY_D                  68
#define GLFW_KEY_E                  69
#define GLFW_KEY_F                  70
#define GLFW_KEY_G                  71
#define GLFW_KEY_H                  72
#define GLFW_KEY_I                  73
#define GLFW_KEY_J                  74
#define GLFW_KEY_K                  75
#define GLFW_KEY_L                  76
#define GLFW_KEY_M                  77
#define GLFW_KEY_N                  78
#define GLFW_KEY_O                  79
#define GLFW_KEY_P                  80
#define GLFW_KEY_Q                  81
#define GLFW_KEY_R                  82
#define GLFW_KEY_S                  83
#define GLFW_KEY_T                  84
#define GLFW_KEY_U                  85
#define GLFW_KEY_V                  86
#define GLFW_KEY_W                  87
#define GLFW_KEY_X                  88
#define GLFW_KEY_Y                  89
#define GLFW_KEY_Z                  90
#define GLFW_PRESS                  1
#define GLFW_REPEAT                 2


void Gameplay::BaseCamera::OnKeyPress(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_A && ( action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		m_yaw = 0.005f;
		m_needToUpdate = true;
	}

	if (key == GLFW_KEY_D && ( action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		m_yaw = -0.005f;
		m_needToUpdate = true;
	}

	if (key == GLFW_KEY_W && ( action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		m_pitch = 0.005f;
		m_needToUpdate = true;
	}

	if (key == GLFW_KEY_S && ( action == GLFW_PRESS))
	{
		m_pitch = -0.005f;
		m_needToUpdate = true;
	}

}
