#include "stdafx.h"
#include "Window.h"

std::function<void(double, double)> Framework::Window::m_scrollCallback = nullptr;

Framework::Window::Window(const WindowDescriptor& p_desc):
	m_descriptor(p_desc),
    m_window(nullptr)
{

}
Framework::Window::~Window()
{

}
void Framework::Window::m_scrollCallbackFp(GLFWwindow* p_window, double x, double y)
{
    m_scrollCallback(x, y);
}
void Framework::Window::OnInit()
{

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    m_window = glfwCreateWindow(m_descriptor.width, m_descriptor.height, m_descriptor.title.c_str(), NULL, NULL);
    if (!m_window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    //glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(m_window, &m_scrollCallbackFp);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    
}

void Framework::Window::OnUpdate()
{
    if (!glfwWindowShouldClose(m_window))
    {
        m_renderCallback();
        glfwPollEvents();
    }
}

void Framework::Window::OnDestory()
{
    glfwDestroyWindow(m_window);
    m_window = nullptr;
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
