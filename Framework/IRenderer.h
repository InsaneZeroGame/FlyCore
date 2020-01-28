#pragma once
#include "Window.h"

namespace Interface
{
	class IRenderer
	{
	public:
		virtual void RenderScene() = 0;

		__forceinline void SetTargetWindow(fcf::Window* p_window) 
		{
			m_window = p_window;
		};
	protected:
		IRenderer(): m_window(nullptr) {};

		virtual ~IRenderer() {};

		fcf::Window* m_window = nullptr;
	};
}