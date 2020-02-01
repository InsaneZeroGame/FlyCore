#pragma once
#include "stdafx.h"
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include "IModule.h"
#include <functional>

namespace fcf
{
	struct WindowDescriptor
	{
		uint32_t width;
		uint32_t height;
		std::string title;
	};


	class Window : public Interface::IModule
	{
	public:
		Window(const WindowDescriptor&);

		~Window();

		void SetRendererCallback(std::function<void(void)> p_callback)
		{
			m_renderCallback = p_callback;
		}
	private:
		WindowDescriptor m_descriptor;

		GLFWwindow* m_window;

		std::function<void(void)> m_renderCallback;

	public:
		// Inherited via IModule
		virtual void OnInit() override;

		virtual void OnUpdate() override;

		virtual void OnDestory() override;

		__forceinline uint32_t GetWidth() const { return m_descriptor.width; }
		
		__forceinline uint32_t GetHeight() const { return m_descriptor.height; }

		__forceinline HWND GetWin32Window() { return GetActiveWindow(); }
	};
}