#include "j1App.h"
#include "j1Render.h"
#include "j1Input.h"
#include "Collision.h"



ModuleCollision::ModuleCollision()
{
	name.create("colliders");

	for (uint i = 0; i < MAX_COLLIDERS; ++i) {
		colliders[i] = nullptr;
	}
}


// Destructor
ModuleCollision::~ModuleCollision()
{}


bool ModuleCollision::PreUpdate(float dt)
{
	// Remove all colliders scheduled for deletion
	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] != nullptr && colliders[i]->to_delete == true)
		{
			delete colliders[i];
			colliders[i] = nullptr;
		}
	}

	return true;
}


// Called before render is available
bool ModuleCollision::Update(float dt)
{
	return true;
}


bool ModuleCollision::PostUpdate(float dt)
{
	DebugDraw();
	return true;
}


void ModuleCollision::DebugDraw()
{
	Uint8 alpha = 255;
	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] == nullptr)
			continue;

		switch (colliders[i]->type)
		{
		case COLLIDER_NONE: // white
			App->render->DrawQuad(colliders[i]->rect, 255, 255, 255, alpha);
			break;

		case COLLIDER_ENTITY: // blue
			App->render->DrawQuad(colliders[i]->rect, 0, 0, 255, alpha);
			break;

		}
	}
}


// Called before quitting
bool ModuleCollision::CleanUp()
{

	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] != nullptr)
		{
			delete colliders[i];
			colliders[i] = nullptr;
		}
	}

	return true;
}


Collider* ModuleCollision::AddCollider(SDL_Rect rect, COLLIDER_TYPE type, j1Module* callback)
{
	Collider* ret = nullptr;

	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] == nullptr)
		{
			ret = colliders[i] = new Collider(rect, type, callback);
			break;
		}
	}

	return ret;
}


void ModuleCollision::AddColliderEntity(Collider* collider)
{
	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] == nullptr)
		{
			colliders[i] = collider;
			break;
		}
	}
}


bool Collider::CheckCollision(const SDL_Rect& r) const
{
	return !((this->rect.x + this->rect.w < r.x || r.x + r.w < this->rect.x) || (this->rect.y + this->rect.h < r.y || r.y + r.h < this->rect.y));
}