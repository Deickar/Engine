Engine
======

DirectX11 / C++ videogame "Engine" made by me, Carl "Lemmi" Engström. Originally started out as a Bachelor's Degree project together with my friend Felix Nawrin Oxing, with the focus on procedurally generated terrain and deferred rendering.

***********************************
IMPORTANT:

This project has really only been tested on Windows 7 64-bit with an nvidia card. I know for a fact that it's shittier on my laptop with an ATI 5650; for some reason there's loads of z-fighting and texture filtering is all fucked.

For a real installer to the project, here's a pretty new one:
ftp://lemminas.dlinkddns.com/Public/LemmiInstaller.msi

It should work fine, as long as your x86 vc++ 2010 redist is up to date: (http://www.microsoft.com/en-us/download/details.aspx?id=5555)

and your directx redist is up to date:
http://www.microsoft.com/en-us/download/details.aspx?id=8109

If you want to download the project as a zip and compile it, you need to open it through the Engine.vcxproj inside the code folder!
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

* G to bounce the... ball around.
* N to generate a new world.
* R and F to move point lights up and down.

That should be all of the keybindings for the moment. :)


//These are currently turned off while I fix crashes when other people try to save screenshots
* U to generate random R8G8B8A8 noise to a debug texture.
* I to generate random simplex noise to a debug texture.
* O to generate mirrored random simplex noise to a debug texture.
* P to save this texture to harddrive (gets saved where the code files are located).
* J to generate a random "grass" texture.
* K to generate a random "dirt" texture.
* L to save the grass or dirt texture to harddrive (gets saved where the code files are located).

