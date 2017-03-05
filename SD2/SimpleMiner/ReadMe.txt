Project: SimplerMiner
Milestone: #6 ("SimplerMiner Gold")
Name: Benjamin D. Gibson
Details/Videos: http://bdgibson.com/simplerminer

//-----------------------------------------------------------------------------	
Build Notes
  1. The EXE will crash if these folders do not exist under the Run folder: 
    Data/Images 
    Data/Fonts 
    Data/Audio
    Data/Saves -- where the Player.txt position file is saved, delete this freely to reset to a default position.
      Data/Saves/Overworld -- where .chunk data files are saved.
      Data/Saves/Nether
  2. Debug testing flags can be controlled via below hotkeys or hard-set in the source code's GameCommon.hpp/cpp.
  3. To access other dimensions, find a purple portal block hiding inside a village's tall tower.
    The easiest way to reach it is to jump while holding shift to use the boosted jump.
    The portal in one dimension will currently drop the player from the sky directly above the portal in the other dimension.
	
//-----------------------------------------------------------------------------
How to Use: Keyboard
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
      (It's also just frankly really fun to use!)
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
		NOTE: below extra blocks seen on HUD can be accessed via mouse wheel, or pinch-zoom on laptop touchpad:
      10: Lava
      11: Red Sand
      12: Mycelium (no spreading logic)
      13: Netherrack
      14: Brownstone
      15: Portal
      16: Gold Brick
      17: Gravel
		F1-F6 Keys: Toggle Debug Flags (other non-dynamic settings in GameCommon.hpp)
			F1: Toggle Debug Info
				Note: includes red-white line depicting player selection ray.
			F2: Toggle Raycast Mode 
				Note: changing this alters the raycast type used not only in block selection, 
        but also casted x12 in player preventive physics collision as a box-trace.
			F3: Toggle Day-Night Mode
				Note: reloads all chunks' vertex arrays, causing slight lag.
			F4: Toggle Lighting Test Texture
				Note: will not show for loaded chunks until a block placed/broken in said chunk, or F3 is hit.
			F5: Toggle Camera Mode (displayed top-right, note additional "free spectator" flycam mode will become stuck when player becomes stuck!)
			F6: Colorize Light Levels 
				Note: will not show for loaded chunks until a block placed/broken in said chunk to update it (or F3 is hit, updating all chunks).
				(Note to self: really cool visual results with F3 and optionally F4, esp. while loading new chunks with water...)
			F7: Render Sky Blocks as Debug Points
				Note: only adds debug points to be rendered if flag is initially true, can be set in GameCommon.cpp.
			F8: Render Chunks via VBO or Vertex Array
				Note: if the default isn't vertex array, switching to them will make all things vanish until a block is placed or broken (i.e. array is dirtied).
			F9: Stop/Start Applying Simple Dot-Product-based View Culling of Chunks (test via bottom-left "Rendered Chunk Count" debug string)

How to Use: Mouse
    Move Mouse: Look
		LMB: Dig Block
		RMB: Place HUD-Selected Block
		Wheel: Switch HUD's Selected Block
	
//-----------------------------------------------------------------------------
Attributions
	Minecraft Texture Atlas		Squirrel Eiserloh
	Monospace Bitmap Font		  Squirrel Eiserloh
	Minecraft Audio Files		  Clay Howell, Mojang

//-----------------------------------------------------------------------------
Latest Changelog: Finished good-faith effort to revise coding style elements that I've grown to understand how to do them better, e.g.
  - Line-breaking same-line "if (...) return;" (etc.) statements, as you then cannot breakpoint to see if execution hits their clauses.
  - Making local variables for values accessed multiple times, as it is cache-friendlier to not fetch further out into memory each time.
  - Moving myArray[ MyFunction(...) ] calls out as preceding local variable assignments, as its minute perf cost (if any) <<< readability.
  - Comments and algorithms have rarely been significantly altered, or added, beyond what was already there at the project's final milestone.
  - Adding a few more instances of 'inline' where relevant to clarify the intent, as the optimizations were more critical in this project.
  - Removing "forward declarations go here"-type boilerplate comments I had in my base game project back when I was still figuring out game system architecture, no longer needed in the current game project.
  
//-----------------------------------------------------------------------------
Known Issues (in general, note to noclip out when stuck in walls by changing to NOCLIP movement mode with P-key)
	0. Trying to use the macro keys possibly other non-standard inputs) will cause an exception in TheInput.cpp by writing "true" to m_controllers[0].
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
		My current recommendation should this happen is to just quit, delete the Player.txt file (not chunk folder content), and reload.

//-----------------------------------------------------------------------------
Deep Learning: Evolving Beyond the Game VS Demo Tug-of-War
	Following discussions in Game Studies about how portfolios should emphasize the technical abilities that a company can rely on me foremost, I think I can finally settle my internal game-versus-demo tensions. This came about when I brought to C20's Matt Miller (designer, not coder) a question about whether to showcase an engine-side bug I had saved my team the risk of updating our engine to fix. Thanks to how we had designed the game, I could appropriate that design to effectively lock out the bug from happening. However, Matt then articulated for me that this wasn't really anything a company or I could expect to reliably repeat or apply in future projects. Instead, he said to look for the showcase how I had learned to implement solutions to technically significant problems (mechanic designs, bugs, or otherwise). When I apply this mindset to the tension I've felt in our SD curriculum towards whether to make fun games or technically impressive demos, I see the key. It isn't the end product I create that matters, as I may feel led to believe, at least not directly. That program, game OR demo, ONLY holds significance with regards to how I can repeat aspects learned from it. The ease with which I can repeat the skill of refining fun, for example, holds just as much validity as the skill of implementing multithreading. The only difference, I think, then lies with the size of those problem spaces. There are a set number of issues multithreading may raise for us, but it's nothing compared to designing fun in games. Yet still, obviously game coders influence each field. This isn't necessarily a "therefore, this side wins" conclusion that the entire game-vs-demo premise is a false dichotomy. I would like to continue thinking these through alongside any future game developer: how do we make these skills repeatable on future projects?
