#include "Entity.h"

Gameplay::EntityManager::EntityManager():
	m_nextToSpawn(0),
	m_entities({ false })
{
}

Gameplay::EntityManager::~EntityManager()
{
}
