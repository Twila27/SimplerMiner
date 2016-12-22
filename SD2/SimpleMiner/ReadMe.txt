Project: SimpleMiner
Milestone: #6 ("SimpleMiner Gold")
Name: Benjamin Gibson
Class: Software Development II
Instructor: Squirrel Eiserloh


//-----------------------------------------------------------------------------
Known Issues (in general, note to noclip out when stuck in walls by changing to NOCLIP movement mode with P-key)
	0. Trying to use the macro keys (possibly other non-standard inputs) will cause an exception in TheInput.cpp by writing "true" to m_controllers[0].
		This causes the next frame to attempt to delete the unconnected controller's data, causing the exception.
	1. Third-person camera does not currently raycast-collision check to bring itself forward.
	2. Ladders at times can stick the player in a block while walking off the top of one onto a solid block.
		They seem to slide forward before the ladder's boost to player velocity's z-component can get them above the block the top ladder is adjacent to.
		This moves them up and forward in one frame, causing the slight stick at times on the forward bottom edge of the player's bounding box volume.
	3. Lower framerates bug out the physics, e.g. ladders send you far too high, walking still seems to find different max velocities when moving or stepping up.
	4. Break a block underneath the player while in walking movement mode and free spectator camera mode causes the falling to be applied non-stop to the camera.
		Can just F5 to change camera modes and back out of it.
	5. Though audio in the various forms is supported, the full range of sounds in the game is limited.
		Not enough free time to go digging for that many sound files.
	6. Because active dimension is not currently saved to the player file, but position is, saving and quitting and then reloading while in the Nether gets you stuck.
		It's suggested should this happen to just delete the player file.
	
Build Notes
	.exe will crash if it cannot find the directories Data/Images, Data/Fonts, Data/Audio in its folder.
	Note also the game saves out .chunk data files to the Data/Saves folder, each dimension to a folder Data/Saves/Overworld and Data/Saves/Nether.
	Player information is likewise stored in Data/Saves/Player.txt.
	Flags to turn on/off various features can be set in GameCommon.hpp (or keys below).
	To access other dimensions, find a purple portal block hiding inside a village's tall tower. 
		The easiest way to reach it is to jump while holding shift to use the boosted jump.
		The portal in one dimension will currently drop the player from a location in the sky directly above the portal in the other dimension.
	
//-----------------------------------------------------------------------------
How to Use
	1: Keyboard
		P: Toggle Movement Mode (displayed top-right)
		B: Toggle Player Bounding Volume
		W: Forward
		S: Backward
		A/D: Strafe
		Spacebar: Up/Jump
			Note: holding space while falling from gravity (even on ladders) causes a faster descent. 
		X: Down
		Hold Shift: Speed-up (x4)
			Made it valid while jumping and moving to test roaming in XY and falling due to gravity.
		ESC: Save & Quit
			Saving preserves chunk content as well as player position and orientation, may be disabled from GameCommon.cpp.
		1-9 Keys: Switch HUD's Selected Block
			1: Water
			2: Dirt
			3: Grass
			4: Sand
			5: Stone
			6: Cobblestone
			7: Glowstone
			8: Stairs (currently Slabs to avoid the less generic rendering needs of stairs)
			9: Ladders
			NOTE: extra blocks seen on HUD can be accessed via mouse wheel.
		F1-F6 Keys: Toggle Debug Flags (other non-dynamic settings in GameCommon.hpp)
			F1: Toggle Debug Info
				Note: includes player selection ray.
			F2: Toggle Raycast Mode 
				Note: changing this affects raycast type used not only in block selection but also which is casted x12 in player preventive physics collision.
			F3: Toggle Day-Night Mode
				Note: reloads all chunks' vertex arrays, causing slight lag.
			F4: Toggle Lighting Test Texture
				Note: will not show for loaded chunks until a block placed/broken in said chunk, or F3 is hit.
			F5: Toggle Camera Mode (displayed top-right, note additional flycam "free spectator" mode will become stuck when player becomes stuck!)
			F6: Colorize Light Levels 
				Note to self: really cool visual results with F3 and optionally F4, esp. while loading new chunks with water because of how it 
				Note: will not show for loaded chunks until a block placed/broken in said chunk, or F3 is hit.
			F7: Render Sky Blocks as Debug Points
				Note: only adds debug points to be rendered if flag is initially true, can be set in GameCommon.cpp.
			F8: Render Chunks via VBO or Vertex Array
				Note: if the default isn't vertex array, switching to them will make all things vanish until a block is placed or broken (i.e. array is dirtied).
			F9: Apply Simple Dot-Product-based View Culling of Chunks
	2: Mouse: Look
		LMB: Dig Block
		RMB: Place HUD-Selected Block
		Wheel: Switch HUD's Selected Block
	
//-----------------------------------------------------------------------------
Attributions
	Minecraft Texture Atlas		Squirrel Eiserloh
	Monospace Bitmap Font		Squirrel Eiserloh
	Minecraft Audio Files		Clay Howell, Mojang

//-----------------------------------------------------------------------------
Deep Learning: Evolving Beyond the Game VS Demo Tug-of-War
	Following discussions in Game Studies about how portfolios should emphasize the technical abilities that a company can rely on me foremost, I think I can finally settle my internal game-versus-demo tensions. This came about when I brought to C20's Matt Miller (designer, not coder) a question about whether to showcase an engine-side bug I had saved my team the risk of updating our engine to fix. Thanks to how we had designed the game, I could appropriate that design to effectively lock out the bug from happening. However, Matt then articulated for me that this wasn't really anything a company or I could expect to reliably repeat or apply in future projects. Instead, he said to look for the showcase how I had learned to implement solutions to technically significant problems (mechanic designs, bugs, or otherwise). When I apply this mindset to the tension I've felt in our SD curriculum towards whether to make fun games or technically impressive demos, I see the key. It isn't the end product I create that matters, as I may feel led to believe, at least not directly. That program, game OR demo, ONLY holds significance with regards to how I can repeat aspects learned from it. The ease with which I can repeat the skill of refining fun, for example, holds just as much validity as the skill of implementing multithreading. The only difference, I think, then lies with the size of those problem spaces. There are a set number of issues multithreading may raise for us, but it's nothing compared to designing fun in games. Yet still, obviously game coders influence each field. This isn't necessarily a "therefore, this side wins" conclusion that the entire game-vs-demo premise is a false dichotomy. I would like to continue thinking these through alongside any future game developer: how do we make these skills repeatable on future projects?