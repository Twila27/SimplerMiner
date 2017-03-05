#pragma once

#include <vector>
#include "Engine/Renderer/Vertexes.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderCommand.hpp"

class Chunk;

//-----------------------------------------------------------------------------
//Naming Windows Virtual Keybinds for Clarity
#define STATIC
#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_SPACE 0x20
#define VK_SHIFT 0x10 
#define VK_ESCAPE 0x1B
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78

//-----------------------------------------------------------------------------
static const char* g_appName = "SimplerMiner Gold (Milestone 06) by Benjamin D. Gibson";

//-----------------------------------------------------------------------------
//Debug Tools
extern bool g_useCulling;
extern bool g_renderRaycasts;
extern bool g_renderPlayerCollider;
extern bool g_renderDebugInfo;
extern bool g_flushChunksEnabled;
extern bool g_activateChunksEnabled;
extern bool g_updateVertexDataEnabled;
extern bool g_disableSaving;
extern bool g_disableLoading;
extern bool g_useAmanWooRaycastOverStepAndSample;
extern bool g_renderChunksWithVertexArrays;
extern bool g_useLightTestingTexture;
extern bool g_renderSkyBlocksAsDebugPoints; //If you'd like to test debug points, use this!
extern bool g_colorizeLightLevels; //See GetLightColorForLightLevel for values.
extern bool g_useNightLightLevel;
extern int g_chunksRendered;
extern bool g_generateVillages;

//Toggling back and forth WILL cause some chunks to become and STAY dirty until updated (usually by player raycast dirtying VAO), hence it's just for debug.
extern char KEY_TO_TOGGLE_DEBUG_INFO;
extern char KEY_TO_TOGGLE_RAYCAST_MODE;
extern char KEY_TO_TOGGLE_DAYNIGHT_MODE; //This actually does dirty neighboring chunks, so fair warning to wait lag to pass when using.
extern char KEY_TO_TOGGLE_LIGHTING_TEST_TEXTURE;
extern char KEY_TO_TOGGLE_CAMERA;
extern char KEY_TO_TOGGLE_COLORIZE_LIGHT_LEVELS;
extern char KEY_TO_TOGGLE_PLAYER_COLLIDER;
extern char KEY_TO_TOGGLE_SKY_DEBUG;
extern char KEY_TO_TOGGLE_MOVEMENT_MODE;
extern char KEY_TO_TOGGLE_VBO_AND_VA;
extern char KEY_TO_TOGGLE_CULLING;
extern char KEY_TO_TOGGLE_DIMENSION;

//Old Debug Render Commands (use Engine/Rendering/RenderCommand now).
extern std::vector< Vertex3D_PCT > g_debugPoints;
inline void AddDebugPoint( const Vector3& position, const Rgba& color ) { g_debugPoints.push_back( Vertex3D_PCT(position, color) ); }
void DrawDebugPoints( float pointSize, bool enableDepthTesting );

//-----------------------------------------------------------------------------
//Village

static const int VILLAGE_RADIUS_Y_BLOCKS = 9; //Doesn't include center.
static const int VILLAGE_RADIUS_X_BLOCKS = 9; //Doesn't include center.
static const int VILLAGE_NUM_CHUNKS_TO_LOOK_FOR_VILLAGE = 5;
static const float VILLAGE_PERLIN_GRID_CELL_SIZE = 200.f;
static const int VILLAGE_PERLIN_GRID_NUM_OCTAVES = 1;
static const float VILLAGE_PERLIN_AMPLITUDE = 1.f;
static const float VILLAGE_PERLIN_GRID_PERSISTANCE = .2f; //How much each wave persists.
static const int VILLAGE_POND_HEIGHT = 10;
static const int VILLAGE_PORTAL_SHRINE_HEIGHT = 30;
static const int VILLAGE_TREASURE_SHRINE_HEIGHT = 10; 
static const int VILLAGE_BLOCKS_FROM_CENTER_TO_CONSTRUCT_INCLUDING_CENTER = 3;

//-----------------------------------------------------------------------------
//Player
static const float THRESHOLD_TO_BE_CONSIDERED_ON_GROUND = .15f;

static const float PLAYER_INITIAL_HEIGHT = 90.f;
static const float PLAYER_HEIGHT = 1.85f; //Adjusted to match Minecraft's number.
static const float PLAYER_HALF_HEIGHT = PLAYER_HEIGHT * .5f; //For box trace, etc.
static const float PLAYER_WIDTH = .6f; //Assuming this is along x and along y.
static const float PLAYER_HALF_WIDTH = PLAYER_WIDTH * .5f;
static const Vector3 PLAYER_DEFAULT_POSITION = Vector3( 0.f, 0.f, PLAYER_INITIAL_HEIGHT );

static const float FLYCAM_SPEED_SCALAR = 8.f;
static const float CAMERA_FIRST_PERSON_HEIGHT = 1.62f;
static const float CAMERA_FROM_BEHIND_PUSHBACK_LENGTH = 8.f;
static const Vector3 CAMERA_DEFAULT_POSITION = Vector3( 0.f, 0.f, PLAYER_INITIAL_HEIGHT );

static const float PLAYER_MAX_SPEED = 4.f; //Note not velocity, doesn't account for direction.
static const float HORIZONTAL_DECEL_BEFORE_STOP_KNOB = .90f;
static const float VERTICAL_DECEL_BEFORE_STOP_KNOB = .99f;
static const float LADDER_VERTICAL_DECEL_KNOB = .6f;
static const float LADDER_RISING_SPEED_KNOB = 120.f;
static const Vector3 STAIR_BOOST = Vector3(0.f, 0.f, 1.01f);
static const float PLAYER_SPEED_BOOST = 4.f; //World units of blocks per second. Squirrel pushes for 10.f.
static const float PLAYER_DEFAULT_SPEED = 1.f;
static const float PLAYER_GRAVITY_FORCE = -9.81f; //Applied along z-axis.
static const float PLAYER_JUMP_SPEED = 5.f;

static const char KEY_TO_MOVE_FORWARD = 'W';
static const char KEY_TO_MOVE_BACKWARD = 'S';
static const char KEY_TO_MOVE_LEFT = 'A';
static const char KEY_TO_MOVE_RIGHT = 'D';
static const char KEY_TO_MOVE_UP = VK_SPACE;
static const char KEY_TO_MOVE_DOWN = 'X'; //Be careful, also mutes BGM from Main_Win32.

static const int RAYCAST_NUM_STEPS = 1000; //For step and sample.
static const float LENGTH_OF_SELECTION_RAYCAST = 8.f; //World units.

enum CameraMode : unsigned int { FIRST_PERSON = 0, FROM_BEHIND, FIXED_SPECTATOR, FREE_SPECTATOR, NUM_CAMERA_MODES };
extern CameraMode g_currentCameraMode;

enum MovementMode : unsigned int { WALKING = 0, FLYING, NOCLIP, NUM_MOVEMENT_MODES };
extern MovementMode g_currentMovementMode;

//-----------------------------------------------------------------------------
//Audio
typedef unsigned int SoundID;
static const float VOLUME_ADJUST = .15f;
static const float SECONDS_BETWEEN_DIG_SOUNDS = .25f;
static const float DISTANCE_BETWEEN_WALK_SOUNDS = 1.f; //World units, or 1.0 per block.

//-----------------------------------------------------------------------------
//Lighting & Textures
static const float LOWEST_LIGHT_RGBA_VALUE = 0.1f;
static const int NUM_BITS_FOR_LIGHT_LEVEL = 4; //Now 0-7, was originally 4 for 0-15. Needed bit for two stair/ladder orientation bits.
static const int MAX_LIGHTING_LEVEL = BIT( NUM_BITS_FOR_LIGHT_LEVEL ) - 1; //2^NUM_BITS minus one, for 0-based use.
static const int NIGHT_LIGHTING_LEVEL = GetMax( 0, ( MAX_LIGHTING_LEVEL / 2 ) - 1 ); //-1 preserves 6 for max 15. Not speed-critical /'s here.
static const int GLOWSTONE_LIGHT_LEVEL = GetMax( 0, ( ( 3 * MAX_LIGHTING_LEVEL ) / 4 ) + 1); //+1 preserves 12 for max 15.

extern const SpriteSheet* g_textureAtlas;
static const int NUMBER_DIG_DAMAGE_FRAMES = 10;

//-----------------------------------------------------------------------------
//Chunk and block and dimension initialization.
enum BlockType : unsigned char { 
	AIR = 0, WATER, DIRT, GRASS, SAND, STONE, COBBLESTONE, GLOWSTONE,
	STAIRS, LADDER, 
	LAVA, RED_SAND, MYCELIUM, NETHERRACK, BROWNSTONE, 
	PORTAL, GOLD_BRICK, GRAVEL,
	NUM_BLOCK_TYPES };
enum BlockFace { NONE = -1, BOTTOM = 0, TOP, LEFT, RIGHT, FRONT, BACK, NUM_FACES };

static const int INITIAL_ACTIVE_RADIUS = 128; //World units.
static const int INITIAL_FLUSH_RADIUS = 144;

static const int CHUNK_BITS_X = 4;
static const int CHUNK_BITS_Y = 4;
static const int CHUNK_BITS_Z = 7;
static const int CHUNK_X_LENGTH_IN_BLOCKS = BIT( CHUNK_BITS_X ); //2^CHUNK_BITS_X.
static const int CHUNK_Y_WIDTH_IN_BLOCKS = BIT( CHUNK_BITS_Y ); //2^CHUNK_BITS_Y.
static const int CHUNK_Z_HEIGHT_IN_BLOCKS = BIT( CHUNK_BITS_Z ); //2^CHUNK_BITS_Z.
static const int NUM_COLUMNS_PER_CHUNK = CHUNK_X_LENGTH_IN_BLOCKS * CHUNK_Y_WIDTH_IN_BLOCKS;
static const int NUM_BLOCKS_PER_CHUNK = NUM_COLUMNS_PER_CHUNK * CHUNK_Z_HEIGHT_IN_BLOCKS;
static const int BITS_PER_XY_LAYER = CHUNK_BITS_X + CHUNK_BITS_Y;
static const int LOCAL_X_BITMASK = CHUNK_X_LENGTH_IN_BLOCKS - 1; //Lowest chunk_x_length-1 bits.
static const int LOCAL_Y_BITMASK = CHUNK_Y_WIDTH_IN_BLOCKS - 1;

enum Dimension { DIM_OVERWORLD, DIM_NETHER, NUM_DIMENSIONS };

static const float GROUND_HEIGHT_PERLIN_GRID_CELL_SIZE = 200.f;
static const int GROUND_HEIGHT_PERLIN_NUM_OCTAVES = 5;
static const float GROUND_HEIGHT_PERLIN_PERSISTANCE_PERCENTAGE = .60f;
static const float GROUND_HEIGHT_PERLIN_AMPLITUDE = 50;
static const int GROUND_HEIGHT_MINIMUM = 64;
static const float CEILING_HEIGHT_PERLIN_GRID_CELL_SIZE = 200.f;
static const int CEILING_HEIGHT_PERLIN_NUM_OCTAVES = 3;
static const float CEILING_HEIGHT_PERLIN_PERSISTANCE_PERCENTAGE = .75f;
static const int CEILING_HEIGHT_PERLIN_AMPLITUDE = 50;
static const int CEILING_HEIGHT_OFFSET = 100;

static const int NUM_DIRT_LAYERS = 6; //Between grass and stone, not including the grass layer.
static const int SEA_LEVEL_HEIGHT_LIMIT = CHUNK_Z_HEIGHT_IN_BLOCKS / 2; //preserves z=64 for a max z=128.
static const int NETHER_LAVA_HEIGHT_LIMIT = CHUNK_Z_HEIGHT_IN_BLOCKS / 4; //Not speed-critical divisions here.

//Below constants used for the non-Perlin flat-world generation.
static const int DIRT_EXCLUSIVE_HEIGHT_LIMIT = (int)( NUM_COLUMNS_PER_CHUNK * ( 0.515625f * CHUNK_Z_HEIGHT_IN_BLOCKS ) ); //Preserves z=66 for a max z=128.
static const int STONE_EXCLUSIVE_HEIGHT_LIMIT = DIRT_EXCLUSIVE_HEIGHT_LIMIT - ( NUM_COLUMNS_PER_CHUNK * NUM_DIRT_LAYERS );
static const int GRASS_EXCLUSIVE_HEIGHT_LIMIT = DIRT_EXCLUSIVE_HEIGHT_LIMIT + ( NUM_COLUMNS_PER_CHUNK + 1 );
static const int AIR_EXCLUSIVE_HEIGHT_LIMIT = NUM_COLUMNS_PER_CHUNK * CHUNK_Z_HEIGHT_IN_BLOCKS;

//-----------------------------------------------------------------------------
//HUD
static const Vector2 HUD_BOTTOM_LEFT_POSITION = Vector2( 100.f, 27.f ); //In from left, up from bottom of screen.
static const float HUD_WIDTH_BETWEEN_ELEMENTS = 25.f;
static const float HUD_ELEMENT_WIDTH = 50.f;
static const float HUD_HEIGHT = HUD_ELEMENT_WIDTH; //Ensures HUD choices are square.
static const float HUD_CROSSHAIR_RADIUS = 20.f;
static const float HUD_CROSSHAIR_THICKNESS = 4.f;


//-----------------------------------------------------------------------------
//Coordinate System
typedef Vector3 WorldCoords; //May be negative.
typedef Vector2 WorldCoordsXY; //Just assume z = 0, e.g. (1,0) ChunkCoords should be (16.0f, 0.f) in ChunkCoordsInWorld. Used in chunk activation search grid.
typedef Vector2 GlobalColumnCoords; //Mostly for chunk construction. Not IntVec2 because it's passed into ComputePerlin2D which takes float-based vector2.
typedef IntVector3 LocalBlockCoords; //Local within or with respect to a certain chunk. Only positive.
typedef IntVector3 GlobalBlockCoords; //e.g. 2 chunks east (+x) is (16*2, 0, 0) blocks from origin. May be negative.
typedef IntVector2 ChunkCoords; //No third dimension, unlike actual Minecraft. Functions like TileCoords did in Assault, but may now be negative.
typedef IntVector2 LocalColumnCoords;
typedef unsigned int LocalBlockIndex; //No chunk index as those are infinite, this refers to the Block array of a given chunk (hence 1D unlike LocalBlockCoords).
typedef unsigned int ChunkColumnIndex;

static const Vector3 WORLD_UP = Vector3( 0.f, 0.f, 1.f );
static const Vector3 WORLD_DOWN = Vector3( 0.f, 0.f, -1.f );
static const Vector3 WORLD_LEFT = Vector3( 0.f, 1.f, 0.f );
static const Vector3 WORLD_RIGHT = Vector3( 0.f, -1.f, 0.f );
static const Vector3 WORLD_FORWARD = Vector3( 1.f, 0.f, 0.f );
static const Vector3 WORLD_BACKWARD = Vector3( -1.f, 0.f, 0.f );

//-----------------------------------------------------------------------------
//Flags in the byte-packed member variable of the Block class.
static const unsigned char BLOCKFLAGS_LIGHT_LEVEL_BITMASK = MAX_LIGHTING_LEVEL; //Note used as unshifted and hence lowest order bits.
static const unsigned char BLOCKFLAGS_IS_SKY_BITMASK = BIT( 7 ); //Nothing opaque above block to z-max, implies max light level.
static const unsigned char BLOCKFLAGS_IS_LIGHTING_DIRTY_BITMASK = BIT( 6 ); //Whether block is in lighting's dirty-deque.
static const unsigned char BLOCKFLAGS_IS_OPAQUE_BITMASK = BIT( 5 );
static const unsigned char BLOCKFLAGS_ORIENTATION_BITMASK = BIT( 4 ) | BIT( 3 );

//--------------------------------------------------------------------------------------------------------------
inline LocalBlockCoords GetLocalBlockCoordsFromLocalBlockIndex( LocalBlockIndex lbi )
{
	ASSERT_OR_DIE( ( lbi >= 0 ) && ( lbi < NUM_BLOCKS_PER_CHUNK ), "LocalBlockIndex Out of Range" );

	LocalBlockCoords lbc;

	
	lbc.x = lbi & LOCAL_X_BITMASK; //LOCAL_X_BITMASK = CHUNK_X_LENGTH_IN_BLOCKS - 1;
	lbc.y = ( lbi >> CHUNK_BITS_X ) & LOCAL_Y_BITMASK; //LOCAL_Y_BITMASK = CHUNK_Y_LENGTH_IN_BLOCKS - 1;
	lbc.z = lbi >> BITS_PER_XY_LAYER;

	ASSERT_OR_DIE( ( lbc.x >= 0 ) && ( lbc.x < CHUNK_X_LENGTH_IN_BLOCKS ), "LocalBlockCoords.x Out of Range" );
	ASSERT_OR_DIE( ( lbc.y >= 0 ) && ( lbc.y < CHUNK_Y_WIDTH_IN_BLOCKS ), "LocalBlockCoords.y Out of Range" );
	ASSERT_OR_DIE( ( lbc.z >= 0 ) && ( lbc.z < CHUNK_Z_HEIGHT_IN_BLOCKS ), "LocalBlockCoords.z Out of Range" );

	return lbc;
}


//--------------------------------------------------------------------------------------------------------------
inline LocalBlockIndex GetLocalBlockIndexFromLocalBlockCoords( const LocalBlockCoords& lbc )
{
	return lbc.x | ( lbc.y << CHUNK_BITS_X ) | ( lbc.z << BITS_PER_XY_LAYER ); //+'s became OR's because the bits are independent.
}


//--------------------------------------------------------------------------------------------------------------
//Primarily for use from the VS debugger watch window, hence no inline.
ChunkCoords GetChunkCoordsFromWorldCoordsXY( const WorldCoordsXY& wc ); //Needed by both World and Chunk classes.
const char* GetDimensionAsString( Dimension dimension );
LocalBlockCoords GetLocalBlockCoordsFromLocalBlockIndexNoBitMath( LocalBlockIndex lbi );
LocalBlockIndex GetLocalBlockIndexFromLocalBlockCoordsNoBitMath( const LocalBlockCoords& lbc );
GlobalBlockCoords GetGlobalBlockCoordsFromChunkAndLocalBlockIndex( const Chunk* chunkOfBlock, LocalBlockIndex blockIndexInChunk );
GlobalBlockCoords GetGlobalBlockCoordsFromChunkAndLocalBlockCoords( const Chunk* chunkOfBlock, int lbcX, int lbcY, int lbcZ ); //Because VS-watch doesn't support ctor calling.