#pragma once
#include "TransformComponent.h"
#include "InputComponent.h"

static const glm::vec3 Y_UP = glm::vec3(0.0f,1.0f,0.0f);

namespace Gameplay
{
	class BaseCamera : public TranformComponent,public InputComponent
	{
	public:
		BaseCamera(const glm::vec3& p_pos,const glm::vec3& p_lookAt);
		virtual ~BaseCamera();;

		void Zoom(float p_zoom);

		__forceinline const glm::mat4& GetShadowProj() const { return m_shadowProj; }

		__forceinline const glm::mat4& GetProj() const { return m_proj; }

		__forceinline const glm::mat4& GetView() const { return m_view; }

		__forceinline const glm::mat4& GetInverseProj() const { return m_inverseProj; }

		__forceinline const float& GetNear() const { return m_near; }

		__forceinline const float& GetFar() const { return m_far; }

		void UpdateCamera();

		// Inherited via InputComponent
		virtual void OnMouseWheelScroll(double x, double y) override;

		virtual void OnMouseMove(double x, double y) override;

		virtual void OnKeyPress(int key, int scancode, int action, int mods) override;

		__forceinline bool IsCameraUpdated() { return m_needToUpdate ? m_needToUpdate++:false; }
	protected:
		glm::mat4 m_proj;

		glm::mat4 m_shadowProj;

		glm::mat4 m_view;

		glm::mat4 m_inverseProj;

		float m_fov;

		float m_near;

		float m_far;

		float m_width;

		float m_height;

		bool m_needToUpdate;
	};
}