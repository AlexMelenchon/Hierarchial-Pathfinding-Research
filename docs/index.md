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
<img src="https://github.com/AlexMelenchon/Hierarchial-Pathfinding-Research/blob/master/docs/images/hpaGraph.jpg" >
</p>

This image of the Indie Developer [K Lodeman] (https://twitter.com/ManOfLode/status/854406316890128384) shows how the Hierchy principe works; the huge map is divided into shorter steps in order to then refine the path.

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
    - I have not mentioned this because I didn't want to bias anyone, but the Hierchial method is often used as an upgrade of the A* pathfinding algorithm (if you need a quick A* reminder, there is a very nice & concise article in  [Red Blob Games] (https://www.redblobgames.com/pathfinding/a-star/introduction.html)) but, as I said it can be used for any other algorithm; for example [this paper] (https://www.cs.ru.nl/bachelors-theses/2013/Linus_van_Elswijk___0710261___Hierarchical_Path-Finding_Theta_star_Combining_HPA_star_and_Theta_star.pdf) implements it in a variation of A*, called Theta*.
    
    > Theta* is basically A* with a Jump Point Search implemented; if you wanna know more about JPS*, there is a very nice research from my colleague, [Lucho Suaya] (https://www.linkedin.com/in/luchosuaya/) [HERE] (https://lucho1.github.io/JumpPointSearch/); which I took as reference & inspiration to make the document you are reading right now.
    
   - **Dynamic Approach**: you can also get the Hierchy to work with dynamic objects & make it recalculate paths whenever a path gets interrupted, being in the need of handling multiple agents that request a path in a very large world full of interesting obstacles (such as stairs, mazes, etc.). There is a very cool GDC presentation about  this implementation by Alain Benoit, Lead Programmer and CTO (Chief Technology Officer) of [Sauropod Studio](https://www.sauropodstudio.com/english-1) in their game [Castle Story](http://www.castlestory.net/). You can find the 2018 GDC presentation [HERE] (https://twvideo01.ubm-us.net/o1/vault/gdc2018/presentations/Alain_Benoit_HierarchicalDynamicPathfinding.pdf).
   
   

    


