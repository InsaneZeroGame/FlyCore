#pragma once
#include "Window.h"
#include "../Gameplay/BaseCamera.h"

namespace Interface
{
	class IRenderer
	{
	public:
		virtual void RenderScene() = 0;

		virtual void SetCamera(Gameplay::BaseCamera* p_camera) { m_mainCamera = p_camera; };

		__forceinline void SetTargetWindow(Framework::Window* p_window) 
		{
			m_window = p_window;
		};
	protected:
		IRenderer(): m_window(nullptr) {};

		virtual ~IRenderer() {};

		Framework::Window* m_window = nullptr;

		Gameplay::BaseCamera* m_mainCamera;

		struct SceneUniformData
		{
			glm::mat4x4 m_proj;
			glm::mat4x4 m_view;
			glm::mat4x4 m_inverProj;
			glm::mat4x4 m_shadowMatrix;
			std::array<float, 4> zNearFar;
		}; //m_uniformBuffer;
	};
}