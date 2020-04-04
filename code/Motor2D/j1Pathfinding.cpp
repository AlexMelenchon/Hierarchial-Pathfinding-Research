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

	//HPA-----------------------------------------
	absGraph.entrances.clear();

	for (int i = 0; i < absGraph.lvlClusters.size(); i++)
	{
		absGraph.lvlClusters[i].clear();
	}
	absGraph.lvlClusters.clear();

	for (int i = 0; i < absGraph.nodes.size(); i++)
	{
		absGraph.nodes[i].clear();
	}
	absGraph.nodes.clear();

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
	absGraph.nodes.resize(absGraph.nodes.size() + 1);

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
				n1 = new HierNode({ currEntrance->pos.x, i });
				n2 = new HierNode({ currEntrance->pos.x + 1, i });

				c1->clustNodes.push_back(n1);
				c2->clustNodes.push_back(n2);

				absGraph.nodes[0].push_back(*n1);
				absGraph.nodes[0].push_back(*n2);

				n1->edges.push_back(Edge(n2, 1, EDGE_TYPE::TP_INTER));
				n1->edges.push_back(Edge(n1, 1, EDGE_TYPE::TP_INTER));
			}
		}
		break;
		case ADJACENT_DIR::VERTICAL:
		{
			for (int i = currEntrance->pos.x; i < (currEntrance->pos.x + currEntrance->width); i += NODE_MIN_DISTANCE)
			{
				n1 = new HierNode({ i, currEntrance->pos.y });
				n2 = new HierNode({ i, currEntrance->pos.y + 1 });

				c1->clustNodes.push_back(n1);
				c2->clustNodes.push_back(n2);

				absGraph.nodes[0].push_back(*n1);
				absGraph.nodes[0].push_back(*n2);

				n1->edges.push_back(Edge(n2, 1, EDGE_TYPE::TP_INTER));
				n1->edges.push_back(Edge(n1, 1, EDGE_TYPE::TP_INTER));
			}
		}
		break;
		}

	}

	Cluster* clusterIt;
	int distanceTo = 0;

	for (int i = 0; i < absGraph.lvlClusters[0].size(); i++)
	{
		clusterIt = &absGraph.lvlClusters[0].at(i);

		for (int y = 0; y < clusterIt->clustNodes.size(); y++)
		{
			for (int k = y + 1; k < clusterIt->clustNodes.size(); k++)
			{
				distanceTo = App->pathfinding->CreatePath(clusterIt->clustNodes[y]->pos, clusterIt->clustNodes[k]->pos, PATH_TYPE::CALCULATE_COST, 0);
				clusterIt->clustNodes[y]->edges.push_back(Edge(clusterIt->clustNodes[k], distanceTo, EDGE_TYPE::TP_INTRA));
				clusterIt->clustNodes[k]->edges.push_back(Edge(clusterIt->clustNodes[y], distanceTo, EDGE_TYPE::TP_INTRA));
			}
		}
	}
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

void graphLevel::insertNode(iPoint pos, int maxLvl)
{
	HierNode newNode(pos);
	Cluster* c;

	if (!App->pathfinding->IsWalkable(pos))
		return;

	for (int l = 1; l <= maxLvl; l++)
	{
		if (l > this->lvlClusters.size())
			break;

		c = determineCluster(newNode, l);
		ConnectNodeToBorder(newNode, c, l);
	}

}

void graphLevel::ConnectNodeToBorder(HierNode node, Cluster* c, int lvl)
{
	int distanceTo = 0;
	for (int i = 0; i < c->clustNodes.size(); i++)
	{
		distanceTo = node.pos.DistanceTo(c->clustNodes[i]->pos);
		distanceTo = App->pathfinding->CreatePath(node.pos, c->clustNodes[i]->pos, PATH_TYPE::CALCULATE_COST, 0);

		node.edges.push_back(Edge(c->clustNodes[i], distanceTo, EDGE_TYPE::TP_INTRA));
		c->clustNodes[i]->edges.push_back(Edge(&node, distanceTo, EDGE_TYPE::TP_INTRA));
	}

}

Cluster* graphLevel::determineCluster(HierNode node, int lvl)
{
	Cluster* it;
	for (int i = 0; i < lvlClusters[lvl].size(); i++)
	{
		it = &lvlClusters[lvl].at(i);

		if (node.pos.x > it->pos.x && node.pos.x < it->pos.x + it->width &&
			node.pos.y > it->pos.y && node.pos.y < it->pos.y + it->height)
			return it;
	}

	return nullptr;
}

Edge::Edge(HierNode* dest, int distanceTo, EDGE_TYPE type) : dest(dest), distanceTo(distanceTo), type(type)
{}

Entrance::Entrance() : pos{ 0,0 }, width(0), height(0), dir(ADJACENT_DIR::DIR_NONE), from(nullptr), to(nullptr)
{}

Entrance::Entrance(iPoint pos, int width, int height, ADJACENT_DIR dir, Cluster* from, Cluster* to)
	: pos(pos), width(width), height(height), dir(dir), from(from), to(to)
{}

HierNode::HierNode(iPoint pos) : PathNode(-1, -1, pos, this, -1, -1, false)
{}

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

int ModulePathfinding::HierarchicalCreatePath(const iPoint& origin, const iPoint& destination, int maxLvl)
{
	int ret = 0;
	absGraph.insertNode(origin, maxLvl);
	absGraph.insertNode(destination, maxLvl);


	return ret;
}


int ModulePathfinding::CreatePath(const iPoint& origin, const iPoint& destination, PATH_TYPE type, int lvl)
{
	if (IsWalkable(origin) == false || IsWalkable(destination) == false)
	{
		return -1;
	}

	last_path.clear();

	std::multimap<int, PathNode> open;

	std::vector<PathNode> closed;
	open.insert(std::pair<int, PathNode>(0, PathNode(0, origin.DistanceTo(destination), origin, nullptr, 0, 0)));

	while (open.empty() == false)
	{
		std::multimap<int, PathNode>::iterator lowest = open.begin();
		closed.push_back(lowest->second);
		std::vector<PathNode>::pointer node = &closed.back();

		node->myDirection = (closed.size() - 1);
		open.erase(lowest);


		if (node->pos == destination)
		{
			switch (type)
			{
			case PATH_TYPE::GENERATE_PATH:
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
			break;
			case PATH_TYPE::CALCULATE_COST:
			{
				return node->g;
			}
			break;
			}

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