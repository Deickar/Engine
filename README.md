Engine
======

DirectX11 / C++ videogame "Engine" made by me, Carl "Lemmi" Engström. Originally started out as a Bachelor's Degree project together with my friend Felix Nawrin Oxing, with the focus on procedurally generated terrain and deferred rendering.

***********************************
IMPORTANT:

If you want to download the project as a zip and start it, you need to open it through the Engine.vcxproj inside the code folder!
I don't really know why but it seems like git breaks the .sln files somehow.
***********************************

Here are the controls for this project. They are probably prone to change and I probably won't update this readme every time, but here goes:

* Arrow keys OR mouse to look around.
* WASD to move around.
* Space to move upward.
* C to move downward.
* Hold left shift to move faster.
* Hold left control to move slower.

* Q to toggle drawing debug info.
* 1 to speed up the day/night cycle.
* 2 to toggle material drawing mode.
* 3 and 4 are supposed to change fog draw distance but is currently not supported...
* 5 to toggle drawing of point lights
* 6 to toggle SSAO drawing mode. (Default is with SSAO enabled, next is the scene without any SSAO, and the last is the scene with only SSAO applied).

***********************************
* F1-F9 to change what type of terrain you want to generate next.

* F1: TypesSeaBottom.
* F2: TypePlains.
* F3: TypeHills.
* F4: TypeTerraces.
* F5: TypeDramaticHills.
* F6: TypeFlyingIslands.
* F7: TypeAlien.
* F8: TypeFancy.
* F9: TypeCave.
***********************************
  
* N to generate a new world.
* R and F to move point lights up and down.
* U to generate random R8G8B8A8 noise to a debug texture.
* I to generate random simplex noise to a debug texture.
* O to generate mirrored random simplex noise to a debug texture.
* P to save this texture to harddrive (gets saved where the code files are located).
* J to generate a random "grass" texture.
* K to generate a random "dirt" texture.
* L to save the grass or dirt texture to harddrive (gets saved where the code files are located).

That should be all of the keybindings for the moment. :)

