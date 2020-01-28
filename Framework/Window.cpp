#include "Window.h"
fcf::Window::Window(const WindowDescriptor& p_desc):
	m_descriptor(p_desc),
    m_window(nullptr)
{

}
fcf::Window::~Window()
{

}
void fcf::Window::OnInit()
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

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    
}

void fcf::Window::OnUpdate()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
    }
}

void fcf::Window::OnDestory()
{
    glfwDestroyWindow(m_window);
    m_window = nullptr;
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
