#pragma once


#include <map>
#include <deque>

#include "Engine/Renderer/TheRenderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/BlockInfo.hpp"

//-----------------------------------------------------------------------------
class Chunk;
class Player;
class Camera3D;

//-----------------------------------------------------------------------------
struct RaycastResult3D
{
	RaycastResult3D() : impactFraction(1.f) {}
	BlockInfo lastBlockHit;
	BlockInfo penultimateBlockHit;
	WorldCoords impactPosition;
	float impactFraction;
};


//-----------------------------------------------------------------------------
class World
{
public:

	World( Camera3D* camera, Player* player );
	~World();
	void RenderChunk( const Chunk& chunk ) const;
	bool IsChunkVisible( const Chunk& chunk ) const;
	void Render() const;
	void Update( float deltaSeconds );
	void SaveAndExitWorld();
	void LoadPlayerFile( const std::string& filePath );
	Dimension GetActiveDimension() const { return m_activeDimension; }
	int GetActiveHudElement() const { return m_activeHudElement; }
	void SetActiveHudElement( int newValue ) { m_activeHudElement = newValue; }

	Dimension m_activeDimension;
	std::map< ChunkCoords, Chunk* > m_activeChunks[ NUM_DIMENSIONS ];
	Chunk* m_chunkOfSelectedBlock;
	int m_currentDigDamageFrame;
	BlockInfo* m_blockBeingDug;

private:

	void UpdateCameraAndPlayer( float deltaSeconds );

	void UpdateFromMovementKeys( float deltaSeconds, const Vector3& camForwardXY, const Vector3& camLeftXY, Vector3 &posToMove );

	void ApplyFrictionStopping( Vector3 &playerVel, BlockType blockTypeForPlayerFeet );

	Vector3 GetPhysicsCorrectedVelocityForDeltaSeconds( const Vector3& velocityToPrevent, Vector3& posToMove, float deltaSeconds );
	BlockType GetBlockTypeFromWorldCoords( WorldCoords wc );

	bool BoxTraceWithStepAndSample( const Vector3& rayStartPos, const Vector3& rayEndPos, RaycastResult3D& hitResult );
	bool BoxTraceWithTwelveRaygancasts( const Vector3& rayStartPos, const Vector3& rayEndPos, RaycastResult3D& minHitResult );
	bool BoxTraceWithAmanatidesWoo( Vector3 boxCenterStartPos, Vector3 boxCenterEndPos, float boxHalfLengthX, float boxHalfWidthY, float boxHalfHeightZ, RaycastResult3D& out_result );

	void UpdateMouseAndCameraOffset( Vector3& cameraPos, Vector3& playerPos, Vector3 camDirection );

	void SelectBlock( const WorldCoords& selectorsPos, const WorldCoords& endOfSelectionRay, float deltaSeconds );
	void UnhighlightSelectedBlock();
	bool RaycastWithStepAndSample( const WorldCoords& selectorsPos, const WorldCoords& endOfSelectionRay, RaycastResult3D& out_result );
	bool RaygancastWithAmanatidesWoo( const WorldCoords& selectorsPos, const WorldCoords& endOfSelectionRay, RaycastResult3D& out_result );

	void DeactivateFarthestObsoleteChunk();
	void ActivateNearestMissingChunk();
	void InitializeLightingForChunk( Chunk* chunk );
	void UpdateDirtyVertexArrays();

	bool IsChunkBeyondFlushRadius( const Chunk* currentChunk ) const;
	bool IsChunkWithinActiveRadius( const WorldCoordsXY& chunkPos ) const;
	Chunk* GetChunkFartherFromPlayer( Chunk* chunk1, Chunk* chunk2 ) const;
	WorldCoordsXY GetChunkPosNearerToPlayer( const WorldCoordsXY& chunkPos1, const WorldCoordsXY& chunkPos2 ) const;
	void FlushChunk( Chunk* obsoleteChunk );
	void CreateOrLoadChunk( const ChunkCoords& unloadedChunkPos );
	void UpdateNeighborPointers( Chunk* newChunk );
	void NullifyNeighborPointers( Chunk* obsoleteChunk );

	BlockInfo GetBlockInfoFromWorldCoords( const WorldCoords& wc );
	BlockInfo GetBlockInfoFromGlobalBlockCoords( const GlobalBlockCoords& blockPos );
	GlobalBlockCoords GetGlobalBlockCoordsFromBlockInfo( const BlockInfo& blockInfo );
	void MarkBlockLightingDirty( const BlockInfo& bi );
	Vector3 FindDirectionBetweenBlocks( BlockInfo lastBlockHit, BlockInfo hitBlockInfo );
	void UpdateLighting();
	void DirtyNonSkyNeighborsForBlock( BlockInfo& bi, bool includeVerticalNeighbors = true );
	int GetIdealLightForBlock( BlockInfo& bi );

	void UpdateChunks();
	void UpdateLightingForBlockPlaced( BlockInfo blockPlacedInto );
	void UpdateLightingForBlockBroken( BlockInfo blockBroken );
	void MarkChunkLightingDirty( Chunk* chunk );
	bool IsBlockDugEnoughToBreak( BlockInfo block );

	bool IsPlayerOnGround();

	void PlayBreakingSound( BlockType blockTypeBroken );
	void PlayPlacingSound( BlockType blockTypePlaced );
	void PlayDiggingSound( BlockType blockTypeDug, float deltaSeconds );
	void PlayWalkingSound( float deltaMove ); //Grabs types at player's feet itself.
	bool IsWorldPositionOnGround( WorldCoords position );
private:

	std::deque< BlockInfo > m_dirtyBlocks;
	Camera3D* m_playerCamera;
	Player* m_player;

	int m_activeHudElement;

	int m_activeRadius;
	int m_flushRadius;

	static SoundID m_hudChangeSoundID;
	int m_lastFrameHudElement;

	static float m_distanceSinceLastWalkSound;
};