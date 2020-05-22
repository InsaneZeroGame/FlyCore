#pragma once
#include "stdafx.h"
#include "../Framework/INoCopy.h"
#include "imgui.h"
#include "Window.h"
#include "imgui_impl_win32.h"


namespace UI
{
	class UISystem : public Interface::INoCopy
	{
	public:
		virtual ~UISystem();

	protected:
		UISystem(Framework::Window* p_window);
		Framework::Window* m_window;
	};

	
}