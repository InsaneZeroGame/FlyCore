#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include "IModule.h"
#include <functional>

namespace Framework
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

		__forceinline void SetRendererCallback(std::function<void(void)> p_callback)
		{
			m_renderCallback = p_callback;
		}

		__forceinline void SetScrollCallback(std::function<void(double, double)> p_callback)
		{
			m_scrollCallback = p_callback;
		}

		__forceinline void SetMouseMoveCallback(std::function<void(double, double)> p_callback)
		{
			m_mouseMoveCallback = p_callback;
		}
	private:
		WindowDescriptor m_descriptor;

		GLFWwindow* m_window;

		std::function<void(void)> m_renderCallback;

		static std::function<void(double, double)> m_scrollCallback;

		static std::function<void(double, double)> m_mouseMoveCallback;

		static void m_scrollCallbackFp(GLFWwindow* p_window,double x, double y);

		static void m_mouseMoveCallbackFp(GLFWwindow* p_window, double x, double y);

		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

		static bool m_exit;
	public:
		// Inherited via IModule
		virtual void OnInit() override;

		virtual void OnUpdate() override;

		virtual void OnDestory() override;

		__forceinline uint32_t GetWidth() const { return m_descriptor.width; }
		
		__forceinline uint32_t GetHeight() const { return m_descriptor.height; }

		__forceinline HWND GetWin32Window() { return GetActiveWindow(); }

		struct MouseWheel
		{
			double x;
			double y;
		}m_mouseWheel;

	};
}