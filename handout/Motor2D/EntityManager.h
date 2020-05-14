#ifndef __EntityManager_H__
#define __EntityManager_H__
#include "SDL_image/include/SDL_image.h"
#include "j1Module.h"
#include "Entity.h"
#include <vector>

class Entity;

class j1EntityManager : public j1Module
{
	//FUNCTIONS
public:
	j1EntityManager();
	~j1EntityManager();


	bool Awake(pugi::xml_node&);

	bool Start();

	bool PreUpdate();

	bool Update(float dt);

	bool PostUpdate();

	bool CleanUp();

	Entity* AddNewEntity(ENTITY_TYPE type, fPoint pos);
	void CheckUnitsOnSelection(SDL_Rect selectRect);
	bool CommandSelectedUnits(iPoint moveTo, int lvl);


public:
	SDL_Texture* textures = nullptr;
private:
	std::vector<Entity*> entities;

};


#endif // !__EntityManager_H__