#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "SDL/include/SDL_rect.h"
#include "p2Point.h"
#include <vector>

struct Collider;
struct SDL_Texture;

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
	virtual bool PostUpdate(float dt);

	virtual void CollisionPosUpdate();
	virtual Collider* GetCollider() const;
	void SetPosition(int x, int y);

	fPoint GetPosition();

	void SetToDelete(bool toDelete);
	virtual void Draw(float dt);
	void DebugDraw();

public:
	bool toDelete;
	int moveRange1, moveRange2;

protected:
	fPoint position;
	Collider* collider;
	ENTITY_TYPE type;
	fPoint speed;

	iPoint origin, mouse;
	std::vector <iPoint> path;
	std::vector<Entity*> closeEntityList;
	std::vector<Entity*> collidingEntityList;

	bool Move(float dt);
	bool GeneratePath(int x, int y);


	fPoint GetDirectionSpeed(std::vector<Entity*>closeEntityList);
	fPoint GetCohesionSpeed(std::vector<Entity*>closeEntityList, fPoint position);
	fPoint GetSeparationSpeed(std::vector<Entity*>collidingEntityList, fPoint position);
};

#endif//__ENTITY_H__