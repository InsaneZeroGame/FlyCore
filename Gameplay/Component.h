#pragma once
#include <memory>


namespace Gameplay
{
	template<typename T>
	class Component
	{
	public:
		virtual ~Component()
		{
			if (m_component)
			{
				delete m_component;
				m_component = nullptr;
			}
		};

		template<typename... Args>
		Component(Args&&... args)
		{
			m_component = new T(std::forward<Args>(args)...);
		}

		T* GetComponent()
		{
			return m_component;
		}


	protected:

		T* m_component;
	};
}