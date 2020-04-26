#pragma once
#include "stdafx.h"
#include "../Framework/INoCopy.h"

namespace Gameplay
{
	using Entity = uint32_t;

	class EntityManager :  public Interface::INoCopy
	{
	public:
		enum { MAX_NUM_ENTITYES = 1024 };

		static EntityManager& GetManager()
		{
			static EntityManager l_manger;
			return l_manger;
		}

		Entity SpwanEntity()
		{
			auto l_newEntity = m_nextToSpawn++;
			m_entities[l_newEntity] = true;
			return l_newEntity;
		}

		void EndEntity(const Entity& p_entity)
		{
			m_entities[p_entity] = false;
		}

		const std::array<bool, MAX_NUM_ENTITYES>& GetAllEntities()
		{
			return m_entities;
		}

		~EntityManager();

	private:

		EntityManager();

		Entity m_nextToSpawn;

		std::array<bool, MAX_NUM_ENTITYES> m_entities;

	};
}