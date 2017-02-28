#include "Game/GameCommon.hpp"

#include "Game/Chunk.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

//--------------------------------------------------------------------------------------------------------------
//Externed Variables' Definitions
bool g_useCulling = true;
bool g_renderRaycasts = true; //No toggle key currently.
bool g_renderPlayerCollider = true;
bool g_renderDebugInfo = true;
bool g_flushChunksEnabled = true;
bool g_activateChunksEnabled = true;
bool g_updateVertexDataEnabled = true;
bool g_disableSaving = false;
bool g_disableLoading = false; //e.g. to test chunk generation and ignore saved chunk files.
bool g_useAmanWooRaycastOverStepAndSample = true;
bool g_renderChunksWithVertexArrays = false; //Uses VBOs if false.
bool g_useLightTestingTexture = false;
bool g_renderSkyBlocksAsDebugPoints = false; //If you'd like to test debug points, use this!
bool g_colorizeLightLevels = false; //See GetLightColorForLightLevel for values.
bool g_useNightLightLevel = false;
int g_chunksRendered = 0;
bool g_generateVillages = true;

CameraMode g_currentCameraMode = FIRST_PERSON;
MovementMode g_currentMovementMode = NOCLIP;

char KEY_TO_TOGGLE_DEBUG_INFO = VK_F1;
char KEY_TO_TOGGLE_RAYCAST_MODE = VK_F2;
char KEY_TO_TOGGLE_DAYNIGHT_MODE = VK_F3; //This actually does dirty neighboring chunks, so fair warning to wait lag to pass when using.
char KEY_TO_TOGGLE_LIGHTING_TEST_TEXTURE = VK_F4;
char KEY_TO_TOGGLE_CAMERA = VK_F5;
char KEY_TO_TOGGLE_COLORIZE_LIGHT_LEVELS = VK_F6;
char KEY_TO_TOGGLE_SKY_DEBUG = VK_F7; //Will not render unless true at compile-time first, else nothing added to g_debugPoints.
char KEY_TO_TOGGLE_PLAYER_COLLIDER = 'B';
char KEY_TO_TOGGLE_MOVEMENT_MODE = 'P';
char KEY_TO_TOGGLE_VBO_AND_VA = VK_F8;
char KEY_TO_TOGGLE_CULLING = VK_F9;

const SpriteSheet* g_textureAtlas;

std::vector< Vertex3D_PCT > g_debugPoints;


//--------------------------------------------------------------------------------------------------------------
void DrawDebugPoints( float pointSize, bool enableDepthTesting )
{
	g_theRenderer->EnableDepthTesting( enableDepthTesting );
	g_theRenderer->SetPointSize( pointSize );
	g_theRenderer->DrawVertexArray_PCT( TheRenderer::VertexGroupingRule::AS_POINTS, g_debugPoints, g_debugPoints.size() );
	g_theRenderer->SetPointSize( 1.0f );
	g_theRenderer->EnableDepthTesting( false );
}


//--------------------------------------------------------------------------------------------------------------
GlobalBlockCoords GetGlobalBlockCoordsFromChunkAndLocalBlockIndex( const Chunk* chunkOfBlock, LocalBlockIndex blockIndexInChunk )
{
	WorldCoordsXY chunkPos = chunkOfBlock->GetChunkMinsInWorldUnits();
	LocalBlockCoords lbc = GetLocalBlockCoordsFromLocalBlockIndex( blockIndexInChunk );
	return GetGlobalBlockCoordsFromChunkAndLocalBlockCoords( chunkOfBlock, lbc.x, lbc.y, lbc.z );
}


//--------------------------------------------------------------------------------------------------------------
GlobalBlockCoords GetGlobalBlockCoordsFromChunkAndLocalBlockCoords( const Chunk* chunkOfBlock, int lbcX, int lbcY, int lbcZ )
{
	WorldCoordsXY chunkPos = chunkOfBlock->GetChunkMinsInWorldUnits();

	GlobalBlockCoords gbc;

	gbc.x = (int)chunkPos.x * lbcX;
	gbc.y = (int)chunkPos.y * lbcY;
	gbc.z = lbcZ;

	return gbc;
}


//--------------------------------------------------------------------------------------------------------------
LocalBlockIndex GetLocalBlockIndexFromLocalBlockCoordsNoBitMath( const LocalBlockCoords& lbc )
{
	int z = lbc.z * NUM_COLUMNS_PER_CHUNK; //1 +z per [length]*[width], a chunk layer.
	int y = lbc.y * CHUNK_X_LENGTH_IN_BLOCKS; //1 +y per [length], a single row of blocks in a single chunk.
	int x = lbc.x * 1;

	return ( x + y + z );
}


//--------------------------------------------------------------------------------------------------------------
LocalBlockCoords GetLocalBlockCoordsFromLocalBlockIndexNoBitMath( LocalBlockIndex lbi )
{
	ASSERT_OR_DIE( ( lbi >= 0 ) && ( lbi < NUM_BLOCKS_PER_CHUNK ), "LocalBlockIndex Out of Range" );

	LocalBlockCoords lbc;

	lbc.x = lbi % CHUNK_X_LENGTH_IN_BLOCKS;
	
	//Earning +1 y per [length] x's, but then restart every [width] y's, hence the outer mod.
	lbc.y = ( lbi / CHUNK_X_LENGTH_IN_BLOCKS ) % CHUNK_Y_WIDTH_IN_BLOCKS;
	
	//Earning +1 positive z-coordinate per [length]x[width] x's and y's.
	lbc.z = lbi / NUM_COLUMNS_PER_CHUNK;

	return lbc;
}


//--------------------------------------------------------------------------------------------------------------
ChunkCoords GetChunkCoordsFromWorldCoordsXY( const WorldCoordsXY& wc )
{
	ChunkCoords cc;

	//Floor because chunk grid is mins-aligned.
	int wcX = (int)floor( wc.x );
	int wcY = (int)floor( wc.y );

	bool isMultipleOfSixteenX = ( ( wcX & ( CHUNK_X_LENGTH_IN_BLOCKS - 1 ) ) == 0 );
	bool isMultipleOfSixteenY = ( ( wcY & ( CHUNK_Y_WIDTH_IN_BLOCKS - 1 ) ) == 0 );

	//Not replacing the divisions because sometimes (wcX/Y >> CHUNK_BITS_X/Y) != (wcX/Y / CHUNK_X/Y_IN_BLOCKS).
	cc.x = wcX / CHUNK_X_LENGTH_IN_BLOCKS;
	cc.y = wcY / CHUNK_Y_WIDTH_IN_BLOCKS;

	//Compensate for mins-aligned chunk coordinate system, where (0,0) implicates immediate top and right chunk.
	if ( wcX < 0 && !isMultipleOfSixteenX ) 
		cc.x--;
	if ( wcY < 0 && !isMultipleOfSixteenY ) 
		cc.y--;

	return cc;
}


//--------------------------------------------------------------------------------------------------------------
const char* GetDimensionAsString(Dimension dimension)
{
	switch ( dimension )
	{
		case DIM_OVERWORLD: return "Overworld";
		case DIM_NETHER: return "Nether";
		default: return "UnnamedDimension";
	}
}