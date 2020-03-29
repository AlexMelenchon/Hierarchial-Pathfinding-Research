#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Pathfinding.h"


ModulePathfinding::ModulePathfinding() : j1Module(), walkabilityMap(NULL), last_path(DEFAULT_PATH_LENGTH), width(0), height(0)
{
	name.create("pathfinding");
}

ModulePathfinding::~ModulePathfinding()
{
	RELEASE_ARRAY(walkabilityMap);
}


bool ModulePathfinding::CleanUp()
{
	LOG("Freeing pathfinding library");

	last_path.clear();
	RELEASE_ARRAY(walkabilityMap);
	return true;
}


void ModulePathfinding::SetMap(uint width, uint height, uchar* data)
{
	//Basic A*----------------------------------------
	this->width = width;
	this->height = height;

	RELEASE_ARRAY(walkabilityMap);
	walkabilityMap = new uchar[width * height];
	memcpy(walkabilityMap, data, width * height);

	//HPA*--------------------------------------------
	preprocessing(2);
}

void ModulePathfinding::preprocessing(int maxLevel)
{
	abstractMaze();
	//buildGraph();
	//for (int l = 2; l <= maxLevel; l++)
	//{
	//	addLevelToGraph(l);
	//}
}

void ModulePathfinding::abstractMaze()
{
	absGraph.nodes.clear();
	absGraph.buildClusters(1);

	//Cluster c1, c2;
	//for (uint i = 0; i < absGraph.lvlClusters.size(); i++)
	//{
	//	c1 = absGraph.lvlClusters[i];

	//	for (uint k = i + 1; k < absGraph.lvlClusters.size(); ++k)
	//	{
	//		c2 = absGraph.lvlClusters[k];

	//		if (adjacent(c1, c2))
	//			E = E ∪ buildEntrances(c1, c2);
	//	}
	//}
}

void graphLevel::buildClusters(int lvl)
{
	std::vector <Cluster> clusterVector;
	int width = App->pathfinding->width;
	int height = App->pathfinding->height;

	Cluster c;

	for (int i = 0; i < width; i += CLUSTER_SIZE_LVL)
	{
		if (i + CLUSTER_SIZE_LVL > width)
			c.width = width - (i);
		else
			c.width = CLUSTER_SIZE_LVL;

		for (int k = 0; k < height; k += CLUSTER_SIZE_LVL)
		{
			if (k + CLUSTER_SIZE_LVL > height)
				c.height = height - (k);
			else
				c.height = CLUSTER_SIZE_LVL;

			c.pos = { i,k };
			clusterVector.push_back(Cluster(c));
		}
	}

	this->lvlClusters.push_back(clusterVector);

}

Cluster::Cluster() : pos{ -1,-1 }, width(-1), height(-1)
{}

Cluster::Cluster(int width, int height, iPoint& pos) :

	width(width), height(height), pos(pos)
{}

Cluster::Cluster(const Cluster& clust) :

	width(clust.width), height(clust.height), pos(clust.pos)
{}


//---------------------------------------------------

bool ModulePathfinding::CheckBoundaries(const iPoint& pos) const
{
	return (pos.x >= 0 && pos.x <= (int)width &&
		pos.y >= 0 && pos.y <= (int)height);
}


bool ModulePathfinding::IsWalkable(const iPoint& pos) const
{
	uchar t = GetTileAt(pos);
	return t != INVALID_WALK_CODE && t > 0;
}


uchar ModulePathfinding::GetTileAt(const iPoint& pos) const
{
	if (CheckBoundaries(pos))
		return walkabilityMap[(pos.y * width) + pos.x];

	return INVALID_WALK_CODE;
}


std::vector <iPoint>* ModulePathfinding::GetLastPath()
{
	return &last_path;
}


PathNode::PathNode() : g(-1), h(-1), pos{ -1, -1 }, parent(NULL), is_Diagonal(false)
{}

PathNode::PathNode(float g, float h, const iPoint& pos, PathNode* parent, int parentDir, int myDir, bool isdiagonal) :

	g(g), h(h), pos(pos), parent(parent), parentDir(parentDir), myDirection(myDir), is_Diagonal(isdiagonal)
{}

PathNode::PathNode(const PathNode& node) :

	g(node.g), h(node.h), pos(node.pos), parent(node.parent), parentDir(node.parentDir), myDirection(node.myDirection), is_Diagonal(node.is_Diagonal)
{}

uint PathNode::FindWalkableAdjacents(std::vector<PathNode>& list_to_fill)
{
	iPoint cell;

	// north
	cell.create(pos.x, pos.y + 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1, -1, cell, this, myDirection, 0, false));

	// south
	cell.create(pos.x, pos.y - 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1, -1, cell, this, myDirection, 0, false));

	// east
	cell.create(pos.x + 1, pos.y);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1, -1, cell, this, myDirection, 0, false));

	// west
	cell.create(pos.x - 1, pos.y);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1, -1, cell, this, myDirection, 0, false));

	cell.create(pos.x + 1, pos.y + 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1, -1, cell, this, myDirection, 0, true));

	// south
	cell.create(pos.x + 1, pos.y - 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1, -1, cell, this, myDirection, 0, true));

	// east
	cell.create(pos.x - 1, pos.y + 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1, -1, cell, this, myDirection, 0, true));

	// west
	cell.create(pos.x - 1, pos.y - 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1, -1, cell, this, myDirection, 0, true));

	return list_to_fill.size();
}


float PathNode::Score() const
{
	return g + h;
}


float PathNode::CalculateF(const iPoint& destination)
{

	if (is_Diagonal)
	{

		g = parent->g + 1.41;
	}
	else
	{
		g = parent->g + 1;
	}

	h = pos.DistanceTo(destination);

	return g + h;
}


int ModulePathfinding::CreatePath(const iPoint& origin, const iPoint& destination)
{
	if (IsWalkable(origin) == false || IsWalkable(destination) == false) {
		return -1;
	}

	last_path.clear();

	std::multimap<int, PathNode> open;

	std::vector<PathNode> closed;
	open.insert(std::pair<int, PathNode>(0, PathNode(0, origin.DistanceTo(destination), origin, nullptr, 0, 0)));
	int i = 0;
	while (open.empty() == false)
	{
		std::multimap<int, PathNode>::iterator lowest = open.begin();
		closed.push_back(lowest->second);
		std::vector<PathNode>::pointer node = &closed.back();

		node->myDirection = closed.size() - 1;
		open.erase(lowest);


		if (node->pos == destination)
		{
			int dir;
			for (node; node->pos != origin; node = &closed[dir])
			{
				last_path.push_back(node->pos);
				dir = node->parentDir;
			}
			last_path.push_back(origin);

			std::reverse(last_path.begin(), last_path.end());

			return 0;
		}

		std::vector<PathNode> adjList;
		uint limit = node->FindWalkableAdjacents(adjList);

		std::multimap<int, PathNode>::iterator it2;

		for (uint i = 0; i < limit; i++)
		{
			if (FindV(adjList[i].pos, closed) == closed.size())
			{
				it2 = Find(adjList[i].pos, open);
				adjList[i].CalculateF(destination);
				if (it2 == open.end())
				{
					open.insert(std::pair<int, PathNode>(adjList[i].Score(), adjList[i]));
				}
				else
				{
					if (adjList[i].g < it2->second.g)
					{
						open.erase(Find(adjList[i].pos, open));
						open.insert(std::pair<int, PathNode>(adjList[i].Score(), adjList[i]));
					}
				}
			}
		}

	}
}

void ModulePathfinding::SavePath(std::vector <iPoint>* path)
{
	const std::vector <iPoint>* last_path = App->pathfinding->GetLastPath();
	path->clear();

	for (uint i = 0; i < last_path->size(); ++i)
	{
		path->push_back({ (last_path->begin() + i)->x, (last_path->begin() + i)->y });
	}

}


std::multimap<int, PathNode>::iterator ModulePathfinding::Find(iPoint point, std::multimap<int, PathNode>& map)
{
	std::multimap<int, PathNode>::iterator iterator = map.begin();

	for (iterator; iterator != map.end(); iterator++)
	{
		if (iterator->second.pos == point)
		{
			return iterator;
		}
	}

	return map.end();
}


int ModulePathfinding::FindV(iPoint point, std::vector<PathNode>& vec)
{
	int numElements = vec.size();

	for (int i = 0; i < numElements; i++)
	{
		if (vec[i].pos == point)
		{
			return i;
		}
	}

	return vec.size();
}