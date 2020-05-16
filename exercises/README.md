# Hierarchial-Pathfinding-Research

I am [Àlex Melenchón](https://www.linkedin.com/in/alex-melench%C3%B3n-maza-a46981195/), student of the [Bachelor’s Degree in
Video Games by UPC at CITM](https://www.citm.upc.edu/ing/estudis/graus-videojocs/). This content is generated for the second year’s
subject Project 2, under supervision of lecturer [Marc Garrigó](https://www.linkedin.com/in/mgarrigo/).

You can take a look at the website with detauiled information about the research [HERE](https://github.com/AlexMelenchon/Hierarchial-Pathfinding-Research) & you can check the project [HERE](https://github.com/AlexMelenchon/Hierarchial-Pathfinding-Research)


## Controls

- W,A,S,D: Controls camera movement.
- UP,DOWN: Changes between the levels of hierarchy. 
- Keyboard 1: Spawns entities.
- LEFT CLICK (hold): creates a rectangle to select units.
- LEFT CLICK (release): Selects entities inside the selection rectangle.
- RIGHT CLICK (down): makes selected entities do pathfinding to the clicked position.
- Esc: Closes the application.


## Debug keys

- F1: shows the Entrances created for the Hierarchial graph. 
- F2: shows the Clusters created for the current level of abstraction.
- F3: shows the Nodes created for the current level of abstraction.
- F4: if the Nodes are visible, it shows the Edges between them for the current level of abstraction.
- F5: Shows the non-walkable areas of the map.
- LEFT CLICK (down & no units selected): indicates the origin from which the debug pathfinding will be done.
- RIGHT CLICK (down & no units selected): indicates the destination. If the destination & origin are walkable, executes the abstract path.
- SPACE: if a debug abstract pathfinding is created & still has refinement to do; it refines it.
