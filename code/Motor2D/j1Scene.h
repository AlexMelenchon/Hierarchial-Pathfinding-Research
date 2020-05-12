#ifndef __j1SCENE_H__
#define __j1SCENE_H__

#include "j1Module.h"
#include <vector>

struct SDL_Texture;

class j1Scene : public j1Module
{
public:

	j1Scene();

	// Destructor
	virtual ~j1Scene();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

private:
	void Select();
	SDL_Texture* GetRandomTexture(int i);


public:

	SDL_Texture* debug_tex;
	SDL_Texture* debug_texRed;
	SDL_Texture* debug_texGreen;
	SDL_Texture* debug_texBlue;

	SDL_Texture* debug_texPink;
	SDL_Texture* debug_texYellow;
	SDL_Texture* debug_texCyan;



private:
	bool entrances = false;
	bool nodes = false;
	bool edges = false;
	bool clusters = false;

	std::vector<iPoint> absPath;

	float updatePathTimer = 0.f;

	int abstractDebugLvl = 0;

	SDL_Rect selectRect;
	iPoint clickPos;
};

#endif // __j1SCENE_H__