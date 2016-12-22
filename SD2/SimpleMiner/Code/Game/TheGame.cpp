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

//Be sure to #include all forward declared classes' .hpp's.
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
	g_textureAtlas = new SpriteSheet( "Data/Images/SimpleMinerAtlas.png", 16, 16, 16, 16 );
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

	//delete m_ship; //Whatever represents player.
	//for ( int i = 0; i < m_numBulletsAllocated; i++ ) delete m_bullets[ i ]; //Etc.
}


//-----------------------------------------------------------------------------
void TheGame::Update( float deltaSeconds )
{
	m_world->Update( deltaSeconds ); //Also updates player.
									
	//Update debug commands.
	UpdateDebugCommands( deltaSeconds );

	//Debug GameCommon flag setting. Toggling back and forth will cause some chunks to become and stay dirty until updated (usually by player raycast dirtying VAO), hence it's just for debug.
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_DEBUG_INFO ) ) g_renderDebugInfo = !g_renderDebugInfo;
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_RAYCAST_MODE ) ) g_useAmanWooRaycastOverStepAndSample = !g_useAmanWooRaycastOverStepAndSample;
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_DAYNIGHT_MODE ) ) g_useNightLightLevel = !g_useNightLightLevel;
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_LIGHTING_TEST_TEXTURE ) ) g_useLightTestingTexture = !g_useLightTestingTexture;
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_SKY_DEBUG ) ) g_renderSkyBlocksAsDebugPoints = !g_renderSkyBlocksAsDebugPoints;
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_COLORIZE_LIGHT_LEVELS ) ) g_colorizeLightLevels = !g_colorizeLightLevels;
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_VBO_AND_VA ) ) g_renderChunksWithVertexArrays = !g_renderChunksWithVertexArrays;
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_CULLING ) ) g_useCulling = !g_useCulling;
}


//-----------------------------------------------------------------------------
void TheGame::SetUpPerspectiveProjection()
{
	float aspect = ( 16.f / 9.f ); //Not speed-critical /'s because const expression handled during compilation.
	//float fovDegreesHorizontal = 180.f; //Lowering me zooms in! Raising me produces fisheye effect!
	float fovDegreesVertical = 60.f; // ( fovDegreesHorizontal / aspect );
	float zNear = .1f;
	float zFar = 1000.f;

	g_theRenderer->SetPerspective( fovDegreesVertical, aspect, zNear, zFar ); // The math for how this works is very nuanced and above this class, will be in SD3. 
}


//-----------------------------------------------------------------------------
void TheGame::ApplyCameraTransform()
{
	//Change of basis detour application to move from OGL (y-up right-handed world-not-screen) to our coordinate system (z-up right-handed world) orientation.
	g_theRenderer->RotateViewByDegrees( -90.f, Vector3( 1.f, 0.f, 0.f ) ); //Rotate z-forward to z-up, a negative rotation about the x-axis.
	g_theRenderer->RotateViewByDegrees( 90.f, Vector3( 0.f, 0.f, 1.f ) ); //Rotate x-right to x-forward, a positive rotation about the z-axis.

	//Anti-rotation. (MP1 Recap: transposing rotation == inverse... so why does negating work?)
	g_theRenderer->RotateViewByDegrees( -1.f * m_playerCamera->m_orientation.m_rollDegreesAboutX, Vector3( 1.f, 0.f, 0.f ) ); //Anti-roll.
	g_theRenderer->RotateViewByDegrees( -1.f * m_playerCamera->m_orientation.m_pitchDegreesAboutY, Vector3( 0.f, 1.f, 0.f ) ); //Anti-pitch.
	g_theRenderer->RotateViewByDegrees( -1.f * m_playerCamera->m_orientation.m_yawDegreesAboutZ, Vector3( 0.f, 0.f, 1.f ) ); //Anti-yaw.

	//Anti-translation. (MP1 Recap: negating translation == inverse!)
	g_theRenderer->TranslateView( m_playerCamera->m_worldPosition * -1.f ); 
}


//-----------------------------------------------------------------------------
void TheGame::SetupView3D()
{
	if ( m_world->GetActiveDimension() == DIM_NETHER ) g_theRenderer->ClearScreenToColor( Rgba::BLACK );
	if ( m_world->GetActiveDimension() == DIM_OVERWORLD ) g_theRenderer->ClearScreenToColor( Rgba::CYAN );
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
		Render3DOverlayWithSprite( g_textureAtlas->GetTexCoordsFromSpriteCoords( m_world->m_currentDigDamageFrame, 14 ), true, true, true );

	//Draw block selection overlay, given a block is selected.
	Render3DOverlayWithSprite( g_textureAtlas->GetTexCoordsFromSpriteCoords( 5, 13 ), false );
}


//-----------------------------------------------------------------------------
void TheGame::RenderDebug3D()
{
	g_theRenderer->DrawDebugAxes( 10.f, 1.f, true );

	//Draw debug commands.
	//g_theRenderCommands->push_back( new RenderCommandPoint( Vector3(2.f, 2.f, 2.f), 0.f, DepthMode::DEPTH_TEST_DUAL, Rgba::MAGENTA ) );
	//g_theRenderCommands->push_back( new RenderCommandArrow( Vector3::ZERO, Vector3::ONE * -2.f, 0.f, DepthMode::DEPTH_TEST_DUAL, Rgba::GRAY, 2.f ) );
	//g_theRenderCommands->push_back( new RenderCommandLine( Vector3::ZERO, Vector3( 0.f, 0.f, 1.f ), 0.f, DepthMode::DEPTH_TEST_DUAL, Rgba::YELLOW, 3.f ) );
	//g_theRenderCommands->push_back( new RenderCommandAABB3( AABB3( Vector3( -1.f, -1.f, -1.f ), Vector3( -2.f, -2.f, -2.f ) ), 0.f, DepthMode::DEPTH_TEST_DUAL, Rgba::MAGENTA, 0.5f ) );
	g_theRenderCommands->push_back( new RenderCommandSphere( Vector3::ONE, 1.f, 0.f, DepthMode::DEPTH_TEST_DUAL, Rgba::WHITE, 4.f ) );


	RenderAndExpireDebugCommands();

	//Draw player ray.
	Vector3 camDirection = m_playerCamera->GetForwardXYZ();
	camDirection.Normalize();
	Vector3 endOfPlayerRaycast = m_player->m_worldPosition + ( camDirection * LENGTH_OF_SELECTION_RAYCAST );

	g_theRenderer->EnableDepthTesting( true );
	g_theRenderer->DrawLine( m_player->m_worldPosition, endOfPlayerRaycast, Rgba(), Rgba( 1.f, 0.f, 0.f ), 10.f );
	g_theRenderer->EnableDepthTesting( false );
}


//-----------------------------------------------------------------------------
void TheGame::SetupView2D()
{
	g_theRenderer->EnableDepthTesting( false );
	g_theRenderer->EnableBackfaceCulling( false );

	g_theRenderer->SetOrtho( Vector2( 0.f, 0.f ), 
		Vector2( (float)g_theApp->GetScreenWidth( ), (float)g_theApp->GetScreenHeight( ) ) );
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

	g_theRenderer->SetBlendFunc( 0x0307, 0 ); //GL CONSTANTS.
	g_theRenderer->DrawLine( crosshairLeft, crosshairRight, Rgba(), Rgba(), HUD_CROSSHAIR_THICKNESS );
	g_theRenderer->DrawLine( crosshairUp, crosshairDown, Rgba(), Rgba(), HUD_CROSSHAIR_THICKNESS );
	g_theRenderer->SetBlendFunc( 0x0302, 0x0303 );
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
	if ( g_currentMovementMode == WALKING ) mode = "WALKING";
	if ( g_currentMovementMode == FLYING ) mode = "FLYING";
	if ( g_currentMovementMode == NOCLIP ) mode = "NOCLIP";
	g_theRenderer->DrawText2D
		(
		Vector2( (float)g_theApp->GetScreenWidth() - 375.f, (float)g_theApp->GetScreenHeight() - 50.f ),
		Stringf( "Movement Mode: %s", mode ),
		18.f,
		Rgba(),
		nullptr,
		.65f
	);

	if ( g_currentCameraMode == FREE_SPECTATOR ) mode = "FREE SPECTATOR";
	if ( g_currentCameraMode == FIRST_PERSON ) mode = "FIRST PERSON";
	if ( g_currentCameraMode == FIXED_SPECTATOR ) mode = "FIXED SPECTATOR";
	if ( g_currentCameraMode == FROM_BEHIND ) mode = "FROM BEHIND";
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

	g_theRenderer->DrawText2D( Vector2( 100.f, (float)g_theApp->GetScreenHeight() - 50.f ),
		Stringf( "Camera Position: %f %f %f", camPos.x, camPos.y, camPos.z ), 18.f );
	g_theRenderer->DrawText2D( Vector2( 100.f, (float)g_theApp->GetScreenHeight() - 100.f ),
		Stringf( "Camera Orientation: %f %f %f", camOri.m_rollDegreesAboutX, camOri.m_pitchDegreesAboutY, camOri.m_yawDegreesAboutZ ), 18.f, Rgba( .7f, .7f, .7f ), nullptr, .65f );
	g_theRenderer->DrawText2D( Vector2( 100.f, (float)g_theApp->GetScreenHeight() - 150.f ),
		Stringf( "Camera Forward XYZ: %f %f %f", camDir.x, camDir.y, camDir.z ), 18.f, Rgba( .7f, .7f, .7f ), nullptr, .65f );

	g_theRenderer->DrawText2D( Vector2( 100.f, (float)g_theApp->GetScreenHeight() - 200.f ),
		( g_useAmanWooRaycastOverStepAndSample ) ? "Raycast Mode: Amanatides-Woo" : "Raycast Mode: Step-and-Sample", 18.f, Rgba( .8f, .8f, .8f ), nullptr, .65f );

	g_theRenderer->DrawText2D( Vector2( 100.f, (float)g_theApp->GetScreenHeight() - 250.f ),
							   Stringf( "Player Position: %f %f %f", playerPos.x, playerPos.y, playerPos.z ), 18.f );
	g_theRenderer->DrawText2D( Vector2( 100.f, (float)g_theApp->GetScreenHeight() - 300.f ),
							   Stringf( "Player Velocity: %f %f %f", playerVel.x, playerVel.y, playerVel.z ), 18.f, Rgba( .7f, .7f, .7f ), nullptr, .65f );

	g_theRenderer->DrawText2D( Vector2( 100.f, (float)g_theApp->GetScreenHeight() - 350.f ),
							   Stringf( "Active Chunk Count: %i", (int)m_world->m_activeChunks[ m_world->m_activeDimension ].size() ), 18.f, Rgba( .7f, .7f, .7f ), nullptr, .65f );

	g_theRenderer->DrawText2D( Vector2( 100.f, (float)g_theApp->GetScreenHeight() - 400.f ),
							   Stringf( "Rendered Chunk Count: %i", g_chunksRendered ), 18.f, Rgba( .7f, .7f, .7f ), nullptr, .65f );
}


//-----------------------------------------------------------------------------
void TheGame::Render3DOverlayWithSprite( const AABB2& overlayTexCoords, bool onAllSides, bool enableBackfaceCulling /*= false*/, bool enableDepthTesting /*= false*/ )
{
	g_theRenderer->EnableDepthTesting( enableDepthTesting ); //When true, other blocks will cover it up. Could offset by translating just before the selected face.
	g_theRenderer->EnableBackfaceCulling( enableBackfaceCulling );
	std::vector< Vertex3D_PCT > selectionOverlayVertexes;
	float OFFSET_AMOUNT = .05f;
	AABB3* bounds = new AABB3();
	GetSelectedBlockBounds( bounds );
	if ( bounds == nullptr ) return;

	Vector3 bottomLeftVertex, bottomRightVertex, topRightVertex, topLeftVertex;

	BlockFace faceSelected = GetSelectedBlockFace();
	if ( faceSelected == NONE ) return;

	if ( onAllSides || faceSelected == BOTTOM )
	{
		bottomLeftVertex = Vector3( bounds->maxs.x, bounds->maxs.y, bounds->mins.z - OFFSET_AMOUNT );
		bottomRightVertex = Vector3( bounds->maxs.x, bounds->mins.y, bounds->mins.z - OFFSET_AMOUNT );
		topRightVertex = Vector3( bounds->mins.x, bounds->mins.y, bounds->mins.z - OFFSET_AMOUNT );
		topLeftVertex = Vector3( bounds->mins.x, bounds->maxs.y, bounds->mins.z - OFFSET_AMOUNT );

		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomLeftVertex, Vector2( overlayTexCoords.mins.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomRightVertex, Vector2( overlayTexCoords.maxs.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topRightVertex, Vector2( overlayTexCoords.maxs.x, overlayTexCoords.mins.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topLeftVertex, Vector2( overlayTexCoords.mins.x, overlayTexCoords.mins.y ) ) );
	}
	if ( onAllSides || faceSelected == TOP )
	{
		bottomLeftVertex = Vector3( bounds->mins.x, bounds->maxs.y, bounds->maxs.z + OFFSET_AMOUNT );
		bottomRightVertex = Vector3( bounds->mins.x, bounds->mins.y, bounds->maxs.z + OFFSET_AMOUNT );
		topRightVertex = Vector3( bounds->maxs.x, bounds->mins.y, bounds->maxs.z + OFFSET_AMOUNT );
		topLeftVertex = Vector3( bounds->maxs.x, bounds->maxs.y, bounds->maxs.z + OFFSET_AMOUNT );

		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomLeftVertex, Vector2( overlayTexCoords.mins.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomRightVertex, Vector2( overlayTexCoords.maxs.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topRightVertex, Vector2( overlayTexCoords.maxs.x, overlayTexCoords.mins.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topLeftVertex, Vector2( overlayTexCoords.mins.x, overlayTexCoords.mins.y ) ) );
	}
	if ( onAllSides || faceSelected == LEFT )
	{
		bottomLeftVertex = Vector3( bounds->maxs.x, bounds->maxs.y + OFFSET_AMOUNT, bounds->mins.z );
		bottomRightVertex = Vector3( bounds->mins.x, bounds->maxs.y + OFFSET_AMOUNT, bounds->mins.z );
		topRightVertex = Vector3( bounds->mins.x, bounds->maxs.y + OFFSET_AMOUNT, bounds->maxs.z );
		topLeftVertex = Vector3( bounds->maxs.x, bounds->maxs.y + OFFSET_AMOUNT, bounds->maxs.z );

		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomLeftVertex, Vector2( overlayTexCoords.mins.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomRightVertex, Vector2( overlayTexCoords.maxs.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topRightVertex, Vector2( overlayTexCoords.maxs.x, overlayTexCoords.mins.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topLeftVertex, Vector2( overlayTexCoords.mins.x, overlayTexCoords.mins.y ) ) );
	}
	if ( onAllSides || faceSelected == RIGHT )
	{
		bottomLeftVertex = Vector3( bounds->mins.x, bounds->mins.y - OFFSET_AMOUNT, bounds->mins.z );
		bottomRightVertex = Vector3( bounds->maxs.x, bounds->mins.y - OFFSET_AMOUNT, bounds->mins.z );
		topRightVertex = Vector3( bounds->maxs.x, bounds->mins.y - OFFSET_AMOUNT, bounds->maxs.z );
		topLeftVertex = Vector3( bounds->mins.x, bounds->mins.y - OFFSET_AMOUNT, bounds->maxs.z );

		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomLeftVertex, Vector2( overlayTexCoords.mins.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomRightVertex, Vector2( overlayTexCoords.maxs.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topRightVertex, Vector2( overlayTexCoords.maxs.x, overlayTexCoords.mins.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topLeftVertex, Vector2( overlayTexCoords.mins.x, overlayTexCoords.mins.y ) ) );
	}
	if ( onAllSides || faceSelected == FRONT )
	{
		bottomLeftVertex = Vector3( bounds->mins.x - OFFSET_AMOUNT, bounds->maxs.y, bounds->mins.z );
		bottomRightVertex = Vector3( bounds->mins.x - OFFSET_AMOUNT, bounds->mins.y, bounds->mins.z );
		topRightVertex = Vector3( bounds->mins.x - OFFSET_AMOUNT, bounds->mins.y, bounds->maxs.z );
		topLeftVertex = Vector3( bounds->mins.x - OFFSET_AMOUNT, bounds->maxs.y, bounds->maxs.z );

		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomLeftVertex, Vector2( overlayTexCoords.mins.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomRightVertex, Vector2( overlayTexCoords.maxs.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topRightVertex, Vector2( overlayTexCoords.maxs.x, overlayTexCoords.mins.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topLeftVertex, Vector2( overlayTexCoords.mins.x, overlayTexCoords.mins.y ) ) );
	}
	if ( onAllSides || faceSelected == BACK )
	{
		bottomLeftVertex = Vector3( bounds->maxs.x + OFFSET_AMOUNT, bounds->mins.y, bounds->mins.z );
		bottomRightVertex = Vector3( bounds->maxs.x + OFFSET_AMOUNT, bounds->maxs.y, bounds->mins.z );
		topRightVertex = Vector3( bounds->maxs.x + OFFSET_AMOUNT, bounds->maxs.y, bounds->maxs.z );
		topLeftVertex = Vector3( bounds->maxs.x + OFFSET_AMOUNT, bounds->mins.y, bounds->maxs.z );

		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomLeftVertex, Vector2( overlayTexCoords.mins.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( bottomRightVertex, Vector2( overlayTexCoords.maxs.x, overlayTexCoords.maxs.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topRightVertex, Vector2( overlayTexCoords.maxs.x, overlayTexCoords.mins.y ) ) );
		selectionOverlayVertexes.push_back( Vertex3D_PCT( topLeftVertex, Vector2( overlayTexCoords.mins.x, overlayTexCoords.mins.y ) ) );
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
		if ( out_selectedBlockBounds != nullptr ) delete out_selectedBlockBounds;
		out_selectedBlockBounds = nullptr;
		return;
	}

	LocalBlockIndex lbiSelected = selectedChunk->m_selectedBlock;
	if ( lbiSelected < 0 || lbiSelected >= NUM_BLOCKS_PER_CHUNK )
	{
		if ( out_selectedBlockBounds != nullptr ) delete out_selectedBlockBounds;
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
	if ( selectedChunk == nullptr ) return NONE;

	return selectedChunk->m_selectedFace;
}


//-----------------------------------------------------------------------------
void TheGame::Render()
{
	SetupView3D();
	Render3D();
	if ( g_renderDebugInfo ) RenderDebug3D();

	SetupView2D();
	Render2D();
	if ( g_renderDebugInfo ) RenderDebug2D( );
}
