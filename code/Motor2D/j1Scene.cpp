#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Map.h"
#include "j1PathFinding.h"
#include "j1Scene.h"
#include "Brofiler/Brofiler/Brofiler.h"

j1Scene::j1Scene() : j1Module()
{
	name.create("scene");
}

// Destructor
j1Scene::~j1Scene()
{}

// Called before render is available
bool j1Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	return ret;
}

// Called before the first frame
bool j1Scene::Start()
{
	if(App->map->Load("iso_walk.tmx") == true)
	{
		int w, h;
		uchar* data = NULL;
		if(App->map->CreateWalkabilityMap(w, h, &data))
			App->pathfinding->SetMap(w, h, data);

		RELEASE_ARRAY(data);
	}

	debug_tex = App->tex->Load("maps/path2.png");
	debug_tex5x5 = App->tex->Load("maps/path.png");

	return true;
}

// Called each loop iteration
bool j1Scene::PreUpdate()
{

	// debug pathfing ------------------
	static iPoint origin;
	static bool origin_selected = false;

	int x, y;
	App->input->GetMousePosition(x, y);
	iPoint p = App->render->ScreenToWorld(x, y);
	p = App->map->WorldToMap(p.x, p.y);



	if(App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		if(origin_selected == true)
		{
			BROFILER_CATEGORY("HPA", Profiler::Color::DarkGreen);
			App->pathfinding->CreatePath(origin, p,1);
			origin_selected = false;
		}
		else
		{
			origin = p;
			origin_selected = true;
		}
	}

	if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
	{
		if (origin_selected == true)
		{
			BROFILER_CATEGORY("A*", Profiler::Color::Gold);
			App->pathfinding->SimpleAPathfinding(origin, p, PATH_TYPE::GENERATE_PATH);
			origin_selected = false;
		}
	}

	return true;
}

// Called each loop iteration
bool j1Scene::Update(float dt)
{
	if(App->input->GetKey(SDL_SCANCODE_L) == KEY_DOWN)
		App->LoadGame("save_game.xml");

	if(App->input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
		App->SaveGame("save_game.xml");

	if(App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
		App->render->camera.y += 750 * dt;

	if(App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
		App->render->camera.y -= 750 * dt;

	if(App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
		App->render->camera.x += 750 * dt;

	if(App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
		App->render->camera.x -= 750 * dt;

	if (App->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN)
		entrances = !entrances;

	App->map->Draw();

	int x, y;
	App->input->GetMousePosition(x, y);
	iPoint map_coordinates = App->map->WorldToMap(x - App->render->camera.x, y - App->render->camera.y);
	p2SString title("Map:%dx%d Tiles:%dx%d Tilesets:%d Tile:%d,%d",
					App->map->data.width, App->map->data.height,
					App->map->data.tile_width, App->map->data.tile_height,
					App->map->data.tilesets.count(),
					map_coordinates.x, map_coordinates.y);

	App->win->SetTitle(title.GetString());

	// Debug pathfinding ------------------------------
	App->input->GetMousePosition(x, y);
	iPoint p = App->render->ScreenToWorld(x, y);
	p = App->map->WorldToMap(p.x, p.y);
	p = App->map->MapToWorld(p.x, p.y);

	App->render->Blit(debug_tex, p.x, p.y);

	std::vector<iPoint>* path = App->pathfinding->GetLastPath();

	for (std::vector<iPoint>::iterator it = path->begin(); it != path->end(); ++it)
	{
		iPoint pos = App->map->MapToWorld(it->x, it->y);
		App->render->Blit(debug_tex, pos.x, pos.y);
	}


	std::vector<iPoint>* absPath = App->pathfinding->GetLastAbsPath();

	for (std::vector<iPoint>::iterator it = absPath->begin(); it != absPath->end(); ++it)
	{
		iPoint pos = App->map->MapToWorld(it->x, it->y);
		App->render->Blit(debug_tex, pos.x, pos.y);
	}

	if(entrances)
	for (int i = 0; i < App->pathfinding->absGraph.entrances.size(); i++)
	{
		iPoint pos = App->map->MapToWorld(App->pathfinding->absGraph.entrances[i].pos.x, App->pathfinding->absGraph.entrances[i].pos.y);
		App->render->Blit(debug_tex, pos.x, pos.y);
	}

	for (int i = 0; i < App->pathfinding->absGraph.lvlClusters[0].size(); i++)
	{
		return true;
		//iPoint pos = App->map->MapToWorld(App->pathfinding->absGraph.lvlClusters[0].at(i)->pos.x-2, App->pathfinding->absGraph.lvlClusters[0].at(i)->pos.y-2);
		//App->render->Blit(debug_tex5x5, pos.x, pos.y);

		//for (int y = 0; y < App->pathfinding->absGraph.lvlClusters[0].at(i)->clustNodes.size(); y++)
		//{
		//	iPoint pos = App->map->MapToWorld(App->pathfinding->absGraph.lvlClusters[0].at(i)->clustNodes[y]->pos.x, App->pathfinding->absGraph.lvlClusters[0].at(i)->clustNodes[y]->pos.y);
		//	App->render->Blit(debug_tex, pos.x, pos.y);
		//}

	}

	return true;
}

// Called each loop iteration
bool j1Scene::PostUpdate()
{
	bool ret = true;

	if(App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool j1Scene::CleanUp()
{
	LOG("Freeing scene");

	return true;
}
