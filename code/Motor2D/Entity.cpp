#include "Entity.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1App.h"
#include "Collision.h"
#include "Brofiler/Brofiler/Brofiler.h"
#include "j1Map.h"
#include "EntityManager.h"
#include "j1Scene.h"

Entity::Entity()
{}

Entity::Entity(fPoint position, ENTITY_TYPE type) :

	position(position),
	type(type),

	toDelete(false),

	collider(nullptr),
	speed{ 0,0 }

{}


Entity::~Entity()
{
	collider->to_delete = true;

	collider = nullptr;
	
}


bool Entity::Start()
{

	if (collider != nullptr)
	{
		collider = new Collider(collider->rect, collider->type, collider->callback, this);
		collider->thisEntity = this;
		App->coll->AddColliderEntity(collider);
	}

	return true;
}


bool Entity::PreUpdate(float dt)
{
	return true;
}


bool Entity::Update(float dt)
{
	return true;
}


bool Entity::PostUpdate(float dt)
{
	return true;
}


void Entity::CollisionPosUpdate()
{
	collider->SetPos(position.x, position.y);
}


Collider* Entity::GetCollider() const
{
	return collider;
}


void Entity::Draw(float dt)
{}


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
	BROFILER_CATEGORY("Move Unit", Profiler::Color::BlanchedAlmond);

	//Speed is resetted to 0 each iteration
	speed = { 0, 0 };

	// ----------------------------------------------------------------

	fPoint pathSpeed;
	pathSpeed.create(0, 0);
	fPoint nextPoint;

	if (path.size() > 0)
	{
		App->map->MapToWorldCoords(path[0].x, path[0].y, App->map->data, nextPoint.x, nextPoint.y);

		pathSpeed.create((nextPoint.x - position.x), (nextPoint.y - position.y)).Normalize();
	}

	// ----------------------------------------------------------------

	App->entMan->GetEntityNeighbours(&closeEntityList, &collidingEntityList, this);

	//---------------------------------------------------------------- 
	fPoint separationSpeed;

	if (!collidingEntityList.empty())
	{
		separationSpeed = GetSeparationSpeed(collidingEntityList, position);
	}
	else
	{
		separationSpeed.x = 0;
		separationSpeed.y = 0;
	}

	// ---------------------------------------------------------------- 

	fPoint cohesionSpeed = { 0,0 };
	if (!closeEntityList.empty())
	{
		cohesionSpeed = GetCohesionSpeed(closeEntityList, position);
	}
	else
	{
		cohesionSpeed.x = 0;
		cohesionSpeed.y = 0;
	}

	//---------------------------------------------------------------- 

	fPoint alignmentSpeed = { 0,0 };
	if (!closeEntityList.empty() && (abs(pathSpeed.x) > 0 || abs(pathSpeed.y) > 0))
	{
		alignmentSpeed = GetDirectionSpeed(closeEntityList);
	}
	else
	{
		alignmentSpeed.x = 0;
		alignmentSpeed.y = 0;
	}

	// ----------------------------------------------------------------- 

	speed += pathSpeed + separationSpeed * 1 + cohesionSpeed * 0.5f + alignmentSpeed * 0.1f;

	// ------------------------------------------------------------------

	position.x += (speed.x) * dt;
	position.y += (speed.y) * dt;

	if (path.size() > 0 && abs(position.x - nextPoint.x) <= 5 && abs(position.y - nextPoint.y) <= 5)
	{
		path.erase(path.begin());
	}

	if (pathSpeed.IsZero())
	{
		return true;
	}
	else
	{
		return false;
	}

}

fPoint Entity::GetSeparationSpeed(std::vector<Entity*>colliding_entity_list, fPoint position)
{
	BROFILER_CATEGORY("SEPARATION SPEED", Profiler::Color::Aquamarine);

	fPoint separationSpeed = { 0,0 };

	if (colliding_entity_list.size() == 0)
	{
		return separationSpeed;
	}

	Entity* it;

	for (int i = 0; i < colliding_entity_list.size(); i++)
	{
		it = colliding_entity_list[i];

		separationSpeed.x += (position.x - it->GetPosition().x);
		separationSpeed.y += (position.y - it->GetPosition().y);
	}

	separationSpeed.x /= colliding_entity_list.size();
	separationSpeed.y /= colliding_entity_list.size();

	float spdNorm = sqrtf(separationSpeed.x * separationSpeed.x + separationSpeed.y * separationSpeed.y);

	if (spdNorm == 0)
	{
		separationSpeed = { 0,0 };
	}
	else
	{
		separationSpeed.x /= spdNorm;
		separationSpeed.y /= spdNorm;
	}


	return separationSpeed;
}

fPoint Entity::GetCohesionSpeed(std::vector<Entity*>close_entity_list, fPoint position)
{
	BROFILER_CATEGORY("COHESION SPEED", Profiler::Color::DarkOliveGreen);

	fPoint MassCenter{ (float)position.x, (float)position.y };

	Entity* it = nullptr;

	for (int i = 0; i < close_entity_list.size(); i++)
	{
		it = close_entity_list[i];

		MassCenter.x += it->GetPosition().x;
		MassCenter.y += it->GetPosition().y;
	}

	MassCenter.x = MassCenter.x / (close_entity_list.size() + 1);
	MassCenter.y = MassCenter.y / (close_entity_list.size() + 1);

	fPoint cohesionSpeed = { MassCenter.x - position.x, MassCenter.y - position.y };

	float norm = sqrt(cohesionSpeed.x * cohesionSpeed.x + cohesionSpeed.y * cohesionSpeed.y);

	int diameter = (it->moveRange1 * 2 + 1);

	if (cohesionSpeed.x < diameter && cohesionSpeed.x > -diameter)
	{
		cohesionSpeed.x = 0;
	}
	else
	{
		cohesionSpeed.x = cohesionSpeed.x / norm;
	}
	if (cohesionSpeed.y < diameter && cohesionSpeed.y > -diameter)
	{
		cohesionSpeed.y = 0;
	}
	else
	{
		cohesionSpeed.y = cohesionSpeed.y / norm;
	}

	return cohesionSpeed;
}

fPoint Entity::GetDirectionSpeed(std::vector<Entity*>close_entity_list)
{
	BROFILER_CATEGORY("DIRECTION SPEED", Profiler::Color::Magenta);

	fPoint alignmentSpeed{ 0, 0 };

	Entity* it;

	for (int i = 0; i < close_entity_list.size(); i++)
	{
		it = close_entity_list[i];

		alignmentSpeed += it->speed;
	}

	alignmentSpeed.x = alignmentSpeed.x / close_entity_list.size();
	alignmentSpeed.y = alignmentSpeed.y / close_entity_list.size();

	float norm = sqrt(alignmentSpeed.x * alignmentSpeed.x + alignmentSpeed.y * alignmentSpeed.y);

	if (norm != 0)
	{
		alignmentSpeed = alignmentSpeed / norm;
	}

	return alignmentSpeed;
}

bool Entity::GeneratePath(int x, int y)
{
	iPoint goal = { 0,0 };

	App->map->WorldToMapCoords(round(position.x), round(position.y), App->map->data, origin.x, origin.y);
	goal = App->map->WorldToMap(x, y);

	//if (App->pathfinding->CreatePath(origin, goal) == 0)
	//{
	//	path.clear();
	//	app->pathfinding->SavePath(&path);
	//	path.erase(path.begin());
	//	return true;
	//}

	return false;
}

void Entity::DebugDraw()
{
	//Position --------------------------------------
	App->render->DrawQuad({ (int)position.x, (int)position.y, 2,2 }, 255, 0, 0);

	fPoint nextPoint = { 0,0 };
	App->map->WorldToMapCoords(round(position.x), round(position.y),App->map->data, origin.x, origin.y);

	fPoint origin2 = App->map->MapToWorld(origin.x, origin.y);

	App->render->DrawQuad({ (int)origin2.x, (int)origin2.y, 10,10 }, 255, 255, 255, 125);

	origin = { (int)origin2.x, (int)origin2.y };

	// Debug pathfinding ------------------------------

	SDL_Texture* debugTex = App->scene->debug_tex;

	std::vector<iPoint>* path = &this->path;
	for (std::vector<iPoint>::iterator it = path->begin(); it != path->end(); ++it)
	{
		fPoint pos = App->map->MapToWorld(it->x - 1, it->y);
		App->render->Blit(debugTex, pos.x, pos.y);
	}
}