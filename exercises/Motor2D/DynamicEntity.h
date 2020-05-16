#pragma once

#ifndef __DYNAMICENTITY_H__
#define __DYNAMICENTITY_H__

#include "Entity.h"
#include <list>

class DynamicEntity : public Entity
{
public:

	DynamicEntity(fPoint position, ENTITY_TYPE type, ENTITY_ALIGNEMENT align, Collider* collider, int moveRange1, int moveRange2);

	~DynamicEntity();

	//Returns true while moving, returns false when stops
	bool Move(float dt);

	bool GeneratePath(int x, int y);
	void DebugDraw();
	virtual void OnCollision(Collider* collider) {};
	void Draw(float dt);

public:
	bool isMoving;

	//used for moving close entities to have an smooth group movement
	int moveRange1;
	int moveRange2;

	fPoint speed;

protected:

	iPoint origin, mouse;
	std::vector <iPoint> path;
	std::vector<DynamicEntity*> closeEntityList;
	std::vector<DynamicEntity*> collidingEntityList;

private:
	fPoint DynamicEntity::GetDirectionSpeed(std::vector<DynamicEntity*>closeEntityList);
	fPoint DynamicEntity::GetCohesionSpeed(std::vector<DynamicEntity*>closeEntityList, fMPoint position);
	fPoint DynamicEntity::GetSeparationSpeed(std::vector<DynamicEntity*>collidingEntityList, fMPoint position);
	SDL_Rect GetAnimationRect(float dt);

};

#endif//__DYNAMICENTITY_H__