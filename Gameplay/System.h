#pragma once
#include "stdafx.h"
#include "Entity.h"
#include "Component.h"
#include <unordered_map>
 
namespace Gameplay
{
	template<typename S>
	class System
	{
	public:

		virtual ~System() 
		{
			
		};
		template<typename... Arg>
		void AddComponent(const Entity& p_entity, Arg&&... Args)
		{
			m_entites.emplace(std::pair<Entity, Component<S>*>(p_entity, new Component<S>(std::forward<Arg>(Args)...)));
		};

	protected:
		System() {};

		std::unordered_map<Entity, Component<S>*> m_entites;

	};
	
}