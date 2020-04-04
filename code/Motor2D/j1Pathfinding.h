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

//HPA*-------------------------------------------
#define NODE_MIN_DISTANCE 3
#define CLUSTER_SIZE_LVL 5
#define MAX_LEVELS 1


class HierNode;

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
	int distanceTo;

	EDGE_TYPE type;
};

struct graphLevel
{
	std::vector <std::vector<Cluster>> lvlClusters;
	std::vector <std::vector<HierNode>> nodes;

	std::vector <Entrance> entrances;


	void buildClusters(int lvl);
	void buildEntrances(int lvl);

	ADJACENT_DIR adjacents(Cluster* c1, Cluster* c2, int lvl);
	void createEntrance(Cluster* c1, Cluster* c2, ADJACENT_DIR adjDir, int lvl);

	void insertNode(iPoint pos, int maxLvl);
	Cluster* determineCluster(HierNode node, int lvl);
	void ConnectNodeToBorder(HierNode node, Cluster* c, int lvl);
};

//Basic A*---------------------------------------
struct PathList;

enum class PATH_TYPE
{
	NO_TYPE = -1,

	GENERATE_PATH,
	CALCULATE_COST

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

	float CalculateF(const iPoint& destination) { return 1.f; };
	std::vector <Edge> edges;
	uint FindWalkableAdjacents(std::vector<PathNode>& list_to_fill) { return 1; };
};


class ModulePathfinding : public j1Module
{
public:

	ModulePathfinding();


	~ModulePathfinding();

	bool CleanUp();


	void SetMap(uint width, uint height, uchar* data);


	int CreatePath(const iPoint& origin, const iPoint& destination, PATH_TYPE type, int lvl);


	int HierarchicalCreatePath(const iPoint& origin, const iPoint& destination, int maxLvl);


	std::vector<iPoint>* GetLastPath();


	bool CheckBoundaries(const iPoint& pos) const;


	bool IsWalkable(const iPoint& pos) const;

	uchar GetTileAt(const iPoint& pos) const;

	void SavePath(std::vector<iPoint>* path);

	std::multimap<int, PathNode>::iterator Find(iPoint point, std::multimap<int, PathNode>& map);
	int FindV(iPoint point, std::vector<PathNode>& vec);



public:
	//HPA*---------------------------------------------
	void preProcessing(int maxLevel);
	void buildGraph();
	void abstractMaze();


	uint width;
	uint height;
private:

	uchar* walkabilityMap;
	std::vector<iPoint> last_path;


	graphLevel absGraph;
};


#endif // __j1PATHFINDING_H__