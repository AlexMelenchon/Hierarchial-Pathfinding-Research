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
	Edge(HierNode* dest, int distanceTo);

	HierNode* dest;
	float moveCost;

};


//The actual abstract Graph
struct HPAGraph
{
	//Graph Storage
	std::vector <std::vector<Cluster>> lvlClusters;
	std::vector <Entrance> entrances;

	//PreProcessing
	void PrepareGraph();
	void CreateGraphLvl(int lvl);
	void AddLevelToGraph(int lvl);

	void buildClusters(int lvl);
	void buildEntrances();
	void createEntrance(Cluster* c1, Cluster* c2, ADJACENT_DIR adjDir, int lvl);


	//Utility
	ADJACENT_DIR adjacents(Cluster* c1, Cluster* c2, int lvl);
	HierNode* NodeExists(iPoint pos, Cluster* lvl);
	Cluster* determineCluster(iPoint nodePos, int lvl, Cluster* firstCheck = nullptr);

	//Node Insertion
	HierNode* insertNode(iPoint pos, int lvl, bool* toDelete = nullptr);
	void deleteNode(HierNode* toDelete, int lvl);
	void ConnectNodeToBorder(HierNode* node, Cluster* c, int lvl);

};

//Basic A*---------------------------------------


//Store the Generated Paths
struct generatedPath
 {
	generatedPath(std::vector <iPoint> vector, PATH_TYPE type, int lvl);

	std::vector<iPoint> path;
	PATH_TYPE type;
	int pathLvl;
};


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

//PathNode for higher hierchy Nodes
class HierNode : public PathNode
{
public:
	HierNode(iPoint pos);
	HierNode(iPoint pos, bool tmp);
	HierNode(float g, const iPoint& pos, PathNode* parent,int myDir, int parentdir, std::vector<Edge*> edges);

	float CalculateF(const iPoint& destination);
	uint FindWalkableAdjacents(std::vector<HierNode>& list_to_fill);

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

	//Pathfinding Algorithms
	//A*
	float SimpleAPathfinding(const iPoint& origin, const iPoint& destination);
	//HPA
	int HPAPathfinding(const HierNode& origin, const iPoint& destination, int lvl);

private:

	//HPA Preprocessing
	void HPAPreProcessing(int maxLevel);

	//Pathfinding FIND Utilities
	std::multimap<int, PathNode>::iterator Find(iPoint point, std::multimap<int, PathNode>* map);
	std::multimap<int, HierNode>::iterator Find(iPoint point, std::multimap<int, HierNode>* map);
	int FindV(iPoint point, std::vector<PathNode>* vec);
	int FindV(iPoint point, std::vector<HierNode>* vec);

	//HPA Utilities
	bool RefineAndSmoothPath(std::vector<iPoint>* absPath, int lvl, std::vector<iPoint>* refinedPath);
	bool IsStraightPath(iPoint from, iPoint to);


public:
	uint mapWidth;
	uint mapHeight;

	//This should be private, it's in public for debug purposes
	HPAGraph absGraph;
private:

	uchar* walkabilityMap;

	std::vector<iPoint> last_path;

	std::unordered_map <Entity*, generatedPath> generatedPaths;
};

#endif // __j1PATHFINDING_H__