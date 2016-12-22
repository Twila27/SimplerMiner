#include "Game/World.hpp"


#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/FileUtils/FileUtils.hpp"
#include "Engine/String/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/TheInput.hpp" //For input polling.

#include "Game/Chunk.hpp"
#include "Game/BlockDefinition.hpp"
#include "Game/Camera3D.hpp"
#include "Game/Player.hpp"


STATIC SoundID World::m_hudChangeSoundID = 0;
STATIC float World::m_distanceSinceLastWalkSound = 0.f;

//--------------------------------------------------------------------------------------------------------------
World::World( Camera3D* camera, Player* player )
	: m_activeRadius( INITIAL_ACTIVE_RADIUS )
	, m_flushRadius( INITIAL_FLUSH_RADIUS )
	, m_playerCamera( camera )
	, m_activeHudElement( 0 )
	, m_chunkOfSelectedBlock( nullptr )
	, m_currentDigDamageFrame( 0 )
	, m_player( player )
	, m_blockBeingDug( new BlockInfo() )
	, m_activeDimension( DIM_OVERWORLD )
{
	ASSERT_OR_DIE( m_activeRadius < m_flushRadius, "Active Exceeds Flush Radius!" ); //Ensure chunk can't activate and flush at once.
	ASSERT_OR_DIE( (m_activeRadius - m_flushRadius) % CHUNK_X_LENGTH_IN_BLOCKS == 0, "Active/Flush Radii Not a Chunk-multiple Apart!" ); //Not a speed-critical %.

	BlockDefinition::InitializeBlockDefinitions();
	LoadPlayerFile( "Data/Saves/Player.txt" );

	World::m_hudChangeSoundID = g_theAudio->CreateOrGetSound( "Data/Audio/Boxing_SlapStick1.wav" );
}


//--------------------------------------------------------------------------------------------------------------
World::~World()
{
	//m_camera, m_player deleted by TheGame.
	delete m_blockBeingDug;

	for ( int dimensionIndex = 0; dimensionIndex < NUM_DIMENSIONS; dimensionIndex++ )
	{
		auto chunkIterEnd = m_activeChunks[ dimensionIndex ].end( );
		for ( auto chunkIter = m_activeChunks[ dimensionIndex ].begin( ); chunkIter != chunkIterEnd; ++chunkIter )
		{
			delete chunkIter->second;
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void World::RenderChunk( const Chunk& chunk ) const
{
	++g_chunksRendered;
	chunk.Render();
}


//--------------------------------------------------------------------------------------------------------------
bool World::IsChunkVisible( const Chunk& chunk ) const
{
	if ( g_useCulling == false )
		return true;

	for ( int chunkCornerIndex = 0; chunkCornerIndex < Chunk::ChunkCornerPosition::NUM_CHUNK_CORNERS; chunkCornerIndex++ )
	{
		Vector3 cameraToChunkCornerDisp = chunk.m_chunkCornersInWorldUnits[ chunkCornerIndex ] - m_player->m_worldPosition;

		//If any dot products of 3D camera forward XYZ vector and the chunk corner world positions are negative, don't render.
		if ( DotProduct( cameraToChunkCornerDisp, m_playerCamera->GetForwardXYZ() ) > 0.f ) return true;
	}

	return false;
}


//--------------------------------------------------------------------------------------------------------------
void World::Render() const
{
	g_chunksRendered = 0;

	if ( g_renderDebugInfo )
		DrawDebugPoints( 10.f, true );

	g_theRenderer->EnableDepthTesting( true );
	m_player->Render();

	auto chunkIterEnd = m_activeChunks[ m_activeDimension ].end();
	for ( auto chunkIter = m_activeChunks[ m_activeDimension ].begin( ); chunkIter != chunkIterEnd; ++chunkIter )
	{
		const Chunk& chunk = *chunkIter->second;
		if ( IsChunkVisible( chunk ) )
		{
			RenderChunk( chunk );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void World::Update( float deltaSeconds )
{
	UpdateCameraAndPlayer( deltaSeconds ); //Because movement == camera == player.
	if ( g_flushChunksEnabled ) DeactivateFarthestObsoleteChunk(); //Deactivate comes before activate if for example close to memory limit, we wouldn't want to allocate when we can free first.
	if ( g_activateChunksEnabled ) ActivateNearestMissingChunk(); //Name implies if there's more than one missing, we'll only activate one--and if none missing, none activated.		
	UpdateChunks(); //I have a TNT that went off, started fires, etc. but I will also change lighting. i.e. these are events inside the chunk like grass propagating.
	UpdateLighting(); //Because lights spill chunk to chunk, so it can't be in a chunk.
		//Chunks hold the blocks that know they are dirty and their light levels--so in global list of lighting-dirty blocks, may or may not be same-chunk.
		//"While they are any dirty blocks left, process the next lighting-dirty block."
	if ( g_updateVertexDataEnabled ) UpdateDirtyVertexArrays();
}


//--------------------------------------------------------------------------------------------------------------
void World::UpdateCameraAndPlayer( float deltaSeconds )
{

	UnhighlightSelectedBlock(); //Dirties a chunk in process, TODO: revise.

	BlockType blockTypeAtPlayerFeet = GetBlockTypeFromWorldCoords( m_player->GetFeetPos() );

	//Dimension warping.
	if ( blockTypeAtPlayerFeet == BlockType::PORTAL || g_theInput->WasKeyPressedOnce( 'N' ) )
	{
		m_activeDimension = ( m_activeDimension == DIM_OVERWORLD ? DIM_NETHER : DIM_OVERWORLD );

		if ( m_activeDimension == DIM_NETHER ) 
			m_player->m_worldPosition.z = CEILING_HEIGHT_OFFSET - 5.f; //To get out of ceiling.

		if ( m_activeDimension == DIM_OVERWORLD )
				m_player->m_worldPosition.z = CHUNK_Z_HEIGHT_IN_BLOCKS - 1.f; //To clear player head.
	}

	if ( g_theInput->IsKeyDown( '1' ) ) m_activeHudElement = 0;
	else if ( g_theInput->IsKeyDown( '2' ) ) m_activeHudElement = 1;
	else if ( g_theInput->IsKeyDown( '3' ) ) m_activeHudElement = 2;
	else if ( g_theInput->IsKeyDown( '4' ) ) m_activeHudElement = 3;
	else if ( g_theInput->IsKeyDown( '5' ) ) m_activeHudElement = 4;
	else if ( g_theInput->IsKeyDown( '6' ) ) m_activeHudElement = 5;
	else if ( g_theInput->IsKeyDown( '7' ) ) m_activeHudElement = 6;
	else if ( g_theInput->IsKeyDown( '8' ) ) m_activeHudElement = 7;
	else if ( g_theInput->IsKeyDown( '9' ) ) m_activeHudElement = 8;
	else
	{
		int mouseWheelDelta = g_theInput->GetMouseWheelDelta();
		if ( mouseWheelDelta != 0 ) //No increments.
		{
			m_activeHudElement += mouseWheelDelta;
			g_theInput->SetMouseWheelDelta( 0 ); //HACKY.
		}

		m_activeHudElement = WrapNumberWithinCircularRange
		( 
			m_activeHudElement, 
			0,
			NUM_BLOCK_TYPES - 1 //SKIP AIR.
		); 
			
	}
	if ( m_lastFrameHudElement != m_activeHudElement )
		g_theAudio->PlaySound( m_hudChangeSoundID, VOLUME_ADJUST );

	m_lastFrameHudElement = m_activeHudElement;

	//--------------------------------------------------
	//Keyboard.
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_MOVEMENT_MODE ) ) g_currentMovementMode = (MovementMode)WrapNumberWithinCircularRange( g_currentMovementMode + 1, 0, NUM_MOVEMENT_MODES );

	Vector3& playerPos = m_player->m_worldPosition;
	Vector3& cameraPos = m_playerCamera->m_worldPosition;
	Vector3& posToMove = ( g_currentCameraMode == FREE_SPECTATOR ? cameraPos : playerPos );
	Vector3 camForwardXY = m_playerCamera->GetForwardXY(); //Heading for forward/back.
	Vector3 camLeftXY = m_playerCamera->GetLeftXY(); //Heading for strafing.
	Vector3 camDirection = m_playerCamera->GetForwardXYZ();
	camDirection.Normalize();
	camLeftXY.Normalize();
	camForwardXY.Normalize();

	UpdateFromMovementKeys( deltaSeconds, camForwardXY, camLeftXY, posToMove );

	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_PLAYER_COLLIDER ) ) g_renderPlayerCollider = !g_renderPlayerCollider;
	m_player->UpdateCollidersAndDigTime( deltaSeconds );

	//--------------------------------------------------
	//Mouse!
	UpdateMouseAndCameraOffset( cameraPos, playerPos, camDirection );


	//--------------------------------------------------
	//Controller.
	//const XboxController& controller = g_theInput->GetController( 0 );
	//const XboxStickState& leftStick = controller.GetStickState( XBOX_STICK_LEFT );
	//float fwdStrength = leftStick.m_correctedNormalizedY; //Positive is forward, so this is how much of the fwd vec we apply.
	//float leftStrength = -1.f * leftStick.m_correctedNormalizedX;
	//Vector3 moveIntent = (camForwardXY * fwdStrength) + (camLeftXY * leftStrength);
	//m_cam.m_position += moveIntent * deltaMove;

	//Raycast enclosed by defensive code against rays cast from outside sky/ground limit.
	if ( playerPos.z < 0 || playerPos.z > CHUNK_Z_HEIGHT_IN_BLOCKS ) return;
	Vector3 endOfRay = playerPos + ( camDirection * LENGTH_OF_SELECTION_RAYCAST );
	if ( endOfRay.z < 0 || endOfRay.z > CHUNK_Z_HEIGHT_IN_BLOCKS ) return;

	SelectBlock( playerPos, endOfRay, deltaSeconds );
}


//--------------------------------------------------------------------------------------------------------------
void World::UpdateFromMovementKeys( float deltaSeconds, const Vector3& camForwardXY, const Vector3& camLeftXY, Vector3 &posToMove )
{
	float speedUp = ( g_theInput->IsKeyDown( VK_SHIFT ) ? PLAYER_SPEED_BOOST : PLAYER_DEFAULT_SPEED );
	float deltaSecondsSpeedBoosted = speedUp * deltaSeconds; //This makes speed in blocks/second, algebraically.
	Vector3& playerVel = m_player->m_velocity;
	BlockType blockTypeAtPlayerFeet = GetBlockTypeFromWorldCoords( m_player->GetFeetPos() );

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_BACKWARD ) )
	{
		if ( g_theInput->WasKeyJustPressed( KEY_TO_MOVE_FORWARD ) ) 
			playerVel *= ( 1.f - HORIZONTAL_DECEL_BEFORE_STOP_KNOB ); //Applying drag/friction-stop to move abruptly in an opposite direction.

		playerVel -= camForwardXY;
	}
	else if ( g_theInput->IsKeyDown( KEY_TO_MOVE_FORWARD ) )
	{
		if ( g_theInput->WasKeyJustPressed( KEY_TO_MOVE_BACKWARD ) ) 
			playerVel *= ( 1.f - HORIZONTAL_DECEL_BEFORE_STOP_KNOB );

		playerVel += camForwardXY;
	}

	if ( g_theInput->IsKeyDown( KEY_TO_MOVE_LEFT ) )
	{
		if ( g_theInput->WasKeyJustPressed( KEY_TO_MOVE_RIGHT ) ) 
			playerVel *= ( 1.f - HORIZONTAL_DECEL_BEFORE_STOP_KNOB );

		playerVel += camLeftXY;
	}
	else if ( g_theInput->IsKeyDown( KEY_TO_MOVE_RIGHT ) )
	{
		if ( g_theInput->WasKeyJustPressed( KEY_TO_MOVE_LEFT ) ) 
			playerVel *= ( 1.f - HORIZONTAL_DECEL_BEFORE_STOP_KNOB );

		playerVel -= camLeftXY;
	}

	playerVel.z += ( g_currentMovementMode == WALKING ) ? ( PLAYER_GRAVITY_FORCE * deltaSeconds ) : 0.f; //Note *dt makes it an accel.
	
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_MOVE_UP ) && g_currentMovementMode == WALKING && IsPlayerOnGround() ) 
		playerVel.z += PLAYER_JUMP_SPEED * speedUp;

	if ( g_currentMovementMode != WALKING )
	{
		if ( g_theInput->IsKeyDown( KEY_TO_MOVE_UP ) ) 
			playerVel.z += 1.f * deltaSecondsSpeedBoosted; //+=1.f*mag for dir == +z-axis.

		if ( g_theInput->IsKeyDown( KEY_TO_MOVE_DOWN ) ) 
			playerVel.z -= 1.f * deltaSecondsSpeedBoosted; //-=1.f*mag for dir == -z-axis.
	}

	//Friction-stopping.
	ApplyFrictionStopping( playerVel, blockTypeAtPlayerFeet );

	//Ladder boost. TODO: Make function!
	if ( blockTypeAtPlayerFeet == LADDER && g_theInput->IsKeyDown( KEY_TO_MOVE_FORWARD ) )
	{
		playerVel.z = LADDER_RISING_SPEED_KNOB * deltaSecondsSpeedBoosted;
	}


	if ( playerVel != Vector3::ZERO )
	{
		playerVel = GetPhysicsCorrectedVelocityForDeltaSeconds( playerVel, posToMove, deltaSecondsSpeedBoosted );
		if ( g_theInput->IsKeyDown( KEY_TO_MOVE_LEFT ) || g_theInput->IsKeyDown( KEY_TO_MOVE_RIGHT ) )
		{
			playerVel.x = ClampFloat( playerVel.x, -camLeftXY.x * PLAYER_MAX_SPEED, camLeftXY.x * PLAYER_MAX_SPEED );
			playerVel.y = ClampFloat( playerVel.y, -camLeftXY.y * PLAYER_MAX_SPEED, camLeftXY.y * PLAYER_MAX_SPEED );
		}
		if ( g_theInput->IsKeyDown( KEY_TO_MOVE_FORWARD ) || g_theInput->IsKeyDown( KEY_TO_MOVE_BACKWARD ) )
		{
			playerVel.x = ClampFloat( playerVel.x, -camForwardXY.x * PLAYER_MAX_SPEED, camForwardXY.x * PLAYER_MAX_SPEED );
			playerVel.y = ClampFloat( playerVel.y, -camForwardXY.y * PLAYER_MAX_SPEED, camForwardXY.y * PLAYER_MAX_SPEED );
		} 
		Vector3 moveVector = WorldCoords( playerVel.x * deltaSecondsSpeedBoosted, playerVel.y * deltaSecondsSpeedBoosted, playerVel.z * deltaSeconds ); //No speed boosting gravity or jumps.
		posToMove += moveVector;
		PlayWalkingSound( moveVector.CalcLength() );
	}

	//"Corrective physics" for z-axis:
	if ( posToMove.z > CHUNK_Z_HEIGHT_IN_BLOCKS ) posToMove.z = CHUNK_Z_HEIGHT_IN_BLOCKS - PLAYER_HALF_HEIGHT; //Ceiling.
	if ( posToMove.z < 0 ) posToMove.z = PLAYER_HALF_HEIGHT; //Floor.
}


//--------------------------------------------------------------------------------------------------------------
void World::ApplyFrictionStopping( Vector3 &playerVel, BlockType blockTypeForPlayerFeet )
{
	if (
			!( g_theInput->IsKeyDown( KEY_TO_MOVE_BACKWARD )
			|| g_theInput->IsKeyDown( KEY_TO_MOVE_FORWARD )
			|| g_theInput->IsKeyDown( KEY_TO_MOVE_LEFT )
			|| g_theInput->IsKeyDown( KEY_TO_MOVE_RIGHT )
			|| g_theInput->IsKeyDown( KEY_TO_MOVE_UP )
			|| g_theInput->IsKeyDown( KEY_TO_MOVE_DOWN ) )
		)
	{
		if ( g_currentMovementMode == NOCLIP )
		{
			playerVel *= 0.f;
		}
		else
		{
			//Get the block player's center's bottom is in, and if it's a ladder apply extra vertical friction-stop
			if ( blockTypeForPlayerFeet == LADDER )
			{
				playerVel.z *= LADDER_VERTICAL_DECEL_KNOB;
			}
			else
			{
				playerVel.x *= HORIZONTAL_DECEL_BEFORE_STOP_KNOB;
				playerVel.y *= HORIZONTAL_DECEL_BEFORE_STOP_KNOB;
				playerVel.z *= VERTICAL_DECEL_BEFORE_STOP_KNOB;
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
Vector3 World::GetPhysicsCorrectedVelocityForDeltaSeconds( const Vector3& velocityToPrevent, Vector3& posToMove, float deltaSeconds )
{
	if ( g_currentMovementMode == NOCLIP ) return velocityToPrevent; //Allowed unscaled velocity to clip past solid blocks in Noclip mode.

	//First implementation of preventive phyics: Scale down the velocity by the min of 12 raycasts from the player's box top, bottom, and midpoint's 4 corners.
	RaycastResult3D minHitResult;
	bool hitSomething = false;
	Vector3 newRedirectedVelocity = velocityToPrevent; //Initial start at the unscaled, uncorrected velocity values.
	Vector3 lastIterationVelocity;

	while ( newRedirectedVelocity != lastIterationVelocity || newRedirectedVelocity == Vector3::ZERO )
	{
		lastIterationVelocity = newRedirectedVelocity; //If the latter hasn't changed by end of loop, nothing to re-check and we're done.
		Vector3 newPositionBeforeCorrection = m_player->m_worldPosition + ( newRedirectedVelocity * deltaSeconds );

		if ( !g_useAmanWooRaycastOverStepAndSample )
		{
			hitSomething = BoxTraceWithStepAndSample( m_player->m_worldPosition, newPositionBeforeCorrection, minHitResult );
		}
		else
		{
			hitSomething = /*BoxTraceWithAmanatidesWoo*/ BoxTraceWithTwelveRaygancasts( m_player->m_worldPosition, newPositionBeforeCorrection, /*PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT,*/ minHitResult );
		}

		if ( !hitSomething ) 
			return newRedirectedVelocity; //Move without any further correction if not hitting anything, but it may have had correction in past iteration so can't just return velocityToPrevent.

		//Had a problem where with gravity you had constant 0% impact fraction against floor--zeroing x and y via scaling by impactFraction, when it should have been 1 - impactFraction.
		float redirectionScalar = ( 1.0f - minHitResult.impactFraction );

		//Direction opposite selected face AKA surface normal.
		Vector3 directionOppositeCollidedBlock = FindDirectionBetweenBlocks( minHitResult.penultimateBlockHit, minHitResult.lastBlockHit );
		if ( directionOppositeCollidedBlock == Vector3::ZERO ) 
			return Vector3::ZERO; //e.g. penultimate and ultimate block hit were the same block.

		//If we collided with a stairs slab, check for an existing block above and push upward, then re-run physics.
		if ( !( directionOppositeCollidedBlock == WORLD_DOWN || directionOppositeCollidedBlock == WORLD_UP ) && minHitResult.lastBlockHit.GetBlock( )->GetBlockType( ) == STAIRS )
		{
			BlockInfo blockAboveStairs = minHitResult.lastBlockHit;

			do
			{
				blockAboveStairs.StepUp();
			} 
			while ( blockAboveStairs.GetBlock()->GetBlockType() == STAIRS );

			if ( BlockDefinition::IsSolid( blockAboveStairs.GetBlock( )->GetBlockType( ) ) )
				continue;

			posToMove += STAIR_BOOST;
		}

		//Redirect prior iteration's velocity to slide by the % we moved when we had the collision into dimensions EXCEPTING the one the hit occurred in.
		if ( directionOppositeCollidedBlock == WORLD_DOWN || directionOppositeCollidedBlock == WORLD_UP )
		{
			newRedirectedVelocity = Vector3( newRedirectedVelocity.x * redirectionScalar, newRedirectedVelocity.y * redirectionScalar, 0.f );
		}
		else if ( directionOppositeCollidedBlock == WORLD_LEFT || directionOppositeCollidedBlock == WORLD_RIGHT )
		{
			newRedirectedVelocity = Vector3( newRedirectedVelocity.x * redirectionScalar, 0.f, newRedirectedVelocity.z * redirectionScalar );
		}
		else if ( directionOppositeCollidedBlock == WORLD_FORWARD || directionOppositeCollidedBlock == WORLD_BACKWARD )
		{
			newRedirectedVelocity = Vector3( 0.f, newRedirectedVelocity.y * redirectionScalar, newRedirectedVelocity.z * redirectionScalar );
		}
		else break; //Nothing redirected!

		if ( newRedirectedVelocity == Vector3::ZERO ) 
			break; //No possibility of shoving into block (assuming we aren't in one already), can exit loop.
		
	}

	return newRedirectedVelocity;
}


//--------------------------------------------------------------------------------------------------------------
BlockType World::GetBlockTypeFromWorldCoords( WorldCoords wc )
{
	BlockInfo blockForGivenPos = GetBlockInfoFromWorldCoords( wc );
	if ( blockForGivenPos != BlockInfo( ) ) //e.g. before any chunks have loaded.
	{
		return blockForGivenPos.GetBlock( )->GetBlockType( );
	}
	else return NUM_BLOCK_TYPES;
}


//--------------------------------------------------------------------------------------------------------------
bool World::BoxTraceWithStepAndSample( const Vector3& rayStartPos, const Vector3& rayEndPos, RaycastResult3D& minHitResult )
{
	RaycastResult3D hitResult;
	
	//Top of player bounding box, front-left corner.
	RaycastWithStepAndSample(
		rayStartPos + Vector3( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ), hitResult );
	minHitResult = hitResult;

	//Top of player bounding box, front-right corner.
	RaycastWithStepAndSample(
		rayStartPos + Vector3( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Top of player bounding box, back-right corner.
	RaycastWithStepAndSample(
		rayStartPos + Vector3( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Top of player bounding box, back-left corner.
	RaycastWithStepAndSample(
		rayStartPos + Vector3( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Middle of player bounding box, front-left corner.
	RaycastWithStepAndSample(
		rayStartPos + Vector3( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, 0.f ),
		rayEndPos + Vector3( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, 0.f ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Middle of player bounding box, front-right corner.
	RaycastWithStepAndSample(
		rayStartPos + Vector3( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, 0.f ),
		rayEndPos + Vector3( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, 0.f ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Middle of player bounding box, back-right corner.
	RaycastWithStepAndSample(
		rayStartPos + Vector3( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, 0.f ),
		rayEndPos + Vector3( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, 0.f ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Middle of player bounding box, back-left corner.
	RaycastWithStepAndSample(
		rayStartPos + Vector3( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, 0.f ),
		rayEndPos + Vector3( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, 0.f ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Bottom of player bounding box, front-left corner.`
	RaycastWithStepAndSample(
		rayStartPos + Vector3( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Bottom of player bounding box, front-right corner.
	RaycastWithStepAndSample(
		rayStartPos + Vector3( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Bottom of player bounding box, back-right corner.
	RaycastWithStepAndSample(
		rayStartPos + Vector3( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Bottom of player bounding box, back-left corner.
	RaycastWithStepAndSample(
		rayStartPos + Vector3( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	if ( minHitResult.impactFraction > .999f ) return false; //No collisions.
	else return true;
}


//--------------------------------------------------------------------------------------------------------------
bool World::BoxTraceWithTwelveRaygancasts( const Vector3& rayStartPos, const Vector3& rayEndPos, RaycastResult3D& minHitResult )
{
	RaycastResult3D hitResult;

	//Top of player bounding box, front-left corner.
	RaygancastWithAmanatidesWoo(
		rayStartPos + Vector3( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ), hitResult );
	minHitResult = hitResult;

	//Top of player bounding box, front-right corner.
	RaygancastWithAmanatidesWoo(
		rayStartPos + Vector3( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Top of player bounding box, back-right corner.
	RaygancastWithAmanatidesWoo(
		rayStartPos + Vector3( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Top of player bounding box, back-left corner.
	RaygancastWithAmanatidesWoo(
		rayStartPos + Vector3( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Middle of player bounding box, front-left corner.
	RaygancastWithAmanatidesWoo(
		rayStartPos + Vector3( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, 0.f ),
		rayEndPos + Vector3( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, 0.f ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Middle of player bounding box, front-right corner.
	RaygancastWithAmanatidesWoo(
		rayStartPos + Vector3( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, 0.f ),
		rayEndPos + Vector3( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, 0.f ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Middle of player bounding box, back-right corner.
	RaygancastWithAmanatidesWoo(
		rayStartPos + Vector3( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, 0.f ),
		rayEndPos + Vector3( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, 0.f ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Middle of player bounding box, back-left corner.
	RaygancastWithAmanatidesWoo(
		rayStartPos + Vector3( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, 0.f ),
		rayEndPos + Vector3( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, 0.f ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Bottom of player bounding box, front-left corner.`
	RaygancastWithAmanatidesWoo(
		rayStartPos + Vector3( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Bottom of player bounding box, front-right corner.
	RaygancastWithAmanatidesWoo(
		rayStartPos + Vector3( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Bottom of player bounding box, back-right corner.
	RaygancastWithAmanatidesWoo(
		rayStartPos + Vector3( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	//Bottom of player bounding box, back-left corner.
	RaygancastWithAmanatidesWoo(
		rayStartPos + Vector3( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ),
		rayEndPos + Vector3( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, -PLAYER_HALF_HEIGHT ), hitResult );
	if ( minHitResult.impactFraction > hitResult.impactFraction )
		minHitResult = hitResult;

	if ( minHitResult.impactFraction > .999f ) return false; //No collisions.
	else return true;
}


//---//Did not work: need to check 8 blocks around each new position, as the box can snag at any of its corners.
bool World::BoxTraceWithAmanatidesWoo( Vector3 boxCenterStartPos, Vector3 boxCenterEndPos, float boxHalfLengthX, float boxHalfWidthY, float boxHalfHeightZ, RaycastResult3D& out_result )
{//Regan-casting
	BlockInfo originBlockInfo = GetBlockInfoFromWorldCoords( boxCenterStartPos );
	if ( originBlockInfo.m_myChunk == nullptr ) return false;

	//Initialization of Regan-cast -- NEED TO TAKE THE MINS OF EACH OF THE 8 CORNERS OF OUR PLAYER, AS WE CAN OVERLAP UP TO 8 BLOCKS THAT MAY BE SOLID!
	BlockInfo initialCheckingBlockInfo;
	GlobalBlockCoords currentCheckingBlockPos;
	for ( int initialCheckingBlockIndex = 0; initialCheckingBlockIndex < 8; initialCheckingBlockIndex++ )
	{
		switch ( initialCheckingBlockIndex )
		{
		case 0: //upperBackLeftBlockPos
			currentCheckingBlockPos.x = (int)floor( boxCenterStartPos.x + boxHalfLengthX );
			currentCheckingBlockPos.y = (int)floor( boxCenterStartPos.y + boxHalfWidthY );
			currentCheckingBlockPos.z = (int)floor( boxCenterStartPos.z + boxHalfHeightZ );
		case 1: //upperFrontLeftBlockPos
			currentCheckingBlockPos.x = (int)floor( boxCenterStartPos.x - boxHalfLengthX );
			currentCheckingBlockPos.y = (int)floor( boxCenterStartPos.y + boxHalfWidthY );
			currentCheckingBlockPos.z = (int)floor( boxCenterStartPos.z + boxHalfHeightZ );
		case 2: //upperBackRightBlockPos
			currentCheckingBlockPos.x = (int)floor( boxCenterStartPos.x - boxHalfLengthX );
			currentCheckingBlockPos.y = (int)floor( boxCenterStartPos.y - boxHalfWidthY );
			currentCheckingBlockPos.z = (int)floor( boxCenterStartPos.z + boxHalfHeightZ );
		case 3: //upperFrontRightBlockPos
			currentCheckingBlockPos.x = (int)floor( boxCenterStartPos.x + boxHalfLengthX );
			currentCheckingBlockPos.y = (int)floor( boxCenterStartPos.y - boxHalfWidthY );
			currentCheckingBlockPos.z = (int)floor( boxCenterStartPos.z + boxHalfHeightZ );
		case 4: //lowerBackLeftBlockPos
			currentCheckingBlockPos.x = (int)floor( boxCenterStartPos.x + boxHalfLengthX );
			currentCheckingBlockPos.y = (int)floor( boxCenterStartPos.y + boxHalfWidthY );
			currentCheckingBlockPos.z = (int)floor( boxCenterStartPos.z - boxHalfHeightZ );
		case 5: //lowerFrontLeftBlockPos
			currentCheckingBlockPos.x = (int)floor( boxCenterStartPos.x - boxHalfLengthX );
			currentCheckingBlockPos.y = (int)floor( boxCenterStartPos.y + boxHalfWidthY );
			currentCheckingBlockPos.z = (int)floor( boxCenterStartPos.z - boxHalfHeightZ );
		case 6: //lowerBackRightBlockPos
			currentCheckingBlockPos.x = (int)floor( boxCenterStartPos.x - boxHalfLengthX );
			currentCheckingBlockPos.y = (int)floor( boxCenterStartPos.y - boxHalfWidthY );
			currentCheckingBlockPos.z = (int)floor( boxCenterStartPos.z - boxHalfHeightZ );
		case 7: //lowerFrontRightBlockPos
			currentCheckingBlockPos.x = (int)floor( boxCenterStartPos.x + boxHalfLengthX );
			currentCheckingBlockPos.y = (int)floor( boxCenterStartPos.y - boxHalfWidthY );
			currentCheckingBlockPos.z = (int)floor( boxCenterStartPos.z - boxHalfHeightZ );
		}

		initialCheckingBlockInfo = GetBlockInfoFromGlobalBlockCoords( currentCheckingBlockPos );

		if ( initialCheckingBlockInfo.m_myChunk->IsBlockSolid( initialCheckingBlockInfo.m_myBlockIndex ) ) //Are initial blocks solid?
		{
			out_result.penultimateBlockHit = out_result.lastBlockHit = initialCheckingBlockInfo;
			out_result.impactFraction = 0.f;
			out_result.impactPosition = boxCenterStartPos;
			return true;
		}
	}

	/////////////////////////////////////////////////////////////

	GlobalBlockCoords blockPos = GlobalBlockCoords(
		(int)floor( boxCenterStartPos.x ),
		(int)floor( boxCenterStartPos.y ),
		(int)floor( boxCenterStartPos.z )
	);

	Vector3 rayDisplacement = boxCenterEndPos - boxCenterStartPos;
	float totalRayLength = rayDisplacement.CalcLength();

	//tDelta: how much t it takes to cross to the next x/y/z.
	float tDeltaX = abs( 1.0f / rayDisplacement.x );
	float tDeltaY = abs( 1.0f / rayDisplacement.y );
	float tDeltaZ = abs( 1.0f / rayDisplacement.z );

	//Which direction to step along, computable because a ray's dir is constant.
	int tileStepX = ( rayDisplacement.x > 0 ) ? 1 : -1;
	int tileStepY = ( rayDisplacement.y > 0 ) ? 1 : -1;
	int tileStepZ = ( rayDisplacement.z > 0 ) ? 1 : -1;

	//0 or 1 fudge factor, and the 0 means that casts in negative directions have first intersection at the above block-mins (see blockPos init above).
	int offsetToLeadingEdgeX = ( tileStepX + 1 ) >> 1; //replaces div by 2^1.
	int offsetToLeadingEdgeY = ( tileStepY + 1 ) >> 1;
	int offsetToLeadingEdgeZ = ( tileStepZ + 1 ) >> 1;

	//This is just the next crossing in the world that changes integer parts of our world positions, INDEPENDENT of player box bounds.
	float firstIntersectionOnX = (float)( blockPos.x + offsetToLeadingEdgeX );
	float firstIntersectionOnY = (float)( blockPos.y + offsetToLeadingEdgeY );
	float firstIntersectionOnZ = (float)( blockPos.z + offsetToLeadingEdgeZ );

	//How many "t's" it takes to reach the next intersection with the world from the start position -- NEED TO TAKE MINS OF EACH OF THE CORNERS?
	float tOfNextCrossingOnX = abs( firstIntersectionOnX - ( boxCenterStartPos.x + ( boxHalfLengthX * tileStepX ) ) ) * tDeltaX;
	float tOfNextCrossingOnY = abs( firstIntersectionOnY - ( boxCenterStartPos.y + ( boxHalfWidthY * tileStepY ) ) ) * tDeltaY;
	float tOfNextCrossingOnZ = abs( firstIntersectionOnZ - ( boxCenterStartPos.z + ( boxHalfHeightZ * tileStepZ ) ) ) * tDeltaZ;

	//Main Loop of Regan-cast
	BlockInfo currentBlockInfo = originBlockInfo;
	BlockInfo blockBeforeCurrentInfo;

	bool shutUpVisualStudioWarning = true;
	while ( shutUpVisualStudioWarning ) // Loop until impact or end of line segment.
	{
		//Find min of tOfNextCrossingOnX,Y,Z to determine what line gets crossed next.

		//if OnX is lowest
		if ( ( tOfNextCrossingOnX < tOfNextCrossingOnY ) && ( tOfNextCrossingOnX < tOfNextCrossingOnZ ) )
		{
			if ( tOfNextCrossingOnX > 1 ) return false; //No impact, i.e. next crossing past endpoint.

			blockPos.x += tileStepX; //move into next tile on x
			currentBlockInfo = GetBlockInfoFromGlobalBlockCoords( blockPos );
			if ( currentBlockInfo.m_myChunk == nullptr ) return false;
			if ( currentBlockInfo.m_myChunk->IsBlockSolid( currentBlockInfo.m_myBlockIndex ) ) //Impact.
			{
				out_result.lastBlockHit = currentBlockInfo;
				out_result.penultimateBlockHit = blockBeforeCurrentInfo;
				out_result.impactPosition = boxCenterStartPos + ( rayDisplacement * tOfNextCrossingOnX );
				WorldCoords displacementToImpact = out_result.impactPosition - boxCenterStartPos;
				float steppedRayLength = displacementToImpact.CalcLength();
				out_result.impactFraction = ( steppedRayLength / totalRayLength );

				return true;
			}
			else tOfNextCrossingOnX += tDeltaX; //Move t to next crossing onX.			
		}

		//if OnY is lowest
		else if ( ( tOfNextCrossingOnY < tOfNextCrossingOnX ) && ( tOfNextCrossingOnY < tOfNextCrossingOnZ ) )
		{
			if ( tOfNextCrossingOnY > 1 ) return false; //No impact, i.e. next crossing past endpoint.

			blockPos.y += tileStepY; //move into next tile on y
			currentBlockInfo = GetBlockInfoFromGlobalBlockCoords( blockPos );
			if ( currentBlockInfo.m_myChunk == nullptr ) return false;
			if ( currentBlockInfo.m_myChunk->IsBlockSolid( currentBlockInfo.m_myBlockIndex ) ) //Impact.
			{
				out_result.lastBlockHit = currentBlockInfo;
				out_result.penultimateBlockHit = blockBeforeCurrentInfo;
				out_result.impactPosition = boxCenterStartPos + ( rayDisplacement * tOfNextCrossingOnY );
				WorldCoords displacementToImpact = out_result.impactPosition - boxCenterStartPos;
				float steppedRayLength = displacementToImpact.CalcLength();
				out_result.impactFraction = ( steppedRayLength / totalRayLength );
				
				return true;
			}
			else tOfNextCrossingOnY += tDeltaY; //Move t to next crossing onY.
		}

		// tOfNextCrossingOnZ is lowest
		else
		{
			if ( tOfNextCrossingOnZ > 1 ) return false; //No impact, i.e. next crossing past endpoint.

			blockPos.z += tileStepZ; //move into next tile on z
			currentBlockInfo = GetBlockInfoFromGlobalBlockCoords( blockPos );
			if ( currentBlockInfo.m_myChunk == nullptr ) return false;
			if ( currentBlockInfo.m_myChunk->IsBlockSolid( currentBlockInfo.m_myBlockIndex ) ) //Impact.
			{
				out_result.lastBlockHit = currentBlockInfo;
				out_result.penultimateBlockHit = blockBeforeCurrentInfo;
				out_result.impactPosition = boxCenterStartPos + ( rayDisplacement * tOfNextCrossingOnZ );
				WorldCoords displacementToImpact = out_result.impactPosition - boxCenterStartPos;
				float steppedRayLength = displacementToImpact.CalcLength();
				out_result.impactFraction = ( steppedRayLength / totalRayLength );

				return true;
			}
			else tOfNextCrossingOnZ += tDeltaZ; //Move t to next crossing onZ.
		}

		blockBeforeCurrentInfo = currentBlockInfo;
	}

	return false;
}


//--------------------------------------------------------------------------------------------------------------
void World::UpdateMouseAndCameraOffset( Vector3& cameraPos, Vector3& playerPos, Vector3 camDirection )
{
	const float mouseSensitivityYaw = 0.044f;
	m_playerCamera->m_orientation.m_yawDegreesAboutZ -= mouseSensitivityYaw * (float)g_theInput->GetCursorDeltaX();
	m_playerCamera->m_orientation.m_pitchDegreesAboutY += mouseSensitivityYaw * (float)g_theInput->GetCursorDeltaY();
	m_playerCamera->FixAndClampAngles();

	//Push camera to correct offset relative to player based on movement mode.
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_TOGGLE_CAMERA ) ) g_currentCameraMode = (CameraMode)WrapNumberWithinCircularRange( g_currentCameraMode + 1, 0, NUM_CAMERA_MODES );

	switch ( g_currentCameraMode )
	{
		case FIRST_PERSON: cameraPos = playerPos + Vector3( 0.f, 0.f, PLAYER_HEIGHT - CAMERA_FIRST_PERSON_HEIGHT ); break;
		case FIXED_SPECTATOR: cameraPos = playerPos + Vector3( 10.f, 10.f, 10.f ); break;
		case FROM_BEHIND: cameraPos = playerPos + ( -camDirection * CAMERA_FROM_BEHIND_PUSHBACK_LENGTH ); break;
	}
}


//--------------------------------------------------------------------------------------------------------------
void World::DeactivateFarthestObsoleteChunk()
{
	if ( m_activeChunks[ m_activeDimension ].size() <= 0 ) return;

	//Find chunk.
	auto chunkIterEnd = m_activeChunks[ m_activeDimension ].end( );
	Chunk* farthestObsoleteChunk = nullptr;
	for ( auto chunkIter = m_activeChunks[ m_activeDimension ].begin( ); chunkIter != chunkIterEnd; ++chunkIter )
	{
		Chunk* currentChunk = chunkIter->second;
		if ( IsChunkBeyondFlushRadius( currentChunk ) ) //Even if true, another chunk may be farther away.
		{
			if ( farthestObsoleteChunk == nullptr )
			{
				farthestObsoleteChunk = currentChunk;
			}
			else
			{
				farthestObsoleteChunk =	GetChunkFartherFromPlayer( farthestObsoleteChunk, currentChunk );
			}
		}
	}

	if ( farthestObsoleteChunk != nullptr ) FlushChunk( farthestObsoleteChunk );
}


//--------------------------------------------------------------------------------------------------------------
void World::ActivateNearestMissingChunk() //TODO: amortize over all dimensions.
{
	//Loop around the player position +- active radius.
	WorldCoords playerPos = m_playerCamera->m_worldPosition;

	WorldCoordsXY searchMins = WorldCoordsXY( playerPos.x - m_activeRadius, playerPos.y - m_activeRadius ); //Start from mins.
	WorldCoordsXY searchMaxs = WorldCoordsXY( playerPos.x + m_activeRadius, playerPos.y + m_activeRadius );
	WorldCoordsXY currentChunkInWorldPos = searchMins;
	WorldCoordsXY closestUnloadedChunkInWorldPos;
	bool foundCandidate = false;


	//For some reason this below loop actually causes the game to run slower than the older hackier loop below? Maybe exits less?
// 	for ( float y = searchMins.y; y <= searchMaxs.y; y += 1.0f )
// 	{
// 		for ( float x = searchMins.x; x <= searchMaxs.x; x += 1.0f )
// 		{
// 			//Candidacy check--even if true, another chunk may be closer.
// 			currentChunkInWorldPos = WorldCoordsXY( x, y );
// 			ChunkCoords asCC = GetChunkCoordsFromWorldCoordsXY( currentChunkInWorldPos );
// 
// 			if ( m_activeChunks[ m_activeDimension ].count( asCC ) == 0 && IsChunkWithinActiveRadius( currentChunkInWorldPos ) )
// 			{
// 				if ( foundCandidate == false )
// 					closestUnloadedChunkInWorldPos = currentChunkInWorldPos;
// 				else
// 					closestUnloadedChunkInWorldPos = GetChunkPosNearerToPlayer( closestUnloadedChunkInWorldPos, currentChunkInWorldPos );
// 
// 				foundCandidate = true;
// 			}
// 		}
// 	}

	//Old hacky loop.
	int searchDiameter = 2 * m_activeRadius;
	int numChunksSearched = (searchDiameter * searchDiameter) / NUM_COLUMNS_PER_CHUNK; //Square search.
	for ( int chunkIndex = 0; chunkIndex < numChunksSearched; chunkIndex++ ) //TODO: Just change to a 2D loop on chunkSearchMaxX, maxY!
	{
		//Candidacy check--even if true, another chunk may be closer.
		ChunkCoords asCC = GetChunkCoordsFromWorldCoordsXY( currentChunkInWorldPos );

		if ( m_activeChunks[ m_activeDimension ].count( asCC ) == 0 && IsChunkWithinActiveRadius( currentChunkInWorldPos ) )
		{
			if ( foundCandidate == false )
				closestUnloadedChunkInWorldPos = currentChunkInWorldPos;
			else
				closestUnloadedChunkInWorldPos = GetChunkPosNearerToPlayer( closestUnloadedChunkInWorldPos, currentChunkInWorldPos );

			foundCandidate = true;
		}

		//Move search forward.
		currentChunkInWorldPos.x += CHUNK_X_LENGTH_IN_BLOCKS;
		if ( currentChunkInWorldPos.x > searchMaxs.x ) //Start search on next row.
		{
			currentChunkInWorldPos.x = searchMins.x;
			currentChunkInWorldPos.y += CHUNK_Y_WIDTH_IN_BLOCKS;
		}
	}

	if ( foundCandidate ) CreateOrLoadChunk( GetChunkCoordsFromWorldCoordsXY ( closestUnloadedChunkInWorldPos ) );
}


//--------------------------------------------------------------------------------------------------------------
void World::UpdateDirtyVertexArrays()
{
	auto chunkIterEnd = m_activeChunks[ m_activeDimension ].end( );
	for ( auto chunkIter = m_activeChunks[ m_activeDimension ].begin( ); chunkIter != chunkIterEnd; ++chunkIter )
	{
		Chunk* currentChunk = chunkIter->second;
		if ( currentChunk->IsDirty() )
		{
			currentChunk->RebuildVertexArray();
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
bool World::IsChunkBeyondFlushRadius( const Chunk* currentChunk ) const
{
	WorldCoordsXY playerPos = WorldCoordsXY( m_playerCamera->m_worldPosition.x, m_playerCamera->m_worldPosition.y );
	WorldCoordsXY chunkPos = currentChunk->GetChunkMinsInWorldUnits(); //As below radius is in world units.

	Vector2 playerChunkDisplacement = playerPos - chunkPos;
	float playerChunkDistance = playerChunkDisplacement.CalcLength( );

	if ( playerChunkDistance > m_flushRadius ) return true;
	else return false;
}


//--------------------------------------------------------------------------------------------------------------
bool World::IsChunkWithinActiveRadius( const WorldCoordsXY& chunkPos ) const
{
	WorldCoordsXY playerPos = WorldCoordsXY( m_playerCamera->m_worldPosition.x, m_playerCamera->m_worldPosition.y );

	Vector2 playerChunkDisplacement = playerPos - chunkPos;
	float playerChunkDistance = playerChunkDisplacement.CalcLength( );

	//Radii are in world units.
	if ( playerChunkDistance <= m_activeRadius ) return true;
	else return false;
}


//--------------------------------------------------------------------------------------------------------------
Chunk* World::GetChunkFartherFromPlayer( Chunk* chunk1, Chunk* chunk2 ) const
{
	WorldCoordsXY playerPos = WorldCoordsXY( m_playerCamera->m_worldPosition.x, m_playerCamera->m_worldPosition.y );

	WorldCoordsXY firstChunkPos = chunk1->GetChunkMinsInWorldUnits();
	WorldCoordsXY secondChunkPos = chunk2->GetChunkMinsInWorldUnits( );

	Vector2 firstChunkDisplacement = playerPos - firstChunkPos;
	float firstChunkDistance = firstChunkDisplacement.CalcLength();

	Vector2 secondChunkDisplacement = playerPos - secondChunkPos;
	float secondChunkDistance = secondChunkDisplacement.CalcLength();

	if ( firstChunkDistance >= secondChunkDistance ) return chunk1;
	else return chunk2;
}


//--------------------------------------------------------------------------------------------------------------
WorldCoordsXY World::GetChunkPosNearerToPlayer( const WorldCoordsXY& chunkPos1, const WorldCoordsXY& chunkPos2 ) const
{
	WorldCoordsXY playerPos = WorldCoordsXY( m_playerCamera->m_worldPosition.x, m_playerCamera->m_worldPosition.y );

	Vector2 firstChunkDisplacement = playerPos - chunkPos1;
	float firstChunkDistance = firstChunkDisplacement.CalcLength();

	Vector2 secondChunkDisplacement = playerPos - chunkPos2;
	float secondChunkDistance = secondChunkDisplacement.CalcLength();

	if ( firstChunkDistance <= secondChunkDistance ) return chunkPos1;
	else return chunkPos2;
}


//--------------------------------------------------------------------------------------------------------------
void World::FlushChunk( Chunk* obsoleteChunk )
{
	ChunkCoords cc = obsoleteChunk->GetChunkCoords();

	NullifyNeighborPointers( obsoleteChunk );

	if ( !g_disableSaving )
	{
		std::vector< unsigned char > rleBuffer;
		obsoleteChunk->GetRleString( rleBuffer );

		const char* dimensionName = GetDimensionAsString( obsoleteChunk->GetDimension() );
		SaveBufferToBinaryFile( Stringf( "Data/Saves/%s/Chunk_at_(%i,%i).chunk", dimensionName, cc.x, cc.y ), rleBuffer );
	}

	delete obsoleteChunk;
	m_activeChunks[ m_activeDimension ].erase( cc );
}


//--------------------------------------------------------------------------------------------------------------
void World::SaveAndExitWorld()
{
	if ( g_disableSaving ) return;

	for ( int dimensionIndex = 0; dimensionIndex < NUM_DIMENSIONS; dimensionIndex++ )
	{
		auto chunkIterEnd = m_activeChunks[ dimensionIndex ].end();
		std::vector< unsigned char > rleBuffer;
		for ( auto chunkIter = m_activeChunks[ dimensionIndex ].begin(); chunkIter != chunkIterEnd; ++chunkIter )
		{
			Chunk* currentChunk = chunkIter->second;
			ChunkCoords currentChunkPos = currentChunk->GetChunkCoords();
			rleBuffer.clear();
			currentChunk->GetRleString( rleBuffer );

			const char* dimensionName = GetDimensionAsString( currentChunk->GetDimension() );
			SaveBufferToBinaryFile( Stringf( "Data/Saves/%s/Chunk_at_(%i,%i).chunk", dimensionName, currentChunkPos.x, currentChunkPos.y ), rleBuffer );
		}
	}

	std::vector< float > playerDataBuffer;
	playerDataBuffer.clear();
	playerDataBuffer.push_back( m_player->m_worldPosition.x );
	playerDataBuffer.push_back( m_player->m_worldPosition.y );
	playerDataBuffer.push_back( m_player->m_worldPosition.z );
	playerDataBuffer.push_back( m_playerCamera->m_orientation.m_rollDegreesAboutX );
	playerDataBuffer.push_back( m_playerCamera->m_orientation.m_pitchDegreesAboutY );
	playerDataBuffer.push_back( m_playerCamera->m_orientation.m_yawDegreesAboutZ );
	SaveFloatsToTextFile( Stringf( "Data/Saves/Player.txt" ), playerDataBuffer );
}


//--------------------------------------------------------------------------------------------------------------
void World::LoadPlayerFile( const std::string& filePath )
{
	std::vector< float > playerDataBuffer;
	bool success = LoadFloatsFromTextFileIntoBuffer( filePath, playerDataBuffer );
	if ( !success ) return; //Will use defaults from TheGame ctor.

	//Hardcoding decoding based on SaveAndExitWorld():
	m_playerCamera->m_worldPosition.x = playerDataBuffer[ 0 ];
	m_playerCamera->m_worldPosition.y = playerDataBuffer[ 1 ];
	m_playerCamera->m_worldPosition.z = playerDataBuffer[ 2 ];
	m_playerCamera->m_orientation.m_rollDegreesAboutX = playerDataBuffer[ 3 ];
	m_playerCamera->m_orientation.m_pitchDegreesAboutY = playerDataBuffer[ 4 ];
	m_playerCamera->m_orientation.m_yawDegreesAboutZ = playerDataBuffer[ 5 ];

	m_player->m_worldPosition = m_playerCamera->m_worldPosition;
}


//--------------------------------------------------------------------------------------------------------------
void World::CreateOrLoadChunk( const ChunkCoords& unloadedChunkPos )
{
	Chunk* newChunk = new Chunk( unloadedChunkPos, m_activeDimension );
	m_activeChunks[ m_activeDimension ][ unloadedChunkPos ] = newChunk; //Nothing populates it yet.

	//Neighbor pointer configuration.
	UpdateNeighborPointers( newChunk );

	//Check if the chunk has a save file.
	std::vector< unsigned char > out_buffer;

	if ( g_disableLoading )
	{
		newChunk->PopulateChunkWithPerlinNoise();
	}
	else 
	{
		const char* dimensionName = GetDimensionAsString( m_activeDimension );
		bool fileOperationSuccess = LoadBinaryFileIntoBuffer( Stringf( "Data/Saves/%s/Chunk_at_(%i,%i).chunk", dimensionName, unloadedChunkPos.x, unloadedChunkPos.y ), out_buffer );

		if ( fileOperationSuccess )
			newChunk->PopulateChunkWithRleString( out_buffer );
		else
			newChunk->PopulateChunkWithPerlinNoise();
	}

	InitializeLightingForChunk( newChunk );
	newChunk->RebuildVertexArray( );
}


//--------------------------------------------------------------------------------------------------------------
void World::UpdateNeighborPointers( Chunk* newChunk )
{
	ChunkCoords newChunkPos = newChunk->GetChunkCoords();
	std::map< ChunkCoords, Chunk* >& activeChunksInActiveDimension = m_activeChunks[ m_activeDimension ];

	//North.
	ChunkCoords tmpCoords = ChunkCoords( newChunkPos.x + 1, newChunkPos.y );
	if ( activeChunksInActiveDimension.count( tmpCoords ) != 0 )
	{
		newChunk->m_northNeighbor = activeChunksInActiveDimension[ tmpCoords ];
		activeChunksInActiveDimension[ tmpCoords ]->m_southNeighbor = newChunk;
		activeChunksInActiveDimension[ tmpCoords ]->MarkVertexArrayDirty( );
		MarkChunkLightingDirty( activeChunksInActiveDimension[ tmpCoords ] );
	}
	else newChunk->m_northNeighbor = nullptr;

	//South.
	tmpCoords.x -= 2;
	if ( activeChunksInActiveDimension.count( tmpCoords ) != 0 )
	{
		newChunk->m_southNeighbor = activeChunksInActiveDimension[ tmpCoords ];
		activeChunksInActiveDimension[ tmpCoords ]->m_northNeighbor = newChunk;
		activeChunksInActiveDimension[ tmpCoords ]->MarkVertexArrayDirty( );
		MarkChunkLightingDirty( activeChunksInActiveDimension[ tmpCoords ] );
	}
	else newChunk->m_southNeighbor = nullptr;

	//West.
	tmpCoords.x++;
	tmpCoords.y++;
	if ( activeChunksInActiveDimension.count( tmpCoords ) != 0 )
	{
		newChunk->m_westNeighbor = activeChunksInActiveDimension[ tmpCoords ];
		activeChunksInActiveDimension[ tmpCoords ]->m_eastNeighbor = newChunk;
		activeChunksInActiveDimension[ tmpCoords ]->MarkVertexArrayDirty( );
		MarkChunkLightingDirty( activeChunksInActiveDimension[ tmpCoords ] );
	}
	else newChunk->m_westNeighbor = nullptr;

	//East.
	tmpCoords.y -= 2;
	if ( activeChunksInActiveDimension.count( tmpCoords ) != 0 )
	{
		newChunk->m_eastNeighbor = activeChunksInActiveDimension[ tmpCoords ];
		activeChunksInActiveDimension[ tmpCoords ]->m_westNeighbor = newChunk;
		activeChunksInActiveDimension[ tmpCoords ]->MarkVertexArrayDirty( );
		MarkChunkLightingDirty( activeChunksInActiveDimension[ tmpCoords ] );
	}
	else newChunk->m_eastNeighbor = nullptr;
}


//--------------------------------------------------------------------------------------------------------------
void World::NullifyNeighborPointers( Chunk* obsoleteChunk )
{
	ChunkCoords obsoleteChunkPos = obsoleteChunk->GetChunkCoords();
	std::map< ChunkCoords, Chunk* >& activeChunksInActiveDimension = m_activeChunks[ m_activeDimension ];

	//North.
	ChunkCoords tmpCoords = ChunkCoords( obsoleteChunkPos.x + 1, obsoleteChunkPos.y );
	if ( activeChunksInActiveDimension.count( tmpCoords ) != 0 )
	{
		activeChunksInActiveDimension[ tmpCoords ]->m_southNeighbor = nullptr;
		activeChunksInActiveDimension[ tmpCoords ]->MarkVertexArrayDirty( );
	}

	//South.
	tmpCoords.x -= 2;
	if ( activeChunksInActiveDimension.count( tmpCoords ) != 0 )
	{
		activeChunksInActiveDimension[ tmpCoords ]->m_northNeighbor = nullptr;
		activeChunksInActiveDimension[ tmpCoords ]->MarkVertexArrayDirty( );
	}

	//West.
	tmpCoords.x++;
	tmpCoords.y++;
	if ( activeChunksInActiveDimension.count( tmpCoords ) != 0 )
	{
		activeChunksInActiveDimension[ tmpCoords ]->m_eastNeighbor = nullptr;
		activeChunksInActiveDimension[ tmpCoords ]->MarkVertexArrayDirty( );
	}

	//East.
	tmpCoords.y -= 2;
	if ( activeChunksInActiveDimension.count( tmpCoords ) != 0 )
	{
		activeChunksInActiveDimension[ tmpCoords ]->m_westNeighbor = nullptr;
		activeChunksInActiveDimension[ tmpCoords ]->MarkVertexArrayDirty( );
	}
}


//--------------------------------------------------------------------------------------------------------------
void World::SelectBlock( const WorldCoords& selectorsPos, const WorldCoords& endOfSelectionRay, float deltaSeconds )
{
	RaycastResult3D hitResult;
	bool hitSomething = false;

	if ( !g_useAmanWooRaycastOverStepAndSample ) hitSomething = RaycastWithStepAndSample( selectorsPos, endOfSelectionRay, hitResult );
	else hitSomething = RaygancastWithAmanatidesWoo( selectorsPos, endOfSelectionRay, hitResult );
	if ( !hitSomething )
	{
		m_chunkOfSelectedBlock = nullptr;
		return;
	}

	BlockInfo selectedBlock = hitResult.lastBlockHit;
	m_chunkOfSelectedBlock = selectedBlock.m_myChunk;

	//Direction opposite selected face AKA surface normal.
	Vector3 directionOppositeSelectedFace = FindDirectionBetweenBlocks( hitResult.penultimateBlockHit, selectedBlock );
	if ( directionOppositeSelectedFace == Vector3::ZERO )
	{
		m_chunkOfSelectedBlock = nullptr;
		return; //There was either an error or we're in the block we hit.
	}


	hitResult.lastBlockHit.m_myChunk->HighlightBlock( selectedBlock.m_myBlockIndex, directionOppositeSelectedFace );

	if ( m_blockBeingDug->m_myChunk != nullptr ) //Stop digging if we moved to another block.
	{
		if ( selectedBlock != *m_blockBeingDug )
		{
			m_player->StopDigging();
			*m_blockBeingDug = BlockInfo();
			m_currentDigDamageFrame = 0;
		}
	}

	if ( g_theInput->IsMouseButtonDown( VK_LBUTTON ) )
	{
		if ( IsBlockDugEnoughToBreak( selectedBlock ) )
		{
			m_player->StopDigging();
			*m_blockBeingDug = BlockInfo();
			m_currentDigDamageFrame = 0;
			PlayBreakingSound( selectedBlock.GetBlock()->GetBlockType() );
			hitResult.lastBlockHit.m_myChunk->BreakBlock( selectedBlock.m_myBlockIndex );
			UpdateLightingForBlockBroken( selectedBlock );
		}
		else //Progressive dig damage.
		{
			if ( m_player->IsDigging() == false )
			{
				m_player->StartDigging();
				*m_blockBeingDug = selectedBlock;
			}
			PlayDiggingSound( selectedBlock.GetBlock()->GetBlockType(), deltaSeconds );
			float currentDigProgress = ( m_player->GetSecondsSpentDigging() / BlockDefinition::GetSecondsToBreak( selectedBlock.GetBlock()->GetBlockType() ) );
			m_currentDigDamageFrame = (int)floor( currentDigProgress * NUMBER_DIG_DAMAGE_FRAMES );
		}
	}
	else
	{
		m_player->StopDigging();
		*m_blockBeingDug = BlockInfo();
		m_currentDigDamageFrame = 0;
	}

	if ( g_theInput->WasMouseButtonPressedOnce( VK_RBUTTON ) )
	{
		BlockInfo blockPlacedInto;
		BlockType typeToPlace = static_cast<BlockType>( m_activeHudElement + 1 ); //SKIP AIR.
		PlayPlacingSound( typeToPlace );
		selectedBlock.m_myChunk->PlaceBlock(
			selectedBlock.m_myBlockIndex,
			directionOppositeSelectedFace, 
			typeToPlace,
			&blockPlacedInto );
		UpdateLightingForBlockPlaced( blockPlacedInto );
	}

	return;
}


//--------------------------------------------------------------------------------------------------------------
bool World::RaycastWithStepAndSample( const WorldCoords& selectorsPos, const WorldCoords& endOfRaycast, RaycastResult3D& out_result )
{
	const int NUM_STEPS_INT = RAYCAST_NUM_STEPS;
	const float NUM_STEPS_FLOAT = static_cast<float>( NUM_STEPS_INT );

	WorldCoords displacement = endOfRaycast - selectorsPos;
	if ( displacement == Vector3::ZERO ) return false;
	float totalRayLength = displacement.CalcLength();
	WorldCoords stepAlongDisplacement = displacement / NUM_STEPS_FLOAT;
	WorldCoords currentPos = selectorsPos;

	GlobalBlockCoords blockPos = GlobalBlockCoords(
		(int)floor( selectorsPos.x ),
		(int)floor( selectorsPos.y ),
		(int)floor( selectorsPos.z )
		); //Makes block coords mins-based.


	BlockInfo currentBlockInfo = GetBlockInfoFromWorldCoords( selectorsPos );
	BlockInfo blockBeforeCurrentInfo;

	for ( int i = 0; i < NUM_STEPS_INT; i++ )
	{
		currentPos += stepAlongDisplacement;

		currentBlockInfo = GetBlockInfoFromWorldCoords( currentPos );
		if ( currentBlockInfo.m_myChunk == nullptr ) return false; //Ray shot outside loaded chunks.

		if ( currentBlockInfo.m_myChunk->IsBlockSolid( currentBlockInfo.m_myBlockIndex ) )
		{
			out_result.lastBlockHit = currentBlockInfo; //Set out variable and return.
			out_result.penultimateBlockHit = blockBeforeCurrentInfo;
			out_result.impactPosition = currentPos;
			WorldCoords displacementToImpact = currentPos - selectorsPos;
			float steppedRayLength = displacementToImpact.CalcLength();
			out_result.impactFraction = ( steppedRayLength / totalRayLength );

			return true;
		}

		blockBeforeCurrentInfo = currentBlockInfo;
	}

	return false;
}


//--------------------------------------------------------------------------------------------------------------
bool World::RaygancastWithAmanatidesWoo( const WorldCoords& selectorsPos, const WorldCoords& endOfSelectionRay, RaycastResult3D& out_result )
{
	//Regan-casting
	BlockInfo originBlockInfo = GetBlockInfoFromWorldCoords( selectorsPos );
	if ( originBlockInfo.m_myChunk == nullptr ) return false;

	//Initialization of Regan-cast
	GlobalBlockCoords blockPos = GlobalBlockCoords(
		(int)floor( selectorsPos.x ),
		(int)floor( selectorsPos.y ),
		(int)floor( selectorsPos.z )
		); //Makes block coords mins-based.

	if ( originBlockInfo.m_myChunk->IsBlockSolid( originBlockInfo.m_myBlockIndex ) ) //Is ray's block origin solid?
	{
		out_result.penultimateBlockHit = out_result.lastBlockHit = GetBlockInfoFromGlobalBlockCoords( blockPos );
		out_result.impactFraction = 0.f;
		out_result.impactPosition = selectorsPos;
		return true;
	}

	Vector3 rayDisplacement = endOfSelectionRay - selectorsPos;
	float totalRayLength = rayDisplacement.CalcLength();

	//tDelta: how much t it takes to cross to the next x/y/z.
	float tDeltaX = abs( 1.0f / rayDisplacement.x );
	float tDeltaY = abs( 1.0f / rayDisplacement.y );
	float tDeltaZ = abs( 1.0f / rayDisplacement.z );

	//Which direction to step along, computable because a ray's dir is constant.
	int tileStepX = ( rayDisplacement.x > 0 ) ? 1 : -1;
	int tileStepY = ( rayDisplacement.y > 0 ) ? 1 : -1;
	int tileStepZ = ( rayDisplacement.z > 0 ) ? 1 : -1;

	int offsetToLeadingEdgeX = ( tileStepX + 1 ) >> 1; //replaces div by 2^1.
	int offsetToLeadingEdgeY = ( tileStepY + 1 ) >> 1; //replaces div by 2^1.
	int offsetToLeadingEdgeZ = ( tileStepZ + 1 ) >> 1; //replaces div by 2^1.

	float firstIntersectionOnX = (float)( blockPos.x + offsetToLeadingEdgeX );
	float firstIntersectionOnY = (float)( blockPos.y + offsetToLeadingEdgeY );
	float firstIntersectionOnZ = (float)( blockPos.z + offsetToLeadingEdgeZ );

	float tOfNextCrossingOnX = abs( firstIntersectionOnX - selectorsPos.x ) * tDeltaX;
	float tOfNextCrossingOnY = abs( firstIntersectionOnY - selectorsPos.y ) * tDeltaY;
	float tOfNextCrossingOnZ = abs( firstIntersectionOnZ - selectorsPos.z ) * tDeltaZ;

	//Main Loop of Regan-cast
	BlockInfo currentBlockInfo = originBlockInfo;
	BlockInfo blockBeforeCurrentInfo = currentBlockInfo;

	bool shutUpVisualStudioWarning = true;
	while ( shutUpVisualStudioWarning ) // Loop until impact or end of line segment.
	{
		//Find min of tOfNextCrossingOnX,Y,Z to determine what line gets crossed next.

		//if OnX is lowest
		if ( ( tOfNextCrossingOnX < tOfNextCrossingOnY ) && ( tOfNextCrossingOnX < tOfNextCrossingOnZ ) )
		{
			if ( tOfNextCrossingOnX > 1 ) return false; //No impact, i.e. next crossing past endpoint.
		
			blockPos.x += tileStepX; //move into next tile on x
			currentBlockInfo = GetBlockInfoFromGlobalBlockCoords( blockPos );
			if ( currentBlockInfo.m_myChunk == nullptr ) return false;
			if ( currentBlockInfo.m_myChunk->IsBlockSolid( currentBlockInfo.m_myBlockIndex ) ) //Impact.
			{
				out_result.lastBlockHit = currentBlockInfo;
				out_result.penultimateBlockHit = blockBeforeCurrentInfo;
				out_result.impactPosition = selectorsPos + (rayDisplacement * tOfNextCrossingOnX);
				WorldCoords displacementToImpact = out_result.impactPosition - selectorsPos;
				float steppedRayLength = displacementToImpact.CalcLength();
				out_result.impactFraction = ( steppedRayLength / totalRayLength );

				return true;
			}
			else tOfNextCrossingOnX += tDeltaX; //Move t to next crossing onX.			
		}
		
		//if OnY is lowest
		else if ( ( tOfNextCrossingOnY < tOfNextCrossingOnX ) && ( tOfNextCrossingOnY < tOfNextCrossingOnZ ) )
		{
			if ( tOfNextCrossingOnY > 1 ) return false; //No impact, i.e. next crossing past endpoint.

			blockPos.y += tileStepY; //move into next tile on y
			currentBlockInfo = GetBlockInfoFromGlobalBlockCoords( blockPos );
			if ( currentBlockInfo.m_myChunk == nullptr ) return false;
			if ( currentBlockInfo.m_myChunk->IsBlockSolid( currentBlockInfo.m_myBlockIndex ) ) //Impact.
			{
				out_result.lastBlockHit = currentBlockInfo;
				out_result.penultimateBlockHit = blockBeforeCurrentInfo;
				out_result.impactPosition = selectorsPos + ( rayDisplacement * tOfNextCrossingOnY );
				WorldCoords displacementToImpact = out_result.impactPosition - selectorsPos;
				float steppedRayLength = displacementToImpact.CalcLength();
				out_result.impactFraction = ( steppedRayLength / totalRayLength );

				return true;
			}
			else tOfNextCrossingOnY += tDeltaY; //Move t to next crossing onY.
		}
		
		// tOfNextCrossingOnZ is lowest
		else 
		{
			if ( tOfNextCrossingOnZ > 1 ) return false; //No impact, i.e. next crossing past endpoint.

			blockPos.z += tileStepZ; //move into next tile on z
			currentBlockInfo = GetBlockInfoFromGlobalBlockCoords( blockPos );
			if ( currentBlockInfo.m_myChunk == nullptr ) return false;
			if ( currentBlockInfo.m_myChunk->IsBlockSolid( currentBlockInfo.m_myBlockIndex ) ) //Impact.
			{
				out_result.lastBlockHit = currentBlockInfo;
				out_result.penultimateBlockHit = blockBeforeCurrentInfo;
				out_result.impactPosition = selectorsPos + ( rayDisplacement * tOfNextCrossingOnZ );
				WorldCoords displacementToImpact = out_result.impactPosition - selectorsPos;
				float steppedRayLength = displacementToImpact.CalcLength();
				out_result.impactFraction = ( steppedRayLength / totalRayLength );

				return true;
			}
			else tOfNextCrossingOnZ += tDeltaZ; //Move t to next crossing onZ.
		}

		blockBeforeCurrentInfo = currentBlockInfo;
	}

	return false;
}


//--------------------------------------------------------------------------------------------------------------
Vector3 World::FindDirectionBetweenBlocks( BlockInfo lastBlockHit, BlockInfo hitBlockInfo )
{
	if ( lastBlockHit.m_myChunk == nullptr ) return Vector3::ZERO;

	BlockInfo steppingBlock = lastBlockHit;
	if ( hitBlockInfo.m_myBlockIndex < 0 || hitBlockInfo.m_myBlockIndex > NUM_BLOCKS_PER_CHUNK ) return Vector3::ZERO;
	if ( steppingBlock.m_myBlockIndex < 0 || steppingBlock.m_myBlockIndex > NUM_BLOCKS_PER_CHUNK ) return Vector3::ZERO;
	steppingBlock.StepDown();
	if ( steppingBlock.m_myBlockIndex < 0 || steppingBlock.m_myBlockIndex > NUM_BLOCKS_PER_CHUNK ) return Vector3::ZERO;
	LocalBlockCoords steppingBlockCoords = GetLocalBlockCoordsFromLocalBlockIndex( steppingBlock.m_myBlockIndex );
	LocalBlockCoords hitBlockCoords = GetLocalBlockCoordsFromLocalBlockIndex( hitBlockInfo.m_myBlockIndex );
	if ( steppingBlockCoords.z == hitBlockCoords.z )
		return WORLD_DOWN;

	steppingBlock = lastBlockHit;
	steppingBlock.StepUp();
	if ( steppingBlock.m_myBlockIndex < 0 || steppingBlock.m_myBlockIndex > NUM_BLOCKS_PER_CHUNK ) return Vector3::ZERO;
	steppingBlockCoords = GetLocalBlockCoordsFromLocalBlockIndex( steppingBlock.m_myBlockIndex );
	if ( steppingBlockCoords.z == hitBlockCoords.z )
		return WORLD_UP;

	steppingBlock = lastBlockHit;
	steppingBlock.StepSouth();
	steppingBlockCoords = GetLocalBlockCoordsFromLocalBlockIndex( steppingBlock.m_myBlockIndex );
	if ( steppingBlockCoords.y == hitBlockCoords.y )
		return WORLD_RIGHT;

	steppingBlock = lastBlockHit;
	steppingBlock.StepNorth();
	steppingBlockCoords = GetLocalBlockCoordsFromLocalBlockIndex( steppingBlock.m_myBlockIndex );
	if ( steppingBlockCoords.y == hitBlockCoords.y )
		return WORLD_LEFT;

	steppingBlock = lastBlockHit;
	steppingBlock.StepEast();
	steppingBlockCoords = GetLocalBlockCoordsFromLocalBlockIndex( steppingBlock.m_myBlockIndex );
	if ( steppingBlockCoords.x == hitBlockCoords.x )
		return WORLD_BACKWARD;

	steppingBlock = lastBlockHit;
	steppingBlock.StepWest(); //Looking down +x, so front face goes with stepping south.
	steppingBlockCoords = GetLocalBlockCoordsFromLocalBlockIndex( steppingBlock.m_myBlockIndex );
	if ( steppingBlockCoords.x == hitBlockCoords.x )
		return WORLD_FORWARD;

	return Vector3::ZERO;
}


//--------------------------------------------------------------------------------------------------------------
void World::UnhighlightSelectedBlock()
{
	auto chunkIterEnd = m_activeChunks[ m_activeDimension ].end();
	for ( auto chunkIter = m_activeChunks[ m_activeDimension ].begin( ); chunkIter != chunkIterEnd; ++chunkIter )
	{
		Chunk* currentChunk = chunkIter->second;
		if ( currentChunk->IsHighlighting() )
			currentChunk->Unhighlight();
	}
}


//--------------------------------------------------------------------------------------------------------------
BlockInfo World::GetBlockInfoFromWorldCoords( const WorldCoords& wc )
{
	WorldCoordsXY wc2D = WorldCoordsXY( wc.x, wc.y );
	ChunkCoords chunkCoordsOfCurrentPos = GetChunkCoordsFromWorldCoordsXY( wc2D );
	if ( m_activeChunks[ m_activeDimension ].count( chunkCoordsOfCurrentPos ) == 0 ) return BlockInfo(); //WorldCoords outside loaded chunks.

	Chunk* currentChunk = m_activeChunks[ m_activeDimension ][ chunkCoordsOfCurrentPos ];
	WorldCoordsXY currentChunkCoordsInWorldUnits = currentChunk->GetChunkMinsInWorldUnits(); //e.g. (1,0) becomes (16.f,0.f).

	LocalBlockCoords lbc;
	lbc.x = (int)( wc.x - currentChunkCoordsInWorldUnits.x );
	lbc.y = (int)( wc.y - currentChunkCoordsInWorldUnits.y );
	lbc.z = (int)wc.z;
	
	return BlockInfo( currentChunk, GetLocalBlockIndexFromLocalBlockCoords( lbc ) );
}


//--------------------------------------------------------------------------------------------------------------
BlockInfo World::GetBlockInfoFromGlobalBlockCoords( const GlobalBlockCoords& blockPos )
{
	WorldCoords wc;

	wc.x = (float)blockPos.x;
	wc.y = (float)blockPos.y;
	wc.z = (float)blockPos.z;

	return GetBlockInfoFromWorldCoords( wc );
}


//--------------------------------------------------------------------------------------------------------------
GlobalBlockCoords World::GetGlobalBlockCoordsFromBlockInfo( const BlockInfo& blockInfo )
{
	GlobalBlockCoords gbc;
	LocalBlockCoords lbc = GetLocalBlockCoordsFromLocalBlockIndex( blockInfo.m_myBlockIndex );
	WorldCoordsXY chunkPosInWorld = blockInfo.m_myChunk->GetChunkMinsInWorldUnits();

	gbc.x = (int)( chunkPosInWorld.x * lbc.x );
	gbc.y = (int)( chunkPosInWorld.y * lbc.y );
	gbc.z = lbc.z;

	return gbc;
}


//--------------------------------------------------------------------------------------------------------------
void World::InitializeLightingForChunk( Chunk* newChunk )
{
	//Pass 1: mark sky blocks.
	for ( int columnIndex = 0; columnIndex < NUM_COLUMNS_PER_CHUNK; columnIndex++ )
	{
		for ( int blockHeight = CHUNK_Z_HEIGHT_IN_BLOCKS-1; blockHeight >= 0; blockHeight-- )
		{
			LocalColumnCoords lcc = newChunk->GetLocalColumnCoordsFromChunkColumnIndex( columnIndex );
			LocalBlockCoords lbc = IntVector3( lcc.x, lcc.y, blockHeight );

			Block* currentBlock = newChunk->GetBlockFromLocalBlockCoords( lbc );
			if ( currentBlock == nullptr ) return;

			if ( currentBlock->IsOpaque( ) ) break; //Stop descent, no more sky blocks, nor is this one.

			currentBlock->SetBlockToBeSky( );
			currentBlock->SetLightLevel( MAX_LIGHTING_LEVEL );
			if ( g_renderSkyBlocksAsDebugPoints && blockHeight < 68 )
			{
				AddDebugPoint
				(
					newChunk->GetWorldCoordsFromLocalBlockIndex( GetLocalBlockIndexFromLocalBlockCoords( lbc ) ) + Vector3( .5f, .5f, .5f ),
					Rgba( 1.f, 0.f, 0.f )
				);
			}
		}
	}

	//Pass 2: start letting light bleed into non-sky XY-neighbors via dirty flag.
	for ( int columnIndex = 0; columnIndex < NUM_COLUMNS_PER_CHUNK; columnIndex++ )
	{
		for ( int blockHeight = CHUNK_Z_HEIGHT_IN_BLOCKS - 1; blockHeight >= 0; blockHeight-- )
		{
			LocalColumnCoords lcc = newChunk->GetLocalColumnCoordsFromChunkColumnIndex( columnIndex );
			LocalBlockCoords lbc = IntVector3( lcc.x, lcc.y, blockHeight );
			LocalBlockIndex lbi = GetLocalBlockIndexFromLocalBlockCoords( lbc );

			Block* currentBlock = newChunk->GetBlockFromLocalBlockIndex( lbi );
			if ( currentBlock == nullptr ) return;

			if ( currentBlock->IsOpaque() ) break; //Stop descent, no more sky blocks, nor is this one.

			BlockInfo currentBlockInfo = BlockInfo( newChunk, lbi );
			DirtyNonSkyNeighborsForBlock( currentBlockInfo, false );
		}
	}

	//Pass 3: handle blocks that are non-sky light sources.
	for ( int blockIndex = 0; blockIndex < NUM_BLOCKS_PER_CHUNK; blockIndex++ )
	{
		Block* currentBlock = newChunk->GetBlockFromLocalBlockIndex( blockIndex );
		if ( currentBlock == nullptr ) continue;

		if ( !currentBlock->IsSky() && ( BlockDefinition::GetLightLevel( currentBlock->GetBlockType() ) > 0 ) )
		{
			MarkBlockLightingDirty( BlockInfo( newChunk, blockIndex ) );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void World::UpdateLighting()
{
	while ( !m_dirtyBlocks.empty() )
	{
		BlockInfo bi = m_dirtyBlocks.front();
		m_dirtyBlocks.pop_front();
		
		Block* currentBlock = bi.GetBlock();
		if ( currentBlock == nullptr ) continue;
		currentBlock->SetLightingNotDirty();
		
		int idealLight = GetIdealLightForBlock( bi );
		int currentLight = currentBlock->GetLightLevel();
		if ( idealLight == currentLight ) continue; //Already correct, implying neighbors don't need dirtying.

		currentBlock->SetLightLevel( idealLight ); //Problem is this is not hit!
		DirtyNonSkyNeighborsForBlock( bi );
		bi.m_myChunk->MarkVertexArrayDirty();
	}
}


//--------------------------------------------------------------------------------------------------------------
void World::DirtyNonSkyNeighborsForBlock( BlockInfo& bi, bool includeVerticalNeighbors /*= true */ )
{
	BlockInfo neighbor;

	neighbor = bi;
	neighbor.StepEast();
	if ( !neighbor.GetBlock()->IsSky() ) MarkBlockLightingDirty( neighbor );

	neighbor = bi;
	neighbor.StepWest();
	if ( !neighbor.GetBlock()->IsSky() ) MarkBlockLightingDirty( neighbor );

	neighbor = bi;
	neighbor.StepNorth();
	if ( !neighbor.GetBlock()->IsSky() ) MarkBlockLightingDirty( neighbor );

	neighbor = bi;
	neighbor.StepSouth();
	if ( !neighbor.GetBlock()->IsSky() ) MarkBlockLightingDirty( neighbor );

	if ( includeVerticalNeighbors )
	{
		neighbor = bi;
		neighbor.StepUp();
		if ( !neighbor.GetBlock()->IsSky() ) MarkBlockLightingDirty( neighbor );

		neighbor = bi;
		neighbor.StepDown();
		if ( !neighbor.GetBlock()->IsSky() ) MarkBlockLightingDirty( neighbor );
	}
}


//--------------------------------------------------------------------------------------------------------------
int World::GetIdealLightForBlock( BlockInfo& bi )
{
	Block* block = bi.GetBlock();
	if ( block == nullptr ) ERROR_AND_DIE( "Can't GetIdealLightFor Nullptr Block" );

	//Easy-out for opaque blocks, else neighbors lift them to nonzero values.
	BlockType currentBlockType = block->GetBlockType();
	if ( BlockDefinition::IsOpaque( currentBlockType ) ) return BlockDefinition::GetLightLevel( currentBlockType );

	//"Light-deciding Committee"
	int currentBlockLight = BlockDefinition::GetLightLevel( block->GetBlockType() ); //Need to add glowstone.
	int skyFactor = block->IsSky() ? bi.m_myChunk->GetCurrentSkyLightLevel() : 0;
	int highestNeighborLight = 0; //Taking the max of our neighbors' light levels.

	BlockInfo neighbor;

	neighbor = bi;
	neighbor.StepEast();
	highestNeighborLight = GetMax( highestNeighborLight, neighbor.GetBlock( )->GetLightLevel( ) );

	neighbor = bi;
	neighbor.StepWest();
	highestNeighborLight = GetMax( highestNeighborLight, neighbor.GetBlock( )->GetLightLevel( ) );

	neighbor = bi;
	neighbor.StepNorth();
	highestNeighborLight = GetMax( highestNeighborLight, neighbor.GetBlock( )->GetLightLevel( ) );

	neighbor = bi;
	neighbor.StepSouth();
	highestNeighborLight = GetMax( highestNeighborLight, neighbor.GetBlock( )->GetLightLevel( ) );

	neighbor = bi;
	neighbor.StepUp();
	highestNeighborLight = GetMax( highestNeighborLight, neighbor.GetBlock( )->GetLightLevel( ) );

	neighbor = bi;
	neighbor.StepDown();
	highestNeighborLight = GetMax( highestNeighborLight, neighbor.GetBlock( )->GetLightLevel( ) );

	return GetMax( highestNeighborLight - 1, GetMax( skyFactor, currentBlockLight ) );
}


//--------------------------------------------------------------------------------------------------------------
void World::UpdateChunks()
{
	int chunkLightLevel = ( g_useNightLightLevel ? NIGHT_LIGHTING_LEVEL : MAX_LIGHTING_LEVEL ); //Currently just a constant.
	
	auto chunkIterEnd = m_activeChunks[ m_activeDimension ].end();
	for ( auto chunkIter = m_activeChunks[ m_activeDimension ].begin(); chunkIter != chunkIterEnd; ++chunkIter )
	{
		Chunk* currentChunk = chunkIter->second;

		if ( currentChunk->GetCurrentSkyLightLevel() != chunkLightLevel )
		{
			currentChunk->SetCurrentSkyLightLevel( chunkLightLevel );
			MarkChunkLightingDirty( currentChunk );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void World::UpdateLightingForBlockPlaced( BlockInfo blockPlacedInto )
{
	//Will become placed block type before entering this method, but only updates opaque flag.
	BlockInfo originalBlock = blockPlacedInto;


	Block* currentBlock = blockPlacedInto.GetBlock( );
	if ( currentBlock == nullptr ) return;

	if ( currentBlock->IsSky( ) ) //If so, need to dim things below it.
	{
		//Loop down until opaque block, marking (including start) not sky and is dirty.
		do 
		{
			currentBlock->SetBlockToNotBeSky();
			MarkBlockLightingDirty( blockPlacedInto );

			blockPlacedInto.StepDown();
			currentBlock = blockPlacedInto.GetBlock();
			if ( currentBlock == nullptr ) break;

		} while ( !currentBlock->IsOpaque() ); //What if it's air all the way down?

	}
	
	MarkBlockLightingDirty( originalBlock );
}


//--------------------------------------------------------------------------------------------------------------
void World::UpdateLightingForBlockBroken( BlockInfo blockBroken )
{
	//Will become air before entering this method, but only updates opaque flag.

	BlockInfo originalBlock = blockBroken;

	blockBroken.StepUp();
	Block* blockAboveBrokenBlock = blockBroken.GetBlock();
	if ( blockAboveBrokenBlock == nullptr ) return;

	if ( blockAboveBrokenBlock->IsSky( ) )
	{
		blockBroken.StepDown(); //Back to the actual position a block was broken at.
		Block* currentBlock = blockBroken.GetBlock();

		//Loop down until opaque block, marking (including start) is sky and is dirty.
		while ( !currentBlock->IsOpaque() )
		{
			currentBlock = blockBroken.GetBlock();
			if ( currentBlock == nullptr ) break;

			currentBlock->SetBlockToBeSky();
			MarkBlockLightingDirty( blockBroken );

			blockBroken.StepDown();

		} //What if it's air all the way down?
	}

	MarkBlockLightingDirty( originalBlock );
}


//--------------------------------------------------------------------------------------------------------------
void World::MarkChunkLightingDirty( Chunk* chunk )
{
	for ( int blockIndex = 0; blockIndex < NUM_BLOCKS_PER_CHUNK; blockIndex++ )
		MarkBlockLightingDirty( BlockInfo( chunk, blockIndex ) );
	//Note this is a bit excessive because we only really need to dirty blocks that are non-opaque && on border.
}


//--------------------------------------------------------------------------------------------------------------
bool World::IsBlockDugEnoughToBreak( BlockInfo block )
{
	if ( g_currentMovementMode == NOCLIP ) return true;
	return m_player->GetSecondsSpentDigging() > BlockDefinition::GetSecondsToBreak( block.GetBlock()->GetBlockType() );
}


//--------------------------------------------------------------------------------------------------------------
bool World::IsPlayerOnGround()
{
	//Get the player's x, y, and the z of their feet.
	WorldCoords playerFeetPos = m_player->GetFeetPos();
	if ( IsWorldPositionOnGround( playerFeetPos ) ) 
		return false; //Blocks exist as [1x1x1]'s, so z should have virtually no decimal part if on a block.

	//Now we just need to return if the block under us is solid--if so, true; if not, false.
	BlockInfo biForBackLeftCorner = GetBlockInfoFromWorldCoords( playerFeetPos + WorldCoords( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, 0.f ) ); //World is mins-based, so still need to step down from this.
	BlockInfo biForBackRightCorner = GetBlockInfoFromWorldCoords( playerFeetPos + WorldCoords( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, 0.f ) );
	BlockInfo biForFrontLeftCorner = GetBlockInfoFromWorldCoords( playerFeetPos + WorldCoords( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, 0.f ) );
	BlockInfo biForFrontRightCorner = GetBlockInfoFromWorldCoords( playerFeetPos + WorldCoords( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, 0.f ) );
	
	if ( !( biForBackLeftCorner.StepDown( ) && biForBackRightCorner.StepDown( ) && biForFrontLeftCorner.StepDown( ) && biForFrontRightCorner.StepDown( ) ) ) return false; //Move failed.

	//Want to be able to jump even if it's just our corner!
	if ( BlockDefinition::IsSolid( biForBackLeftCorner.GetBlock()->GetBlockType() ) ) return true;
	if ( BlockDefinition::IsSolid( biForBackRightCorner.GetBlock( )->GetBlockType( ) ) ) return true;
	if ( BlockDefinition::IsSolid( biForFrontLeftCorner.GetBlock( )->GetBlockType( ) ) ) return true;
	if ( BlockDefinition::IsSolid( biForFrontRightCorner.GetBlock( )->GetBlockType( ) ) ) return true;

	return false;
}


//--------------------------------------------------------------------------------------------------------------
void World::PlayBreakingSound( BlockType blockTypeBroken )
{
	BlockDefinition::PlayBreakingSound( blockTypeBroken );
}


//--------------------------------------------------------------------------------------------------------------
void World::PlayPlacingSound( BlockType blockTypePlaced )
{
	BlockDefinition::PlayPlacingSound( blockTypePlaced );
}


//--------------------------------------------------------------------------------------------------------------
void World::PlayDiggingSound( BlockType blockTypeDug, float deltaSeconds )
{
	BlockDefinition::PlayDiggingSound( blockTypeDug, deltaSeconds );
}


//--------------------------------------------------------------------------------------------------------------
void World::PlayWalkingSound( float deltaMove )
{
	m_distanceSinceLastWalkSound += deltaMove;
	if ( m_distanceSinceLastWalkSound - GetRandomFloatZeroToOne() < DISTANCE_BETWEEN_WALK_SOUNDS ) return;
	else m_distanceSinceLastWalkSound = 0.f;

	//Get the player's x, y, and the z of their feet.
	WorldCoords playerFeetPos = m_player->GetFeetPos();
	if ( IsWorldPositionOnGround( playerFeetPos ) )
		return; //Blocks exist as [1x1x1]'s, so z should have virtually no decimal part if on a block.

	BlockInfo biForBackLeftCorner = GetBlockInfoFromWorldCoords( playerFeetPos + WorldCoords( -PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, 0.f ) ); //World is mins-based, so still need to step down from this.
	BlockInfo biForBackRightCorner = GetBlockInfoFromWorldCoords( playerFeetPos + WorldCoords( -PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, 0.f ) );
	BlockInfo biForFrontLeftCorner = GetBlockInfoFromWorldCoords( playerFeetPos + WorldCoords( PLAYER_HALF_WIDTH, PLAYER_HALF_WIDTH, 0.f ) );
	BlockInfo biForFrontRightCorner = GetBlockInfoFromWorldCoords( playerFeetPos + WorldCoords( PLAYER_HALF_WIDTH, -PLAYER_HALF_WIDTH, 0.f ) );

	if ( !( biForBackLeftCorner.StepDown() && biForBackRightCorner.StepDown() && biForFrontLeftCorner.StepDown() && biForFrontRightCorner.StepDown() ) ) return; //Move failed.

	std::vector< BlockType > soundCandidates;
	BlockType backLeftType = biForBackLeftCorner.GetBlock()->GetBlockType();
	if ( backLeftType != AIR ) soundCandidates.push_back( backLeftType );
	BlockType backRightType = biForBackRightCorner.GetBlock()->GetBlockType();
	if ( backRightType != AIR ) soundCandidates.push_back( backRightType );
	BlockType frontLeftType = biForFrontLeftCorner.GetBlock()->GetBlockType();
	if ( frontLeftType != AIR ) soundCandidates.push_back( frontLeftType );
	BlockType frontRightType = biForFrontRightCorner.GetBlock()->GetBlockType();
	if ( frontRightType != AIR ) soundCandidates.push_back( frontRightType );

	int numCandidates = (int)soundCandidates.size();
	if ( numCandidates == 0 ) return;
	else BlockDefinition::PlayWalkingSound( soundCandidates[ GetRandomIntInRange( 0.f, numCandidates-1.f ) ] );
}


bool World::IsWorldPositionOnGround( WorldCoords position )
{
	return position.z - floor( position.z ) > THRESHOLD_TO_BE_CONSIDERED_ON_GROUND;
}

//--------------------------------------------------------------------------------------------------------------
void World::MarkBlockLightingDirty( const BlockInfo& bi )
{
	Block* blockToDirty = bi.GetBlock();
	if ( blockToDirty == nullptr ) return;
	if ( blockToDirty->IsLightingDirty() ) return; //Ensures no duplicate push-backs.

	blockToDirty->SetLightingDirty();
	m_dirtyBlocks.push_back( bi );
}