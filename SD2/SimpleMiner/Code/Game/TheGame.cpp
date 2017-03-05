#include "Game/TheGame.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Audio/TheAudio.hpp"
#include "Engine/String/StringUtils.hpp"
#include "Engine/Input/TheInput.hpp"
#include "Engine/Renderer/RenderCommand.hpp"

#include "Game/GameCommon.hpp"
#include "Game/TheApp.hpp" 

#include "Game/BlockDefinition.hpp"
#include "Game/Camera3D.hpp"
#include "Game/Player.hpp"
#include "Game/World.hpp"
#include "Game/Chunk.hpp"


//-----------------------------------------------------------------------------
TheGame* g_theGame = nullptr;


//-----------------------------------------------------------------------------
TheGame::TheGame()
	: m_playerCamera( new Camera3D( CAMERA_DEFAULT_POSITION ) )
	, m_player( new Player( PLAYER_DEFAULT_POSITION ) )
{
	constexpr int TILE_DIMENSION = 16;
	constexpr int TILE_WIDTH = TILE_DIMENSION;
	constexpr int TILE_HEIGHT = TILE_DIMENSION;
	constexpr int NUM_TILE_ROWS = 16;
	constexpr int NUM_TILE_COLUMNS = 16;

	g_textureAtlas = new SpriteSheet( "Data/Images/SimpleMinerAtlas.png", 
									  TILE_WIDTH, TILE_HEIGHT, 
									  NUM_TILE_ROWS, NUM_TILE_COLUMNS );
	m_world = new World( m_playerCamera, m_player ); //Comes after to preserve init player pos.
}


//-----------------------------------------------------------------------------
TheGame::~TheGame( )
{
	m_world->SaveAndExitWorld();
	delete m_playerCamera;
	delete m_player;
	delete m_world;
	delete g_textureAtlas;
}


//-----------------------------------------------------------------------------
void TheGame::Update( float deltaSeconds )
{
	m_world->Update( deltaSeconds ); //Also updates player.
							
	UpdateDebugCommands( deltaSeconds );

	//Debug GameCommon flag setting. Toggling back and forth will cause some chunks to become and stay dirty until updated (usually by player raycast dirtying VAO), hence it's just for debug.
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_DEBUG_INFO ) ) 
		g_renderDebugInfo = !g_renderDebugInfo;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_RAYCAST_MODE ) ) 
		g_useAmanWooRaycastOverStepAndSample = !g_useAmanWooRaycastOverStepAndSample;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_DAYNIGHT_MODE ) ) 
		g_useNightLightLevel = !g_useNightLightLevel;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_LIGHTING_TEST_TEXTURE ) ) 
		g_useLightTestingTexture = !g_useLightTestingTexture;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_SKY_DEBUG ) ) 
		g_renderSkyBlocksAsDebugPoints = !g_renderSkyBlocksAsDebugPoints;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_COLORIZE_LIGHT_LEVELS ) ) 
		g_colorizeLightLevels = !g_colorizeLightLevels;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_VBO_AND_VA ) ) 
		g_renderChunksWithVertexArrays = !g_renderChunksWithVertexArrays;

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_CULLING ) ) 
		g_useCulling = !g_useCulling;

}


//-----------------------------------------------------------------------------
void TheGame::SetUpPerspectiveProjection()
{
	constexpr float aspect = ( 16.f / 9.f ); //Not speed-critical /'s because const expression handled during compilation.
	constexpr float fovDegreesVertical = 60.f; // ( fovDegreesHorizontal / aspect );
	constexpr float zNear = .1f;
	constexpr float zFar = 1000.f;

	g_theRenderer->SetPerspective( fovDegreesVertical, aspect, zNear, zFar );
}


//-----------------------------------------------------------------------------
void TheGame::ApplyCameraTransform()
{
	const Vector3 UNIT_X = Vector3( 1.f, 0.f, 0.f );
	const Vector3 UNIT_Y = Vector3( 0.f, 1.f, 0.f );
	const Vector3 UNIT_Z = Vector3( 0.f, 0.f, 1.f );

	//Change of basis detour application to move from OGL (y-up right-handed world-not-screen) to our coordinate system (z-up right-handed world) orientation.
	g_theRenderer->RotateViewByDegrees( -90.f, UNIT_X ); //Rotate z-forward to z-up, a negative rotation about the x-axis.
	g_theRenderer->RotateViewByDegrees( 90.f, UNIT_Z ); //Rotate x-right to x-forward, a positive rotation about the z-axis.

	//Anti-rotation. (MP1 Recap: transposing rotation == inverse... so why does negating work?)
	g_theRenderer->RotateViewByDegrees( -1.f * m_playerCamera->m_orientation.m_rollDegreesAboutX, UNIT_X ); //Anti-roll.
	g_theRenderer->RotateViewByDegrees( -1.f * m_playerCamera->m_orientation.m_pitchDegreesAboutY, UNIT_Y ); //Anti-pitch.
	g_theRenderer->RotateViewByDegrees( -1.f * m_playerCamera->m_orientation.m_yawDegreesAboutZ, UNIT_Z ); //Anti-yaw.

	//Anti-translation. (MP1 Recap: negating translation == inverse!)
	g_theRenderer->TranslateView( m_playerCamera->m_worldPosition * -1.f ); 
}


//-----------------------------------------------------------------------------
void TheGame::SetupView3D()
{
	if ( m_world->GetActiveDimension() == DIM_NETHER ) 
		g_theRenderer->ClearScreenToColor( Rgba::BLACK );

	if ( m_world->GetActiveDimension() == DIM_OVERWORLD ) 
		g_theRenderer->ClearScreenToColor( Rgba::CYAN );

	g_theRenderer->ClearScreenDepthBuffer();

	SetUpPerspectiveProjection();

	ApplyCameraTransform();

	g_theRenderer->EnableDepthTesting( true );
	g_theRenderer->EnableBackfaceCulling( true );
	g_theRenderer->EnableAlphaTesting( true );
}


//-----------------------------------------------------------------------------
void TheGame::Render3D()
{
	m_world->Render();

	if ( m_player->IsDigging() )
	{
		AABB2 damageTexCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( m_world->m_currentDigDamageFrame, 14 );
		constexpr bool SHOW_DAMAGE_ON_ALL_SIDES = true;
		constexpr bool WITH_BACKFACE_CULLING = true;
		constexpr bool WITH_DEPTH_TEST = true;
		Render3DOverlayWithSprite( damageTexCoords, SHOW_DAMAGE_ON_ALL_SIDES, WITH_BACKFACE_CULLING, WITH_DEPTH_TEST );
	}

	//Draw block selection overlay, given a block is selected.
	AABB2 overlayTexCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 5, 13 );
	constexpr bool SHOW_SELECTION_OVERLAY_ON_ALL_SIDES = false;
	Render3DOverlayWithSprite( overlayTexCoords, SHOW_SELECTION_OVERLAY_ON_ALL_SIDES );
}


//-----------------------------------------------------------------------------
void TheGame::RenderDebug3D()
{
	constexpr bool WITH_Z_FOR_3D = true;
	g_theRenderer->DrawDebugAxes( 10.f, 1.f, WITH_Z_FOR_3D );

	//Draw debug commands, for now a sphere slightly off-origin to affirm they render along proper +/- direction.
	g_theRenderCommands->push_back( new RenderCommandSphere( Vector3::ONE, 1.f, 0.f, DepthMode::DEPTH_TEST_DUAL, Rgba::WHITE, 4.f ) );

	RenderAndExpireDebugCommands();

	//Draw player ray.
	Vector3 camDirection = m_playerCamera->GetForwardXYZ();
	camDirection.Normalize();
	Vector3 endOfPlayerRaycast = m_player->m_worldPosition + ( camDirection * LENGTH_OF_SELECTION_RAYCAST );

	g_theRenderer->EnableDepthTesting( true );
	g_theRenderer->DrawLine( m_player->m_worldPosition, endOfPlayerRaycast, Rgba::WHITE, Rgba::RED, 10.f );
	g_theRenderer->EnableDepthTesting( false );
}


//-----------------------------------------------------------------------------
void TheGame::SetupView2D()
{
	g_theRenderer->EnableDepthTesting( false );
	g_theRenderer->EnableBackfaceCulling( false );

	float screenWidth = (float)g_theApp->GetScreenWidth();
	float screenHeight = (float)g_theApp->GetScreenHeight();
	g_theRenderer->SetOrtho( Vector2::ZERO, 
							 Vector2( screenWidth, screenHeight ) );
}


//-----------------------------------------------------------------------------
void TheGame::Render2D()
{
	if ( g_renderDebugInfo )
	{
		RenderReticle();
	}
	RenderBlockHotbar();
}


//-----------------------------------------------------------------------------
void TheGame::RenderBlockHotbar()
{
	for ( int hudElementIndex = 0; hudElementIndex < NUM_BLOCK_TYPES; hudElementIndex++ )
	{
		Vector2 renderMins = HUD_BOTTOM_LEFT_POSITION; //Reset each time deliberately for below line.
		renderMins.x += hudElementIndex * ( HUD_ELEMENT_WIDTH + HUD_WIDTH_BETWEEN_ELEMENTS );
		Vector2 renderMaxs = renderMins + Vector2( HUD_ELEMENT_WIDTH, HUD_HEIGHT );
		AABB2 renderBounds = AABB2( renderMins, renderMaxs );

		Rgba tint = ( m_world->GetActiveHudElement() == hudElementIndex ) ? Rgba::GRAY : Rgba::WHITE;

		g_theRenderer->DrawAABB
		(
			TheRenderer::VertexGroupingRule::AS_QUADS,
			renderBounds,
			*g_textureAtlas->GetAtlasTexture(),
			BlockDefinition::GetSideTexCoords( static_cast<BlockType>( hudElementIndex + 1 ) ), //SKIP AIR.
			tint
		);
	}
}


//-----------------------------------------------------------------------------
void TheGame::RenderReticle()
{
	Vector2 screenCenter = Vector2( (float)g_theApp->GetScreenWidth(), (float)g_theApp->GetScreenHeight() );
	screenCenter.x /= 2.0f;
	screenCenter.y /= 2.0f;

	Vector2 crosshairLeft = Vector2( screenCenter.x - HUD_CROSSHAIR_RADIUS, screenCenter.y );
	Vector2 crosshairRight = Vector2( screenCenter.x + HUD_CROSSHAIR_RADIUS, screenCenter.y );
	Vector2 crosshairUp = Vector2( screenCenter.x, screenCenter.y - HUD_CROSSHAIR_RADIUS );
	Vector2 crosshairDown = Vector2( screenCenter.x, screenCenter.y + HUD_CROSSHAIR_RADIUS );

	constexpr int GL_ONE_MINUS_DST_COLOR = 0x0307; //For clarity, as otherwise GL.h is not in view.
	constexpr int GL_ZERO = 0;
	constexpr int GL_SRC_ALPHA = 0x0302;
	constexpr int GL_ONE_MINUS_SRC_ALPHA = 0x0303;

	//Draw reticle with an inverse blend to keep it high-contrast for visibility:
	g_theRenderer->SetBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ZERO );
	g_theRenderer->DrawLine( crosshairLeft, crosshairRight, Rgba(), Rgba(), HUD_CROSSHAIR_THICKNESS );
	g_theRenderer->DrawLine( crosshairUp, crosshairDown, Rgba(), Rgba(), HUD_CROSSHAIR_THICKNESS );
	g_theRenderer->SetBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); //And then restore it back to the normal alpha blend.
}


//-----------------------------------------------------------------------------
void TheGame::RenderDebug2D()
{
	RenderLeftSideDebug2D();
	RenderRightSideDebug2D();
}


//-----------------------------------------------------------------------------
void TheGame::RenderRightSideDebug2D()
{
	char* mode = "";
	if ( g_currentMovementMode == WALKING )
		mode = "WALKING";
	if ( g_currentMovementMode == FLYING )
		mode = "FLYING";
	if ( g_currentMovementMode == NOCLIP )
		mode = "NOCLIP";

	g_theRenderer->DrawText2D
		(
		Vector2( (float)g_theApp->GetScreenWidth() - 375.f, (float)g_theApp->GetScreenHeight() - 50.f ),
		Stringf( "Movement Mode: %s", mode ),
		18.f,
		Rgba(),
		nullptr,
		.65f
	);

	if ( g_currentCameraMode == FREE_SPECTATOR )
		mode = "FREE SPECTATOR";
	if ( g_currentCameraMode == FIRST_PERSON )
		mode = "FIRST PERSON";
	if ( g_currentCameraMode == FIXED_SPECTATOR )
		mode = "FIXED SPECTATOR";
	if ( g_currentCameraMode == FROM_BEHIND )
		mode = "FROM BEHIND";

	g_theRenderer->DrawText2D
		(
		Vector2( (float)g_theApp->GetScreenWidth() - 375.f, (float)g_theApp->GetScreenHeight() - 100.f ),
		Stringf( "Camera Mode: %s", mode ),
		18.f,
		Rgba(),
		nullptr,
		.65f
		);

	g_theRenderer->DrawText2D
		(
		Vector2( (float)g_theApp->GetScreenWidth() - 375.f, (float)g_theApp->GetScreenHeight() - 150.f ),
		( g_renderChunksWithVertexArrays ) ? "Chunk Render Mode: VAs" : "Chunk Render Mode: VBOs",
		18.f,
		Rgba(),
		nullptr,
		.65f
		);

	g_theRenderer->DrawText2D
		(
		Vector2( (float)g_theApp->GetScreenWidth() - 375.f, (float)g_theApp->GetScreenHeight() - 200.f ),
		( m_world->m_activeDimension == DIM_OVERWORLD ) ? "Dimension: Overworld" : "Dimension: Nether",
		18.f,
		Rgba(),
		nullptr,
		.65f
		);
}


//-----------------------------------------------------------------------------
void TheGame::RenderLeftSideDebug2D()
{
	WorldCoords camPos = m_playerCamera->m_worldPosition;
	EulerAngles camOri = m_playerCamera->m_orientation;
	WorldCoords playerPos = m_player->m_worldPosition;
	Vector3 playerVel = m_player->m_velocity;
	Vector3 camDir = m_playerCamera->GetForwardXYZ();

	float screenHeight = (float)g_theApp->GetScreenHeight();
	const Rgba darkerGray = Rgba( .7f, .7f, .7f );
	const Rgba lighterGray = Rgba( .8f, .8f, .8f );
	constexpr float CELL_HEIGHT = 18.f;
	constexpr float CELL_ASPECT = .65f;
	constexpr float TEXT_LEFT_X = 100.f;

	g_theRenderer->DrawText2D( Vector2( TEXT_LEFT_X, screenHeight - 50.f ),
								Stringf( "Camera Position: %f %f %f", camPos.x, camPos.y, camPos.z ),
							   CELL_HEIGHT );

	g_theRenderer->DrawText2D( Vector2( TEXT_LEFT_X, screenHeight - 100.f ),
								Stringf( "Camera Orientation: %f %f %f", camOri.m_rollDegreesAboutX, camOri.m_pitchDegreesAboutY, camOri.m_yawDegreesAboutZ ),
								CELL_HEIGHT, darkerGray, nullptr, CELL_ASPECT );

	g_theRenderer->DrawText2D( Vector2( TEXT_LEFT_X, screenHeight - 150.f ),
								Stringf( "Camera Forward XYZ: %f %f %f", camDir.x, camDir.y, camDir.z ),
								CELL_HEIGHT, darkerGray, nullptr, CELL_ASPECT );

	g_theRenderer->DrawText2D( Vector2( TEXT_LEFT_X, screenHeight - 200.f ),
								( g_useAmanWooRaycastOverStepAndSample ) ? "Raycast Mode: Amanatides-Woo" : "Raycast Mode: Step-and-Sample",
								CELL_HEIGHT, lighterGray, nullptr, CELL_ASPECT );

	g_theRenderer->DrawText2D( Vector2( TEXT_LEFT_X, screenHeight - 250.f ),
								Stringf( "Player Position: %f %f %f", playerPos.x, playerPos.y, playerPos.z ),
								CELL_HEIGHT );

	g_theRenderer->DrawText2D( Vector2( TEXT_LEFT_X, screenHeight - 300.f ),
								Stringf( "Player Velocity: %f %f %f", playerVel.x, playerVel.y, playerVel.z ),
							   CELL_HEIGHT, darkerGray, nullptr, CELL_ASPECT );

	g_theRenderer->DrawText2D( Vector2( TEXT_LEFT_X, screenHeight - 350.f ),
								Stringf( "Active Chunk Count: %i", (int)m_world->m_activeChunks[ m_world->m_activeDimension ].size() ),
							   CELL_HEIGHT, darkerGray, nullptr, CELL_ASPECT );

	g_theRenderer->DrawText2D( Vector2( TEXT_LEFT_X, screenHeight - 400.f ),
								Stringf( "Rendered Chunk Count: %i", g_chunksRendered ),
							   CELL_HEIGHT, darkerGray, nullptr, CELL_ASPECT );
}


//-----------------------------------------------------------------------------
void TheGame::Render3DOverlayWithSprite( const AABB2& overlayTexCoords, bool onAllSides, bool enableBackfaceCulling /*= false*/, bool enableDepthTesting /*= false*/ )
{
	//With depth test enabled, other blocks will cover up selection overlay. Could offset by translating just before the selected face.
	g_theRenderer->EnableDepthTesting( enableDepthTesting ); 
	g_theRenderer->EnableBackfaceCulling( enableBackfaceCulling );

	std::vector< Vertex3D_PCT > selectionOverlayVertexes;
	constexpr float OFFSET_AMOUNT = .05f;
	AABB3* bounds = new AABB3();
	GetSelectedBlockBounds( bounds );
	if ( bounds == nullptr )
		return;

	Vector3 bottomLeftVertex, bottomRightVertex, topRightVertex, topLeftVertex;

	BlockFace faceSelected = GetSelectedBlockFace();
	if ( faceSelected == NONE )
		return;

	const float BOUNDS_MAX_X = bounds->maxs.x;
	const float BOUNDS_MAX_Y = bounds->maxs.y;
	const float BOUNDS_MAX_Z = bounds->maxs.z;
	const float BOUNDS_MIN_X = bounds->mins.x;
	const float BOUNDS_MIN_Y = bounds->mins.y;
	const float BOUNDS_MIN_Z = bounds->mins.z;

	if ( onAllSides || faceSelected == BOTTOM )
	{
		bottomLeftVertex =	Vector3( BOUNDS_MAX_X, BOUNDS_MAX_Y, BOUNDS_MIN_Z - OFFSET_AMOUNT );
		bottomRightVertex = Vector3( BOUNDS_MAX_X, BOUNDS_MIN_Y, BOUNDS_MIN_Z - OFFSET_AMOUNT );
		topRightVertex =	Vector3( BOUNDS_MIN_X, BOUNDS_MIN_Y, BOUNDS_MIN_Z - OFFSET_AMOUNT );
		topLeftVertex =		Vector3( BOUNDS_MIN_X, BOUNDS_MAX_Y, BOUNDS_MIN_Z - OFFSET_AMOUNT );

		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomLeftVertex,		Vector2( overlayTexCoords.mins.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomRightVertex,	Vector2( overlayTexCoords.maxs.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topRightVertex,		Vector2( overlayTexCoords.maxs.x, overlayTexCoords.mins.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topLeftVertex,		Vector2( overlayTexCoords.mins.x, overlayTexCoords.mins.y ) ) );
	}
	if ( onAllSides || faceSelected == TOP )
	{
		bottomLeftVertex =	Vector3( BOUNDS_MIN_X, BOUNDS_MAX_Y, BOUNDS_MAX_Z + OFFSET_AMOUNT );
		bottomRightVertex = Vector3( BOUNDS_MIN_X, BOUNDS_MIN_Y, BOUNDS_MAX_Z + OFFSET_AMOUNT );
		topRightVertex =	Vector3( BOUNDS_MAX_X, BOUNDS_MIN_Y, BOUNDS_MAX_Z + OFFSET_AMOUNT );
		topLeftVertex =		Vector3( BOUNDS_MAX_X, BOUNDS_MAX_Y, BOUNDS_MAX_Z + OFFSET_AMOUNT );

		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomLeftVertex,		Vector2( overlayTexCoords.mins.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomRightVertex,	Vector2( overlayTexCoords.maxs.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topRightVertex,		Vector2( overlayTexCoords.maxs.x, overlayTexCoords.mins.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topLeftVertex,		Vector2( overlayTexCoords.mins.x, overlayTexCoords.mins.y ) ) );
	}
	if ( onAllSides || faceSelected == LEFT )
	{
		bottomLeftVertex =	Vector3( BOUNDS_MAX_X, BOUNDS_MAX_Y + OFFSET_AMOUNT, BOUNDS_MIN_Z );
		bottomRightVertex = Vector3( BOUNDS_MIN_X, BOUNDS_MAX_Y + OFFSET_AMOUNT, BOUNDS_MIN_Z );
		topRightVertex =	Vector3( BOUNDS_MIN_X, BOUNDS_MAX_Y + OFFSET_AMOUNT, BOUNDS_MAX_Z );
		topLeftVertex =		Vector3( BOUNDS_MAX_X, BOUNDS_MAX_Y + OFFSET_AMOUNT, BOUNDS_MAX_Z );

		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomLeftVertex,		Vector2( overlayTexCoords.mins.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomRightVertex,	Vector2( overlayTexCoords.maxs.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topRightVertex,		Vector2( overlayTexCoords.maxs.x, overlayTexCoords.mins.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topLeftVertex,		Vector2( overlayTexCoords.mins.x, overlayTexCoords.mins.y ) ) );
	}
	if ( onAllSides || faceSelected == RIGHT )
	{
		bottomLeftVertex =	Vector3( BOUNDS_MIN_X, BOUNDS_MIN_Y - OFFSET_AMOUNT, BOUNDS_MIN_Z );
		bottomRightVertex = Vector3( BOUNDS_MAX_X, BOUNDS_MIN_Y - OFFSET_AMOUNT, BOUNDS_MIN_Z );
		topRightVertex =	Vector3( BOUNDS_MAX_X, BOUNDS_MIN_Y - OFFSET_AMOUNT, BOUNDS_MAX_Z );
		topLeftVertex =		Vector3( BOUNDS_MIN_X, BOUNDS_MIN_Y - OFFSET_AMOUNT, BOUNDS_MAX_Z );

		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomLeftVertex,		Vector2( overlayTexCoords.mins.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomRightVertex,	Vector2( overlayTexCoords.maxs.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topRightVertex,		Vector2( overlayTexCoords.maxs.x, overlayTexCoords.mins.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topLeftVertex,		Vector2( overlayTexCoords.mins.x, overlayTexCoords.mins.y ) ) );
	}
	if ( onAllSides || faceSelected == FRONT )
	{
		bottomLeftVertex =	Vector3( BOUNDS_MIN_X - OFFSET_AMOUNT, BOUNDS_MAX_Y, BOUNDS_MIN_Z );
		bottomRightVertex = Vector3( BOUNDS_MIN_X - OFFSET_AMOUNT, BOUNDS_MIN_Y, BOUNDS_MIN_Z );
		topRightVertex =	Vector3( BOUNDS_MIN_X - OFFSET_AMOUNT, BOUNDS_MIN_Y, BOUNDS_MAX_Z );
		topLeftVertex =		Vector3( BOUNDS_MIN_X - OFFSET_AMOUNT, BOUNDS_MAX_Y, BOUNDS_MAX_Z );

		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomLeftVertex,		Vector2( overlayTexCoords.mins.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomRightVertex,	Vector2( overlayTexCoords.maxs.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topRightVertex,		Vector2( overlayTexCoords.maxs.x, overlayTexCoords.mins.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topLeftVertex,		Vector2( overlayTexCoords.mins.x, overlayTexCoords.mins.y ) ) );
	}
	if ( onAllSides || faceSelected == BACK )
	{
		bottomLeftVertex =	Vector3( BOUNDS_MAX_X + OFFSET_AMOUNT, BOUNDS_MIN_Y, BOUNDS_MIN_Z );
		bottomRightVertex = Vector3( BOUNDS_MAX_X + OFFSET_AMOUNT, BOUNDS_MAX_Y, BOUNDS_MIN_Z );
		topRightVertex =	Vector3( BOUNDS_MAX_X + OFFSET_AMOUNT, BOUNDS_MAX_Y, BOUNDS_MAX_Z );
		topLeftVertex =		Vector3( BOUNDS_MAX_X + OFFSET_AMOUNT, BOUNDS_MIN_Y, BOUNDS_MAX_Z );

		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomLeftVertex,		Vector2( overlayTexCoords.mins.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomRightVertex,	Vector2( overlayTexCoords.maxs.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topRightVertex,		Vector2( overlayTexCoords.maxs.x, overlayTexCoords.mins.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topLeftVertex,		Vector2( overlayTexCoords.mins.x, overlayTexCoords.mins.y ) ) );
	}
	delete bounds;

	g_theRenderer->BindTexture( g_textureAtlas->GetAtlasTexture() );
	g_theRenderer->DrawVertexArray_PCT( TheRenderer::VertexGroupingRule::AS_QUADS, selectionOverlayVertexes, selectionOverlayVertexes.size() );
	g_theRenderer->EnableDepthTesting( false );
	g_theRenderer->EnableBackfaceCulling( false );
}


//-----------------------------------------------------------------------------
void TheGame::GetSelectedBlockBounds( AABB3* out_selectedBlockBounds )
{
	Chunk* selectedChunk = m_world->m_chunkOfSelectedBlock;
	if ( selectedChunk == nullptr ) //No such selected block exists.
	{
		if ( out_selectedBlockBounds != nullptr ) 
			delete out_selectedBlockBounds;

		out_selectedBlockBounds = nullptr;
		return;
	}

	LocalBlockIndex lbiSelected = selectedChunk->m_selectedBlock;
	if ( ( lbiSelected < 0 ) || ( lbiSelected >= NUM_BLOCKS_PER_CHUNK ) )
	{
		if ( out_selectedBlockBounds != nullptr ) 
			delete out_selectedBlockBounds;

		out_selectedBlockBounds = nullptr;
		return;
	}

	const Vector3& blockSize = Vector3::ONE;
	WorldCoords renderBoundsMins = selectedChunk->GetWorldCoordsFromLocalBlockIndex( lbiSelected );
	*out_selectedBlockBounds = AABB3( renderBoundsMins, renderBoundsMins + blockSize );
}


//-----------------------------------------------------------------------------
BlockFace TheGame::GetSelectedBlockFace()
{
	Chunk* selectedChunk = m_world->m_chunkOfSelectedBlock;
	if ( selectedChunk == nullptr )
		return NONE;

	return selectedChunk->m_selectedFace;
}


//-----------------------------------------------------------------------------
void TheGame::Render()
{
	SetupView3D();
	Render3D();
	if ( g_renderDebugInfo )
		RenderDebug3D();

	SetupView2D();
	Render2D();
	if ( g_renderDebugInfo )
		RenderDebug2D( );
}
