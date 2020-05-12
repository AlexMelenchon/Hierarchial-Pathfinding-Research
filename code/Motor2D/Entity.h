#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "SDL/include/SDL_rect.h"
#include "p2Point.h"
#include <vector>

struct Collider;
struct SDL_Texture;

#define ENTITY_SPEED 50

enum class ENTITY_TYPE
{
	UNKNOWN,
	DYNAMIC,

	MAX_TYPE
};

enum class ENTITY_ALIGNEMENT
{
	UNKNOWN,
	PLAYER,
	ENEMY,
	NEUTRAL,


	MAX_ALIGN
};

class Entity
{
public:

	Entity();
	Entity(fPoint position, ENTITY_TYPE type);
	virtual ~Entity();

	virtual bool Start();
	virtual bool PreUpdate(float dt);
	virtual bool Update(float dt);
	virtual bool PostUpdate();

	void SetPosition(int x, int y);

	fPoint GetPosition();

	void SetToDelete(bool toDelete);
	void DebugDraw();

	bool GeneratePath(int x, int y, int lvl);


public:
	bool toDelete;
	bool selected;

	SDL_Color color;

protected:
	fPoint position;
	ENTITY_TYPE type;

	iPoint origin, mouse;
	std::vector <iPoint> path;


	bool Move(float dt);

};

#endif//__ENTITY_H__