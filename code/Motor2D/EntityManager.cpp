#include "EntityManager.h"
#include "j1App.h"
#include "j1Textures.h"
#include "p2Log.h"

j1EntityManager::j1EntityManager()
{}


j1EntityManager::~j1EntityManager()
{}


bool j1EntityManager::Awake(pugi::xml_node&)
{
	bool ret = true;
	return ret;
}


bool j1EntityManager::Start()
{
	bool ret = true;
	return ret;
}


bool j1EntityManager::PreUpdate()
{
	bool ret = true;
	int numElements = entities.size();

	for (int i = 0; i < numElements; i++)
	{
		if (entities[i]->toDelete)
		{
			delete entities[i];
			entities[i] = nullptr;
			entities.erase(entities.begin() + i);
			i--;
		}
	}

	return ret;
}


bool j1EntityManager::Update(float dt)
{
	bool ret = true;

	for (int i = 0; i < entities.size(); i++)
	{
		entities[i]->Update(dt);
	}

	return ret;
}


bool j1EntityManager::PostUpdate()
{
	bool ret = true;


	return ret;
}


bool j1EntityManager::CleanUp()
{
	bool ret = true;

	//clean the entites
	int numElements = entities.size();

	for (int i = 0; i < numElements; i++)
	{
		RELEASE(entities[i]);
		entities[i] = nullptr;
	}

	entities.clear();

	return ret;
}

Entity* j1EntityManager::AddNewEntity(ENTITY_TYPE type, fPoint pos)
{
	Entity* entity = nullptr;

	switch (type)
	{
	case ENTITY_TYPE::DYNAMIC:
		entity = new Entity(pos, type);
		break;

	case ENTITY_TYPE::UNKNOWN:

		break;
	}

	if (entity != nullptr)
		entities.push_back(entity);

	return entity;
}

void j1EntityManager::GetEntityNeighbours(std::vector<Entity*>* close_entity_list, std::vector<Entity*>* colliding_entity_list, Entity* thisUnit)
{
	close_entity_list->clear();
	colliding_entity_list->clear();

	Entity* it;

	for (int i = 0; i < entities.size(); ++i)
	{

		it = entities[i];

		if (it != thisUnit)
		{
			fPoint pos = it->GetPosition();

			float distance = pos.DistanceTo(thisUnit->GetPosition());
			if (distance < it->moveRange2)
			{
				colliding_entity_list->push_back(it);

			}
			if (distance < thisUnit->moveRange1)
			{
				close_entity_list->push_back(it);
			}
		}

	}

}
