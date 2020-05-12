#include "Entity.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1App.h"
#include "Collision.h"
#include "Brofiler/Brofiler/Brofiler.h"
#include "j1Map.h"
#include "EntityManager.h"
#include "j1Scene.h"
#include "j1Pathfinding.h"

Entity::Entity()
{}

Entity::Entity(fPoint position, ENTITY_TYPE type) :

	position(position),
	type(type),

	toDelete(false),

	color{ 255,255,255 }

{}


Entity::~Entity()
{
	path.clear();
}


bool Entity::Start()
{

	return true;
}


bool Entity::PreUpdate(float dt)
{
	return true;
}


bool Entity::Update(float dt)
{
	Move(dt);
	return true;
}


bool Entity::PostUpdate()
{
	DebugDraw();
	return true;
}

fPoint Entity::GetPosition()
{
	return position;
}


void Entity::SetPosition(int x, int y)
{
	position.x = x;
	position.y = y;
}

void Entity::SetToDelete(bool toDel)
{
	if (toDel != toDelete)
	{
		toDelete = !toDelete;
	}
}

bool Entity::Move(float dt)
{
	//Speed is resetted to 0 each iteration
	// ----------------------------------------------------------------

	fPoint pathSpeed = { 0,0 };
	fPoint nextPoint = { 0,0 };

	if ( path.size() < 2)
	{
		App->pathfinding->RequestPath(this, &path);
	}

	if (path.size() > 0)
	{
		App->map->MapToWorldCoords(path[0].x, path[0].y, App->map->data, nextPoint.x, nextPoint.y);

		nextPoint.x += 32;
		nextPoint.y += 16;

		pathSpeed.create((nextPoint.x - position.x), (nextPoint.y - position.y)).Normalize();
	}

	// -----------------------------------------------------------------
	pathSpeed = pathSpeed * ENTITY_SPEED;

	position.x += (pathSpeed.x) * dt;
	position.y += (pathSpeed.y) * dt;

	// ------------------------------------------------------------------

	if (path.size() > 0 && abs(position.x - nextPoint.x) <= 5 && abs(position.y - nextPoint.y) <= 5)
	{
		path.erase(path.begin());
	}

	if (!pathSpeed.IsZero())
	{
		return true;
	}

}


bool Entity::GeneratePath(int x, int y, int lvl)
{
	iPoint goal = { 0,0 };

	origin = App->map->WorldToMap((position.x ), (position.y ));
	goal = { x,y };

	if (App->pathfinding->CreatePath(origin, goal, lvl, this) != PATH_TYPE::NO_TYPE)
	{
		path.clear();
		App->pathfinding->RequestPath(this, &path);

		if (!path.empty())
			path.erase(path.begin());

		return true;
	}

	return false;
}

void Entity::DebugDraw()
{
	//Position --------------------------------------
	App->render->DrawQuad({ (int)position.x, (int)position.y, 15,15 }, color.r, color.g, color.b);

	// Debug pathfinding ------------------------------

	SDL_Texture* debugTex = App->scene->debug_tex;
	SDL_SetTextureAlphaMod(debugTex, 127);

	std::vector<iPoint>* path = &this->path;
	for (std::vector<iPoint>::iterator it = path->begin(); it != path->end(); ++it)
	{
		fPoint pos = App->map->MapToWorld(it->x, it->y);
		App->render->Blit(debugTex, pos.x, pos.y);

	}
	SDL_SetTextureAlphaMod(debugTex, 255);


}
