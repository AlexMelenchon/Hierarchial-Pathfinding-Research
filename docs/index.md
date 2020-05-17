I am [Àlex Melenchón](https://www.linkedin.com/in/alex-melench%C3%B3n-maza-a46981195/), student of the [Bachelor’s Degree in
Video Games by UPC at CITM](https://www.citm.upc.edu/ing/estudis/graus-videojocs/). This content is generated for the second year’s
subject Project 2, under supervision of lecturer [Marc Garrigó](https://www.linkedin.com/in/mgarrigo/).

You can take a look at the website with detailed information about the research [HERE](https://github.com/AlexMelenchon/Hierarchial-Pathfinding-Research) & you can check the project [HERE](https://github.com/AlexMelenchon/Hierarchial-Pathfinding-Research)

## Definition & Introduction to the Problem - What is  Hierarchial Pathfinding?
The problem that Hierchial Pathfinding is trying to solve is that, in Videogames, most calculated paths are never fully walked; since most of the times something happens in the middle of the path that forces to change plans.

In order to achieve this, what is used by other reference videogames is the map abstraction into a hierarchy with different levels representing groups of tiles from the lower levels (this is achieved by grouping tiles into Nodes, that can be regular or not).

Then is where Hierchial Pathfinding comes into play: since it plans the path in much the same way as people would. You plan an overview route first and then refine it as needed. For example: let's say you are in your house in Barcelona & you want to go visit Rome, you woudn't just raw calculate each foot to get there, you will go something as: leave the house -> grab a Taxi -> go to the Airport - etc. That is basically what Hierchial Pathfinding does, it makes a quick & abstract research of the key abstracted-steps we calculated before to get from A to B &, when necessary it refines a simple path between the key steps.

The abstract plan is simple, and we split the pathfinding problem over a long period of time, only doing the next step when the current step is complete.

As I was saying, this is very useful for Videogames, since most of the times knowing the first few moves of a valid path is enough, allowing the requester to start moving in the right direction. Subsequent events may result in the agent having to change its plan, obviating the need for the rest of the paths. This has a huge advantage in performance, since we don't have to compute all the path that will not be used.

Right off the bat. this sounds really good, but there is one key disadvantage: this does not create optimal paths, but, for real-time pathfinding applications such as Videogames, this is not a huge problem most of the times, providing the path looks reasonable.

<p align="right">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/hpaGraph.jpg" >
</p>

This image of the Indie Developer [K Lodeman](https://twitter.com/ManOfLode/status/854406316890128384) shows how the hierarchy principal works; the huge map is divided into shorter steps in order to then refine the path.

## Ways of implementation
First of all, there aren't static religious ways to implement the Hierarchial Pathfinding (let's call it HP for now) because it's not an algorithm. We could say it's a "technique" to divide the map so the desired Pathfinding Algorithm has an easier time construction the abstract & refined paths.

### Regular Implementation

With this said, let's see the principles that the "hierarchy graph must follow" in order to have an expandable & solid grid configuration (you can find the [Near Optimal Hierarchical Path-Finding paper](http://webdocs.cs.ualberta.ca/~kulchits/Jonathan_Testing/publications/ai_publications/jogd.pdf) this explanation is based on [HERE](http://webdocs.cs.ualberta.ca/~kulchits/Jonathan_Testing/publications/ai_publications/jogd.pdf)):

#### **Agents**:

  - **Clusters**: these are basically groups of the lower-level tiles; the higher the level of abstraction of the graph, the bigger they are.
    
  - **Entraces**: these are created just with the lowest level of abstraction & they are basically bridges between the clusters that determine which clusters are walkable adjacents.

  - **Nodes**: they are the most crucial part of the hierarchy. Once the entrances are created, we must create Nodes in the spaces in which the entrances are placed. This will be determined not only by the Entrance position & size, but for an arbitrary number we will define that will determine how separated are the Nodes in one entrance.
 
   - **Edges**: another crucial part of the hierarchy. This can basically be defined as the connections between nodes. They We will be the indicators for connection & store the movement cost between the nodes. We distinguish two types of connections: between INTER & INTRA nodes.
      - INTER: these are the connections made between Nodes of different Clusters. Since they are adjacent, it's movement cost is always one.
       - INTRA: these are the connections made between nodes of the same cluster. The movement cost between them is determined by running the desired pathfinding algorithm  & getting it's computation cost.
     

   
<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/full_structuresExplanation.png" >
</p>

> Here you can see how the agents come into play making the graph. In the image on the left, you can see the blue divisions represent the clusters & the red lines the created nodes in the entrances. Then, in the right, the Edges between the nodes are visible, with it's movement cost being shown.

####  **General Methods**:

   - **Preparing the Graph**: this process has to be executed before the runtime of the game; it can be pre-calculated & stored in the game files or can also be calculated while the game map is loading. Basically, this creates the first abstraction level, which means, creating the Entrances, Cluster, Nodes & Edges for the level.
    
   - **Adding further Graph Levels**: with our first level created, we can add only higher abstraction levels on top of that, but, for these  further abstraction levels, we will  re-utilize the Entrances calculated for the first level, because the higher level clusters would be groups of lower level clusters. Then, since the entrances are the same, the nodes will be as well, so we can add a variable level to our nodes to indicate the code that this node is from a higher level. We can also do it for the INTER edges as well, but, for the INTRA edges, we will have to calculate them again, since we need the different INTRA connections for each level, since they do change.
   
   - **Hierchial Serch**: assumed an abstraction level, and origin & a destination are defined, we will perform a search with our desired algorithm; being the nodes to search NOT the lowest-level tiles but the extraction levels but the defined level abstracted ones. First of all we will introduce the origin & destination as temporal nodes in the abstract level, in order to perform & finish the search. When the path is done (using the abstract Nodes of the level as points to make the path & the Edges as the connections between them) we will remove these temporal nodes from the abstract graph. 
   As you can already see, this makes a distinction between Hierarchial & other methods of pathfinding, since **we don't have just to worry how much time the algorithm wastes, but also how much it costs to introduce & delete the nodes from the graph.**
   
   - **Refine & Smooth Path**: whenever we have the path, we will have to refine it gradually. Going back to the previous example of travelling: we know that we have to go get out of home & then go to the airport; but we have to know in the lowest level possible what is the path to follow to get out of the house & then, once there, to go to the airport. This is when refinement comes into play; they are optimizations to this I will explain later, but basically we have to execute out pathfinding algorithm between the two points of the abstract path &, since it's a much shorter path, it will be faster & divided though time.
   Additionally, if we just "connect the dots" the path will be no where near optimal & will look bad. This is where the Smooth comes in. A general way to implement this is just to check if from an abstract node (when we are refining) we can make a straight line to the following ones, if we can, we execute it & delete the refined nodes.
   
> This explains the general structure an standard Hierarchial Pathfinding should follow;  further on the document you will find the specific method I selected to implement all of this & also other approaches to the problem, but if you want to know more detailed information about this  yo ucan find it [HERE](http://webdocs.cs.ualberta.ca/~kulchits/Jonathan_Testing/publications/ai_publications/jogd.pdf)).

***
####  **Results**:

 **As concluded in the paper**, the addition of the Hierarchial pathfinding vs your regular algorithm pathfinding, produces these benefits:
 
   - **Support for Dynamic Terrain & Changing Paths**
   - **Versatile, Custumizable & Moldable**
   - **Beats A* by a factor of 10-100**; in their test, they introduced it to **[Bioware's Baldur's Gate](https://en.wikipedia.org/wiki/Baldur%27s_Gate)** & beat the previous **computation times by a factor of 10** fold with **just a 1%  degradation of path quality**.
    
<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/baldurs-gate-RTS.png"  width="60%" height="60%">
</p>

### Different Approaches

#### **Different Algorithms**:
   - I have not mentioned this because I didn't want to bias anyone, but the Hierchial method is often used as an upgrade of the A* pathfinding algorithm (if you need a quick A* reminder, there is a very nice & concise article on  [Red Blob Games](https://www.redblobgames.com/pathfinding/a-star/introduction.html) but, as I said it can be used for any other algorithm; for example [this paper](https://www.cs.ru.nl/bachelors-theses/2013/Linus_van_Elswijk___0710261___Hierarchical_Path-Finding_Theta_star_Combining_HPA_star_and_Theta_star.pdf) implements it in a variation of A*, called Theta*.
    
    
   > Theta* is basically A* with a Jump Point Search implemented; if you want to know more about JPS*, there is a very nice research from my colleague, [Lucho Suaya](https://www.linkedin.com/in/luchosuaya/) [HERE](https://lucho1.github.io/JumpPointSearch/); which I took as reference & inspiration to make the document you are reading right now.
    
#### **Dynamic Approach**:  
   - You can also get the Hierarchy to work with dynamic maps & procedural generated graphs, being in the need of handling multiple agents that request a path in a very large world full of interesting obstacles (such as stairs, mazes, etc.). There is a very cool GDC presentation about  this implementation by Alain Benoit, Lead Programmer and CTO of [Sauropod Studio](https://www.sauropodstudio.com/english-1) in their game [Castle Story](http://www.castlestory.net/). You can find the 2018 GDC presentation [HERE](https://twvideo01.ubm-us.net/o1/vault/gdc2018/presentations/Alain_Benoit_HierarchicalDynamicPathfinding.pdf).

<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/Voxel_hier2.png"  width="60%" height="60%">
</p>


#### **Utility beyond Pathfinding**:  
   - As I just mentioned, the Hierchial Pathfinding is just the application of a Hierarchy to the pathfinding system but, without leaving the subject, we can find an implementation that uses the Hierarchy to make more deeper & faster AI decisions.
   This is the case for the videogames [Killzone 2](https://en.wikipedia.org/wiki/Killzone_2) & [Killzone 3](https://en.wikipedia.org/wiki/Killzone_3). In these games, as you can check in their [GDC 2009 talk for Killzone 2](https://drive.google.com/open?id=177H0M2sSvUmjmTRGl6RBFTQIqm7edVUp) & in this [Paper for a more technical approach for Killzone 3](http://www.gameaipro.com/GameAIPro/GameAIPro_Chapter29_Hierarchical_AI_for_Multiplayer_Bots_in_Killzone_3.pdf).
   Basically, ir order to create an AI with strategic vision & control, they designed a Hierarchial order to distribute the enemy bots (individual -> squad -> commander, who gives the orders). On top of that, the bots move in a Hierarchial way; having different levels of abstraction for the different decisions, movements, strategies, etc. in the game. This leads to a more organized, strategic, faster & responsive AI; if you want a more deeeper explanation please consult the sources.

<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/killzone2_1.png"  width="40%" height="40%" margin="0">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/killzone2_2.png"  width="40%" height="40%" margin="0">
</p>
   
#### **Irregular Graphs** (or navigation meshes): 
   - Of course I must mention this, note that the agents that I have explained in the previous sections is partially a lie. This research is focused for general HP implementation and so, the explanation will be about making a regular-all purpose graphs; but a very common approach in Videogames & Game Engines is to create a custom navigation mesh for our levels; but the approach is the same: instead of having Entrances, we will define the nodes as we consider & we calculate the edges between the nodes. Then we can abstractly connect the nodes & make the algorithm run raw to refine the lower level path or we can also refine clusters to improve the refinement.
   This provides much more path stability, we make sure of how dense we want the map to be in certain areas, etc. The main problem is that is not computer-generated but human generated and that takes a lot of time & effort.
   
   <p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/multiple_iregular_graph.jpg">
</p>

   > An example of multiple leveled navigation grid, with irregular nodes & clusters.

   <p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/200KoboldPaths.png"  width="60%" height="60%">
</p>

   > An example of multiple leveled navigation grid, with irregular nodes but regular clusters.

    
 - If you want to know more about how to implement & manipulate Navigation Meshes into a Hierachy and also memory & refinement optimizations, you can check out the [2011 GDC about Starcraft II & Dragon Age Origins](https://www.gdcvault.com/play/1014514/AI-Navigation-It-s-Not). 
 
 >**Important Note**: Navigation Meshes is  a very cool & interesting topic to explore, but in this research I want to focus more in the basics of the Hierarchial Pathfinding, since, I am a big believer that, if you understand the basics (ergo, what I am about to explain) who will know how the Navigation Maps & Meshes work way faster &, also, since you will know what is happening in the lowest level, you will also understand the optimizations or came up with your own!

## My Approach - HPA*
The approach I selected is to do a **Regular Multi-Level Hierarchy A***. The reason is simple and I have already mentioned before: I want this to be an explanation so you will understand how & why a Hiearchial Pathfinding works, so let's start from the basic, shall we?

### Code Structures
#### The Abstract Graph structure:
 
 ```cpp
struct HPAGraph
{
	//Graph Storage
	std::vector <std::vector<Cluster>> lvlClusters; //We will store all the Clusters of a level in this vector. Each level, will be an instance of the vector that will conain all the clusters
	std::vector <Entrance> entrances;
	std::vector <HierNode*> staticNodes;

	//PreProcessing
	void PrepareGraph();
	void CreateGraphLvl(int lvl);

	//Building Inner Structures: 
		//Build is for logic determination & creation of all the elements of the structure
		//Create is for the creation of a single object
	void BuildClusters(int lvl);

	void BuildEntrances();
	void CreateEntrance(Cluster* c1, Cluster* c2, ADJACENT_DIR adjDir, int lvl);

	void BuildInterNode(iPoint n1, Cluster* c1, iPoint n2, Cluster* c2, int lvl);
	void CreateInterNodes(int lvl);

	void CreateIntraNodes(int lvl);

	void CreateEdges(HierNode* from, HierNode* to, int lvl, EDGE_TYPE type);


	//Utility
	ADJACENT_DIR ClustersAreAdjacent(Cluster* c1, Cluster* c2, int lvl); //Checks if two clusters are adjacent
	HierNode* NodeExists(iPoint pos, Cluster* lvl = nullptr); //Checks if a Node exists in the whole graph or just in a concrete Cluster
	bool EdgeExists(HierNode* from, HierNode* to, int lvl, EDGE_TYPE type); // Check if an Edge between two Nodes Exist
	Cluster* DetermineCluster(iPoint nodePos, int lvl, Cluster* firstCheck = nullptr); //Determines the Cluster in whick a Node is in.

	//Node Insertion
	HierNode* insertNode(iPoint pos, int lvl, bool* toDelete = nullptr); //Insert a Node into the Graph
	void DeleteNode(HierNode* toDelete, int lvl); //Deletes a Node from the Graph
	void ConnectNodeToBorder(HierNode* node, Cluster* c, int lvl); //Connects a Node to all the Nodes in the same Cluster

};

```
  - Explanation:
    - As you can see, we will store in memory the **Clusters** for each level, the **Entrances** (we will have enttrances, since it is a regular graph) & the **Nodes of the Hiearchy** (The Nodes are stored here, but note that each cluster has a vector of pointers to its nodes, in order to have a better time searching)
    
    - Let's take a further look into the Containers Structures:
    
```cpp   
    struct Cluster
{
	Cluster();
	Cluster(int width, int height, iPoint& pos);
	Cluster(const Cluster& clust);

	iPoint pos;
	int width, height;

	std::vector <HierNode*> clustNodes;
};

```
  - The Cluster is simple, it's just stores it's position & size. But take note, each Cluster has a vector of the Nodes that are inside him; this is done in order to prevent two things:
    - This makes the code faster, since we don't have to check every node in the Graph, just to ones inside the same Cluster. (Note that, ethier way, we store the Nodes in a vector called StaticNodes, here we will store all the Nodes that are from the beginning & that cannot be changed (Ex. Static walls) because, if we where to insert a Node that can be modified (let's say a wall that you can tear down or that you can build) we would added ONLY to its Cluster Node Vector. This is done to have a faster check in the Node deletion process).
    - In order to have a multi-level search, the Nodes must store it's level. This makes the pathfinding go slower, since we have to instance one more variable. My work around for this is to have the clusters already separated in levels by their position in the vector of Cluster vectors (Ex. lvlClusters[N] stores all the Clusters in the N+1 level, being 0 the non-abstract level). This way we don't have all the Clusters & Nodes in the same place, but have a little structure.

    
```cpp   
  enum class ADJACENT_DIR
{
	DIR_NONE = -1,

	VERTICAL,
	LATERAL
};

struct Entrance
{
	Entrance(iPoint pos, int width, int height, ADJACENT_DIR dir);
	Entrance();

	iPoint pos;
	int width, height;

	ADJACENT_DIR dir;

};

```
  - The entrances are also quite simple; the same way  Clusters do, they store position and size, but, also, store the direction that they are connected (This could be LATERAL or VERTICAL).

 ```cpp   
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
	HierNode(float g, const iPoint& pos, PathNode* parent, int myDir, int parentdir, std::vector<Edge*> edges);

	float CalculateF(const iPoint& destination);
	uint FindWalkableAdjacents(std::vector<HierNode>& list_to_fill, int lvl);

	std::vector <Edge*> edges;
};
```
  - The Nodes (called HierNodes here) need a little bit more of an  explanation:
    - First of all we have the PathNodes, which are regular A* nodes that indicate a tile in the non-abstract level (with your typical A* stuff)
    - Then we have the Hiernodes, which are a class that inherits from PathNode; since, at the end, both are the same. There are two key diferences, though:
      - Some functions are overrided from the PathNode, since they are different (we will get to that later)
      - All the HierNodes store a vector of pointers of Edges (which are the connections between Nodes we discussed before).
      
  ```cpp   
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
```

  -  All right, the last of the major agents. The Edges are also simple. They just store its type (that remember, can be INTER o INTRA), the destination Node (which is the Node this connection points to) & the amount that Cost going to that Node (in A* terms, the g).
  
- The Code flow & Functions:

  -Basically the code flows like this:
  
 ```cpp   
  MapLoad()
  {
    HPAGraph absGraph;

	HPAPreProcessing(MAX_LEVELS);
  }


  void HPAPreProcessing(int maxLevel)
  {
    absGraph.PrepareGraph();

    for (int l = 2; l <= maxLevel; l++)
    {
      absGraph.CreateGraphLvl(l);
    }

  }

  void PrepareGraph()
  {

    BuildClusters(1);
    BuildEntrances();

    CreateInterNodes(1);
    CreateIntraNodes(1);
  }

  void CreateGraphLvl(int lvl)
  {
    BuildClusters(lvl);

    CreateInterNodes(lvl);
    CreateIntraNodes(lvl);
  }
  
```

  - Basically what we do here is to create the first level of the Graph: with it's Entrances, Clusters, Nodes & Edges. Then for subsequent levels we just create the Clusters, the Nodes & their Connections. [1]
  - Be noted that everything is calculated when we load a new map. [2]
  
  - For better illustration, let's simulate a Graph Construction step by step. Let's say we start from this map:
  

  
<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/Map.png"  width="60%" height="60%">
</p>

   > Being the white tiles walkable & the black ones non-walkable.
   
- Alright, let's now have a look at the Cluster Build Code & see how it affects our map:

```cpp   
  void BuildClusters(int lvl)
  {
    int clustSize = CLUSTER_SIZE_LVL * lvl;

    //Buffer vector
    vector <Cluster> clusterVector;

    Cluster c;

    //We will create the cluster in a row-column order
    for (int i = 0; i < mapWidth , i += clustSize)
    {

      if (i + clustSize > width)
        c.width = width - (i);
      else
        c.width = clustSize;

      for (int k = 0; k < mapHeight, k += clustSize)
      {

        if (k + clustSize > height)
          c.height = height - (k);
        else
          c.height = clustSize;

        c.pos = { i,k };
        clusterVector.push_back(Cluster(c));
      }
    }

    lvlClusters.push_back(clusterVector);
  }
  
 ```
 
 - This function is a simple double for that iterates the map & creates the Clusters but I want to point out one thing:
  - The approach I used to make the Clusters regular is define an arbitrary constant (CLUSTER_SIZE_LVL) & make each level so it's Clusters are this constant bigger than the previous ones. [2]
  
- Let's take a look at the Map & see how it has changed:

<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/Clusters.png"  width="60%" height="60%">
</p>

> As you can see, now the Map has divisions of Clusters that group tiles of the non-abstract level. Note that if we added another Graph Level, it would create another Clusters double the size.

- Let's take a closer look into the Structure Building pseudocode:

```cpp   
  void BuildEntrances()
  {
      for (each clust1, clust2 ∈ lvlClusters[0]) 
      {
        if ClustersAreAdjacent(clust1, clust2, 1)
         CreateEntrance(clust1, clust2, adjacentDir, 1);
      }
  }

```
 - We just iterate the clusters and find the adjacent ones; between them we will create the entrances. Check the CreateEntrances() method for more info, but, basically what it does is create an Entrance for each group of consecutive walkable tiles. [4]
 
 - Let's zoom-in at the top-left corner of the map and see how it has changed:
 
 
<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/Entrances_Draw.png"  width="60%" height="60%">
</p>
  
  > Great! The yellow rectangles represent the entrances between the Clusters, but something weird happened. Some Nodes are grey & have weird lines between them; let's figure out what this is!
  
```cpp  
  void CreateInterNodes(int lvl)
{
	for (each entrance)
	{
		switch (currEntrance->dir)
		{
		case ADJACENT_DIR::LATERAL:
		{
			c1 = DetermineCluster(currEntrance->pos, lvl);
			c2 = DetermineCluster({ currEntrance->pos.x + 1, currEntrance->pos.y }, lvl);

			if (ClustersAreAdjacent(c1, c2, lvl) != ADJACENT_DIR::LATERAL)
				continue;


			int maxSize = (currEntrance->pos.y + currEntrance->height);

			for (int i = currEntrance->pos.y; i < maxSize; i += NODE_MIN_DISTANCE)
			{
				BuildInterNode({ currEntrance->pos.x,i }, c1, { currEntrance->pos.x + 1, i }, c2, lvl);
			}

		}
		break;
		case ADJACENT_DIR::VERTICAL:
		{
			// Same as adove, different axis....
		}
		break;
		}

	}
}

void BuildInterNode(iPoint p1, Cluster* c1, iPoint p2, Cluster* c2, int lvl)
{

	n1 = NodeExists(p1, allTheGraph);

	if (!n1)
	{
		n1 = new HierNode(p1);
		staticNodes.push_back(n1);
	}

	if (NodeExists(p1, c1) == NULL)
		c1->clustNodes.push_back(n1);


	n2 = NodeExists(p2, allTheGraph);
	...


	CreateEdges(n1, n2, lvl, EDGE_TYPE::INTER);
}

void CreateIntraNodes(int lvl)
{
	for (each cluster in currlvl)
	{
		for (each pair of nodes in cluster)
		{
		CreateEdges(n1, cn2, lvl, EDGE_TYPE::INTRA);
		}
	}
}
 ```
  
   - This is how the Nodes are created:
   	-INTER: The simply get every Entrance & for an Arbritary Number that we define (NODE_MIN_DISTANCE) we put a Pair of Nodes in each side of the Entrance &, to indicate that they will be INTER Nodes, we build an Edge between them (Edges Explanation Next)
    	-INTRA: simply iterate through all the Nodes in a same Cluster & Connect them via A*.
	
```cpp
void CreateEdges(HierNode* n1, HierNode* n2, int lvl, EDGE_TYPE type)
{
	float distanceTo = 1.f;
	if (type == EDGE_TYPE::INTRA || !EdgeExists(n1, n2, lvl, type))
	{
		//If the connection if between same cluster nodes, we calculate the moving cost trough A*; otherwise  is 1
		if (type == EDGE_TYPE::INTRA)
		{

			distanceTo = SimpleAPathfinding(n1->pos, n2->pos).GetCost();
		}

		n1->edges.push_back(new Edge(n2, distanceTo, lvl, type));
	}

	//Repeat the process for the second node----
	if (type == EDGE_TYPE::INTRA || !EdgeExists(n2, n1, lvl, type))
	{
		if (type == EDGE_TYPE::INTRA)
		{
			if (distanceTo == 1)
				distanceTo = SimpleAPathfinding(n1->pos, n2->pos).GetCost();
		}

		n2->edges.push_back(new Edge(n1, distanceTo, lvl, type));
	}

}
	
```

   - Just a thing to point here; note that we check if the EdgeExists (& if so we don't make another one) just for the INTER edges, we always create the INTRA edges. This has an explanation:
   
   - For Multiple-Level Search: we have to Level Up the existent Edges. In order to indicate that they are from a different abstraction level (Edge struct has a variable lvl and EdgeExists automatically levels up the Edges) but there is a catch: we can only do this for INTER edges, since they are the same (because higher abstraction Clusters are just groups of lower-level clusters) but it's not the same case for the INTRA edges, since they can change; therefor, when we seach we can do it for INTER edges that are the same level or above (since, I repeat, are the same) but we must just search for INTRA nodes that are from the same level, since they can change; even though it's not guaranteed that they do.
   
   - Also, as a sidenote; instead of savingg up the Cost when calculating the Cost for the Edges, instead, you could just save the Path. These would save up a little of time in the Refinement Process but take up a whole lot of memory. **[5]**
	
> Back at our map, now this makes sense: the RED represents the Nodes, the GREN the Inter Edges & the BLUE the Intra Edges (they are calculated just in one Cluster)
	
<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/nodesAndEdges.png"  width="60%" height="60%">
</p>
  
#### Search Process

- With all the structures made Just lasts to do the Path:
	- Hierarchical Search:
	
```cpp
PATH_TYPE CreatePath(const iPoint& origin, const iPoint& destination, int maxLvl)
{
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
}
```

   - All right, this is how the function that prepares the Graph to do the pathfind from the Origin to the Destination. Let's go function by function and see how it operates. First of all, we have to insert the Origin & Destination in the Graph, in order for the pathfinding to occur (Quick Reminder: This is basically A* but instead of searching tile-per-tile, we are using as a possible pathfinding "positions to iterate" the nodes &, as adjacents, the connections they have, the Edges)
   
```cpp
HierNode* HPAGraph::insertNode(iPoint pos, int Lvl, bool* toDelete)
{

	HierNode* newNode = nullptr;
	Cluster* c;

	if (!App->pathfinding->IsWalkable(pos))
		return nullptr;

	//Determine the cluster
	c = DetermineCluster(pos, Lvl);
	if (!c)
		return nullptr;

	//Check if already exists
	newNode = NodeExists(pos, c);

	//If can be placed inside a cluster & there is no node already there, we create one
	if (!newNode)
	{
		newNode = new HierNode(pos);
		c->clustNodes.push_back(newNode);
		
		//Connects a recently introduced node to it's peers in the same cluster
		ConnectNodeToBorder(newNode, c, Lvl);
		
		*toDelete = true;
	}

	return newNode;
}
```

   - The Node insertion is simple, we exit if the Node position is non-walkable or it doesn't belong to any cluster (this should never happen); then we check if the node is in a exact same position as another one already existent (in order to avoid repeating an insertion process we already made). If the Node doesn't exist, we create it, insert it to the Node Vector of the Cluster & we create the Intra Edges to the Nodes in the same Clusters (ConnectToBorder() ).


```cpp
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
```
   - This method is no different from the Intra Nodes connection we have checked before, but it has one key difference: we first do a quick RayCast check to connect it to its Neightbors before doind the A*; this is because it's way faster & gives more than okay results.


> Following the Map example it would look something like this (being the O & D the Origin and Destination respectively & the Yellow lines the Edges we just create using the ConnectToBorder() method):
	
<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/nodesAndEdgesWithG%26D.png"  width="60%" height="60%">
</p>
  
 > Remember that the images only has the Intra Edges for one Cluster! It should have that for every Cluster of the Graph.
  
  - Since the Deltion Method is trivial, we are going to check out the actual algorithm:
  
```cpp  
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

uint HierNode::FindWalkableAdjacents(std::vector<HierNode>& list_to_fill, int lvl)
{
	int edgeNum = edges.size();
	HierNode* currNode = nullptr;
	Edge* currEdge = nullptr;


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
			list_to_fill.push_back(HierNode(currEdge->moveCost + this->g, currNode->pos, this, myDirection, 0, currNode->edges));
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

```
   - As you can see, this is basically A*. The only difference in that while searching for the Node adjacents, we grab the Nodes that our edges point to, being the cost to move the one we previously calculated with A*.
    - One remarkable thing is that we used OctileDistance to calculate the h; I used this because it allows diagonals & is fastly computed. If you want to know more about Pathfinding Distance Heuristics I encourage you to check the [Red Blob Games article about them](https://theory.stanford.edu/~amitp/GameProgramming/Heuristics.html#euclidean-distance-squared).

#### Refinemt & Smooth Process

> Finishing the Hierarchial Path, we would have a path that would look something like this:
	
<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/abstractPath.png"  width="60%" height="60%">
</p>
  
  - But this is not usable, we have to refine it. My approach for the refinement has been quite inusual; instead of refining all the Path when we calculate it, I made the agents that pathfind request a path every time something happens (say the path they currently have is too small, a certain amount of time has passed, etc.). I think this approach is usable from a videogame perspective, since we will just calculate the path when needed & the player wil not even notice. 
  
  - The code looks something like this (I have not included all the logic behind requesting, just the refinement function; if you want to know about it, ask me directly or check the  [Full Code](https://github.com/AlexMelenchon/Hierarchial-Pathfinding-Research/tree/master/full_code):
  
  ```cpp   
bool ModulePathfinding::RefineAndSmoothPath(std::vector<iPoint>* abstractPath, int lvl, std::vector<iPoint>* pathToFill)
{
	iPoint currPos = { -1, -1 };
	iPoint startPos = { -1, -1 };

	int from = -1;
	Cluster* startCluster = nullptr;

	int pathSize = abstractPath->size();

	for (int i = 0; i < pathSize; i)
	{
		currPos = abstractPath->at(i);

		//Grab the first node
		if (startPos.x == -1)
		{
			startPos = currPos;
			from = i;
			startCluster = absGraph.DetermineCluster(startPos, lvl);
			i++;
			continue;
		}

		//Two Conditions to make path:
			//Check that Distance is not greater than Cluster Size
			//Not be the last node

		if (startCluster != absGraph.DetermineCluster(currPos, lvl) || (i == pathSize - 1 && pathSize > 0))
		{

			std::vector <iPoint>* generatedPath = nullptr;

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
					/// If our code for following the path is solid this isn't necessary but it's nice to have
				if (pathToFill->size() > 0)
					pathToFill->insert(pathToFill->end(), generatedPath->begin() + 1, generatedPath->end());
				else
					pathToFill->insert(pathToFill->end(), generatedPath->begin(), generatedPath->end());
			}


			//Delete the abstract nodes we just refined
			abstractPath->erase(abstractPath->begin() + from, abstractPath->begin() + i);

			return true;
		}
		//---
		else
		{
			abstractPath->erase(abstractPath->begin() + 1);
			pathSize--;
			continue;
		}

	}

	return false;
}
```
  
   - Here we check the Abstract Path we calculate, get the first position available  and from there we check if the next position is on another Cluster or it is the last one of the Abstract Path, if it is not, we delete it from the path and if so we refine it (These are "regular" conditions, you change them if you want but I recommend this for general purporses but, please, change them according to your needs).
      - The process of refinement is quite simple, we just run our algorithm between the positions we determined, delete those positions from the Abstract Path & then return the refined part. But, in order to Smooth it and make it faster, I added that, if the positions we are trying to get to is visible for the Starter one we just make a line (a simple 2D Raycast); this makes the path more accurate & the refinement faster (same as above, you could get creative here; you could check all the nodes with raycast, refine more than 2 positions, etc.)
      
> And that is it! Our path would look something like this:
	
<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/PathFullyRefined.png"  width="60%" height="60%">
</p>      

   > IMPORTANT: this is not the Full Code, if you want to know more about it, I encourage you to take a look at the [Full Code](https://github.com/AlexMelenchon/Hierarchial-Pathfinding-Research/tree/master/full_code) or do the TODO's listed below.

    
    
### The Results
   - To better understand the results, first of all we have to know the worst computational cost of both the A* & the HPA*:
   

	
| A*    | HPA*       |
|-------|------------|
| O (N) | O( log(N)) |


   > Being N the length of the path to check. I got this information from the [GDC Hierarchical Dynamic Pathfinding presentation I mentioned before](https://twvideo01.ubm-us.net/o1/vault/gdc2018/presentations/Alain_Benoit_HierarchicalDynamicPathfinding.pdf)

   - Now into the Results, this was computed on a i5-4960K with 8GB of DDR3 1333Mhz running on a 64bit Windows 10 on a RELEASE:


|                 | Computation Time                             | Solution Length |
|-----------------|----------------------------------------------|-----------------|
| A*              | 7.3ms                                        | 126             |
| HPA* ( on avg.) | 1.18ms (0.2ms to Insert + 0.9ms to Pathfind) | 34              |



   - These are a handful of results for my approach, if you want to check more results you there you have a graphic from [Near Optimal Hierarchical Path-Finding paper](http://webdocs.cs.ualberta.ca/~kulchits/Jonathan_Testing/publications/ai_publications/jogd.pdf):

   
   <p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/HpaVsA.png"  width="60%" height="60%">
</p>      
   
   - There are more interesting metric in the Paper, but here is a quick summary:
   
	  - From these we can conclude that, the more larger the path to compute the more the HPA* takes advantage on regualar A*. 
	  - Multiple levels of abstraction help, but they are useful just when you are dealing with very large paths. Since the bigger the level, the less nodes you will have to explore, but the more expensive will the Connect to Border process be.
	  - HPA* is way faster than A*, but, be careful, it takes a chunk of memory to work.
	  - Can be combined with Time Slicing &  other improvement to the A* that change the algorithm. 
	  - If you play around with the RayCast and the RefineAndSmooth(), you can get near-optimal path quality.
	  
## TODO's
With all the explanation out of the window, we are now ready to complete the TODO's. These are small exercises guided by code comments that are intended to taught you about the HPA* in a more practical sense. I'll walk you through all the process explained above so you can fully understand the implementation. Once you finish all the TODO’s you will have a fully functional HPA*.

> IMPORTANT NOTE: All the places where you have to write code are marked with " //----- ".

> > IMPORTANT NOTE: To check all the debug functionallities, please check the README inside the project, thanks!

### TODO 0
- Not much to say about this one; if you are familiar with A*, just look around the code & check how everything is wired up in order to not get lost in the next TODO's!

### TODO 1
- Here you just have to uncomment the two lines down the comment. Not much, to do here but the sauce is found in the next two sub-divisions:

#### TODO 1.1
- The clusters are created regulary here.The code to calculate how big & where the clusters are is done, just add it to the buffer vector (*1*) & then add it to the  vector of clusters vectors (ask me about this :P) we have already declared (*2*)

```cpp   
void HPAGraph::BuildClusters(int lvl)
{
	//TODO 1.1: The clusters are created regulary here.
	// The code to calculate how big & where the clusters are is done, just add it to the buffer vector (*1) & then add it to the
	// vector of clusters vectors (ask me about this :P) we have already declared (*2)

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
			//------ (*1)

		}
	}

	//Introduce the vector to the main one
	//------ (*2)

}
}

```


- Solution:

```cpp   
void HPAGraph::BuildClusters(int lvl)
{
	//TODO 1.1: The clusters are created regulary here.
	// The code to calculate how big & where the clusters are is done, just add it to the buffer vector (*1) & then add it to the
	// vector of clusters vectors (ask me about this :P) we have already declared (*2)


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
			//------ (*1)
			clusterVector.push_back(Cluster(c));
		}
	}

	//Introduce the vector to the main one
	//------ (*2)
	this->lvlClusters.push_back(clusterVector);
}

```

- Expected Result:
	- When pressing F2, the groups of tiles named Clusters should appear in your screen like this:
	
<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/gifs/TODO1.1.gif"  width="60%" height="60%">
</p>

#### TODO 1.2
- Here we iterate through all the clusters; just check if a pair of clusters are adjacents & if so create an entrance between them. Check out the functions ClustersAreAdjacent() & CreateEntrance()

```cpp   
void HPAGraph::BuildEntrances()
{
	Cluster* c1 = nullptr;
	Cluster* c2 = nullptr;
	ADJACENT_DIR adjacentDir = ADJACENT_DIR::DIR_NONE;

	// Since the Entrances are just created in the lowest abstraction LVL, their lvl is always 0
	int EntranceLvl = 1;

	//TODO 1.2: Here we iterate through all the clusters; just check if a pair of clusters are adjacents & if so create an entrance between them
	// Check the functions ClustersAreAdjacent() & CreateEntrance()

	//We simply iterate though the clusters & check if they are adjacent, if so we put an entrance between them
	for (uint i = 0; i < this->lvlClusters[0].size(); ++i)
	{
		c1 = &this->lvlClusters[0].at(i);

		for (uint k = i + 1; k < this->lvlClusters[0].size(); ++k)
		{
			c2 = &this->lvlClusters[0].at(k);

			//----

		}
	}
}


```


- Solution:

```cpp   

void HPAGraph::BuildEntrances()
{
	Cluster* c1;
	Cluster* c2;
	ADJACENT_DIR adjacentDir = ADJACENT_DIR::DIR_NONE;

	// Since the Entrances are just created in the lowest abstraction LVL, their lvl is always 0
	int EntranceLvl = 1;

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
			adjacentDir = ClustersAreAdjacent(c1, c2, EntranceLvl);

			if (adjacentDir != ADJACENT_DIR::DIR_NONE)
				CreateEntrance(c1, c2, adjacentDir, EntranceLvl);

		}
	}
}

```
- Expected Result:
	- When pressing F1, the entrances between the previous Cluster should appear like this:
	
 <p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/gifs/TODO1.2.gif"  width="60%" height="60%">
</p>

### TODO 2
- Same as TODO 1, just uncomment the two lines down the comment. Not much, to do here but the sauce is found in the next two sub-divisions:

#### TODO 2.1
- Here the Nodes between Clusters are created. Just check how lateral is created & then do the vertical one (the logic is the same, you just have to change the axis). Be careful with the position!

```cpp   
void HPAGraph::CreateInterNodes(int lvl)
{
	Entrance* currEntrance;
	Cluster* c1, * c2;

	//TODO 2.1: Here the Nodes between Clusters are created.
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

		}
		break;
		}

	}
}

```


- Solution:

```cpp   
void HPAGraph::CreateInterNodes(int lvl)
{
	Entrance* currEntrance = nullptr;
	Cluster* c1, * c2;

	//TODO 2.1: Here the Nodes between Clusters are created.
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

		}
		break;
		}

	}
}

```
- Expected Result:
	- When pressing F3, the Nodes connecting the Clusters should appear in BOTH AXIS, just like this:
	
   <p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/gifs/TODO%202.1.gif"  width="60%" height="60%">
	</p>


#### TODO 2.2
- Here we iterate through all the clusters; just check if a pair of clusters are adjacents & if so create an entrance between them. Check out the functions ClustersAreAdjacent() & CreateEntrance()

```cpp   
void HPAGraph::CreateIntraNodes(int lvl)
{
	Cluster* clusterIt;
	float distanceTo = 0;
	int clusterLevel = lvl - 1;
	int edgeLevel = lvl;

	HierNode* n1 = nullptr;
	HierNode* n2 = nullptr;

	if (clusterLevel > lvlClusters.size() || lvlClusters.empty())
		return;

	//TODO 2.2: Here we iterate trough all the nodes in a same cluster for all the cluster in a same level
	// Simply call the function to create edges between the different nodes with the INTRA type
	// Check the function CreateEdges()

	for (int i = 0; i < lvlClusters[clusterLevel].size(); i++)
	{
		clusterIt = &lvlClusters[clusterLevel].at(i);

		for (int y = 0; y < clusterIt->clustNodes.size(); y++)
		{
			n1 = clusterIt->clustNodes[y];
			for (int k = y + 1; k < clusterIt->clustNodes.size(); k++)
			{
				n2 = clusterIt->clustNodes[k];

				//--
				
			}
		}
	}
}

```


- Solution:

```cpp   
oid HPAGraph::CreateIntraNodes(int lvl)
{
	Cluster* clusterIt;
	float distanceTo = 0;
	int clusterLevel = lvl - 1;
	int edgeLevel = lvl;

	HierNode* n1 = nullptr;
	HierNode* n2 = nullptr;

	if (clusterLevel > lvlClusters.size() || lvlClusters.empty())
		return;

	//TODO 2.2: Here we iterate trough all the nodes in a same cluster for all the cluster in a same level
	// Simply call the function to create edges between the different nodes with the INTRA type
	// Check the function CreateEdges()

	for (int i = 0; i < lvlClusters[clusterLevel].size(); i++)
	{
		clusterIt = &lvlClusters[clusterLevel].at(i);

		for (int y = 0; y < clusterIt->clustNodes.size(); y++)
		{
			for (int k = y + 1; k < clusterIt->clustNodes.size(); k++)
			{
				//--
				CreateEdges(clusterIt->clustNodes[y], clusterIt->clustNodes[k], edgeLevel, EDGE_TYPE::INTRA);

			}
		}
	}
}


```
- Expected Result:
	- When pressing F4 (with the Nodes viewable (press F3)), all the connections between Nodes, the Edges, should appear like this:
	
<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/gifs/TODO2.2.gif"  width="60%" height="60%">
</p>
	
	
### TODO 3
- Noww all the Graph processing work is Done! Now we will move into the Pathfinding function itself. First of all, we have to introduce the Nodes into the Graph in order to be able to perform HPA*.

-This is simple, if the node exist in the cluster we do not do anything. If it doesn't exist, create it: you'll have to push it in the cluster Node Vector & connect it t the other nodes in the same cluster (Check ConnectNodeToBorder()).

```cpp   
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


	//TODO 3: This is simple, if the node exist in the cluster we do not do anything 
	// If it doesn't exist, create it: you'll have to push it in the cluster Node Vector (ClustNodes) & connect it t the other nodes in the same
	// cluster (Check ConnectNodeToBorder()).


	//Check if already exists
	newNode = NodeExists(pos, c);


	//If can be placed inside a cluster & there is no node already there, we create one
	if (!newNode)
	{
		//----

		*toDelete = true;
	}

	//---

	return newNode;
}


```


- Solution:

```cpp   

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

	//TODO 3: This is simple, if the node exist in the cluster we do not do anything 
	// If it doesn't exist, create it: you'll have to push it in the cluster Node Vector (ClustNodes)  & connect it t the other nodes in the same
	// cluster (Check ConnectNodeToBorder()).

	//Check if already exists
	newNode = NodeExists(pos, c);

	//If can be placed inside a cluster & there is no node already there, we create one
	if (!newNode)
	{
		newNode = new HierNode(pos);
		c->clustNodes.push_back(newNode);
		ConnectNodeToBorder(newNode, c, Lvl);
		*toDelete = true;
	}

	return newNode;
}

```
- Expected Result:
	- Proceed to the next TODO to check it visually!
	
### TODO 4
- et's find the walkable node adjacents. We have to iterate though all the HierNode's edges. But that is already done, you just have to Create new Hiernodes & Insert them into the list. The Hiernodes will we determined by the Edges movement Cost & it's destination Node (Take a look at the Edge Struct). Remeber that the G has to be calculated when the Hiernode is inserted!

```cpp   
uint HierNode::FindWalkableAdjacents(std::vector<HierNode>& list_to_fill, int lvl)
{
	int edgeNum = edges.size();
	HierNode* currNode = nullptr;
	Edge* currEdge = nullptr;


	// TODO 4: Let's find the walkable node adjacents. We have to iterate though all the HierNode's edges 
	// But that is already done, you just have to Create new Hiernodes & Insert them into the list
	//The Hiernodes will we determined by the Edges movement Cost & it's destination Node (Take a look at the Edge Struct)
	//Remeber that the G has to be calculated when the Hiernode is inserted!
	
	for (int i = 0; i < edgeNum; i++)
	{
		currEdge = edges[i];

		//LOOK OUT!! Checks if the edge has the correct level to put it in the list (Ask me about this :P):
			//INTRA: have to be from the same level
			//INTER: can be from the same level or superior
		if (currEdge->type == EDGE_TYPE::INTRA && currEdge->lvl == lvl || currEdge->type == EDGE_TYPE::INTER && currEdge->lvl >= lvl)
		{
			//----

		}

	}

	return list_to_fill.size();
}
}


```


- Solution:

```cpp   

uint HierNode::FindWalkableAdjacents(std::vector<HierNode>& list_to_fill, int lvl)
{
	int edgeNum = edges.size();
	HierNode* currNode = nullptr;
	Edge* currEdge = nullptr;

	// TODO 4: Let's find the walkable node adjacents. We have to iterate though all the HierNode's edges 
	// But that is already done, you just have to Create new Hiernodes & Insert them into the list
	//The Hiernodes will we determined by the Edges movement Cost & it's destination Node (Take a look at the Edge Struct)
	//Remeber that the G has to be calculated when the Hiernode is inserted!


	//Iterate though all the node edges
	for (int i = 0; i < edgeNum; i++)
	{
		currEdge = edges[i];

		//Checks if the edge has the correct level to put it in the list
			//INTRA: have to be from the same level
			//INTER: can be from the same level or superior
		if (currEdge->type == EDGE_TYPE::INTRA && currEdge->lvl == lvl || currEdge->type == EDGE_TYPE::INTER && currEdge->lvl >= lvl)
		{
			//----
			currNode = currEdge->dest;
			list_to_fill.push_back(HierNode(currEdge->moveCost + this->g, currNode->pos, this, myDirection, 0, currNode->edges));
		}

	}


	return list_to_fill.size();
}

```
- Expected Result:
	- If you click in two Walkable Points (with the Left Click of the mouse), you should see the abstract path reflected on the map, just like this:
	
 <p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/gifs/TODO3%264.gif"  width="60%" height="60%">
</p>

### TODO 5
- This is a big one. First of all read the whole function & understand what is happening

#### TODO 5.1
-  Currently the code just does nothing with the hierchial path, what you have to do is set the conditions to refine the path & actually refined. The conditions are: 
	-Check that the current position is from a diferent cluster than the Start pne OR
	-I t is the last node in the Abstract Path

#### TODO 5.2
-  Then you will have to refine the Path, we have two ways to do this:
	- RayCasting: Very fast, doesn't work through obstacles
	- Direct A*: slower but can make whatever path you send to it.
- The ideal situation would be to check with a RayCast first & if not succesfull make an A*
- To insert the path, check the functions on the std::vector (Insert() might be usefull :P)
			
#### TODO 5.3:
-  Do not forget to delete the nodes from the hierchial path after you have refined them Check the function on the std::vector erase()

> This TODO is divided in three parts to better understanding, but, since it's all in the same function here you will find it as one.

```cpp   
bool ModulePathfinding::RefineAndSmoothPath(std::vector<iPoint>* abstractPath, int lvl, std::vector<iPoint>* pathToFill)
{
	BROFILER_CATEGORY("Refine And Smooth Path", Profiler::Color::RosyBrown);

	iPoint currPos = { -1, -1 };
	iPoint startPos = { -1, -1 };

	int from = -1;
	int pathSize = abstractPath->size();

	Cluster* fromC = nullptr;

	for (int i = 0; i < pathSize; i)
	{
		currPos = abstractPath->at(i);

		//Get the first node
		if (startPos.x == -1)
		{
			startPos = currPos;
			from = i;
			fromC = absGraph.DetermineCluster(startPos, lvl);
			i++;
			continue;
		}

		//TODO 5: This is a big one. First of all read the whole function & understand what is happening

		//TODO 5.1: Currently the code just does nothing with the hierchial path, what you have to do is set the 
				// conditions to refine the path & actually refined. 
				// The conditions are: 
					//Check that the current position is from a diferent cluster than the Start pne OR
					// It is the last node in the Abstract Path
		//----
		if (true)
		{
			//TODO 5.2: Then you will have to refine the Path, we have two ways to do this:
							//RayCasting: Very fast, doesn't work through obstacles
							// Direct A*: slower but can make whatever path you send to it.
						//The ideal situation would be to check with a RayCast first & if not succesfull make an A*
						//To insert the path, check the functions on the std::vector (Insert() might be usefull :P)
			//-----


			// TODO 5.3: Do not forget to delete the nodes from the hierchial path after you have refined them!
			// Check the function on the std::vector erase()
			//------

			return true;
		}
		else
		{
			abstractPath->erase(abstractPath->begin() + 1);
			pathSize--;
			continue;
		}

	}

	return false;
}

```


- Solution:

```cpp   
bool ModulePathfinding::RefineAndSmoothPath(std::vector<iPoint>* abstractPath, int lvl, std::vector<iPoint>* pathToFill)
{
	BROFILER_CATEGORY("Refine And Smooth Path", Profiler::Color::RosyBrown);

	iPoint currPos = { -1, -1 };
	iPoint startPos = { -1, -1 };

	int from = -1;
	Cluster* startCluster = nullptr;

	int pathSize = abstractPath->size();

	for (int i = 0; i < pathSize; i)
	{
		currPos = abstractPath->at(i);

		//Grab the first node
		if (startPos.x == -1)
		{
			startPos = currPos;
			from = i;
			startCluster = absGraph.DetermineCluster(startPos, lvl);
			i++;
			continue;
		}

		//TODO 5: This is a big one. First of all read the whole function & understand what is happening

		//TODO 5.1: Currently the code just does nothing with the hierchial path, what you have to do is set the 
				// conditions to refine the path & actually refined. 
				// The conditions are: 
					//Check that the current position is from a diferent cluster than the Start pne OR
					// It is the last node in the Abstract Path

		//Two Conditions to make path:
			//Check that Distance is not greater than Cluster Size
			//Not be the last node

		//----
		if (startCluster != absGraph.DetermineCluster(currPos, lvl) || (i == pathSize - 1 && pathSize > 0))
		{

			//TODO 5.2: Then you will have to refine the Path, we have two ways to do this:
							//RayCasting: Very fast, doesn't work through obstacles
							// Direct A*: slower but can make whatever path you send to it.
						//The ideal situation would be to check with a RayCast first & if not succesfull make an A*
						//To insert the path, check the functions on the std::vector (Insert() might be usefull :P)
			//-----

			std::vector <iPoint>* generatedPath = nullptr;

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
					/// If our code for following the path is solid this isn't necessary but it's nice to have
				if (pathToFill->size() > 0)
					pathToFill->insert(pathToFill->end(), generatedPath->begin() + 1, generatedPath->end());
				else
					pathToFill->insert(pathToFill->end(), generatedPath->begin(), generatedPath->end());
			}

			// TODO 5.3: Do not forget to delete the nodes from the hierchial path after you have refined them!
			// Check the function on the std::vector erase()
			//------
			//Delete the abstract nodes we just refined
			abstractPath->erase(abstractPath->begin() + from, abstractPath->begin() + i);

			return true;
		}
		//---
		else
		{
			abstractPath->erase(abstractPath->begin() + 1);
			pathSize--;
			continue;
		}

	}

	return false;
}

```
- Expected Result:
	- When you make an Abstract Path in the Map, if you press down SPACE, you should see how it refines itself gradually (one press = one refinement):
	
 <p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/gifs/TODO%205.gif"  width="60%" height="60%">
</p>

- **And congratulations, you have created an HPA* algorithm!! The following TODO's are optinal, but nice to have, although are specific & may or may not fit your project:**

### TODO 6
- This one is going to be about entities and how they HPA*. First of all let's uncomment a very simple code that spawns entities in the mouse position:

```cpp   
	//TODO 6 (EXTRA): Let's have entities & make them HPA*. Uncomment the code

	//if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	//{
	//	int x, y;
	//	App->input->GetMousePosition(x, y);
	//	iPoint mousePos = App->render->ScreenToWorld(x, y);

	//	if (App->pathfinding->IsWalkable(App->map->WorldToMap(mousePos.x, mousePos.y)))
	//		App->entMan->AddNewEntity(ENTITY_TYPE::DYNAMIC, { (float)mousePos.x, (float)mousePos.y });
	//}
```


- Solution:

```cpp   
	//TODO 6.0: Let's have entities & make them HPA*. Uncomment the code
	//----
	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{
		int x, y;
		App->input->GetMousePosition(x, y);
		iPoint mousePos = App->render->ScreenToWorld(x, y);

		if (App->pathfinding->IsWalkable(App->map->WorldToMap(mousePos.x, mousePos.y)))
			App->entMan->AddNewEntity(ENTITY_TYPE::DYNAMIC, { (float)mousePos.x, (float)mousePos.y });
	}
	//----

```
- Expected Result:
	- When you press 1 on the keyboard & have the mouse placed in a walkable tile, a rectangle that represents an entity should appear just like this:
	
 <p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/gifs/TODO%206.gif"  width="60%" height="60%">
</p>

#### TODO 6.1
-  Now you can select units & make them pathfind but there is no refinement going on... Let's change that! 
- With everything set up, we just have to call the RequestPath() function and fill the entity's path. We can do this every frame, with a time condition, with a path legnth conditio...n; whatever fits your project!
	
```cpp   
bool Entity::Move(float dt)
{

	fPoint pathSpeed = { 0,0 };
	fPoint nextPoint = { 0,0 };

	//TODO 6.1 (EXTRA): With everything set up, we just have to call the RequestPath() function and fill the entity's path
		// We can do this every frame, with a time condition, with a path legnth conditio...n; whatever fits your project!
	//----

	if (path.size() > 0)
	{
	....
```


- Solution:

```cpp   
bool Entity::Move(float dt)
{
	//Speed is resetted to 0 each iteration
	// ----------------------------------------------------------------

	fPoint pathSpeed = { 0,0 };
	fPoint nextPoint = { 0,0 };


	//TODO 6.1 (EXTRA): With everything set up, we just have to call the RequestPath function and fill the entity's path
	// We can do this every frame, with a time condition, with a path legnth conditio...n; whatever fits your project!
	//----
	if (path.size() < 2)
	{
		App->pathfinding->RequestPath(this, &path);
	}
	//----


	if (path.size() > 0)
	{
	...

```

- Expected Result:
	- As you can see, I have set a condition to do the path; whenever the unit sees that his path is less than 2 tiles long, it makes another request. You can change this to whatever you want!
	- You can select the entities by keeping the Left Click of the mouse & dragging across the screen. This will create a green rectangle, whatever unit is inside the rectangle will be selected & marked in red. Then, if you press the right click anywhere walkable in the map, they should move to that destination, just like this
	
 <p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/gifs/TODO%206.1.gif"  width="60%" height="60%">
</p>

#### TODO 6.2
-  This is the same as above but placed just below the Abstract Path calculation. This is to make sure that your unit starts pathfinding in the same frame as you path; this is not really necessary if your move code is solid but it never hurts to have it!

```cpp   
bool Entity::GeneratePath(int x, int y, int lvl)
{
	iPoint goal = { 0,0 };

	origin = App->map->WorldToMap((position.x ), (position.y ));
	goal = { x,y };

	if (App->pathfinding->CreatePath(origin, goal, lvl, this) != PATH_TYPE::NO_TYPE)
	{
		path.clear();

		//TODO 6.2 (EXTRA): This is optional, but it's nice to call a single RequestPath() when you generate the path
		//----


		if (!path.empty())
			path.erase(path.begin());

		return true;
	}

	return false;
}
```


- Solution:

```cpp   
bool Entity::Move(float dt)
{
	//Speed is resetted to 0 each iteration
	// ----------------------------------------------------------------

	fPoint pathSpeed = { 0,0 };
	fPoint nextPoint = { 0,0 };


	//TODO 6.1 (EXTRA): With everything set up, we just have to call the RequestPath function and fill the entity's path
	// We can do this every frame, with a time condition, with a path legnth conditio...n; whatever fits your project!
	//----
	if (path.size() < 2)
	{
		App->pathfinding->RequestPath(this, &path);
	}
	//----


	if (path.size() > 0)
	{
	...

```

- Expected Result:
	- Same as 3.1; just a little bit more consistent in code!
			
### TODO 7
-  As you might noticed, we are just doind a 1 level search since now, let's change that. Just uncomment the code and go to the define MAX_LEVELS to set how many levels you want. I encourage you to play around the CLUSTER_SIZE_LVL &  NODE_MIN_DISTANCE too!
```cpp   
	//TODO 7 (EXTRA): If you want more abstraction levels, just uncomment this 
	// Just make sure the MAX_LEVELS define is above 1 & the TODO 4 is done correctly

	//for (int l = 2; l <= maxLevel; l++)
	//{
		//absGraph.CreateGraphLvl(l);
	//}

```


- Solution:

```cpp   
	//TODO 7 (EXTRA): If you want more abstraction levels, just uncomment this 
	// Just make sure the MAX_LEVELS define is above 1 & the TODO 4 is done correctly

	//Creates the rest of the graph levels
	for (int l = 2; l <= maxLevel; l++)
	{
		absGraph.CreateGraphLvl(l);
	}


```
- Expected Result:
	- Now, if you press the F2, F3 & F4 to see the Clusters, Nodes & Edges and, then press the Arrow Keys Up & Down you will see how the abstraction levels change (when you pathfind you do it in that level too!). This example is just changing the MAX_LEVELS to 3:
	
 <p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/gifs/TODO%207.gif"  width="60%" height="60%">
</p>

### Homework
- As a little of extra work I encourage you to do the following:
	- Adapt this to your project!
	- Play around with the values (levels, cluster size, etc.) and with the Refinement Method
	- Make it dynamic (Check the posible improvements [6] )

## Possible Improvements & Changes
- **[1]** : a possible improvement is to, instead of creating the Clusters for the next levels, is to group the Clusters from the previous level in groups of N Clusters.
- **[2]**: another option is to pre-calculate all of this and then load it when the map is calculated (this includes Entrances, Clusters, Nodes & Edges); but since our project is small, I decided to do it this way.
- **[3]**:  you could have a different approach in how you make the Clusters size (irregular is always an option) but you can have it be x2 larger than the previous one, etc.
- **[4]**:  This is inspired by the HNA* method; which basically makes the Clusters moldable to terrain, not the Entrances. If you want to know more check out [THIS](https://web.archive.org/web/20190725152735/http://aigamedev.com/open/tutorials/clearance-based-pathfinding/)
- **[5]**: instead of savingg up the Cost when calculating the Cost for the Edges, instead, you could just save the Path. These would save up a little of time in the Refinement Process (since, instead of using RayCast or the A*; you would just insert the path already calculated) but take up a whole lot of memory.
- **[6]**:  Currently, the code does not support for dynamic obstacles to be placed; in order to do this, you would haveto detect in which Clusters the dynamic object has been placed, "block" the Nodes that overlap with it & re-compute all the edges inside those Clusters. If you want to know more, check out [THIS](http://webdocs.cs.ualberta.ca/~kulchits/Jonathan_Testing/publications/ai_publications/jogd.pdf)

## References and Extra Content

- [Red Blob Games' A* Introduction](https://www.redblobgames.com/pathfinding/a-star/introduction.html)
 - [Using the Hierarchical Pathfinding A* Algorithm in GIS to Find Paths through Rasters with Nonuniform Traversal Cost](file:///E:/Descargas/Using_the_Hierarchical_Pathfinding_A_Algorithm_in_.pdf)
- [Another Approach to Regular HPA* Code](https://docs.rs/hierarchical_pathfinding/0.3.0/hierarchical_pathfinding/)
- [Improvements to Hierarchical Pathfinding for Navigation Meshes](https://www.cs.upc.edu/~npelechano/MIG2017_Rahmani.pdf)
- [Hierarchical Pathfinding in the gdxAI framework ](https://github.com/libgdx/gdx-ai/wiki/Hierarchical-Pathfinding)
- [HPA* for navigation meshes](https://www.sciencedirect.com/science/article/abs/pii/S0097849316300668)
- [Near Optimal Hierarchical Path-Finding](http://webdocs.cs.ualberta.ca/~kulchits/Jonathan_Testing/publications/ai_publications/jogd.pdf)
- [Rust game development Ep. 27: Hierarchical Pathfinding and UI work](https://www.youtube.com/watch?v=qSbSb8vMbLI) by [Alexandru Ene](https://twitter.com/_AlexEne_)
- [Hierarchical Pathfinding and AI-Based Learning Approach in Strategy Game Design](https://www.hindawi.com/journals/ijcgt/2008/873913/)
- [Hierarchical Path-Finding Theta*](https://www.cs.ru.nl/bachelors-theses/2013/Linus_van_Elswijk___0710261___Hierarchical_Path-Finding_Theta_star_Combining_HPA_star_and_Theta_star.pdf)
- [Hierarchical Dynamic Pathfinding Implementation for Castle Story](https://twvideo01.ubm-us.net/o1/vault/gdc2018/presentations/Alain_Benoit_HierarchicalDynamicPathfinding.pdf)
- [Killzone 2 Multiplayer Bots' Hierarchical AI](https://drive.google.com/open?id=177H0M2sSvUmjmTRGl6RBFTQIqm7edVUp)
- [Hierarchical AI for Multiplayer Bots in Killzone 3](http://www.gameaipro.com/GameAIPro/GameAIPro_Chapter29_Hierarchical_AI_for_Multiplayer_Bots_in_Killzone_3.pdf)
- [AI & Navigation Meshes in Starcraft II & Dragon Age Origins](https://www.gdcvault.com/play/1014514/AI-Navigation-It-s-Not)
- [Improvents to A* & Jump Point Search Algorithm](https://lucho1.github.io/JumpPointSearch/) by  [Lucho Suaya](https://www.linkedin.com/in/luchosuaya/)
- [Clearance-based Pathfinding and Hierarchical Annotated A* Search](https://web.archive.org/web/20190725152735/http://aigamedev.com/open/tutorials/clearance-based-pathfinding/)
