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

bool ModulePathfinding::LineRayCast(iPoint& p0, iPoint& p1)
{
	BROFILER_CATEGORY("RayCast", Profiler::Color::Cornsilk);

	std::vector <iPoint> line = CreateLine(p0, p1);

	bool currWalkability = false;

	for (int i = 0; i < line.size(); i++)
	{
		currWalkability = IsWalkable(line[i]);

		if (!currWalkability)
			return false;
	}


	return true;
}

std::vector<iPoint> ModulePathfinding::CreateLine(const iPoint& p0, const iPoint& p1)
{
	last_line.clear();

	float n = p0.DiagonalDistance(p1);

	fPoint p0f = { (float)p0.x, (float)p0.y };
	fPoint p1f = { (float)p1.x, (float)p1.y };

	for (int step = 0; step <= n; step++)
	{

		float t = n == 0 ? 0.0 : step / n;

		fPoint nextPointf = p0f.LerpPoint(p1f, t);
		nextPointf.RoundPoint();

		iPoint nextPoint = { (int)nextPointf.x, (int)nextPointf.y };
		last_line.push_back(nextPoint);

	}

	return last_line;
}

std::vector<iPoint>* ModulePathfinding::GetLastLine()
{
	return &last_line;
}

bool ModulePathfinding::CleanUp()
{
	LOG("Freeing pathfinding library");

	//A*--------------------------------------
	last_path.clear();
	RELEASE_ARRAY(walkabilityMap);

	//HPA-----------------------------------------
	absGraph.entrances.clear();

	//Cluster Clear
	for (int i = 0; i < absGraph.lvlClusters.size(); i++)
	{
		for (int j = 0; j < absGraph.lvlClusters[i].size(); j++)
		{
			absGraph.lvlClusters[i].at(j).clustNodes.clear();
		}
		absGraph.lvlClusters[i].clear();
	}
	absGraph.lvlClusters.clear();


	//Nodes Clear
	for (int i = 0; i < absGraph.staticNodes.size(); i++)
	{
		for (int j = 0; j < absGraph.staticNodes[i]->edges.size(); j++)
		{
			delete absGraph.staticNodes[i]->edges.at(j);
			absGraph.staticNodes[i]->edges.at(j) = nullptr;
		}
		absGraph.staticNodes[i]->edges.clear();

		delete absGraph.staticNodes[i];
		absGraph.staticNodes[i] = nullptr;
	}
	absGraph.staticNodes.clear();



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
	if (maxLevel < 1)
		return;

	//Prepares Entrances & the 1st level of the graph
	absGraph.PrepareGraph();

	//TODO 7 (EXTRA): If you want more abstraction levels, just uncomment this 
	// Just make sure the MAX_LEVELS define is above 1 & the TODO 4 is done correctly

	//Creates the rest of the graph levels
	for (int l = 2; l <= maxLevel; l++)
	{
		absGraph.CreateGraphLvl(l);
	}
}

//Prepares Entrances & the 1st level of the graph
void HPAGraph::PrepareGraph()
{
	entrances.clear();

	//TODO 1: First of all, let's prepare the graph. The first step to do this is creating the clusters & the entrances.
	//Uncomment the code & proceed to the 1.1 & 1.2 TODO

	//Build the first level clusters & entrances
	BuildClusters(1);
	BuildEntrances();

	//TODO 2: Next let's create the Nodes that will connect the clusters among them & themselves.
	//Uncomment the code & proceed to the 2.1 & 2.2 TODO

	//Node creation-----
	CreateInterNodes(1);
	CreateIntraNodes(1);


}

void HPAGraph::CreateGraphLvl(int lvl)
{

	//If we don't have clusters for the level, we create them
	if (this->lvlClusters.size() < lvl)
		BuildClusters(lvl);

	//Node creation-----
	CreateInterNodes(lvl);
	CreateIntraNodes(lvl);
}

//Creates the connections between adjacent nodes of diferent clusters
void HPAGraph::CreateInterNodes(int lvl)
{
	Entrance* currEntrance = nullptr;
	Cluster* c1, * c2;

	//TODO 2.1: Here the Nodes between Clusters are created. The function is separated in two  for better visibility.
	// Just check how lateral is created & then do the vertical one (the logic is the same, you just have to change the axis)
	// Be careful with the position!

	for (uint i = 0; i < entrances.size(); i++)
	{
		currEntrance = &entrances[i];

		switch (currEntrance->dir)
		{
		case ADJACENT_DIR::LATERAL:
		{

			//First of all we check if the clusters are adjacents
			c1 = DetermineCluster(currEntrance->pos, lvl);
			c2 = DetermineCluster({ currEntrance->pos.x + 1, currEntrance->pos.y }, lvl);

			if (ClustersAreAdjacent(c1, c2, lvl) != ADJACENT_DIR::LATERAL)
				continue;


			//Build the max. number of nodes that can we built based on the cluster size & the min distance we established
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
			//---
			//First of all we check if the clusters are adjacents
			c1 = DetermineCluster(currEntrance->pos, lvl);
			c2 = DetermineCluster({ currEntrance->pos.x, currEntrance->pos.y + 1 }, lvl);

			if (ClustersAreAdjacent(c1, c2, lvl) != ADJACENT_DIR::VERTICAL)
				continue;


			//Build the max. number of nodes that can we built based on the cluster size & the min distance we established
			int maxSize = (currEntrance->pos.x + currEntrance->width);

			for (int i = currEntrance->pos.x; i < maxSize; i += NODE_MIN_DISTANCE)
			{
				//if (!NodeExists({ i+1, currEntrance->pos.y }) && !NodeExists({ currEntrance->pos.x,i - 1 }))
				BuildInterNode({ i, currEntrance->pos.y }, c1, { i, currEntrance->pos.y + 1 }, c2, lvl);
			}

			//We make sure that the last spot has a node for path stability
			BuildInterNode({ maxSize - 1, currEntrance->pos.y }, c1, { maxSize - 1, currEntrance->pos.y + 1 }, c2, lvl);
			
			//---
		}
		break;
		}

	}
}

//Builds an interNode
void HPAGraph::BuildInterNode(iPoint p1, Cluster* c1, iPoint p2, Cluster* c2, int lvl)
{
	HierNode* n1, * n2;


	//First of all, we check if the node currently exists
	n1 = NodeExists(p1, nullptr);

	//If it doesn't exist, create it
	if (!n1)
	{
		n1 = new HierNode(p1);
		staticNodes.push_back(n1);
	}

	//If doesn't exist in the current cluster, introduce it
	if (NodeExists(p1, c1) == nullptr)
		c1->clustNodes.push_back(n1);



	//Repeat the process for the second node----
	n2 = NodeExists(p2, nullptr);

	if (!n2)
	{
		n2 = new HierNode(p2);
		staticNodes.push_back(n2);
	}

	if (NodeExists(p2, c2) == nullptr)
		c2->clustNodes.push_back(n2);


	CreateEdges(n1, n2, lvl, EDGE_TYPE::INTER);
}

//Creates the connections between nodes in the same cluster
void HPAGraph::CreateIntraNodes(int lvl)
{
	Cluster* clusterIt;
	float distanceTo = 0;
	int currLvl = lvl - 1;

	//TODO 2.2: Here we iterate trough all the nodes in a same cluster for all the cluster in a same level
	// Simply call the function to create edges between the different nodes with the INTRA type
	// Check the function CreateEdges()


	for (int i = 0; i < lvlClusters[currLvl].size(); i++)
	{
		clusterIt = &lvlClusters[currLvl].at(i);

		for (int y = 0; y < clusterIt->clustNodes.size(); y++)
		{
			for (int k = y + 1; k < clusterIt->clustNodes.size(); k++)
			{
				//--
				CreateEdges(clusterIt->clustNodes[y], clusterIt->clustNodes[k], lvl, EDGE_TYPE::INTRA);
				//---

			}
		}
	}
}

HierNode* HPAGraph::NodeExists(iPoint pos, Cluster* c)
{
	if (c == nullptr)
	{
		for (int i = 0; i < staticNodes.size(); i++)
		{
			if (pos == staticNodes[i]->pos)
				return staticNodes[i];
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

//Creates the connection between nodes
void HPAGraph::CreateEdges(HierNode* n1, HierNode* n2, int lvl, EDGE_TYPE type)
{
	float distanceTo = 1.f;

	//First check if already exists
		//We will always create the INTRA edge (multi-level purposes)
		//If the type is INTER we will check if it exists
	if (type == EDGE_TYPE::INTRA || !EdgeExists(n1, n2, lvl, type))
	{
		//If the connection if between same cluster nodes, we calculate the moving cost trough A*; otherwise  is 1
		if (type == EDGE_TYPE::INTRA)
		{

			distanceTo = App->pathfinding->SimpleAPathfinding(n1->pos, n2->pos);
		}

		n1->edges.push_back(new Edge(n2, distanceTo, lvl, type));
	}

	//Repeat the process for the second node----
	if (type == EDGE_TYPE::INTRA || !EdgeExists(n2, n1, lvl, type))
	{
		if (type == EDGE_TYPE::INTRA)
		{
			if (distanceTo == 1)
				distanceTo = App->pathfinding->SimpleAPathfinding(n1->pos, n2->pos);
		}

		n2->edges.push_back(new Edge(n1, distanceTo, lvl, type));
	}



}

//Checks if an Edge exists
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

//Builds the graph entrances
void HPAGraph::BuildEntrances()
{
	Cluster* c1;
	Cluster* c2;
	ADJACENT_DIR adjacentDir = ADJACENT_DIR::DIR_NONE;

	//TODO 1.2: Here we iterate through all the clusters; just check if a pair of clusters is adjacent & if so create entrances for it
	// Check the functions ClustersAreAdjacent() & CreateEntrance()

	//We simply iterate though the clusters & check if they are adjacent, if so we put an entrance between them
	for (uint i = 0; i < this->lvlClusters[0].size(); ++i)
	{
		c1 = &this->lvlClusters[0].at(i);

		for (uint k = i + 1; k < this->lvlClusters[0].size(); ++k)
		{
			c2 = &this->lvlClusters[0].at(k);

			//----
			adjacentDir = ClustersAreAdjacent(c1, c2, 1);

			if (adjacentDir != ADJACENT_DIR::DIR_NONE)
				CreateEntrance(c1, c2, adjacentDir, 1);
			//---
}
	}
}

void HPAGraph::CreateEntrance(Cluster* c1, Cluster* c2, ADJACENT_DIR adjDir, int lvl)
{
	int adjDist = CLUSTER_SIZE_LVL * lvl;

	Entrance newEntrance;
	int startedAt = INT_MIN;
	bool isCurrentWalkable = true;

	//The creation of the entrances is divided in the two aixs for visibility purporses
	switch (adjDir)
	{
	case ADJACENT_DIR::VERTICAL:
	{
		//First of all we iterate though the cluster
		for (int i = c1->pos.x; i < (c1->pos.x + c1->width); i++)
		{

			//We check if BOTH the tiles that connect the cluster are walkable
			isCurrentWalkable = (App->pathfinding->IsWalkable({ i, c1->pos.y + c1->height - 1 }) && App->pathfinding->IsWalkable({ i, c2->pos.y }));

			if (isCurrentWalkable)
			{
				if (startedAt == INT_MIN)
					startedAt = i;

				//If we reach the end, we create an entrance
				if (i == (c1->pos.x + c1->width - 1))
					this->entrances.push_back(Entrance({ startedAt, c1->pos.y + c1->height - 1 }, c1->width - startedAt + c1->pos.x, 2, adjDir));

			}
			else if (!isCurrentWalkable)
			{

				// We check if we come from a tile that's walkable, if so we create a new entrance
				// This is done to make sure the pathfinding will work even if there are non-walkable tiles between clusters
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
		//Same as vertical but in the y axis

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


ADJACENT_DIR HPAGraph::ClustersAreAdjacent(Cluster* c1, Cluster* c2, int lvl)
{
	int adjDist = CLUSTER_SIZE_LVL * lvl;

	if (c1->pos.x + adjDist == c2->pos.x && c1->pos.y == c2->pos.y)
		return ADJACENT_DIR::LATERAL;

	else if (c1->pos.y + adjDist == c2->pos.y && c1->pos.x == c2->pos.x)
		return ADJACENT_DIR::VERTICAL;

	return ADJACENT_DIR::DIR_NONE;
}

//Builds clusters for an specific level
void HPAGraph::BuildClusters(int lvl)
{
	//TODO 1.1: The clusters are created regulary here.
	// The code to calculate how big & were are the clusters is done, just add it to the buffer vector & then add it to the
	// vector of clusters vectors (ask me about this :P) we have already declared


	//Cluster distance in the current level
	int clustSize = CLUSTER_SIZE_LVL * lvl;

	//Buffer vector
	std::vector <Cluster> clusterVector;


	int width = App->pathfinding->mapWidth;
	int height = App->pathfinding->mapHeight;

	Cluster c;

	//We will create the cluster in a row-column order
	for (int i = 0; i < width; i += clustSize)
	{

		//Check if the cluster would extend beyond the map
		if (i + clustSize > width)
			c.width = width - (i);
		else
			c.width = clustSize;

		for (int k = 0; k < height; k += clustSize)
		{

			//Check if the cluster would extend beyond the map
			if (k + clustSize > height)
				c.height = height - (k);
			else
				c.height = clustSize;

			//Introduce the Cluster into the vector buffer
			c.pos = { i,k };
			//------
			clusterVector.push_back(Cluster(c));
		}
	}

	//Introduce the vector to the main one
	//------
	this->lvlClusters.push_back(clusterVector);
}

//Inserts a node into the graph
HierNode* HPAGraph::insertNode(iPoint pos, int Lvl, bool* toDelete)
{
	BROFILER_CATEGORY("Insert Node", Profiler::Color::Cornsilk);

	HierNode* newNode = nullptr;
	Cluster* c;

	if (!App->pathfinding->IsWalkable(pos))
		return nullptr;

	//Determine the cluster
	c = DetermineCluster(pos, Lvl);
	if (!c)
		return nullptr;


	//TODO 3: This is simple, search if the NodeExists() in the cluster that is checked above 
	// If it doesn't exist, create it: you'll have to push it in the cluster Node Vector & connect it t the other nodes in the same
	// cluster (Check ConnectNodeToBorder()).
	//---

		//Check if already exists
		newNode = NodeExists(pos, c);
	//---



	//If can be placed inside a cluster & there is no node already there, we create one
	if (!newNode)
	{
		newNode = new HierNode(pos);
		c->clustNodes.push_back(newNode);
		ConnectNodeToBorder(newNode, c, Lvl);
		*toDelete = true;
	}
	//---

	return newNode;
}

//Connects a recently introduced node to it's peers in the same cluster
void HPAGraph::ConnectNodeToBorder(HierNode* node, Cluster* c, int lvl)
{
	float distanceTo = 0;
	for (int i = 0; i < c->clustNodes.size(); i++)
	{
		//To calculate the cost to the node peers, we should do A* and get the cost from there
		// But this is very slow, so we  will do a RayCast Check first; this is a little less accurate but far more faster
		if (App->pathfinding->LineRayCast(node->pos, c->clustNodes[i]->pos))
		{
			distanceTo = App->pathfinding->GetLastLine()->size();
		}
		else
			distanceTo = App->pathfinding->SimpleAPathfinding(node->pos, c->clustNodes[i]->pos);


		//Create the edges between the two nodes
		node->edges.push_back(new Edge(c->clustNodes[i], distanceTo, lvl, EDGE_TYPE::INTRA));
		c->clustNodes[i]->edges.push_back(new Edge(node, distanceTo, lvl, EDGE_TYPE::INTRA));
	}

}

//Determines the cluster in which a node is placed
Cluster* HPAGraph::DetermineCluster(iPoint pos, int lvl, Cluster* firstCheck)
{
	BROFILER_CATEGORY("Determine Cluster", Profiler::Color::DeepPink);
	int currLvl = lvl - 1;

	//If we are suspicious is on a concrete cluster, we can check it first
	if (firstCheck)
	{
		if (pos.x >= firstCheck->pos.x && pos.y >= firstCheck->pos.y &&
			pos.x < firstCheck->pos.x + firstCheck->width && pos.y < firstCheck->pos.y + firstCheck->height)
			return firstCheck;
	}

	//Iterate though all the clusters & check if the position of the node is between them
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

//Deletes a node from the graph
void HPAGraph::DeleteNode(HierNode* toDelete, int lvl)
{
	BROFILER_CATEGORY("Delete Node", Profiler::Color::DarkViolet);

	Cluster* c = nullptr;
	int toDeletePos = -1;


	c = DetermineCluster(toDelete->pos, lvl);

	if (c)
	{
		//Deletes the connections to the node scheduled for deletion
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

			if (c->clustNodes[i] == toDelete)
				c->clustNodes.erase(c->clustNodes.begin() + i);

		}

		//Deletes the node from the cluster

		for (int j = 0; j < toDelete->edges.size(); j++)
		{
			toDelete->edges[j]->dest = nullptr;
			delete toDelete->edges[j];
		}

		toDelete->edges.clear();
		delete toDelete;


	}

}



//Constructores------------

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

GeneratedPath::GeneratedPath(std::vector <iPoint> vector, PATH_TYPE type, int lvl) : path(vector), type(type), pathLvl(lvl)
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
	HierNode* currNode = nullptr;
	Edge* currEdge = nullptr;

	// TODO 4: Let's find the walkable node adjacents. We have to iterate though all the HierNode's edges & add them to the list.
	// Be careful, in order to have multiple abstraction levels we will have to check these conditions befor we insert a node to the list:
				//INTRA: have to be from the same level
				//INTER: can be from the same level or superior
	//Also, the g is not calculate in CalculateF, you can do it here
	//---

	//Iterate though all the node edges
	for (int i = 0; i < edgeNum; i++)
	{
		currEdge = edges[i];

		//Checks if the edge has the correct level to put it in the list
			//INTRA: have to be from the same level
			//INTER: can be from the same level or superior
		if (currEdge->type == EDGE_TYPE::INTRA && currEdge->lvl == lvl || currEdge->type == EDGE_TYPE::INTER && currEdge->lvl >= lvl)
		{
			currNode = currEdge->dest;
			list_to_fill.push_back(HierNode(currEdge->moveCost + this->g, currNode->pos, this, myDirection, currNode->parentDir, currNode->edges));
		}

	}

	//---



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

	//HPA*------
	if (maxLvl > 0)
	{

		//Insert the nodes into the graph
		n1 = absGraph.insertNode(origin, maxLvl, &toDeleteN1);
		n2 = absGraph.insertNode(destination, maxLvl, &toDeleteN2);

		if (!n1 || !n2)
			return ret;

		n1->h = n1->pos.OctileDistance(n2->pos);


		//HPA* algorithm
		HPAPathfinding(*n1, n2->pos, maxLvl);


		//Delete the nodes from the graph
		if (toDeleteN1)
			absGraph.DeleteNode((HierNode*)n1, maxLvl);
		if (toDeleteN2)
			absGraph.DeleteNode((HierNode*)n2, maxLvl);

		ret = PATH_TYPE::ABSTRACT;

	}
	//A*------------------
	else
	{
		SimpleAPathfinding(origin, destination);

		ret = PATH_TYPE::SIMPLE;
	}

	//Save the generated path------
	generatedPaths.erase(pathRequest);
	generatedPaths.insert({ pathRequest, GeneratedPath(last_path, ret, maxLvl) });

	return ret;
}


int ModulePathfinding::HPAPathfinding(const HierNode& origin, const iPoint& destination, int lvl)
{
	BROFILER_CATEGORY("HPA Algorithm", Profiler::Color::AliceBlue);

	last_path.clear();

	//Variable declaration-----
	//Open & Closed
	std::multimap<float, HierNode> open;
	std::vector<HierNode> closed;

	//Analize the current
	HierNode* curr = nullptr;
	std::multimap<float, HierNode>::iterator lowest;

	// Get New Nodes
	uint limit = 0;
	std::vector<HierNode> adjList;
	std::multimap<float, HierNode>::iterator it2;


	open.insert(std::pair<float, HierNode>(0.f, origin));


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

	//Variable declaration-----
	//Open & Closed
	std::multimap<float, PathNode> open;
	std::vector<PathNode> closed;

	//Analize the current
	PathNode* curr = nullptr;
	std::multimap<float, PathNode>::iterator lowest;

	// Get New Nodes
	uint limit = 0;
	std::vector<PathNode> adjList;
	std::multimap<float, PathNode>::iterator it2;

	open.insert(std::pair<float, PathNode>(0.f, PathNode(0.f, origin.OctileDistance(destination), origin, nullptr, 0, 0)));

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

	std::unordered_map<Entity*, GeneratedPath>::iterator it = generatedPaths.begin();

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
	int pathSize = absPath->size();

	Cluster* fromC = nullptr;

	for (int i = 0; i < pathSize; i)
	{
		currPos = absPath->at(i);

		//Grab the first node
		if (startPos.x == -1)
		{
			startPos = currPos;
			from = i;
			fromC = absGraph.DetermineCluster(startPos, lvl);
			i++;
			continue;
		}



		//TODO 5: This is a big one; currently the code just does nothing with the hierchial path, what you have to do is set the 
	// conditions to refine the path & actually refined. The conditions are: 
		//Check that Distance is not greater than Cluster Size
		//Not be the last node
	//Then you will have to refine the Path, we have to ways to do this:
		//RayCasting: Very fast, doesn't like obstacles
		// Direct A*: slower but can make whatever path you send to it.
	//The ideal situation would be to check with a RayCast first & if not succesfull make an A*
	//To insert the path, check the functions on the std::vector (Insert() might be usefull :P)
	//Do not forget to delete the nodes from the hierchial path after you have refined them

	//----
		//Two Conditions to make path:
			//Check that Distance is not greater than Cluster Size
			//Not be the last node
		if (fromC != absGraph.DetermineCluster(currPos, lvl) || (i == pathSize - 1 && pathSize > 0))
		{

			//First Quick Check w/Ray Cast
			if (LineRayCast(startPos, currPos) && !last_line.empty())
			{
				generatedPath = &last_line;
			}

			//If the Ray Cast fails, we have to do A* to connect the nodes
			else if (SimpleAPathfinding(startPos, currPos) && !last_path.empty())
			{
				generatedPath = &last_path;
			}

			//If the refinement was succesfull, we added to the request
			if (generatedPath != nullptr)
			{

				//Last & not last cases:
					//We don't want to introduce the first one since it will overlap with the last one already refined
				if (pathToFill->size() > 0)
					pathToFill->insert(pathToFill->end(), generatedPath->begin() + 1, generatedPath->end());
				else
					pathToFill->insert(pathToFill->end(), generatedPath->begin(), generatedPath->end());
			}

			//Delete the abstract nodes we just refined
			absPath->erase(absPath->begin() + from, absPath->begin() + i);
			break;
		}
		//---
		else
		{
			absPath->erase(absPath->begin() + 1);
			pathSize--;
			continue;
		}

	}

	return generatedPath;
}


//Fins for A* MultiMap
std::multimap<float, PathNode>::iterator ModulePathfinding::Find(iPoint point, std::multimap<float, PathNode>* map)
{
	//BROFILER_CATEGORY("A* Find", Profiler::Color::Azure);

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
	//BROFILER_CATEGORY("HPA Find", Profiler::Color::MediumBlue);

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
	//BROFILER_CATEGORY("A* FindV", Profiler::Color::LawnGreen);

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
	//BROFILER_CATEGORY("HPA FindV", Profiler::Color::DarkSeaGreen);

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