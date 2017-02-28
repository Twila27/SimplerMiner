#pragma once


#include "Engine/Renderer/Vertexes.hpp"
#include "Engine/Math/Vector3.hpp"
#include <vector>


#include "Game/GameCommon.hpp"
#include "Game/Block.hpp"


//-----------------------------------------------------------------------------
class SpriteSheet;
struct BlockInfo;
#define BLOCK_UNHIGHLIGHTED (99999)


//-----------------------------------------------------------------------------
class Chunk
{
public:

	enum ChunkCornerPosition {
		NORTHWEST_TOP = 0, NORTHEAST_TOP, SOUTHWEST_TOP, SOUTHEAST_TOP,
		NORTHWEST_BOTTOM, NORTHEAST_BOTTOM, SOUTHWEST_BOTTOM, SOUTHEAST_BOTTOM, 
		NUM_CHUNK_CORNERS
	};
	WorldCoords m_chunkCornersInWorldUnits[ 8 ];

	Chunk( ChunkCoords chunkPosition, Dimension chunkDimension );
	~Chunk();

	void PopulateChunkWithFlatStructure();
	void PopulateChunkWithPerlinNoise();
	void PopulateChunkWithRleString( const std::vector< unsigned char >& rleString );
	void GetRleString( std::vector< unsigned char >& out_rleBuffer );

	void RebuildVertexArray();
	void Render() const;
	inline void HideChunk() { m_isVisible = false; }
	inline void ShowChunk() { m_isVisible = true; }
	inline bool IsDirty() const { return m_isVertexArrayDirty; }
	inline void MarkVertexArrayDirty() { m_isVertexArrayDirty = true; }

	int GetCurrentSkyLightLevel() const { return m_currentSkyLightLevel; }
	void SetCurrentSkyLightLevel( int clampedNewLightLevel );

	ChunkCoords GetChunkCoords() const { return m_chunkPosition; }
	WorldCoords GetChunkCenterInWorldUnits() const;
	WorldCoordsXY GetChunkMinsInWorldUnits() const;
	GlobalColumnCoords GetGlobalColumnCoordsFromChunkColumnIndex( ChunkColumnIndex cci ) const;
	LocalColumnCoords GetLocalColumnCoordsFromChunkColumnIndex( ChunkColumnIndex cci ) const;
	WorldCoords GetWorldCoordsFromLocalBlockIndex( LocalBlockIndex lbi ) const;
	inline Dimension Chunk::GetDimension() const { return m_chunkDimension; }

	inline bool IsHighlighting() const { return ( m_selectedFace == NONE ) || ( m_selectedBlock < NUM_BLOCKS_PER_CHUNK ); }
	void Unhighlight() { m_selectedBlock = BLOCK_UNHIGHLIGHTED; m_selectedFace = NONE; }
	void HighlightBlock( LocalBlockIndex lbi, Vector3 directionOppositeFace );
	Block* GetBlockFromLocalBlockIndex( LocalBlockIndex m_myBlockIndex );
	Block* GetBlockFromLocalBlockCoords( const LocalBlockCoords& lbc );
	void BreakBlock( LocalBlockIndex lbi );
	void PlaceBlock( LocalBlockIndex lbi, const Vector3& directionOppositeFace, BlockType typeToPlace, BlockInfo* out_blockPlaced = nullptr );
	bool IsBlockSolid( LocalBlockIndex lbi ) const;

	Chunk* m_northNeighbor; //+x.
	Chunk* m_eastNeighbor; //-y.
	Chunk* m_westNeighbor; //+y.
	Chunk* m_southNeighbor; //-x.

	LocalBlockIndex m_selectedBlock;
	BlockFace m_selectedFace;

private:

	void RenderWithVbo() const;
	void RenderWithVertexArray() const;
	void PopulateChunkVertexArray( std::vector< Vertex3D_PCT >& out_vertexArray );
	bool ShouldFaceRender( BlockFace face, LocalBlockIndex thisBlockIndex );
	void AddBlockToVertexArray( const Block& block, LocalBlockIndex blockIndex, std::vector< Vertex3D_PCT >& out_vertexArray );

	void RenderWithDrawAABB() const;
	void RenderBlockWithDrawAABB( BlockType blockType, const WorldCoords& renderBoundsMins, const Vector3& blockSize = Vector3::ONE ) const;

	Rgba GetLightColorForLightLevel( int lightLevel ) const;
	void PopulateColumnWithOverworldBlocksWithPerlinNoise( int columnIndex, int groundHeight );
	void PopulateColumnWithNetherBlocksWithPerlinNoise( GlobalColumnCoords globalColumnCoords, int columnIndex, int groundHeight );
	int GetMaxColumnGroundHeightForArea( const GlobalBlockCoords& areaMins, const GlobalBlockCoords& areaMaxs ) const;
	void BuildVillage( GlobalBlockCoords villageWorldCenter );
	void BuildVillageRoads( GlobalBlockCoords villageWorldMins, GlobalBlockCoords villageWorldMaxs, GlobalBlockCoords villageWorldCenter );
	void BuildPortalShrine( GlobalBlockCoords shrineWorldMins, GlobalBlockCoords shrineWorldMaxs );
	void BuildTreasureShrine( GlobalBlockCoords shrineWorldMins, GlobalBlockCoords shrineWorldMaxs );
	void BuildClimbingTower( GlobalBlockCoords shrineWorldMins, GlobalBlockCoords shrineWorldMaxs );
	int GetGroundHeightWithPerlinNoiseForColumn( GlobalColumnCoords globalColumnCoords ) const;
	int GetCeilingHeightWithPerlinNoiseForColumn( GlobalColumnCoords globalColumnCoords ) const;
	GlobalColumnCoords LookForVillageCenterWithPerlinNoiseAroundColumn( GlobalColumnCoords globalColumnCoords );
	void BuildPond( GlobalBlockCoords pondWorldMins, GlobalBlockCoords pondWorldMaxs );

	void SetSelectedFace( const Vector3& directionOppositeFace );
	void SetBlockTypeIfLocal( GlobalBlockCoords blockGlobalMins, BlockType newType );

	Block m_blocks[ NUM_BLOCKS_PER_CHUNK ];
	unsigned int m_vboID;
	std::vector< Vertex3D_PCT > m_vertexes; //ONLY stored into when the debug flag to use vertex arrays is on.
	ChunkCoords m_chunkPosition; //e.g. (1,0) means 1 chunk forward (+x) from origin.
	bool m_isVertexArrayDirty; //Set upon dig/place.
	int m_currentSkyLightLevel;
	bool m_isVisible;
	unsigned int m_numVertexes;
	Dimension m_chunkDimension;
};