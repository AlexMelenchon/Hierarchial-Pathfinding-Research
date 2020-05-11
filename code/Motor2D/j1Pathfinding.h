#ifndef __j1PATHFINDING_H__
#define __j1PATHFINDING_H__

#include "j1Module.h"
#include "p2Point.h"

#define DEFAULT_PATH_LENGTH 50
#define INVALID_WALK_CODE 255

#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <unordered_map>

enum class PATH_TYPE
{
	NO_TYPE = -1,

	// Using HPA
	ABSTRACT,

	//SIMPLE A*
	SIMPLE

};

enum class PATH_DIR
{
	DIR_NONE = -1,

	DIR_HOR_POS,
	DIR_HOR_NEG,

	DIR_VER_POS,
	DIR_VER_NEG,

	DIR_DIA_XY_POS,
	DIR_DIA_XY_NEG,
	DIR_DIA_X_NEG_Y_POS,
	DIR_DIA_Y_NEG_X_POS,
};


//Store the Generated Paths
struct generatedPath
{
	generatedPath(std::vector <iPoint> vector, PATH_TYPE type, int lvl);


	std::vector<iPoint> path;
	PATH_TYPE type;
	int pathLvl;
};

//HPA*-------------------------------------------
#define NODE_MIN_DISTANCE 5
#define CLUSTER_SIZE_LVL 5
#define MAX_LEVELS 2

class HierNode;
class Entity;

// Used to determine the cluster adjacent direction
//This is always relative to c1, being (c1.x + c1.width == c2.x || c1.y + c1.height == c2.y)
enum class ADJACENT_DIR
{
	DIR_NONE = -1,

	VERTICAL,
	LATERAL
};

//Enum to indicate the Node connection type
enum class EDGE_TYPE
{
	NO_YPE = -1,

	INTRA, //Connections between same cluster nodes
	INTER //Connection between other cluster nodes 

};


//The abstraction of various groups of nodes
//Stores pointers to ITS Nodes
struct Cluster
{
	Cluster();
	Cluster(int width, int height, iPoint& pos);
	Cluster(const Cluster& clust);

	iPoint pos;
	int width, height;

	std::vector <HierNode*> clustNodes;
};


//The possible spaces where a HierNode might be
struct Entrance
{
	Entrance(iPoint pos, int width, int height, ADJACENT_DIR dir);
	Entrance();

	iPoint pos;
	int width, height;

	ADJACENT_DIR dir;

};


//Connection between nodes
struct Edge
{
	Edge(HierNode* dest, float distanceTo, int lvl, EDGE_TYPE type);

	void UpdateLvl(int lvl)
	{
		this->lvl = lvl;
	}

	HierNode* dest;
	float moveCost;

	int lvl;
	EDGE_TYPE type;

};


//The actual abstract Graph
struct HPAGraph
{
	//Graph Storage
	std::vector <std::vector<Cluster>> lvlClusters;
	std::vector <Entrance> entrances;
	std::vector <HierNode*> nodes;

	//PreProcessing
	void PrepareGraph();
	void CreateGraphLvl(int lvl);

	//Building Inner Structures: 
		//Build is for logic determination & creation of all the elements of the structure
		//Create is for the creation of a single object
	void buildClusters(int lvl);

	void buildEntrances();
	void createEntrance(Cluster* c1, Cluster* c2, ADJACENT_DIR adjDir, int lvl);

	void BuildInterNode(iPoint n1, Cluster* c1, iPoint n2, Cluster* c2, int lvl);
	void createInterNodes(int lvl);

	void createIntraNodes(int lvl);

	void CreateEdges(HierNode* from, HierNode* to, int lvl, EDGE_TYPE type);


	//Utility
	ADJACENT_DIR ClustersAreAdjacent(Cluster* c1, Cluster* c2, int lvl);
	HierNode* NodeExists(iPoint pos, Cluster* lvl);
	bool EdgeExists(HierNode* from, HierNode* to, int lvl, EDGE_TYPE type);
	Cluster* DetermineCluster(iPoint nodePos, int lvl, Cluster* firstCheck = nullptr);

	//Node Insertion
	HierNode* insertNode(iPoint pos, int lvl, bool* toDelete = nullptr);
	void deleteNode(HierNode* toDelete, int lvl);
	void ConnectNodeToBorder(HierNode* node, Cluster* c, int lvl);

};

//Basic A*---------------------------------------

class PathNode
{
public:
	PathNode();
	PathNode(float g, float h, const iPoint& pos, PathNode* parent, int parentdir, int myDir, bool isdiagonal = false);
	PathNode(const PathNode& node);

	virtual uint FindWalkableAdjacents(std::vector<PathNode>& list_to_fill);
	virtual float CalculateF(const iPoint& destination);
	float Score() const;

	float g;
	float h;
	iPoint pos;

	PathNode* parent;

	int parentDir;
	int myDirection;
	bool is_Diagonal;

};

//HPA* Nodes
class HierNode : public PathNode
{
public:
	HierNode(iPoint pos);
	HierNode(iPoint pos, bool tmp);
	HierNode(float g, const iPoint& pos, PathNode* parent,int myDir, int parentdir, std::vector<Edge*> edges);

	float CalculateF(const iPoint& destination);
	uint FindWalkableAdjacents(std::vector<HierNode>& list_to_fill, int lvl);

	std::vector <Edge*> edges;
};


class ModulePathfinding : public j1Module
{
public:
	//Constr / Destr
	ModulePathfinding();
	~ModulePathfinding();

	bool CleanUp();

	//PreProcessing
	void SetMap(uint width, uint height, uchar* data);

	//Pathfinding
	PATH_TYPE CreatePath(const iPoint& origin, const iPoint& destination, int maxLvl, Entity* pathRequest);
	bool RequestPath(Entity* request, std::vector <iPoint>* path);


	//Utility
	bool CheckBoundaries(const iPoint& pos) const;
	bool IsWalkable(const iPoint& pos) const;
	uchar GetTileAt(const iPoint& pos) const;

	bool LineRayCast(iPoint& p0, iPoint& p1);
	std::vector<iPoint> CreateLine(const iPoint& p0, const iPoint& p1);
	std::vector<iPoint>* GetLastLine();


	//Pathfinding Algorithms
	//A*
	float SimpleAPathfinding(const iPoint& origin, const iPoint& destination);
	//HPA
	int HPAPathfinding(const HierNode& origin, const iPoint& destination, int lvl);

private:

	//HPA Graph Creation
	void HPAPreProcessing(int maxLevel);

	//Pathfinding FIND Utilities
	std::multimap<float, PathNode>::iterator Find(iPoint point, std::multimap<float, PathNode>* map);
	std::multimap<float, HierNode>::iterator Find(iPoint point, std::multimap<float, HierNode>* map);
	int FindV(iPoint point, std::vector<PathNode>* vec);
	int FindV(iPoint point, std::vector<HierNode>* vec);



	//HPA Refining & Smoothing Tools
	bool RefineAndSmoothPath(std::vector<iPoint>* absPath, int lvl, std::vector<iPoint>* refinedPath);


public:
	uint mapWidth;
	uint mapHeight;

	//This should be private, it's in public for debug purposes
	HPAGraph absGraph;

private:

	uchar* walkabilityMap;

	std::vector<iPoint> last_path;
	std::vector<iPoint> last_line;

	std::unordered_map <Entity*, generatedPath> generatedPaths;
};

#endif // __j1PATHFINDING_H__