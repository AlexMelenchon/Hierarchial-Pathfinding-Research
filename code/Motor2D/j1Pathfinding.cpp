﻿#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Pathfinding.h"
#include "Brofiler/Brofiler/Brofiler.h"


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

	//A*--------------------------------------
	last_path.clear();
	RELEASE_ARRAY(walkabilityMap);

	//HPA-----------------------------------------
	absGraph.entrances.clear();

	for (int i = 0; i < absGraph.lvlClusters.size(); i++)
	{

		absGraph.lvlClusters[i].clear();

	}
	absGraph.lvlClusters.clear();

	generatedPaths.clear();
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
	preProcessing(MAX_LEVELS);

}

void ModulePathfinding::preProcessing(int maxLevel)
{
	abstractMaze();
	buildGraph();
	//for (int l = 2; l <= maxLevel; l++)
	//{
	//	addLevelToGraph(l);
	//}
}

void ModulePathfinding::abstractMaze()
{
	absGraph.entrances.clear();

	absGraph.buildClusters(1);
	absGraph.buildEntrances(0);
}

void ModulePathfinding::buildGraph()
{
	Entrance* currEntrance;
	Cluster* c1, * c2;
	HierNode* n1, * n2;

	for (uint i = 0; i < absGraph.entrances.size(); i++)
	{
		currEntrance = &absGraph.entrances[i];
		c1 = currEntrance->from;
		c2 = currEntrance->to;

		switch (currEntrance->dir)
		{
		case ADJACENT_DIR::LATERAL:
		{
			for (int i = currEntrance->pos.y; i < (currEntrance->pos.y + currEntrance->height); i += NODE_MIN_DISTANCE)
			{
				n1 = absGraph.NodeExists({ currEntrance->pos.x,i }, c1);
				if (!n1)
				{
					n1 = new HierNode({ currEntrance->pos.x, i });
					c1->clustNodes.push_back(n1);
				}


				n2 = absGraph.NodeExists({ currEntrance->pos.x + 1, i }, c2);
				if (!n2)
				{
					n2 = new HierNode({ currEntrance->pos.x + 1, i });
					c2->clustNodes.push_back(n2);
				}


				n1->edges.push_back(new Edge(n2, 1, EDGE_TYPE::TP_INTER));
				n2->edges.push_back(new Edge(n1, 1, EDGE_TYPE::TP_INTER));
			}
		}
		break;
		case ADJACENT_DIR::VERTICAL:
		{
			for (int i = currEntrance->pos.x; i < (currEntrance->pos.x + currEntrance->width); i += NODE_MIN_DISTANCE)
			{

				n1 = absGraph.NodeExists({ i, currEntrance->pos.y }, c1);
				if (!n1)
				{
					n1 = new HierNode({ i, currEntrance->pos.y });
					c1->clustNodes.push_back(n1);
				}


				n2 = absGraph.NodeExists({ i, currEntrance->pos.y + 1 }, c2);
				if (!n2)
				{
					n2 = new HierNode({ i, currEntrance->pos.y + 1 });
					c2->clustNodes.push_back(n2);
				}

				n1->edges.push_back(new Edge(n2, 1, EDGE_TYPE::TP_INTER));
				n2->edges.push_back(new Edge(n1, 1, EDGE_TYPE::TP_INTER));
			}
		}
		break;
		}

	}

	Cluster* clusterIt;
	float distanceTo = 0;

	for (int i = 0; i < absGraph.lvlClusters[0].size(); i++)
	{
		clusterIt = &absGraph.lvlClusters[0].at(i);

		for (int y = 0; y < clusterIt->clustNodes.size(); y++)
		{
			for (int k = y + 1; k < clusterIt->clustNodes.size(); k++)
			{
				distanceTo = App->pathfinding->SimpleAPathfinding(clusterIt->clustNodes[y]->pos, clusterIt->clustNodes[k]->pos, REQUEST_TYPE::CALCULATE_COST);
				clusterIt->clustNodes[y]->edges.push_back(new Edge(clusterIt->clustNodes[k], distanceTo, EDGE_TYPE::TP_INTRA));
				clusterIt->clustNodes[k]->edges.push_back(new Edge(clusterIt->clustNodes[y], distanceTo, EDGE_TYPE::TP_INTRA));
			}
		}
	}
}

HierNode* graphLevel::NodeExists(iPoint pos, Cluster* c)
{
	for (int i = 0; i < c->clustNodes.size(); i++)
	{
		if (pos == c->clustNodes[i]->pos)
			return c->clustNodes[i];
	}

	return nullptr;
}

void graphLevel::buildEntrances(int lvl)
{
	Cluster* c1;
	Cluster* c2;
	ADJACENT_DIR adjacentDir = ADJACENT_DIR::DIR_NONE;

	for (uint i = 0; i < this->lvlClusters[lvl].size(); ++i)
	{
		c1 = &this->lvlClusters[lvl].at(i);

		for (uint k = i + 1; k < this->lvlClusters[lvl].size(); ++k)
		{
			c2 = &this->lvlClusters[lvl].at(k);

			adjacentDir = adjacents(c1, c2, lvl + 1);

			if (adjacentDir != ADJACENT_DIR::DIR_NONE)
				createEntrance(c1, c2, adjacentDir, lvl);
		}
	}
}

void graphLevel::createEntrance(Cluster* c1, Cluster* c2, ADJACENT_DIR adjDir, int lvl)
{
	int adjDist = CLUSTER_SIZE_LVL * lvl;

	Entrance newEntrance;
	int startedAt = INT_MIN;
	bool isCurrentWalkable = true;

	switch (adjDir)
	{
	case ADJACENT_DIR::VERTICAL:
	{
		for (int i = c1->pos.x; i < (c1->pos.x + c1->width); i++)
		{
			isCurrentWalkable = (App->pathfinding->IsWalkable({ i, c1->pos.y + c1->height - 1 }) && App->pathfinding->IsWalkable({ i, c2->pos.y }));

			if (isCurrentWalkable)
			{
				if (startedAt == INT_MIN)
					startedAt = i;

				if (i == (c1->pos.x + c1->width - 1))
					this->entrances.push_back(Entrance({ startedAt, c1->pos.y + c1->height - 1 }, c1->width - startedAt + c1->pos.x, 2, adjDir, c1, c2));

			}
			else if (!isCurrentWalkable)
			{
				if (startedAt != INT_MIN)
				{
					this->entrances.push_back(Entrance({ startedAt, c1->pos.y + c1->height - 1 }, i - startedAt, 2, adjDir, c1, c2));
					startedAt = INT_MIN;
				}

			}

		}
	}
	break;

	case ADJACENT_DIR::LATERAL:
	{
		for (int i = c1->pos.y; i < (c1->pos.y + c1->height); i++)
		{
			isCurrentWalkable = (App->pathfinding->IsWalkable({ c1->pos.x + c1->width - 1,i }) && App->pathfinding->IsWalkable({ c2->pos.x, i }));

			if (isCurrentWalkable)
			{
				if (startedAt == INT_MIN)
					startedAt = i;

				if (i == (c1->pos.y + c1->height - 1))
					this->entrances.push_back(Entrance({ c1->pos.x + c1->width - 1,  startedAt }, 2, c1->height - startedAt + c1->pos.y, adjDir, c1, c2));
			}
			else if (!isCurrentWalkable)
			{
				if (startedAt != INT_MIN)
				{
					this->entrances.push_back(Entrance({ c1->pos.x + c1->width - 1, startedAt }, 2, i - startedAt, adjDir, c1, c2));
					startedAt = INT_MIN;
				}


			}

		}
	}
	break;

	}


}

ADJACENT_DIR graphLevel::adjacents(Cluster* c1, Cluster* c2, int lvl)
{
	int adjDist = CLUSTER_SIZE_LVL * lvl;

	if (c1->pos.x + adjDist == c2->pos.x && c1->pos.y == c2->pos.y)
		return ADJACENT_DIR::LATERAL;

	else if (c1->pos.y + adjDist == c2->pos.y && c1->pos.x == c2->pos.x)
		return ADJACENT_DIR::VERTICAL;



	return ADJACENT_DIR::DIR_NONE;
}


void graphLevel::buildClusters(int lvl)
{
	int clustSize = CLUSTER_SIZE_LVL * lvl;

	std::vector <Cluster> clusterVector;
	int width = App->pathfinding->width;
	int height = App->pathfinding->height;

	Cluster c;

	for (int i = 0; i < width; i += clustSize)
	{
		if (i + clustSize > width)
			c.width = width - (i);
		else
			c.width = clustSize;

		for (int k = 0; k < height; k += clustSize)
		{
			if (k + clustSize > height)
				c.height = height - (k);
			else
				c.height = clustSize;

			c.pos = { i,k };
			clusterVector.push_back(Cluster(c));
		}
	}

	this->lvlClusters.push_back(clusterVector);
}

HierNode* graphLevel::insertNode(iPoint pos, int maxLvl, bool tmp)
{
	BROFILER_CATEGORY("Insert Node", Profiler::Color::Cornsilk);

	HierNode* newNode = nullptr;
	Cluster* c;

	if (!App->pathfinding->IsWalkable(pos))
		return nullptr;

	for (int l = 1; l <= maxLvl; l++)
	{
		if (l > this->lvlClusters.size())
			break;

		c = determineCluster(pos, l);
		if (!c)
			return nullptr;

		newNode = NodeExists(pos, c);

		if (!newNode)
		{
			newNode = new HierNode(pos, tmp);
			c->clustNodes.push_back(newNode);
			ConnectNodeToBorder(newNode, c, l);
		}
	}

	return newNode;
}

void graphLevel::ConnectNodeToBorder(HierNode* node, Cluster* c, int lvl)
{
	float distanceTo = 0;
	for (int i = 0; i < c->clustNodes.size(); i++)
	{
		//distanceTo = App->pathfinding->SimpleAPathfinding(node->pos, c->clustNodes[i]->pos, PATH_TYPE::CALCULATE_COST);
		distanceTo = node->pos.DistanceTo(c->clustNodes[i]->pos);

		node->edges.push_back(new Edge(c->clustNodes[i], distanceTo, EDGE_TYPE::TP_INTRA));
		c->clustNodes[i]->edges.push_back(new Edge(node, distanceTo, EDGE_TYPE::TP_INTRA));
	}

}

Cluster* graphLevel::determineCluster(iPoint pos, int lvl, Cluster* firstCheck)
{
	BROFILER_CATEGORY("Determine Cluster", Profiler::Color::DeepPink);

	if (firstCheck)
	{
		if (pos.x >= firstCheck->pos.x && pos.y >= firstCheck->pos.y &&
			pos.x < firstCheck->pos.x + firstCheck->width && pos.y < firstCheck->pos.y + firstCheck->height)
			return firstCheck;
	}

	Cluster* it;
	for (int i = 0; i < lvlClusters[lvl - 1].size(); i++)
	{
		it = &lvlClusters[lvl - 1].at(i);

		if (pos.x >= it->pos.x && pos.y >= it->pos.y &&
			pos.x < it->pos.x + it->width && pos.y < it->pos.y + it->height)
			return it;
	}

	return nullptr;
}

void graphLevel::deleteNode(HierNode* toDelete, int maxLvl)
{
	BROFILER_CATEGORY("Delete Node", Profiler::Color::DarkViolet);

	Cluster* c = nullptr;

	for (int l = 1; l <= maxLvl; l++)
	{
		if (l > this->lvlClusters.size())
			break;

		c = determineCluster(toDelete->pos, l);

		for (int i = 0; i < c->clustNodes.size(); i++)
		{
			if (c->clustNodes[i]->pos == toDelete->pos)
			{
				toDelete->edges.clear();
				c->clustNodes.erase(c->clustNodes.begin() + i);
				break;
			}
		}
	}
}

Edge::Edge(HierNode* dest, int distanceTo, EDGE_TYPE type) : dest(dest), moveCost(distanceTo), type(type)
{}

Entrance::Entrance() : pos{ 0,0 }, width(0), height(0), dir(ADJACENT_DIR::DIR_NONE), from(nullptr), to(nullptr)
{}

Entrance::Entrance(iPoint pos, int width, int height, ADJACENT_DIR dir, Cluster* from, Cluster* to)
	: pos(pos), width(width), height(height), dir(dir), from(from), to(to)
{}

HierNode::HierNode(iPoint pos) : PathNode(-1, -1, pos, nullptr, -1, -1, false), tmp(false)
{}

HierNode::HierNode(iPoint pos, bool tmp) : PathNode(g, h, pos, nullptr, -1, -1, false), tmp(tmp)
{}

HierNode::HierNode(float g, const iPoint& pos, PathNode* parent, int parentdir, int myDir, std::vector<Edge*> edges) :
	PathNode(g, -1, pos, parent, parentdir, myDir, false), tmp(tmp), edges(edges)
{}

Cluster::Cluster() : pos{ -1,-1 }, width(-1), height(-1)
{}

Cluster::Cluster(int width, int height, iPoint& pos) :

	width(width), height(height), pos(pos)
{}

Cluster::Cluster(const Cluster& clust) :

	width(clust.width), height(clust.height), pos(clust.pos)
{}

generatedPath::generatedPath(std::vector <iPoint> vector, PATH_TYPE type, int lvl) : path(vector), type(type), lvl(lvl)
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


PathNode::PathNode() : g(-1), h(-1), pos{ -1, -1 }, parent(NULL), is_Diagonal(false), myDirection(0), parentDir(0)
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

uint HierNode::FindWalkableAdjacents(std::vector<HierNode>& list_to_fill)
{
	int edgeNum = edges.size();
	HierNode* curr = nullptr;

	for (int i = 0; i < edgeNum; i++)
	{
		curr = edges[i]->dest;
		list_to_fill.push_back(HierNode(edges[i]->moveCost + this->g, curr->pos, this, myDirection, curr->parentDir, curr->edges));
	}

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

float HierNode::CalculateF(const iPoint& destination)
{
	h = pos.DistanceTo(destination);

	return g + h;
}

PATH_TYPE ModulePathfinding::CreatePath(const iPoint& origin, const iPoint& destination, int maxLvl, Entity* pathRequest)
{
	PATH_TYPE ret = PATH_TYPE::NO_TYPE;
	HierNode* n1, * n2;

	if (IsWalkable(origin) == false || IsWalkable(destination) == false)
	{
		return ret;
	}

	last_path.clear();

	if (maxLvl > 0)
	{
		n1 = absGraph.insertNode(origin, maxLvl, true);
		n2 = absGraph.insertNode(destination, maxLvl, true);

		if (!n1 || !n2)
			return ret;

		n1->h = n1->pos.DistanceTo(n2->pos);

		//Abs Path
		HPAPathfinding(*n1, n2->pos, 1);

		if (n1->tmp)
			absGraph.deleteNode((HierNode*)n1, maxLvl);
		if (n2->tmp)
			absGraph.deleteNode((HierNode*)n2, maxLvl);

		ret = PATH_TYPE::ABSTRACT;

	}
	else
	{
		SimpleAPathfinding(origin, destination, REQUEST_TYPE::GENERATE_PATH);

		ret = PATH_TYPE::SIMPLE;
	}


	generatedPaths.insert({ pathRequest, generatedPath(last_path, ret, maxLvl) });

	return ret;
}


int ModulePathfinding::HPAPathfinding(const HierNode& origin, const iPoint& destination, int lvl)
{
	BROFILER_CATEGORY("HPA Algorithm", Profiler::Color::AliceBlue);

	std::multimap<int, HierNode> open;

	std::vector<HierNode> closed;
	open.insert(std::pair<int, HierNode>(0, origin));

	HierNode* curr = nullptr;
	std::multimap<int, HierNode>::iterator lowest;
	std::vector<HierNode> adjList;
	uint limit = 0;
	std::multimap<int, HierNode>::iterator it2;

	while (open.empty() == false)
	{
		lowest = open.begin();
		closed.push_back(lowest->second);
		curr = &closed.back();

		curr->myDirection = (closed.size() - 1);
		open.erase(lowest);


		if (curr->pos == destination)
		{
			int dir;
			for (curr; curr->pos != origin.pos; curr = &closed[dir])
			{
				last_path.push_back(curr->pos);
				dir = curr->parentDir;
			}
			last_path.push_back(origin.pos);

			std::reverse(last_path.begin(), last_path.end());

			return 0;
		}

		limit = curr->FindWalkableAdjacents(adjList);

		for (uint i = 0; i < limit; i++)
		{
			if (FindV(adjList[i].pos, &closed) == closed.size())
			{
				it2 = Find(adjList[i].pos, &open);
				adjList[i].CalculateF(destination);
				if (it2 == open.end())
				{
					open.insert(std::pair<int, HierNode>(adjList[i].Score(), adjList[i]));
				}
				else
				{
					if (adjList[i].Score() < it2->second.Score())
					{
						open.erase(Find(adjList[i].pos, &open));
						open.insert(std::pair<int, HierNode>(adjList[i].Score(), adjList[i]));
					}
				}
			}
		}
		adjList.clear();
	}
}


float ModulePathfinding::SimpleAPathfinding(const iPoint& origin, const iPoint& destination, REQUEST_TYPE type)
{
	BROFILER_CATEGORY("A* Algorithm", Profiler::Color::AliceBlue);

	last_path.clear();

	std::multimap<int, PathNode> open;

	std::vector<PathNode> closed;
	open.insert(std::pair<int, PathNode>(0, PathNode(0, origin.DistanceTo(destination), origin, nullptr, 0, 0)));

	//Analize the current
	PathNode* curr = nullptr;
	std::multimap<int, PathNode>::iterator lowest;

	// Get New Nodes
	std::vector<PathNode> adjList;
	std::multimap<int, PathNode>::iterator it2;
	uint limit = 0;

	while (open.empty() == false)
	{
		lowest = open.begin();
		closed.push_back(lowest->second);
		curr = &closed.back();

		curr->myDirection = (closed.size() - 1);
		open.erase(lowest);

		if (curr->pos == destination)
		{
			switch (type)
			{
			case REQUEST_TYPE::GENERATE_PATH:
			{
				int dir;
				for (curr; curr->pos != origin; curr = &closed[dir])
				{
					last_path.push_back(curr->pos);
					dir = curr->parentDir;
				}
				last_path.push_back(origin);

				std::reverse(last_path.begin(), last_path.end());

				return closed.back().g;
			}
			break;
			case REQUEST_TYPE::CALCULATE_COST:
			{
				return curr->g;
			}
			break;
			}
		}

		limit = curr->FindWalkableAdjacents(adjList);

		for (uint i = 0; i < limit; i++)
		{
			if (FindV(adjList[i].pos, &closed) == closed.size())
			{
				it2 = Find(adjList[i].pos, &open);
				adjList[i].CalculateF(destination);
				if (it2 == open.end())
				{
					open.insert(std::pair<int, PathNode>(adjList[i].Score(), adjList[i]));
				}
				else
				{
					if (adjList[i].g < it2->second.g)
					{
						open.erase(Find(adjList[i].pos, &open));
						open.insert(std::pair<int, PathNode>(adjList[i].Score(), adjList[i]));
					}
				}
			}
		}
		adjList.clear();
	}
}

bool ModulePathfinding::RequestPath(Entity* request, std::vector <iPoint>* path)
{
	if (generatedPaths.size() < 1)
		return false;

	std::unordered_map<Entity*, generatedPath>::iterator it = generatedPaths.begin();

	for (int i = 0; i < generatedPaths.size(); i++)
	{
		if (it->first == request)
		{
			switch (it->second.type)
			{
			case PATH_TYPE::SIMPLE:
			{
				return &it->second.path;
			}
			break;
			case PATH_TYPE::ABSTRACT:
			{
				std::vector <iPoint>* refinedPath;

				refinedPath =  RefineAndSmoothPath(&it->second.path, it->second.lvl);

				if (refinedPath)
				{
					for (int i = 0; i < refinedPath->size(); i++)
					{
						path->push_back(refinedPath->at(i));
					}
					return true;
				}
			}
			break;
			}
		}
		it++;
	}

	return false;
}

std::vector<iPoint>* ModulePathfinding::RefineAndSmoothPath(std::vector<iPoint>* path, int lvl)
{
	std::vector <iPoint>* ret = nullptr;
	Cluster* curr = nullptr;
	iPoint currPos = {0,0};
	int from = 0;

	for (int i = 0; i < path->size(); i++)
	{
		currPos = path->at(i);
		if (!curr)
		{
			curr = absGraph.determineCluster(currPos, lvl);
			from = i;
			continue;
		}

		if (absGraph.determineCluster(currPos, lvl, curr) != curr || (i == path->size() - 1 && path->size() > 0))
		{
			SimpleAPathfinding(path->at(from), currPos, REQUEST_TYPE::GENERATE_PATH);
			ret = &last_path;
			path->erase(path->begin() + from, path->begin() + i);
			break;

			//The last one repeats itself

			//Check Diagonals abs(x2 - x) == abs(y2 - y)
			//Check Directions
		}

	}

	return ret;
}


std::multimap<int, PathNode>::iterator ModulePathfinding::Find(iPoint point, std::multimap<int, PathNode>* map)
{
	std::multimap<int, PathNode>::iterator iterator = map->begin();

	for (iterator; iterator != map->end(); iterator++)
	{
		if (iterator->second.pos == point)
		{
			return iterator;
		}
	}

	return map->end();
}

std::multimap<int, HierNode>::iterator ModulePathfinding::Find(iPoint point, std::multimap<int, HierNode>* map)
{
	std::multimap<int, HierNode>::iterator iterator = map->begin();

	for (iterator; iterator != map->end(); iterator++)
	{
		if (iterator->second.pos == point)
		{
			return iterator;
		}
	}

	return map->end();
}



int ModulePathfinding::FindV(iPoint point, std::vector<PathNode>* vec)
{
	int numElements = vec->size();

	for (int i = 0; i < numElements; i++)
	{
		if (vec->at(i).pos == point)
		{
			return i;
		}
	}

	return vec->size();
}

int ModulePathfinding::FindV(iPoint point, std::vector<HierNode>* vec)
{
	int numElements = vec->size();

	for (int i = 0; i < numElements; i++)
	{
		if (vec->at(i).pos == point)
		{
			return i;
		}
	}

	return vec->size();
}