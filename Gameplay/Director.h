#pragma once
#include "stdafx.h"
#include "System.h"
#include "TransformComponent.h"
namespace Gameplay
{
	class Director : public System<TranformComponent>,public Interface::INoCopy
	{
	public:
		static Director& GetDirector()
		{
			static Director l_director;
			return l_director;
		}

		~Director();

		void Scale(const Entity& e,const glm::vec3& p_scale);

	private:

		Director();
	};
	
}