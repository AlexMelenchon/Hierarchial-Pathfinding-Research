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

//HPA*-------------------------------------------
#define NODE_MIN_DISTANCE 5
#define CLUSTER_SIZE_LVL 5
#define MAX_LEVELS 1

class HierNode;
class Entity;

//This is always relative to c1
enum class ADJACENT_DIR
{
	DIR_NONE = -1,

	VERTICAL,
	LATERAL
};

enum class EDGE_TYPE
{
	TP_UNKNOWN = -1,

	TP_INTRA,
	TP_INTER

};

struct Cluster
{
	Cluster();
	Cluster(int width, int height, iPoint& pos);
	Cluster(const Cluster& clust);

	iPoint pos;
	int width, height;

	std::vector <HierNode*> clustNodes;
};

struct Entrance
{
	Entrance(iPoint pos, int width, int height, ADJACENT_DIR dir, Cluster* from, Cluster* to);
	Entrance();

	iPoint pos;
	int width, height;

	ADJACENT_DIR dir;

	Cluster* from;
	Cluster* to;
};

struct Edge
{
	Edge(HierNode* dest, int distanceTo, EDGE_TYPE type);

	HierNode* dest;
	float moveCost;

	EDGE_TYPE type;
};

struct graphLevel
{
	std::vector <std::vector<Cluster>> lvlClusters;

	std::vector <Entrance> entrances;

	void buildClusters(int lvl);
	void buildEntrances(int lvl);

	ADJACENT_DIR adjacents(Cluster* c1, Cluster* c2, int lvl);
	void createEntrance(Cluster* c1, Cluster* c2, ADJACENT_DIR adjDir, int lvl);

	HierNode* insertNode(iPoint pos, int maxLvl, bool tmp);
	void deleteNode(HierNode* toDelete, int maxLvl);
	Cluster* determineCluster(iPoint nodePos, int lvl, Cluster* firstCheck = nullptr);
	void ConnectNodeToBorder(HierNode* node, Cluster* c, int lvl);

	HierNode* NodeExists(iPoint pos, Cluster* lvl);
};

//Basic A*---------------------------------------

enum class REQUEST_TYPE
{
	NO_TYPE = -1,

	GENERATE_PATH,
	CALCULATE_COST

};

enum class PATH_TYPE
{
	NO_TYPE = -1,

	ABSTRACT,
	SIMPLE

};

struct generatedPath
 {
	generatedPath(std::vector <iPoint> vector, PATH_TYPE type, int lvl);

	std::vector<iPoint> path;
	PATH_TYPE type;
	int lvl;
};


class PathNode
{
public:
	PathNode();
	PathNode(float g, float h, const iPoint& pos, PathNode* parent, int parentdir, int myDir, bool isdiagonal = false);
	PathNode(const PathNode& node);
	virtual uint FindWalkableAdjacents(std::vector<PathNode>& list_to_fill);
	float Score() const;
	virtual float CalculateF(const iPoint& destination);

	float g;
	float h;
	iPoint pos;

	PathNode* parent;

	int parentDir;
	int myDirection;
	bool is_Diagonal;

};

class HierNode : public PathNode
{
public:
	HierNode(iPoint pos);
	HierNode(iPoint pos, bool tmp);
	HierNode(float g, const iPoint& pos, PathNode* parent,int myDir, int parentdir, std::vector<Edge*> edges);

	float CalculateF(const iPoint& destination);
	std::vector <Edge*> edges;
	uint FindWalkableAdjacents(std::vector<HierNode>& list_to_fill);

	bool tmp;
};


class ModulePathfinding : public j1Module
{
public:

	ModulePathfinding();


	~ModulePathfinding();

	bool CleanUp();


	void SetMap(uint width, uint height, uchar* data);


	float SimpleAPathfinding(const iPoint& origin, const iPoint& destination, REQUEST_TYPE type);

	int HPAPathfinding(const HierNode& origin, const iPoint& destination, int lvl);

	PATH_TYPE CreatePath(const iPoint& origin, const iPoint& destination, int maxLvl, Entity* pathRequest);

	bool CheckBoundaries(const iPoint& pos) const;

	bool IsWalkable(const iPoint& pos) const;

	uchar GetTileAt(const iPoint& pos) const;

	bool RequestPath(Entity* request, std::vector <iPoint>* path);

	std::multimap<int, PathNode>::iterator Find(iPoint point, std::multimap<int, PathNode>* map);
	std::multimap<int, HierNode>::iterator Find(iPoint point, std::multimap<int, HierNode>* map);
	int FindV(iPoint point, std::vector<PathNode>* vec);
	int FindV(iPoint point, std::vector<HierNode>* vec);

	std::vector<iPoint>* RefineAndSmoothPath(std::vector<iPoint>* path, int lvl);

public:
	//HPA*---------------------------------------------
	void preProcessing(int maxLevel);
	void buildGraph();
	void abstractMaze();

	uint width;
	uint height;

	graphLevel absGraph;

private:

	uchar* walkabilityMap;
	std::vector<iPoint> last_path;

	std::unordered_map <Entity*, generatedPath> generatedPaths;
};

#endif // __j1PATHFINDING_H__