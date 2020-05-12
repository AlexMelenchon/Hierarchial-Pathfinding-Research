#include "EntityManager.h"
#include "j1App.h"
#include "j1Textures.h"
#include "p2Log.h"
#include "Entity.h"
#include "Collision.h"

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

	for (int i = 0; i < entities.size(); i++)
	{
		entities[i]->PostUpdate();
	}

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

void j1EntityManager::CheckUnitsOnSelection(SDL_Rect selection)
{
	int numEntities = entities.size();

	ENTITY_TYPE type;

	for (int i = 0; i < numEntities; i++)
	{
		fPoint entPos = entities[i]->GetPosition();

		Collider col = { { (int)entPos.x, (int)entPos.y, 1, 1 } };

		if (col.CheckCollision(selection))
		{

			entities[i]->selected = true;
			entities[i]->color = { 255,0,0 };
		}
		else
		{
			entities[i]->selected = false;
			entities[i]->color = { 255,255,255 };
		}

	}
}

bool j1EntityManager::CommandSelectedUnits(iPoint moveTo, int lvl)
{
	bool ret = false;
	int numEntities = entities.size();

	for (int i = 0; i < numEntities; i++)
	{
		if (entities[i]->selected == true)
		{
			entities[i]->GeneratePath(moveTo.x, moveTo.y, lvl);
			ret = true;
		}
	}

	return ret;

}




