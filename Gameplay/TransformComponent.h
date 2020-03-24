#pragma once
#include "stdafx.h"

namespace Gameplay
{
	class TranformComponent
	{
	public:

		virtual ~TranformComponent();

		__forceinline void SetLookAt(glm::vec4 p_lookAt) { m_lookAt = p_lookAt; };

		__forceinline const glm::mat4& GetModelMatrix() const { return m_model; };
		
		void UpdateTransform();
	protected:
		TranformComponent(const glm::vec3 p_pos, const glm::vec3& p_lookAt);

		glm::mat4 m_model;

		glm::vec3 m_lookAt;

		glm::vec3 m_pos;

		glm::quat m_quaternion;

		float m_yaw;

		float m_pitch;

		float m_roll;

		double m_mouseX;
		double m_mouseY;
		double m_mouseXMax;
		double m_mouseYMax;

	};
}