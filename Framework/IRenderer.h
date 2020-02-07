#pragma once
#include "Window.h"

namespace Interface
{
	class IRenderer
	{
	public:
		virtual void RenderScene() = 0;

		__forceinline void SetTargetWindow(Framework::Window* p_window) 
		{
			m_window = p_window;
		};
	protected:
		IRenderer(): m_window(nullptr) {};

		virtual ~IRenderer() {};

		Framework::Window* m_window = nullptr;
	};
}