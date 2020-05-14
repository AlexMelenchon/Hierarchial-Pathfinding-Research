# Hierarchial Pathfinding-Research

I am [Àlex Melenchón](https://www.linkedin.com/in/alex-melench%C3%B3n-maza-a46981195/), student of the [Bachelor’s Degree in
Video Games by UPC at CITM](https://www.citm.upc.edu/ing/estudis/graus-videojocs/). This content is generated for the second year’s
subject Project 2, under supervision of lecturer [Marc Garrigó](https://www.linkedin.com/in/mgarrigo/).

You can take a look at the website with detauiled information about the research [HERE](https://github.com/AlexMelenchon/Hierarchial-Pathfinding-Research) & you can check the project [HERE](https://github.com/AlexMelenchon/Hierarchial-Pathfinding-Research)

## Definition & Introduction to the Problem - What is  Hierarchial Pathfinding?
The problem that Hierchial Pathfinding is trying to solve is that, in Videogames, most calculated paths are never fully walked; since most of the times something happens in the middle of the path that forces to change plans.

In order to achieve this what is used by other reference videogames is the map abstraction into a hierarchy with different levels representing groups of tiles from the lower levels (this is achieved by grouping tiles into Nodes, that can be regular or not).

Then is where Hierchial Pathfinding comes into play: since it plans the path in much the same way as people would. You plan an overview route first and then refine it as needed. For example: let's say you are in your house in Barcelona & you want to go visit Rome, you woudn't just raw calculate each foot to get there, you will go something as: leave the house - grab a Taxi - go to the Airport - etc. That is basically what Hierchial Pathfinding does, it makes a quick & abstract research of the key abstractuib-steps we calculated before to get from A to B &, when necessary it refines a simple path between the key steps.

The abstract plan is simple, and we split the pathfinding problem over a long period of time, only doing the next bit when the current bit is complete.

As I was saying, this is very usefull for Videogames, since most of the times knowing the first few moves of a valid path is enough, allowing the requester to start moving in the right direction. Subsequent events may result in the agent having to change its plan, obviating the need for the rest of the path. This has a huge advantage in performance, since we don't have to calculate all the path that will not be used.

Right of the bat. this sounds really good but there is one key disadvantage: this does not create optimal paths but, for real-time pathfinding applications such as Videogames, this is not a huge problem most of the times, provinding the path looks reasonable.

<p align="right">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/hpaGraph.jpg" >
</p>

This image of the Indie Developer [K Lodeman](https://twitter.com/ManOfLode/status/854406316890128384) shows how the Hierchy principe works; the huge map is divided into shorter steps in order to then refine the path.

## Ways of implementation
First of all, there aren't static religious ways to implement the Hierarchial Pathfinding (let's call it HP for now) because it's not an algorithm. We could say it's a "technique" to divide the map so the desired Pathfinding Algorithm has an easier time construction the abstract & refined paths.

### Regular Implementation

With this said, let's see the principles that the "hierchy graph must follow" in order to have an expandable & solid grid configuration (you can find the [Near Optimal Hierarchical Path-Finding paper](http://webdocs.cs.ualberta.ca/~kulchits/Jonathan_Testing/publications/ai_publications/jogd.pdf) this explanation is based on [HERE](http://webdocs.cs.ualberta.ca/~kulchits/Jonathan_Testing/publications/ai_publications/jogd.pdf)):

- **Agents**:

  - **Clusters**: these are basically groups of the lower-level tiles; the higher the level of abstraction of the graph, the bigger they are.
    
  - **Entraces**: these are created just with the lowest level of abstraction & they are basically bridges between the clusters that determine which clusters are walkably adjacents.

  - **Nodes**: they are the most crucial part of the hierarchy. Once the entrances are created, we must create Nodes in the spaces in which the entrances are placed. This will be determined not only by the Entrance position & size, but for an arbitrary number we will define that will determined how separated are the Nodes in one entrance.
 
   - **Edges**: another crucial part of the hierarchy. This can basically be defined as the conections between nodes. They We wil be the indicators for connection & store the movement cost between the nodes. dWe dstinguish two types of connections: between INTER & INTRA nodes.
      - INTER: these are the connections made between Nodes of different Clusters. Since they are adjacent, it's movement cost is always one.
       - INTRA: these are the connections made between nodes of the same cluster. The movement cost between them is determined by running the desired pathfinding algorith & getting it's computation cost.
     

   
<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/full_structuresExplanation.png" >
</p>

> Here you can see how the agents come into play to make the graph. In the image on the left, you can see the blue divisions represent the clusters & the red lines the created nodes in the entrances. Then, in the right, the Edges between the nodes are visible, with it's movement cost beign shown.


 - **General Methods**:
 
    - **Preparing the Graph**: this process has to be executed before the runtime of the game; it can be pre-calculated & stored in the game files or can also be calculated while the game map is loading. Basically this cretes the first abstraction level, which means, creating the Entrances, Cluster, Nodes & Edges for the level.
    
    - **Adding further Graph Levels**: with our first level created, we can add only higher abstraction levels on top of that but, for these  further abstraction levels, we will  reutilize the Entrances calculated for the first level, because the higher level clusters would be groups of lower level clusters. Then, since the entrances are the same, the nodes will be as well so we can add a variable level to our nodes to indicate the code that this node is from a higher level. We can also do it for the INTER edges as well, but, for the INTRA edges, we will have to calculate them again, since we need the differnt INTRA connections for each level, since they do change.
   
   - **Hierchial Serch**: asumed an abstraction level, and origin & a destination are defined, we will perform a search with our desired algorithm; being the nodes to search NOT the lowest-level tiles but the extraction levels but the defined level abstracted ones. First of all we will introduce the origin & destination as temporal nodes in the abstract level, in order to perform & finish the search. When the path is done (using the abstract Nodes of the level as points to do the path & the Edges as the connections between them) we will remove these temporal nodes from the abstract graph. 
   As you can already see, this makes a distinction between Hierchial & other methods of pathfinding, since **we don't have just to worry how much time the algorithm wastes, but also how much it costs to introduce & delete the nodes from the graph.**
   
   - **Refine Path**: whenever we have the path, we will have to refine it gradually. Going back to the previous example of travelling: we know that we have to go get out of home & then go to the airport; but we have to know in the lowest level possible what is the path to follow to get out of the house & then, once there, to go to the airport. This is when refinement comes into play; they are optizations to this I will explain later, but basically we have to execute out pathfinding algorithm between the two points of the abstract path &, since it's a much shorter path, it will be fater & divided though time.
   
> This explains the general structure an standard Hierchial Pathfinding should follow;  further on the document you will find the specific method I selected to implement all of this & also other approaches to the proble, but if you wanna know more detailed information about this  yo ucan find it [HERE](http://webdocs.cs.ualberta.ca/~kulchits/Jonathan_Testing/publications/ai_publications/jogd.pdf)).

***

 ### Different Approaches

 - **Different Algorithms**:
    - I have not mentioned this because I didn't want to bias anyone, but the Hierchial method is often used as an upgrade of the A* pathfinding algorithm (if you need a quick A* reminder, there is a very nice & concise article in  [Red Blob Games](https://www.redblobgames.com/pathfinding/a-star/introduction.html) but, as I said it can be used for any other algorithm; for example [this paper](https://www.cs.ru.nl/bachelors-theses/2013/Linus_van_Elswijk___0710261___Hierarchical_Path-Finding_Theta_star_Combining_HPA_star_and_Theta_star.pdf) implements it in a variation of A*, called Theta*.
    
    
    > Theta* is basically A* with a Jump Point Search implemented; if you wanna know more about JPS*, there is a very nice research from my colleague, [Lucho Suaya](https://www.linkedin.com/in/luchosuaya/) [HERE](https://lucho1.github.io/JumpPointSearch/); which I took as reference & inspiration to make the document you are reading right now.
    
  - **Dynamic Approach**:  
    - You can also get the Hierchy to work with dynamic objects & make it recalculate paths whenever a path gets interrupted, being in the need of handling multiple agents that request a path in a very large world full of interesting obstacles (such as stairs, mazes, etc.). There is a very cool GDC presentation about  this implementation by Alain Benoit, Lead Programmer and CTO (Chief Technology Officer) of [Sauropod Studio](https://www.sauropodstudio.com/english-1) in their game [Castle Story](http://www.castlestory.net/). You can find the 2018 GDC presentation [HERE](https://twvideo01.ubm-us.net/o1/vault/gdc2018/presentations/Alain_Benoit_HierarchicalDynamicPathfinding.pdf).

<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/Voxel_hier2.png"  width="60%" height="60%">
</p>
   
  - **Irregular Graphs** (or navigation meshes): 
    - Of course I must mention this, note that the agents that I have explained in the previous sections is partially a lie. This research is focused for general HP implementation and so, the explanation will be about making a regulat-all purpose graph; but a very common approach in Videogames & Game Engines is to create a custom navigation mesh for our levels; but the approach is the same: instead of having Entrances, we will define the nodes as we consider & we calculate the edges beween the nodes. Then we can abstractly connect the nodes & make the algorithm run raw to refine the lower level path or we can also refine clusters to improve the refinement.
   This provides much more path stability, we make sure of how dense we want the map to be in certain areas, etc. The main problem is that is not computer-generated but human generated and that takes a lot of time & effort.
   
   <p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/multiple_iregular_graph.jpg">
</p>

   > An example of multiple leveled navigation grid, with irregular nodes & clusters.

   <p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/200KoboldPaths.png"  width="60%" height="60%">
</p>

   > An example of multiple leveled navigation grid, with irregular nodes but regular clusters.

    
 >If you wanna know more about how to implement & manipulate Navigation Meshes into a Hierachy and also memory & refinement optimizations, you can check out the [2011 GDC aabout Starcraft II & Dragon Age Origins](https://www.gdcvault.com/play/1014514/AI-Navigation-It-s-Not). 
 
 >**Important Note**:Navigation Meshes is  a very cool & interesting topic to explore, but in this reseach I want to focus more in the basics of the Hierchial Pathfinding, since, I am a big believer that, if you understand the basics (ergo, what I am about to explain) who will know how the Navigatio Maps & Meshes work way faster &, also, since you will know what is happening in the lowest level, you will also understand the optimizations or came up with your own!

### My Approach - HPA*
Tho approach I selected is to do a **Regular Multi-Level Hierarchy A***. The reason is simple and I have already mentioned before: I want this to be an explanation so you will understand how & why a Hiearchial Pathfinding works, so let's start from the basic, shall we?

#### Code Structures
 - The Abstract Graph structure:
 
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
    - As you can see, we will store in memory the **Clusters** for each level, the **Entrances** (we will have enttrances, since it is a regular graph) & the **Nodes of the Hiearchy** (The Nodes are stored here, but note that each cluster has a vector of pointers to it's nodes, in order to have a better time searching)
    
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
    - This makes the code faster, since we don't have to check every node in the Graph, just to ones inside the same Cluster.
    - In order to have multi-level search, the Nodes must store it's level. This makes the pathfinding go slower, since we have to instance one more variable. My work arround for this is to have the clusters already separated in levels by their position in the vector of Cluster vecotrs (Ex. lvlClusters[N] stores all the Clusters in the N+1 level, beign 0 the non-abstract level). This way we don't have all the Clusters & Nodes in the same place, but have a little of structure.

    
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
  - The entrances are also quite simple; the same way  Clusters do, they store position & size but, also, store the direction that they are connecting (This is LATERAL or VERTICAL).

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
  - The Nodes (called HierNodes here) need a little bit more of explanation:
    - First of all we have the PathNodes, which are regular A* nodes that indicate a tile in the non-abstract level (with your typical A* stuff)
    - Then we have the Hiernodes, which are a class that inherits from PathNode; since, at the end, both are the same. There are two key diferences though:
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

  -  All right, the last of the major agents. The Edges are also simple. They just store it's type (that remeber, can be INTER o INTRA), the destination Node (which is the Node this connection points to) & the amount that Cost going to that Node (in A* terms, the g).
  
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

  - Basically what we do here is to create the first level of the Graph: with it's Entrances, Clusters, Nodes & Conections. Then for subsuquent levels we just create the Clusters, the Nodes & their Connections. [1]
  - Be noted that everything is calculated when we load a new map. [2]
  
  - For better illustration, let's simluate a Graph Construction step by step. Let's say we start from this map:
  

  
<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/Map.png"  width="60%" height="60%">
</p>

   > Being the white tiles walkable & the black ones non-alkable.
   
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
 
 - This function is a simple double for that iterates the map & creates the Clusters but I want to point out one thig:
  - The approach I used to make the Clusters regular is define an arbritary constant (CLUSTER_SIZE_LVL) & make each level so it's Clusters are this constant bigger than the previous ones. [2]
  
- Let's take a look on the Map & see how it has changed:

<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/Clusters.png"  width="60%" height="60%">
</p>

> As you can see, now the Map has division of Clusters that group tiles from the non-abstract level. Note that if we added another Graph Level, it would create another Clusters double the size.

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
 - We just iterate the clusters and find the adjacent ones; between them we will create the entrances. Check the CreateEntrances() method for more info but basically what it does is create an Entrance for each group of consecutive walkable tiles. [4]
 
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
   
   	- For Multiple-Level Search: we have to Level Up the existent Edges. I
	n order to indicate that they are from a different abstraction level (Edge struct has a variable lvl & EdgeExists automatically levels up the Edges) but there is a catch: we can only do this for INTER edges, since they are the same (because higher abstraction Clusters are just groups of lower-level clusters) but it's not the same case for the INTRA edges, since they can change; therefor, when we seach we can do it for INTER edges that are the same level or above (since, I repeat, are the same) but we must just search for INTRA nodes that are from the same level, since they can change; even though it's not guaranteed that they do.
	
> Back at our map, now this makes sense: the RED represents the Nodes, the GREN the Inter Edges & the BLUE the Intra Edges (they are calculated just in one Cluster)
	
<p align="center">
<img src="https://raw.githubusercontent.com/AlexMelenchon/Hierarchial-Pathfinding-Research/master/docs/images/nodesAndEdges.png"  width="60%" height="60%">
</p>
  
#### Search & Refinement Process

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
  
### Possible Improvements & Changes
- **[1]** : a possible improvement is to, instead of creating the Clusters for the next levels, is to group the Clusters from the previous level in groups of N Clusters.
- **[2]**: another option is to pre-calculate all of this and then load it when the map is calculated (this includes Entrances, Clusters, Nodes & Edges); but since our project is small, I decided to do it this way.
- **[3]**:  you could have a different approach in how you make the Clusters size (irregular is always an option) but you can have it be x2 larger than the previous one, etc.
- **[4]**:  This is inspired by the HNA* method; which basically makes the Clusters moldable to terrain, not the Entrances. If you wanna know more check out [THIS](https://web.archive.org/web/20190725152735/http://aigamedev.com/open/tutorials/clearance-based-pathfinding/)
