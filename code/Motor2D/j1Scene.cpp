#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
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
	if (App->map->Load("iso_walk.tmx") == true)
	{
		int w, h;
		uchar* data = NULL;
		if (App->map->CreateWalkabilityMap(w, h, &data))
			App->pathfinding->SetMap(w, h, data);

		RELEASE_ARRAY(data);
	}

	debug_tex = App->tex->Load("maps/path2.png");

	debug_texRed = App->tex->Load("maps/path2.png");
	SDL_SetTextureColorMod(debug_texRed, 255, 0, 0);

	debug_texGreen = App->tex->Load("maps/path2.png");
	SDL_SetTextureColorMod(debug_texGreen, 0, 255, 0);

	debug_texBlue = App->tex->Load("maps/path2.png");
	SDL_SetTextureColorMod(debug_texBlue, 0, 0, 255);

	debug_texPink = App->tex->Load("maps/path2.png");
	SDL_SetTextureColorMod(debug_texPink, 255, 0, 255);

	debug_texYellow = App->tex->Load("maps/path2.png");
	SDL_SetTextureColorMod(debug_texYellow, 255, 255, 0);

	debug_texCyan = App->tex->Load("maps/path2.png");
	SDL_SetTextureColorMod(debug_texCyan, 0, 255, 255);

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



	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		if (origin_selected == true)
		{
			BROFILER_CATEGORY("HPA", Profiler::Color::DarkGreen);
			absPath.clear();
			App->pathfinding->CreatePath(origin, p, 2, nullptr);
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
			absPath.clear();
			BROFILER_CATEGORY("A*", Profiler::Color::Gold);
			App->pathfinding->CreatePath(origin, p, 0, nullptr);
			App->pathfinding->RequestPath(nullptr, &absPath);
			origin_selected = false;
		}
	}

	return true;
}

// Called each loop iteration
bool j1Scene::Update(float dt)
{
	if (App->input->GetKey(SDL_SCANCODE_L) == KEY_DOWN)
		App->LoadGame("save_game.xml");

	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
		App->SaveGame("save_game.xml");

	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
		App->render->camera.y += 750 * dt;

	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
		App->render->camera.y -= 750 * dt;

	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		App->render->camera.x += 750 * dt;

	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		App->render->camera.x -= 750 * dt;

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
	{
		if (clusters)
			clusters = false;

		entrances = !entrances;
	}

	if (App->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN)
		nodes = !nodes;

	if (App->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN)
		edges = !edges;

	if (App->input->GetKey(SDL_SCANCODE_F4) == KEY_DOWN)
	{
		if (entrances)
			entrances = false;

		clusters = !clusters;
	}

	if (App->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
	{
		App->map->nonWalkableDraw = !App->map->nonWalkableDraw;
	}

	

	if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_DOWN)
	{
		abstractDebugLvl++;

		if (abstractDebugLvl >= MAX_LEVELS)
			abstractDebugLvl--;
	}

	if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_DOWN)
	{
		abstractDebugLvl--;

		if (abstractDebugLvl < 0)
			abstractDebugLvl = 0;
	}


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
	fPoint p2 = App->map->MapToWorld(p.x, p.y);

	App->render->Blit(debug_tex, p2.x, p2.y);


	if (App->input->GetKey(SDL_SCANCODE_F4) == KEY_DOWN)
	{
		App->pathfinding->RequestPath(nullptr, &absPath);
		updatePathTimer = 0.f;
	}


	//Path debug draw
	if (absPath.size() > 0)
	{
		for (std::vector<iPoint>::iterator it = absPath.begin(); it != absPath.end(); ++it)
		{
			fPoint pos = App->map->MapToWorld(it->x, it->y);
			App->render->Blit(debug_tex, pos.x, pos.y);
		}
	}

	//Entrances debug draw
	if (entrances)
	{

		for (int i = 0; i < App->pathfinding->absGraph.entrances.size(); i++)
		{
			Entrance* currEntrance = &App->pathfinding->absGraph.entrances[i];
			SDL_Texture* randomText = GetRandomTexture(i);

			if (nodes)
				SDL_SetTextureAlphaMod(randomText, 255 * 0.5f);


			for (int j = 0; j < currEntrance->width; j++)
			{
				for (int k = 0; k < currEntrance->height; k++)
				{
					fPoint pos = App->map->MapToWorld(currEntrance->pos.x + j, currEntrance->pos.y + k);
					App->render->Blit(randomText, pos.x, pos.y);
				}
			}

			SDL_SetTextureAlphaMod(randomText, 255);

		}
	}

	//Nodes Debug Draw
	if (nodes)
	{
		for (int i = 0; i < App->pathfinding->absGraph.lvlClusters[abstractDebugLvl].size(); i++)
		{
			SDL_Texture* randomText = GetRandomTexture(i);
			Cluster* currCluster = &App->pathfinding->absGraph.lvlClusters[abstractDebugLvl].at(i);

			for (int y = 0; y < currCluster->clustNodes.size(); y++)
			{
				HierNode* currNode = currCluster->clustNodes[y];

				fPoint pos = App->map->MapToWorld(currNode->pos.x, currNode->pos.y);
				App->render->Blit(randomText, pos.x, pos.y);

				//Edges Debug Draw
				if (edges)
				{
					for (int k = 0; k < currNode->edges.size(); k++)
					{
						Edge* currEdge = currNode->edges[k];

						if (currEdge->type == EDGE_TYPE::INTRA && currEdge->lvl != abstractDebugLvl+1 || currEdge->type == EDGE_TYPE::INTER && currEdge->lvl < abstractDebugLvl+1)
							continue;


						SDL_Color edgeColor = { 255,255,255 };
						SDL_GetTextureColorMod(randomText, &edgeColor.r, &edgeColor.g, &edgeColor.b);

						fPoint offset = { App->map->data.tile_width * 0.5f , App->map->data.tile_height * 0.5f };
						fPoint from = App->map->MapToWorld(currNode->pos.x, currNode->pos.y);
						fPoint to = App->map->MapToWorld(currEdge->dest->pos.x, currEdge->dest->pos.y);



						App->render->DrawLine(from.x + offset.x, from.y + offset.y, to.x + offset.x, to.y + offset.y, edgeColor.r, edgeColor.g, edgeColor.b);

					}
				}

			}

		}
	}


	if (clusters)
	{
		for (int i = 0; i < App->pathfinding->absGraph.lvlClusters[abstractDebugLvl].size(); i++)
		{
			Cluster* currCluster = &App->pathfinding->absGraph.lvlClusters[abstractDebugLvl].at(i);

			SDL_Texture* randomText = GetRandomTexture(i);

			if (nodes)
				SDL_SetTextureAlphaMod(randomText, 255 * 0.5f);


			for (int j = 0; j < currCluster->width; j++)
			{
				for (int k = 0; k < currCluster->height; k++)
				{
					if (!App->map->nonWalkableDraw || App->pathfinding->IsWalkable({ currCluster->pos.x + j, currCluster->pos.y + k }))
					{
						fPoint pos = App->map->MapToWorld(currCluster->pos.x + j, currCluster->pos.y + k);
						App->render->Blit(randomText, pos.x, pos.y);
					}
				}
			}

			SDL_SetTextureAlphaMod(randomText, 255);

		}
	}

	return true;
}

// Called each loop iteration
bool j1Scene::PostUpdate()
{
	bool ret = true;

	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool j1Scene::CleanUp()
{
	LOG("Freeing scene");

	return true;
}

SDL_Texture* j1Scene::GetRandomTexture(int i)
{
	SDL_Texture* ret = debug_tex;

	int random = i % 6;

	switch (random)
	{
	case 0:
		ret = debug_texRed;
		break;

	case 1:
		ret = debug_texGreen;
		break;

	case 2:
		ret = debug_texBlue;
		break;

	case 3:
		ret = debug_texYellow;
		break;

	case 4:
		ret = debug_texPink;
		break;

	case 5:
		ret = debug_texCyan;
		break;

	default:
		ret = debug_tex;
		break;
	}



	return ret;
}
