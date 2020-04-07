#ifndef __EntityManager_H__
#define __EntityManager_H__
#include "SDL_image/include/SDL_image.h"
#include "j1Module.h"
#include "Entity.h"
#include <vector>

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

	void GetEntityNeighbours(std::vector<Entity*>* close_entity_list, std::vector<Entity*>* colliding_entity_list, Entity* thisUnit);

public:
	SDL_Texture* textures = nullptr;
private:
	std::vector<Entity*> entities;

};


#endif // !__EntityManager_H__