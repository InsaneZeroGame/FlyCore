#include "stdafx.h"
#include "Window.h"

std::function<void(double, double)> Framework::Window::m_scrollCallback = nullptr;
std::function<void(double, double)> Framework::Window::m_mouseMoveCallback = nullptr;
std::function<void(int key, int scancode, int action, int mods)> Framework::Window::m_keypress = nullptr;

bool Framework::Window::m_exit = false;
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
void Framework::Window::m_mouseMoveCallbackFp(GLFWwindow* p_window, double x, double y)
{
    m_mouseMoveCallback(x, y);
}




void Framework::Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        m_exit = true;
    }
    m_keypress(key, scancode, action, mods);
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
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetScrollCallback(m_window, &m_scrollCallbackFp);
    glfwSetKeyCallback(m_window, &key_callback);
    glfwSetCursorPosCallback(m_window, &m_mouseMoveCallbackFp);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    
}

void Framework::Window::OnUpdate()
{
    while (!glfwWindowShouldClose(m_window) && !m_exit)
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
