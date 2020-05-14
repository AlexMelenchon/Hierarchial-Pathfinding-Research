#ifndef __j1APP_H__
#define __j1APP_H__

#include "p2List.h"
#include "j1Module.h"
#include "PugiXml\src\pugixml.hpp"
#include "PerfectTimer.h"
#include "SimpleTimer.h"


#define TIME_CONST 50

#define MAX_DT 3


// Modules
class j1Window;
class j1Input;
class j1Render;
class j1Textures;
class j1Scene;
class j1Map;
class ModulePathfinding;
class ModuleCollision;
class j1EntityManager;

class j1App
{
public:

	// Constructor
	j1App(int argc, char* args[]);

	// Destructor
	virtual ~j1App();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update();

	// Called before quitting
	bool CleanUp();

	// Add a new module to handle
	void AddModule(j1Module* module);

	// Exposing some properties for reading
	int GetArgc() const;
	const char* GetArgv(int index) const;
	const char* GetTitle() const;
	const char* GetOrganization() const;

	void LoadGame(const char* file);
	void SaveGame(const char* file) const;
	void GetSaveGames(p2List<p2SString>& list_to_fill) const;

private:

	// Load config file
	pugi::xml_node LoadConfig(pugi::xml_document&) const;

	// Call modules before each loop iteration
	void PrepareUpdate();

	// Call modules before each loop iteration
	void FinishUpdate();

	// Call modules before each loop iteration
	bool PreUpdate();

	// Call modules on each loop iteration
	bool DoUpdate();

	// Call modules after each loop iteration
	bool PostUpdate();

	// Load / Save
	bool LoadGameNow();
	bool SavegameNow() const;

public:

	// Modules
	j1Window*			win;
	j1Input*			input;
	j1Render*			render;
	j1Textures*			tex;
	j1Scene*			scene;
	j1Map*				map;
	ModulePathfinding*	pathfinding;
	ModuleCollision*	coll;
	j1EntityManager*	entMan;

private:

	p2List<j1Module*>	modules;
	uint				frames;
	float				dt;
	int					argc;
	char**				args;

	p2SString			title;
	p2SString			organization;


	bool				capFrames;
	bool				wantToLoad;
	mutable bool		wantToSave;


	p2SString			load_game;
	mutable p2SString	save_game;

	//Timers of the game
	SimpleTimer* gameTimer = nullptr;
	PerfectTimer* gamePerfTimer = nullptr;

	//Calculates the amount of frames rendered in the last second
	SimpleTimer* lastSecFrames = nullptr;

	//The amount of time spended in the calculation of the last frame
	SimpleTimer lastFrameTimer;

	//The amount of frames rendered since the game started
	uint64 frame_count = 0u;

	//The amount of frames rendered last second
	uint last_second_frame_count = 0u;

	//The amount of time spended calculating the last frame
	uint32 last_frame_ms = 0u;

	//The amount of frames calculated in the last second
	uint32 frames_on_last_update = 0u;

	float avg_fps = 0.0f;

	//The frame cap of the game
	uint capTime = 0u;

	//Controls if the frame cap is activated or not
	bool frameCap = false;

};

extern j1App* App; // No student is asking me about that ... odd :-S

#endif