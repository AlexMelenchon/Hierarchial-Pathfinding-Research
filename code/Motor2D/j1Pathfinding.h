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
#define ENTRACE_NODE_DIVISOR 6




//Basic A*---------------------------------------
struct PathList;


struct PathNode
{
	PathNode();
	PathNode(float g, float h, const iPoint& pos, PathNode* parent, int parentdir, int myDir, bool isdiagonal = false);
	PathNode(const PathNode& node);
	uint FindWalkableAdjacents(std::vector<PathNode>& list_to_fill);
	float Score() const;
	float CalculateF(const iPoint& destination);
	float g;
	float h;
	bool is_Diagonal;
	iPoint pos;

	PathNode* parent;

	int parentDir;
	int myDirection;

	bool operator==(PathNode* node)
	{
		if (g == node->g && h == node->h && pos == node->pos && parent == node->parent)
		{
			return true;
		}

		return false;
	}
};


class ModulePathfinding : public j1Module
{
public:

	ModulePathfinding();


	~ModulePathfinding();

	bool CleanUp();


	void SetMap(uint width, uint height, uchar* data);


	int CreatePath(const iPoint& origin, const iPoint& destination);


	 std::vector<iPoint>* GetLastPath();


	bool CheckBoundaries(const iPoint& pos) const;


	bool IsWalkable(const iPoint& pos) const;


	uchar GetTileAt(const iPoint& pos) const;

	void SavePath(std::vector<iPoint>* path);

	std::multimap<int, PathNode>::iterator Find(iPoint point, std::multimap<int, PathNode>& map);
	int FindV(iPoint point, std::vector<PathNode>& vec);

private:
	uint width;
	uint height;
	uchar* map;
	std::vector<iPoint> last_path;
};


#endif // __j1PATHFINDING_H__