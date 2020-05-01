#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Pathfinding.h"
#include "Brofiler/Brofiler/Brofiler.h"


ModulePathfinding::ModulePathfinding() : j1Module(), walkabilityMap(NULL), last_path(DEFAULT_PATH_LENGTH), mapWidth(0), mapHeight(0)
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
		for (int j = 0; j < absGraph.lvlClusters[i].size(); j++)
		{
			absGraph.lvlClusters[i].at(j).clustNodes.clear();
		}
		absGraph.lvlClusters[i].clear();
	}
	absGraph.lvlClusters.clear();



	for (int i = 0; i < absGraph.nodes.size(); i++)
	{
		for (int j = 0; j < absGraph.nodes[i]->edges.size(); j++)
		{
			delete absGraph.nodes[i]->edges.at(j);
			absGraph.nodes[i]->edges.at(j) = nullptr;
		}
		absGraph.nodes[i]->edges.clear();

		delete absGraph.nodes[i];
		absGraph.nodes[i] = nullptr;
	}
	absGraph.nodes.clear();



	generatedPaths.clear();
	return true;
}


void ModulePathfinding::SetMap(uint width, uint height, uchar* data)
{
	//Basic A*----------------------------------------
	this->mapWidth = width;
	this->mapHeight = height;

	RELEASE_ARRAY(walkabilityMap);
	walkabilityMap = new uchar[width * height];
	memcpy(walkabilityMap, data, width * height);

	//HPA*--------------------------------------------
	HPAPreProcessing(MAX_LEVELS);

}

void ModulePathfinding::HPAPreProcessing(int maxLevel)
{
	absGraph.PrepareGraph();

	for (int l = 2; l <= maxLevel; l++)
	{
		absGraph.CreateGraphLvl(l);
	}
}

void HPAGraph::PrepareGraph()
{
	entrances.clear();

	buildClusters(1);
	buildEntrances();

	createInterNodes(1);
	createIntraNodes(1);
}

void HPAGraph::CreateGraphLvl(int lvl)
{
	if (this->lvlClusters.size() < lvl)
		buildClusters(lvl);

	createInterNodes(lvl);
	createIntraNodes(lvl);
}

void HPAGraph::createInterNodes(int lvl)
{
	Entrance* currEntrance;
	Cluster* c1, * c2;

	for (uint i = 0; i < entrances.size(); i++)
	{
		currEntrance = &entrances[i];

		switch (currEntrance->dir)
		{
		case ADJACENT_DIR::LATERAL:
		{
			c1 = determineCluster(currEntrance->pos, lvl);
			c2 = determineCluster({ currEntrance->pos.x + 1, currEntrance->pos.y }, lvl);

			if (adjacents(c1, c2, lvl) != ADJACENT_DIR::LATERAL)
				continue;

			int maxSize = (currEntrance->pos.y + currEntrance->height);

			for (int i = currEntrance->pos.y; i < maxSize; i += NODE_MIN_DISTANCE)
			{
				BuildInterNode({ currEntrance->pos.x,i }, c1, { currEntrance->pos.x + 1, i }, c2, lvl);
			}
			//We make sure that the last spot has a node for path stability
			BuildInterNode({ currEntrance->pos.x, maxSize - 1 }, c1, { currEntrance->pos.x + 1, maxSize - 1 }, c2, lvl);

		}
		break;
		case ADJACENT_DIR::VERTICAL:
		{
			c1 = determineCluster(currEntrance->pos, lvl);
			c2 = determineCluster({ currEntrance->pos.x, currEntrance->pos.y + 1 }, lvl);

			if (adjacents(c1, c2, lvl) != ADJACENT_DIR::VERTICAL)
				continue;

			int maxSize = (currEntrance->pos.x + currEntrance->width);

			for (int i = currEntrance->pos.x; i < maxSize; i += NODE_MIN_DISTANCE)
			{
				BuildInterNode({ i, currEntrance->pos.y }, c1, { i, currEntrance->pos.y + 1 }, c2, lvl);
			}
			//We make sure that the last spot has a node for path stability
			BuildInterNode({ maxSize - 1, currEntrance->pos.y }, c1, { maxSize - 1, currEntrance->pos.y + 1 }, c2, lvl);

		}
		break;
		}

	}
}

void HPAGraph::BuildInterNode(iPoint p1, Cluster* c1, iPoint p2, Cluster* c2, int lvl)
{
	HierNode* n1, * n2;


	if(MULTILEVEL_SEARCH)
	n1 = NodeExists(p1, nullptr);
	else
	n1 = NodeExists(p1, c1);

	if (!n1)
	{
		n1 = new HierNode(p1);
		nodes.push_back(n1);
	}

	if (NodeExists(p1, c1) == nullptr)
		c1->clustNodes.push_back(n1);



	if (MULTILEVEL_SEARCH)
		n2 = NodeExists(p2, nullptr);
	else
		n2 = NodeExists(p2, c2);

	if (!n2)
	{
		n2 = new HierNode(p2);
		nodes.push_back(n2);
	}

	if (NodeExists(p2, c2) == nullptr)
		c2->clustNodes.push_back(n2);

	CreateEdges(n1, n2, lvl, EDGE_TYPE::INTER);

}

void HPAGraph::createIntraNodes(int lvl)
{
	Cluster* clusterIt;
	float distanceTo = 0;
	int currLvl = lvl - 1;

	for (int i = 0; i < lvlClusters[currLvl].size(); i++)
	{
		clusterIt = &lvlClusters[currLvl].at(i);

		for (int y = 0; y < clusterIt->clustNodes.size(); y++)
		{
			for (int k = y + 1; k < clusterIt->clustNodes.size(); k++)
			{
				CreateEdges(clusterIt->clustNodes[y], clusterIt->clustNodes[k], lvl, EDGE_TYPE::INTRA);

				//distanceTo = App->pathfinding->SimpleAPathfinding(clusterIt->clustNodes[y]->pos, clusterIt->clustNodes[k]->pos);
				//clusterIt->clustNodes[y]->edges.push_back(new Edge(clusterIt->clustNodes[k], distanceTo));
				//clusterIt->clustNodes[k]->edges.push_back(new Edge(clusterIt->clustNodes[y], distanceTo));
			}
		}
	}
}

HierNode* HPAGraph::NodeExists(iPoint pos, Cluster* c)
{
	if (c == nullptr)
	{
		for (int i = 0; i < nodes.size(); i++)
		{
			if (pos == nodes[i]->pos)
				return nodes[i];
		}
	}
	else
	{
		for (int i = 0; i < c->clustNodes.size(); i++)
		{
			if (pos == c->clustNodes[i]->pos)
				return c->clustNodes[i];
		}
	}

	return nullptr;
}

void HPAGraph::CreateEdges(HierNode* n1, HierNode* n2, int lvl, EDGE_TYPE type)
{
	float distanceTo = 1.f;


	if (!EdgeExists(n1, n2, lvl, type))
	{
		if (type == EDGE_TYPE::INTRA)
		{
			distanceTo = App->pathfinding->SimpleAPathfinding(n1->pos, n2->pos);
		}

		n1->edges.push_back(new Edge(n2, distanceTo, lvl, type));
	}

	if (!EdgeExists(n2, n1, lvl, type))
	{
		if (type == EDGE_TYPE::INTRA)
		{
			if (distanceTo == 1)
				distanceTo = App->pathfinding->SimpleAPathfinding(n1->pos, n2->pos);
		}

		n2->edges.push_back(new Edge(n1, distanceTo, lvl, type));
	}



}

bool HPAGraph::EdgeExists(HierNode* n1, HierNode* n2, int lvl, EDGE_TYPE type)
{
	for (int i = 0; i < n1->edges.size(); i++)
	{
		if (n1->edges[i]->dest == n2 && n1->edges[i]->type == type)
		{
			n1->edges[i]->UpdateLvl(lvl);
			return true;
		}
	}

	return false;
}

void HPAGraph::buildEntrances()
{
	Cluster* c1;
	Cluster* c2;
	ADJACENT_DIR adjacentDir = ADJACENT_DIR::DIR_NONE;

	for (uint i = 0; i < this->lvlClusters[0].size(); ++i)
	{
		c1 = &this->lvlClusters[0].at(i);

		for (uint k = i + 1; k < this->lvlClusters[0].size(); ++k)
		{
			c2 = &this->lvlClusters[0].at(k);

			adjacentDir = adjacents(c1, c2, 1);

			if (adjacentDir != ADJACENT_DIR::DIR_NONE)
				createEntrance(c1, c2, adjacentDir, 1);
		}
	}
}

void HPAGraph::createEntrance(Cluster* c1, Cluster* c2, ADJACENT_DIR adjDir, int lvl)
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
					this->entrances.push_back(Entrance({ startedAt, c1->pos.y + c1->height - 1 }, c1->width - startedAt + c1->pos.x, 2, adjDir));

			}
			else if (!isCurrentWalkable)
			{
				if (startedAt != INT_MIN)
				{
					this->entrances.push_back(Entrance({ startedAt, c1->pos.y + c1->height - 1 }, i - startedAt, 2, adjDir));
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
					this->entrances.push_back(Entrance({ c1->pos.x + c1->width - 1,  startedAt }, 2, c1->height - startedAt + c1->pos.y, adjDir));
			}
			else if (!isCurrentWalkable)
			{
				if (startedAt != INT_MIN)
				{
					this->entrances.push_back(Entrance({ c1->pos.x + c1->width - 1, startedAt }, 2, i - startedAt, adjDir));
					startedAt = INT_MIN;
				}
			}
		}
	}
	break;

	}
}

ADJACENT_DIR HPAGraph::adjacents(Cluster* c1, Cluster* c2, int lvl)
{
	int adjDist = CLUSTER_SIZE_LVL * lvl;

	if (c1->pos.x + adjDist == c2->pos.x && c1->pos.y == c2->pos.y)
		return ADJACENT_DIR::LATERAL;

	else if (c1->pos.y + adjDist == c2->pos.y && c1->pos.x == c2->pos.x)
		return ADJACENT_DIR::VERTICAL;

	return ADJACENT_DIR::DIR_NONE;
}

void HPAGraph::buildClusters(int lvl)
{
	int clustSize = CLUSTER_SIZE_LVL * lvl;

	std::vector <Cluster> clusterVector;
	int width = App->pathfinding->mapWidth;
	int height = App->pathfinding->mapHeight;

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

HierNode* HPAGraph::insertNode(iPoint pos, int Lvl, bool* toDelete)
{
	BROFILER_CATEGORY("Insert Node", Profiler::Color::Cornsilk);

	HierNode* newNode = nullptr;
	Cluster* c;

	if (!App->pathfinding->IsWalkable(pos))
		return nullptr;

	c = determineCluster(pos, Lvl);
	if (!c)
		return nullptr;

	newNode = NodeExists(pos, c);

	if (!newNode)
	{
		newNode = new HierNode(pos);
		c->clustNodes.push_back(newNode);
		ConnectNodeToBorder(newNode, c, Lvl);
		*toDelete = true;
	}


	return newNode;
}

void HPAGraph::ConnectNodeToBorder(HierNode* node, Cluster* c, int lvl)
{
	float distanceTo = 0;
	for (int i = 0; i < c->clustNodes.size(); i++)
	{
		if (i == 0)
		{
			distanceTo = App->pathfinding->SimpleAPathfinding(node->pos, c->clustNodes[i]->pos);

			if (distanceTo == -1)
				break;
		}
		distanceTo = node->pos.OctileDistance(c->clustNodes[i]->pos);

		node->edges.push_back(new Edge(c->clustNodes[i], distanceTo, lvl, EDGE_TYPE::INTRA));
		c->clustNodes[i]->edges.push_back(new Edge(node, distanceTo, lvl, EDGE_TYPE::INTRA));
	}

}

Cluster* HPAGraph::determineCluster(iPoint pos, int lvl, Cluster* firstCheck)
{
	BROFILER_CATEGORY("Determine Cluster", Profiler::Color::DeepPink);
	int currLvl = lvl - 1;

	if (firstCheck)
	{
		if (pos.x >= firstCheck->pos.x && pos.y >= firstCheck->pos.y &&
			pos.x < firstCheck->pos.x + firstCheck->width && pos.y < firstCheck->pos.y + firstCheck->height)
			return firstCheck;
	}

	Cluster* it;
	for (int i = 0; i < lvlClusters[currLvl].size(); i++)
	{
		it = &lvlClusters[currLvl].at(i);

		if (pos.x >= it->pos.x && pos.y >= it->pos.y &&
			pos.x < it->pos.x + it->width && pos.y < it->pos.y + it->height)
			return it;
	}

	return nullptr;
}

void HPAGraph::deleteNode(HierNode* toDelete, int lvl)
{
	BROFILER_CATEGORY("Delete Node", Profiler::Color::DarkViolet);

	Cluster* c = nullptr;

	c = determineCluster(toDelete->pos, lvl);

	if (c)
	{
		for (int i = 0; i < c->clustNodes.size(); i++)
		{
			for (int j = 0; j < c->clustNodes[i]->edges.size(); j++)
			{
				if (c->clustNodes[i]->edges[j]->dest == toDelete)
				{
					c->clustNodes[i]->edges[j]->dest = nullptr;
					delete c->clustNodes[i]->edges[j];

					c->clustNodes[i]->edges.erase(c->clustNodes[i]->edges.begin() + j);
					break;
				}
			}
		}

		for (int i = 0; i < c->clustNodes.size(); i++)
		{
			if (c->clustNodes[i]->pos == toDelete->pos)
			{
				for (int j = 0; j < toDelete->edges.size(); j++)
				{
					c->clustNodes[i]->edges[j]->dest = nullptr;
					delete c->clustNodes[i]->edges[j];

				}

				c->clustNodes.erase(c->clustNodes.begin() + i);
				toDelete->edges.clear();
				delete toDelete;
				break;
			}
		}
	}
}

Edge::Edge(HierNode* dest, float distanceTo, int lvl, EDGE_TYPE type) : dest(dest), moveCost(distanceTo), lvl(lvl), type(type)
{}

Entrance::Entrance() : pos{ 0,0 }, width(0), height(0), dir(ADJACENT_DIR::DIR_NONE)
{}

Entrance::Entrance(iPoint pos, int width, int height, ADJACENT_DIR dir)
	: pos(pos), width(width), height(height), dir(dir)
{}

HierNode::HierNode(iPoint pos) : PathNode(-1.f, -1.f, pos, nullptr, -1, -1, false)
{}

HierNode::HierNode(iPoint pos, bool tmp) : PathNode(g, h, pos, nullptr, -1, -1, false)
{}

HierNode::HierNode(float g, const iPoint& pos, PathNode* parent, int parentdir, int myDir, std::vector<Edge*> edges) :
	PathNode(g, -1, pos, parent, parentdir, myDir, false), edges(edges)
{}

Cluster::Cluster() : pos{ -1,-1 }, width(-1), height(-1)
{}

Cluster::Cluster(int width, int height, iPoint& pos) :

	width(width), height(height), pos(pos)
{}

Cluster::Cluster(const Cluster& clust) :

	width(clust.width), height(clust.height), pos(clust.pos)
{}

generatedPath::generatedPath(std::vector <iPoint> vector, PATH_TYPE type, int lvl) : path(vector), type(type), pathLvl(lvl)
{}


//---------------------------------------------------

bool ModulePathfinding::CheckBoundaries(const iPoint& pos) const
{
	return (pos.x >= 0 && pos.x <= (int)mapWidth &&
		pos.y >= 0 && pos.y <= (int)mapHeight);
}


bool ModulePathfinding::IsWalkable(const iPoint& pos) const
{
	uchar t = GetTileAt(pos);
	return t != INVALID_WALK_CODE && t > 0;
}


uchar ModulePathfinding::GetTileAt(const iPoint& pos) const
{
	if (CheckBoundaries(pos))
		return walkabilityMap[(pos.y * mapWidth) + pos.x];

	return INVALID_WALK_CODE;
}


PathNode::PathNode() : g(-1.f), h(-1.f), pos{ -1, -1 }, parent(NULL), is_Diagonal(false), myDirection(0), parentDir(0)
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
		list_to_fill.push_back(PathNode(-1.f, -1.f, cell, this, myDirection, 0, false));

	// south
	cell.create(pos.x, pos.y - 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1.f, -1.f, cell, this, myDirection, 0, false));

	// east
	cell.create(pos.x + 1, pos.y);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1.f, -1.f, cell, this, myDirection, 0, false));

	// west
	cell.create(pos.x - 1, pos.y);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1.f, -1.f, cell, this, myDirection, 0, false));

	cell.create(pos.x + 1, pos.y + 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1.f, -1.f, cell, this, myDirection, 0, true));

	// south
	cell.create(pos.x + 1, pos.y - 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1.f, -1.f, cell, this, myDirection, 0, true));

	// east
	cell.create(pos.x - 1, pos.y + 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1.f, -1.f, cell, this, myDirection, 0, true));

	// west
	cell.create(pos.x - 1, pos.y - 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.push_back(PathNode(-1.f, -1.f, cell, this, myDirection, 0, true));

	return list_to_fill.size();
}

float PathNode::CalculateF(const iPoint& destination)
{
	if (is_Diagonal)
	{
		g = parent->g + 1.41f;
	}
	else
	{
		g = parent->g + 1.f;
	}

	h = pos.OctileDistance(destination);

	return g + h;
}


float PathNode::Score() const
{
	return g + h;
}

uint HierNode::FindWalkableAdjacents(std::vector<HierNode>& list_to_fill, int lvl)
{
	int edgeNum = edges.size();
	HierNode* curr = nullptr;
	Edge* currEdge = nullptr;

	for (int i = 0; i < edgeNum; i++)
	{
		currEdge = edges[i];

		if (!MULTILEVEL_SEARCH || (currEdge->type == EDGE_TYPE::INTRA && currEdge->lvl == lvl || currEdge->type == EDGE_TYPE::INTER && currEdge->lvl >= lvl))
		{
			curr = currEdge->dest;
			list_to_fill.push_back(HierNode(currEdge->moveCost + this->g, curr->pos, this, myDirection, curr->parentDir, curr->edges));
		}

	}

	return list_to_fill.size();
}

float HierNode::CalculateF(const iPoint& destination)
{
	h = pos.OctileDistance(destination);

	return g + h;
}


PATH_TYPE ModulePathfinding::CreatePath(const iPoint& origin, const iPoint& destination, int maxLvl, Entity* pathRequest)
{
	PATH_TYPE ret = PATH_TYPE::NO_TYPE;
	HierNode* n1, * n2;
	bool toDeleteN1 = false;
	bool toDeleteN2 = false;

	if (IsWalkable(origin) == false || IsWalkable(destination) == false)
	{
		return ret;
	}

	last_path.clear();

	if (maxLvl > 0)
	{
		n1 = absGraph.insertNode(origin, maxLvl, &toDeleteN1);
		n2 = absGraph.insertNode(destination, maxLvl, &toDeleteN2);

		if (!n1 || !n2)
			return ret;

		n1->h = n1->pos.OctileDistance(n2->pos);

		//Abs Path
		HPAPathfinding(*n1, n2->pos, maxLvl);

		if (toDeleteN1)
			absGraph.deleteNode((HierNode*)n1, maxLvl);
		if (toDeleteN2)
			absGraph.deleteNode((HierNode*)n2, maxLvl);

		ret = PATH_TYPE::ABSTRACT;

	}
	else
	{
		SimpleAPathfinding(origin, destination);

		ret = PATH_TYPE::SIMPLE;
	}

	generatedPaths.erase(pathRequest);
	generatedPaths.insert({ pathRequest, generatedPath(last_path, ret, maxLvl) });

	return ret;
}


int ModulePathfinding::HPAPathfinding(const HierNode& origin, const iPoint& destination, int lvl)
{
	BROFILER_CATEGORY("HPA Algorithm", Profiler::Color::AliceBlue);

	std::multimap<float, HierNode> open;
	std::vector<HierNode> closed;

	open.insert(std::pair<float, HierNode>(0.f, origin));

	//Analize the current
	HierNode* curr = nullptr;
	std::multimap<float, HierNode>::iterator lowest;

	// Get New Nodes
	uint limit = 0;
	std::vector<HierNode> adjList;
	std::multimap<float, HierNode>::iterator it2;

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

		limit = curr->FindWalkableAdjacents(adjList, lvl);

		for (uint i = 0; i < limit; i++)
		{
			if (FindV(adjList[i].pos, &closed) == closed.size())
			{
				it2 = Find(adjList[i].pos, &open);
				adjList[i].CalculateF(destination);
				if (it2 == open.end())
				{
					open.insert(std::pair<float, HierNode>(adjList[i].Score(), adjList[i]));
				}
				else
				{
					if (adjList[i].Score() < it2->second.Score())
					{
						open.erase(Find(adjList[i].pos, &open));
						open.insert(std::pair<float, HierNode>(adjList[i].Score(), adjList[i]));
					}
				}
			}
		}
		adjList.clear();
	}
}


float ModulePathfinding::SimpleAPathfinding(const iPoint& origin, const iPoint& destination)
{
	BROFILER_CATEGORY("A* Algorithm", Profiler::Color::Black);

	last_path.clear();

	std::multimap<float, PathNode> open;
	std::vector<PathNode> closed;

	open.insert(std::pair<float, PathNode>(0.f, PathNode(0.f, origin.OctileDistance(destination), origin, nullptr, 0, 0)));

	//Analize the current
	PathNode* curr = nullptr;
	std::multimap<float, PathNode>::iterator lowest;

	// Get New Nodes
	uint limit = 0;
	std::vector<PathNode> adjList;
	std::multimap<float, PathNode>::iterator it2;

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
			for (curr; curr->pos != origin; curr = &closed[dir])
			{
				last_path.push_back(curr->pos);
				dir = curr->parentDir;
			}
			last_path.push_back(origin);

			std::reverse(last_path.begin(), last_path.end());

			return closed.back().g;
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
					open.insert(std::pair<float, PathNode>(adjList[i].Score(), adjList[i]));
				}
				else
				{
					if (adjList[i].g < it2->second.g)
					{
						open.erase(Find(adjList[i].pos, &open));
						open.insert(std::pair<float, PathNode>(adjList[i].Score(), adjList[i]));
					}
				}
			}
		}
		adjList.clear();
	}

	return -1;
}

//Entities will call this to request their path
bool ModulePathfinding::RequestPath(Entity* request, std::vector <iPoint>* path)
{
	BROFILER_CATEGORY("RequestPath", Profiler::Color::Khaki);

	if (generatedPaths.size() < 1)
		return false;

	std::unordered_map<Entity*, generatedPath>::iterator it = generatedPaths.begin();

	int maxSize = generatedPaths.size();
	for (int i = 0; i < maxSize; i++)
	{
		if (it->first == request)
		{
			switch (it->second.type)
			{
			case PATH_TYPE::SIMPLE:
			{
				path->insert(path->end(), it->second.path.begin(), it->second.path.end());
				return true;
			}
			break;
			case PATH_TYPE::ABSTRACT:
			{
				return RefineAndSmoothPath(&it->second.path, it->second.pathLvl, path);;
			}
			break;
			}
		}
		it++;
	}

	return false;
}


bool ModulePathfinding::RefineAndSmoothPath(std::vector<iPoint>* absPath, int lvl, std::vector<iPoint>* pathToFill)
{
	BROFILER_CATEGORY("Refine And Smooth Path", Profiler::Color::RosyBrown);

	std::vector <iPoint>* generatedPath = nullptr;
	iPoint currPos = { -1, -1 };
	iPoint startPos = { -1, -1 };

	int from = -1;
	int maxPath = lvl * CLUSTER_SIZE_LVL;
	int pathSize = absPath->size();
	PATH_DIR dir = PATH_DIR::DIR_NONE;

	Cluster* fromC = nullptr;

	for (int i = 0; i < pathSize; i)
	{
		currPos = absPath->at(i);

		//Grab the first node
		if (startPos.x == -1)
		{
			startPos = currPos;
			from = i;
			fromC = absGraph.determineCluster(startPos, lvl);
			i++;
			continue;
		}

		//Two Conditions to make path:
			//Check that Distance is not greater than Cluster Size
			//Not be the last node
		if (fromC != absGraph.determineCluster(currPos, lvl) || (i == pathSize - 1 && pathSize > 0))
		{

			dir = IsStraightPath(startPos, currPos);

			if (dir != PATH_DIR::DIR_NONE && !DoStraightPath(dir, pathToFill, startPos, currPos))
			{
				SimpleAPathfinding(startPos, currPos);
			}
			else
				SimpleAPathfinding(startPos, currPos);

			generatedPath = &last_path;

			if (pathToFill->size() > 1)
				pathToFill->insert(pathToFill->end(), generatedPath->begin() + 1, generatedPath->end());
			else
				pathToFill->insert(pathToFill->end(), generatedPath->begin(), generatedPath->end());


			absPath->erase(absPath->begin() + from, absPath->begin() + i);
			break;
		}
		else
		{
			absPath->erase(absPath->begin() + 1);
			pathSize--;
			continue;
		}

	}

	return generatedPath;
}

PATH_DIR ModulePathfinding::IsStraightPath(iPoint from, iPoint to)
{
	if (from.x == to.x)
	{
		if (from.y > to.y)
			return PATH_DIR::DIR_VER_NEG;
		else
			return PATH_DIR::DIR_VER_POS;
	}

	if (from.y == to.y)
	{
		if (from.x > to.x)
			return PATH_DIR::DIR_HOR_NEG;
		else
			return PATH_DIR::DIR_HOR_POS;
	}


	if (abs(from.x - to.x) == abs(from.y - to.y))
	{
		if (from.x > to.x && from.y > to.y)
			return PATH_DIR::DIR_DIA_XY_NEG;

		else if (from.x < to.x && from.y < to.y)
			return PATH_DIR::DIR_DIA_XY_POS;

		else if (from.x > to.x && from.y < to.y)
			return PATH_DIR::DIR_DIA_X_NEG_Y_POS;

		else if (from.x > to.x && from.y - to.y)
			return PATH_DIR::DIR_DIA_Y_NEG_X_POS;
	}


	return PATH_DIR::DIR_NONE;
}

bool ModulePathfinding::DoStraightPath(PATH_DIR dir, std::vector<iPoint>* toFill, iPoint startPos, iPoint currPos)
{
	iPoint curr = { -1 ,-1 };
	last_path.clear();

	switch (dir)
	{
	case PATH_DIR::DIR_HOR_POS:
	{
		for (int i = startPos.x; i <= currPos.x; i++)
		{
			curr = { i, startPos.y };

			if (IsWalkable(curr))
				last_path.push_back(curr);
			else
			{
				last_path.clear();
				return false;
			}
		}

	}
	break;
	case PATH_DIR::DIR_HOR_NEG:
	{
		for (int i = startPos.x; i >= currPos.x; i--)
		{
			curr = { i, startPos.y };

			if (IsWalkable(curr))
				last_path.push_back(curr);
			else
			{
				last_path.clear();
				return false;
			}
		}
	}
	break;
	case PATH_DIR::DIR_VER_POS:
	{
		for (int i = startPos.y; i <= currPos.y; i++)
		{
			curr = { startPos.x, i };

			if (IsWalkable(curr))
				last_path.push_back(curr);
			else
			{
				last_path.clear();
				return false;
			}
		}
	}
	break;
	case PATH_DIR::DIR_VER_NEG:
	{
		for (int i = startPos.y; i >= currPos.y; i--)
		{
			curr = { startPos.x, i };

			if (IsWalkable(curr))
				last_path.push_back(curr);
			else
			{
				last_path.clear();
				return false;
			}
		}
	}
	break;
	case PATH_DIR::DIR_DIA_XY_POS:
	{
		for (int i = 0; startPos.x + i >= currPos.x; i++)
		{
			curr = { startPos.x + i, startPos.y + i };

			if (IsWalkable(curr))
				last_path.push_back(curr);
			else
			{
				last_path.clear();
				return false;
			}
		}
	}
	break;
	case PATH_DIR::DIR_DIA_XY_NEG:
	{
		for (int i = 0; startPos.x + i <= currPos.x; i--)
		{
			curr = { startPos.x + i, startPos.y + i };

			if (IsWalkable(curr))
				last_path.push_back(curr);
			else
			{
				last_path.clear();
				return false;
			}
		}
	}
	break;
	case PATH_DIR::DIR_DIA_X_NEG_Y_POS:
	{
		for (int i = 0; startPos.x + i <= currPos.x; i--)
		{
			curr = { startPos.x + i, startPos.y - i };

			if (IsWalkable(curr))
				last_path.push_back(curr);
			else
			{
				last_path.clear();
				return false;
			}
		}
	}
	break;
	case PATH_DIR::DIR_DIA_Y_NEG_X_POS:
	{
		for (int i = 0; startPos.x + i >= currPos.x; i++)
		{
			curr = { startPos.x + i, startPos.y - i };

			if (IsWalkable(curr))
				last_path.push_back(curr);
			else
			{
				last_path.clear();
				return false;
			}
		}
	}
	break;
	}

	return true;
}


//Fins for A* MultiMap
std::multimap<float, PathNode>::iterator ModulePathfinding::Find(iPoint point, std::multimap<float, PathNode>* map)
{
	BROFILER_CATEGORY("A* Find", Profiler::Color::Azure);

	std::multimap<float, PathNode>::iterator iterator = map->begin();

	int size = map->size();

	for (int i = 0; i < size; i++)
	{
		if (iterator->second.pos == point)
		{
			return iterator;
		}
		iterator++;
	}

	return map->end();
}

//Fins for HPA MultiMap
std::multimap<float, HierNode>::iterator ModulePathfinding::Find(iPoint point, std::multimap<float, HierNode>* map)
{
	BROFILER_CATEGORY("HPA Find", Profiler::Color::MediumBlue);

	std::multimap<float, HierNode>::iterator iterator = map->begin();

	int size = map->size();

	for (int i = 0; i < size; i++)
	{
		if (iterator->second.pos == point)
		{
			return iterator;
		}
		iterator++;
	}

	return map->end();
}


//Find for A* Vector
int ModulePathfinding::FindV(iPoint point, std::vector<PathNode>* vec)
{
	BROFILER_CATEGORY("A* FindV", Profiler::Color::LawnGreen);

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

//Find for HPA Vector
int ModulePathfinding::FindV(iPoint point, std::vector<HierNode>* vec)
{
	BROFILER_CATEGORY("HPA FindV", Profiler::Color::DarkSeaGreen);

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