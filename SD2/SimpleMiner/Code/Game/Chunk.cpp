#include "Game/Chunk.hpp"


#include "Engine/Math/AABB3.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Input/TheInput.hpp"
#include "Engine/Math/Noise.hpp"


#include "Game/BlockDefinition.hpp"
#include "Game/BlockInfo.hpp"


//--------------------------------------------------------------------------------------------------------------
Chunk::Chunk( ChunkCoords chunkPosition, Dimension chunkDimension )
	: m_isVisible( true )
	, m_isVertexArrayDirty( true )
	, m_blocks()
	, m_chunkPosition( chunkPosition )
	, m_currentSkyLightLevel( MAX_LIGHTING_LEVEL )
	, m_chunkDimension( chunkDimension )
{
	g_theRenderer->CreateVbo( m_vboID );
	WorldCoords chunkCenterInWorldUnits = GetChunkCenterInWorldUnits();
	m_chunkCornersInWorldUnits[ NORTHEAST_BOTTOM ] = chunkCenterInWorldUnits + WorldCoords( CHUNK_X_LENGTH_IN_BLOCKS*0.5f, CHUNK_Y_WIDTH_IN_BLOCKS*0.5f, 0.f );
	m_chunkCornersInWorldUnits[ NORTHWEST_BOTTOM ] = chunkCenterInWorldUnits + WorldCoords( -CHUNK_X_LENGTH_IN_BLOCKS*0.5f, CHUNK_Y_WIDTH_IN_BLOCKS*0.5f, 0.f );
	m_chunkCornersInWorldUnits[ SOUTHWEST_BOTTOM ] = chunkCenterInWorldUnits + WorldCoords( -CHUNK_X_LENGTH_IN_BLOCKS*0.5f, -CHUNK_Y_WIDTH_IN_BLOCKS*0.5f, 0.f );
	m_chunkCornersInWorldUnits[ SOUTHEAST_BOTTOM ] = chunkCenterInWorldUnits + WorldCoords( CHUNK_X_LENGTH_IN_BLOCKS*0.5f, -CHUNK_Y_WIDTH_IN_BLOCKS*0.5f, 0.f );
	m_chunkCornersInWorldUnits[ NORTHEAST_TOP ] = chunkCenterInWorldUnits + WorldCoords( CHUNK_X_LENGTH_IN_BLOCKS*0.5f, CHUNK_Y_WIDTH_IN_BLOCKS*0.5f, (float)CHUNK_Z_HEIGHT_IN_BLOCKS );
	m_chunkCornersInWorldUnits[ NORTHWEST_TOP ] = chunkCenterInWorldUnits + WorldCoords( -CHUNK_X_LENGTH_IN_BLOCKS*0.5f, CHUNK_Y_WIDTH_IN_BLOCKS*0.5f, (float)CHUNK_Z_HEIGHT_IN_BLOCKS );
	m_chunkCornersInWorldUnits[ SOUTHWEST_TOP ] = chunkCenterInWorldUnits + WorldCoords( -CHUNK_X_LENGTH_IN_BLOCKS*0.5f, -CHUNK_Y_WIDTH_IN_BLOCKS*0.5f, (float)CHUNK_Z_HEIGHT_IN_BLOCKS );
	m_chunkCornersInWorldUnits[ SOUTHEAST_TOP ] = chunkCenterInWorldUnits + WorldCoords( CHUNK_X_LENGTH_IN_BLOCKS*0.5f, -CHUNK_Y_WIDTH_IN_BLOCKS*0.5f, (float)CHUNK_Z_HEIGHT_IN_BLOCKS );

}


//--------------------------------------------------------------------------------------------------------------
Chunk::~Chunk()
{
	g_theRenderer->DestroyVbo( m_vboID );
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::PopulateChunkWithFlatStructure()
{
	for ( int blockIndex = 0; blockIndex < NUM_COLUMNS_PER_CHUNK; blockIndex++ )
	{
		if ( blockIndex < STONE_EXCLUSIVE_HEIGHT_LIMIT ) m_blocks[ blockIndex ].SetBlockType( BlockType::STONE );
		else if ( blockIndex < DIRT_EXCLUSIVE_HEIGHT_LIMIT ) m_blocks[ blockIndex ].SetBlockType( BlockType::DIRT );
		else if ( blockIndex < GRASS_EXCLUSIVE_HEIGHT_LIMIT ) m_blocks[ blockIndex ].SetBlockType( BlockType::GRASS );
		else if ( blockIndex < AIR_EXCLUSIVE_HEIGHT_LIMIT ) m_blocks[ blockIndex ].SetBlockType( BlockType::AIR );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::PopulateChunkWithPerlinNoise()
{
	for ( int columnIndex = 0; columnIndex < NUM_COLUMNS_PER_CHUNK; columnIndex++ )
	{
		GlobalColumnCoords globalColumnCoords = GetGlobalColumnCoordsFromChunkColumnIndex( columnIndex );
		int columnGroundHeight = GetGroundHeightWithPerlinNoiseForColumn( globalColumnCoords );

		//Populating with basic generation first ensures structures will be injected on top of the basic, not basic on top of the structures.
		if ( m_chunkDimension == DIM_OVERWORLD ) 
			PopulateColumnWithOverworldBlocksWithPerlinNoise( columnIndex, columnGroundHeight );
		else if ( m_chunkDimension == DIM_NETHER ) 
			PopulateColumnWithNetherBlocksWithPerlinNoise( globalColumnCoords, columnIndex, columnGroundHeight );
	}

	if ( g_generateVillages )
	{
		int centerColumnIndex = ( NUM_COLUMNS_PER_CHUNK >> 1 ); //Replacing div by 2.
		GlobalColumnCoords chunkCenter = GetGlobalColumnCoordsFromChunkColumnIndex( centerColumnIndex );
		GlobalColumnCoords villageCenterCoordsIfExists = LookForVillageCenterWithPerlinNoiseAroundColumn( chunkCenter );
		bool foundVillage = ( villageCenterCoordsIfExists != GlobalColumnCoords::ZERO );

		//Unless we never saw a village center in range of this chunk, make village building requests--ignored just before assignment if outside this chunk.
		if ( foundVillage )
		{
			int groundHeightAtVillageCenter = GetGroundHeightWithPerlinNoiseForColumn( villageCenterCoordsIfExists );
			GlobalBlockCoords villageWorldCenter = GlobalBlockCoords( (int)villageCenterCoordsIfExists.x, (int)villageCenterCoordsIfExists.y, groundHeightAtVillageCenter );
			BuildVillage( villageWorldCenter );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::PopulateColumnWithOverworldBlocksWithPerlinNoise( int columnIndex, int groundHeight )
{
	//Assign block types along the column.
	for ( int blockHeight = 0; blockHeight < CHUNK_Z_HEIGHT_IN_BLOCKS; blockHeight++ )
	{
		LocalColumnCoords lcc = GetLocalColumnCoordsFromChunkColumnIndex( columnIndex );
		LocalBlockCoords lbc = LocalBlockCoords( lcc.x, lcc.y, blockHeight );
		LocalBlockIndex lbi = GetLocalBlockIndexFromLocalBlockCoords( lbc );
		if ( blockHeight > groundHeight )
		{
			if ( blockHeight < SEA_LEVEL_HEIGHT_LIMIT ) m_blocks[ lbi ].SetBlockType( BlockType::WATER );
			else m_blocks[ lbi ].SetBlockType( BlockType::AIR );
		}
		else if ( blockHeight == groundHeight )
		{
			if ( blockHeight <= SEA_LEVEL_HEIGHT_LIMIT ) m_blocks[ lbi ].SetBlockType( BlockType::SAND );
			else m_blocks[ lbi ].SetBlockType( BlockType::GRASS );
		}
		else if ( blockHeight > groundHeight - NUM_DIRT_LAYERS )
		{
			if ( blockHeight <= SEA_LEVEL_HEIGHT_LIMIT ) m_blocks[ lbi ].SetBlockType( BlockType::SAND );
			m_blocks[ lbi ].SetBlockType( BlockType::DIRT );
		}
		else m_blocks[ lbi ].SetBlockType( BlockType::STONE );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::PopulateColumnWithNetherBlocksWithPerlinNoise( GlobalColumnCoords globalColumnCoords, int columnIndex, int groundHeight )
{
	//Compute the ceiling height for the current column (ground height isn't here because all dimensions need that code).
	int ceilingHeight = GetCeilingHeightWithPerlinNoiseForColumn( globalColumnCoords );


	//Assign block types along the column.
	for ( int blockHeight = 0; blockHeight < CHUNK_Z_HEIGHT_IN_BLOCKS; blockHeight++ )
	{
		LocalColumnCoords lcc = GetLocalColumnCoordsFromChunkColumnIndex( columnIndex );
		LocalBlockCoords lbc = LocalBlockCoords( lcc.x, lcc.y, blockHeight );
		LocalBlockIndex lbi = GetLocalBlockIndexFromLocalBlockCoords( lbc );

		//Run through a column assigning ceiling and ground alike. Bottom to top though, loop starts z=0.
		if ( blockHeight < groundHeight - NUM_DIRT_LAYERS )
		{
			//Nether's stone.
			m_blocks[ lbi ].SetBlockType( BlockType::BROWNSTONE );
		}
		else if ( blockHeight < groundHeight )
		{
			//Nether's dirt.
			m_blocks[ lbi ].SetBlockType( BlockType::NETHERRACK );
		}
		else if ( blockHeight == groundHeight ) //Nether's grass.
		{
			if ( blockHeight <= NETHER_LAVA_HEIGHT_LIMIT ) m_blocks[ lbi ].SetBlockType( BlockType::RED_SAND );
			else m_blocks[ lbi ].SetBlockType( BlockType::MYCELIUM );
		}
		else if ( blockHeight > groundHeight && blockHeight < ceilingHeight ) //Air.
		{
			if ( blockHeight < NETHER_LAVA_HEIGHT_LIMIT ) m_blocks[ lbi ].SetBlockType( BlockType::LAVA );
			else m_blocks[ lbi ].SetBlockType( BlockType::AIR );
		}
		else if ( blockHeight == ceilingHeight ) //Top.
		{
			if ( ( blockHeight & 1 ) == 0 ) 
				m_blocks[ lbi ].SetBlockType( BlockType::GLOWSTONE );
			else
				m_blocks[ lbi ].SetBlockType( BlockType::NETHERRACK );
		}
		else if ( blockHeight >= ceilingHeight && blockHeight < ceilingHeight + NUM_DIRT_LAYERS )
		{
			if ( blockHeight <= NETHER_LAVA_HEIGHT_LIMIT ) m_blocks[ lbi ].SetBlockType( BlockType::RED_SAND );
			m_blocks[ lbi ].SetBlockType( BlockType::NETHERRACK );
		}
		else
		{
			m_blocks[ lbi ].SetBlockType( BlockType::BROWNSTONE );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::BuildVillage( GlobalBlockCoords villageWorldCenter )
{
	//Use coord types for math operations, but use BlockInfo type for block-neighbor checking.
	GlobalBlockCoords villageWorldMins = villageWorldCenter - GlobalBlockCoords( VILLAGE_RADIUS_X_BLOCKS, VILLAGE_RADIUS_Y_BLOCKS, 0 );
	GlobalBlockCoords villageWorldMaxs = villageWorldCenter + GlobalBlockCoords( VILLAGE_RADIUS_X_BLOCKS, VILLAGE_RADIUS_Y_BLOCKS, 0 );

	//Call a series of build regions, later to be picked out from either more Perlin grids or the village Perlin grid in a repeatable way.
	//Note that "mins" is lower-left, "maxs" is upper-right of a region.
	GlobalBlockCoords regionSize = GlobalBlockCoords( VILLAGE_RADIUS_X_BLOCKS - VILLAGE_BLOCKS_FROM_CENTER_TO_CONSTRUCT_INCLUDING_CENTER, VILLAGE_RADIUS_Y_BLOCKS - VILLAGE_BLOCKS_FROM_CENTER_TO_CONSTRUCT_INCLUDING_CENTER, 0 );

	//Portal shrine in lower-left quadrant of village.
	GlobalBlockCoords portalShrineMins		= villageWorldMins;
	GlobalBlockCoords portalShrineMaxs		= portalShrineMins + regionSize;

	//Pond in upper-right quadrant of village.
	GlobalBlockCoords pondMaxs				= villageWorldMaxs;
	GlobalBlockCoords pondMins				= pondMaxs - regionSize;

	//Treasure shrine in lower-right quadrant of village.
	GlobalBlockCoords treasureShrineMins	= GlobalBlockCoords( villageWorldCenter.x + VILLAGE_BLOCKS_FROM_CENTER_TO_CONSTRUCT_INCLUDING_CENTER, villageWorldMins.y, villageWorldCenter.z ); //+1 else on center.
	GlobalBlockCoords treasureShrineMaxs	= treasureShrineMins + regionSize;

	BuildVillageRoads( villageWorldMins, villageWorldMaxs, villageWorldCenter );
	BuildPond( pondMins, pondMaxs );
	BuildPortalShrine( portalShrineMins, portalShrineMaxs ); //Later send in %'s of the village sizes.
	BuildTreasureShrine( treasureShrineMins, treasureShrineMaxs );

	//Mark the village center for now.
	SetBlockTypeIfLocal( villageWorldCenter, BlockType::GLOWSTONE );
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::BuildVillageRoads( GlobalBlockCoords villageWorldMins, GlobalBlockCoords villageWorldMaxs, GlobalBlockCoords villageWorldCenter )
{
	int roadRadius = VILLAGE_BLOCKS_FROM_CENTER_TO_CONSTRUCT_INCLUDING_CENTER;

	//Place pond blocks.
	for ( int y = villageWorldMins.y; y <= villageWorldMaxs.y; y++ )
	{
		for ( int x = villageWorldMins.x; x <= villageWorldMaxs.x; x++ )
		{
			GlobalColumnCoords currentColumn = GlobalColumnCoords( (float)x, (float)y );
			int currentColumnGroundHeight = GetGroundHeightWithPerlinNoiseForColumn( currentColumn );
			BlockType roadType = GRAVEL; //( m_chunkDimension == DIM_NETHER ? MYCELIUM : GRAVEL );

			bool inRangeOnX = ( x < villageWorldCenter.x + roadRadius && x > villageWorldCenter.x - roadRadius );
			bool inRangeOnY = ( y < villageWorldCenter.y + roadRadius && y > villageWorldCenter.y - roadRadius );
			if ( inRangeOnX || inRangeOnY )
			{
				SetBlockTypeIfLocal( GlobalBlockCoords( x, y, currentColumnGroundHeight ), roadType );
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::BuildPortalShrine( GlobalBlockCoords shrineWorldMins, GlobalBlockCoords shrineWorldMaxs )
{
	int bottomOfShrine = GetMaxColumnGroundHeightForArea( shrineWorldMins, shrineWorldMaxs );

	//Place shrine's floor blocks.
	for ( int y = shrineWorldMins.y; y <= shrineWorldMaxs.y; y++ )
	{
		for ( int x = shrineWorldMins.x; x <= shrineWorldMaxs.x; x++ )
		{
			if ( m_chunkDimension == DIM_NETHER )
			{
				SetBlockTypeIfLocal( GlobalBlockCoords( x, y, bottomOfShrine ), BlockType::MYCELIUM );
			}
			else
			{
				SetBlockTypeIfLocal( GlobalBlockCoords( x, y, bottomOfShrine ), BlockType::BROWNSTONE );
			}
		}
	}

	//Place shrine's pillar blocks.
	int topOfShrine = bottomOfShrine + VILLAGE_PORTAL_SHRINE_HEIGHT;
	for ( int y = shrineWorldMins.y; y <= shrineWorldMaxs.y; y++ )
	{
		for ( int x = shrineWorldMins.x; x <= shrineWorldMaxs.x; x++ )
		{
			for ( int z = 0; z <= topOfShrine; z++ )
			{
				bool shouldFormTowerBand = ( ( z & 7 ) == 0 ); //& 7 replaces mod by 8.

				if ( z < bottomOfShrine ) //Foundation.
				{
					SetBlockTypeIfLocal( GlobalBlockCoords( x, y, z ), BlockType::COBBLESTONE );
				}
				else if ( x == shrineWorldMins.x || x == shrineWorldMaxs.x )
				{
					if ( shouldFormTowerBand || y == shrineWorldMins.y || y == shrineWorldMaxs.y )
					{
						if ( m_chunkDimension == DIM_NETHER )
						{
							SetBlockTypeIfLocal( GlobalBlockCoords( x, y, z ), BlockType::NETHERRACK );
						}
						else
						{
							SetBlockTypeIfLocal( GlobalBlockCoords( x, y, z ), BlockType::COBBLESTONE );
						}
					}
					else //Place the blocks between the corner pillars.
					{
						if ( m_chunkDimension == DIM_NETHER )
						{
							SetBlockTypeIfLocal( GlobalBlockCoords( x, y, z ), BlockType::RED_SAND );
						}
						else
						{
							SetBlockTypeIfLocal( GlobalBlockCoords( x, y, z ), BlockType::BROWNSTONE );
						}
					}
				}
				else if ( y == shrineWorldMins.y || y == shrineWorldMaxs.y )
				{
					if ( shouldFormTowerBand || x == shrineWorldMins.x || x == shrineWorldMaxs.x )
					{
						if ( m_chunkDimension == DIM_NETHER )
						{
							SetBlockTypeIfLocal( GlobalBlockCoords( x, y, z ), BlockType::NETHERRACK );
						}
						else
						{
							SetBlockTypeIfLocal( GlobalBlockCoords( x, y, z ), BlockType::COBBLESTONE );
						}
					}
					else //Place the blocks between the corner pillars.
					{
						if ( m_chunkDimension == DIM_NETHER )
						{
							SetBlockTypeIfLocal( GlobalBlockCoords( x, y, z ), BlockType::RED_SAND );
						}
						else
						{
							SetBlockTypeIfLocal( GlobalBlockCoords( x, y, z ), BlockType::BROWNSTONE );
						}
					}
				}

			}
		}
	}

	//Place shrine's floating platform you have to access via climbing tower.
	int portalPlatformHeight = topOfShrine;
	for ( int y = shrineWorldMins.y; y <= shrineWorldMaxs.y; y++ )
	{
		//Skip outer ring.
		if ( y <= ( shrineWorldMins.y + 1 ) || y >= ( shrineWorldMaxs.y - 1 ) )
			continue;

		//Fill rows inside outer ring.
		for ( int x = shrineWorldMins.x; x <= shrineWorldMaxs.x; x++ )
		{
			//Skip outer ring.
			if ( x <= ( shrineWorldMins.x + 1 ) || x >= ( shrineWorldMaxs.x - 1 ) ) 
				continue;

			SetBlockTypeIfLocal( GlobalBlockCoords( x, y, portalPlatformHeight ), BlockType::GLOWSTONE );
		}
	}

	//Place the portal block(s) after finding shrine center.
	int shrineSizeOnX = abs( shrineWorldMaxs.x - shrineWorldMins.x );
	int shrineSizeOnY = abs( shrineWorldMaxs.y - shrineWorldMins.y );

	GlobalBlockCoords shrineWorldCenter = shrineWorldMins + GlobalBlockCoords( shrineSizeOnX >> 1, shrineSizeOnY >> 1, 0 ); //div by 2 replaced.
	GlobalBlockCoords portalWorldMins = GlobalBlockCoords( shrineWorldCenter.x, shrineWorldCenter.y, topOfShrine-1 );

	SetBlockTypeIfLocal( portalWorldMins, BlockType::PORTAL );
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::BuildTreasureShrine( GlobalBlockCoords shrineWorldMins, GlobalBlockCoords shrineWorldMaxs )
{
	int bottomOfShrine = GetMaxColumnGroundHeightForArea( shrineWorldMins, shrineWorldMaxs );
	int topOfShrine = bottomOfShrine + VILLAGE_TREASURE_SHRINE_HEIGHT;

	for ( int y = shrineWorldMins.y; y <= shrineWorldMaxs.y; y++ )
	{
		for ( int x = shrineWorldMins.x; x <= shrineWorldMaxs.x; x++ )
		{
			GlobalColumnCoords currentColumnInPond = GlobalColumnCoords( (float)x, (float)y );
			int currentColumnGroundHeight = GetGroundHeightWithPerlinNoiseForColumn( currentColumnInPond );
			GlobalBlockCoords currentBlockWorldMins = GlobalBlockCoords( x, y, currentColumnGroundHeight );

			SetBlockTypeIfLocal( GlobalBlockCoords( x, y, topOfShrine ), BlockType::GOLD_BRICK );
			SetBlockTypeIfLocal( GlobalBlockCoords( x, y, bottomOfShrine ), BlockType::GOLD_BRICK );

			for ( int z = 0; z < topOfShrine; z++ ) //Place walls.
			{
				if ( z < bottomOfShrine ) //Foundation.
				{
					SetBlockTypeIfLocal( GlobalBlockCoords( x, y, z ), BlockType::COBBLESTONE );
				}
				else if ( x == shrineWorldMins.x || x == shrineWorldMaxs.x )
				{
					if ( y == shrineWorldMins.y || y == shrineWorldMaxs.y )
					{
						SetBlockTypeIfLocal( GlobalBlockCoords( x, y, z ), BlockType::GOLD_BRICK );
					}
				}
				else if ( y == shrineWorldMins.y || y == shrineWorldMaxs.y )
				{
					if ( x == shrineWorldMins.x || x == shrineWorldMaxs.x )
					{
						SetBlockTypeIfLocal( GlobalBlockCoords( x, y, z ), BlockType::GOLD_BRICK );
					}
				}
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::BuildClimbingTower( GlobalBlockCoords shrineWorldMins, GlobalBlockCoords shrineWorldMaxs )
{
	for ( int y = shrineWorldMins.y; y <= shrineWorldMaxs.y; y++ )
	{
		for ( int x = shrineWorldMins.x; x <= shrineWorldMaxs.x; x++ )
		{
			GlobalColumnCoords currentColumnInPond = GlobalColumnCoords( (float)x, (float)y );
			int currentColumnGroundHeight = GetGroundHeightWithPerlinNoiseForColumn( currentColumnInPond );
			GlobalBlockCoords currentBlockWorldMins = GlobalBlockCoords( x, y, currentColumnGroundHeight );

			SetBlockTypeIfLocal( currentBlockWorldMins, BlockType::BROWNSTONE );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::BuildPond( GlobalBlockCoords pondWorldMins, GlobalBlockCoords pondWorldMaxs )
{
	int topOfPond = GetMaxColumnGroundHeightForArea( pondWorldMins, pondWorldMaxs ); //Pond goes down from ground level.
	int bottomOfPond = topOfPond - VILLAGE_POND_HEIGHT;

	//Place pond blocks.
	for ( int y = pondWorldMins.y; y <= pondWorldMaxs.y; y++ )
	{
		for ( int x = pondWorldMins.x; x <= pondWorldMaxs.x; x++ )
		{
			for ( int z = 0; z < CHUNK_Z_HEIGHT_IN_BLOCKS; z++ )
			{
				GlobalBlockCoords currentBlockWorldMins = GlobalBlockCoords( x, y, z );

				BlockType wallType = ( m_chunkDimension == DIM_NETHER ? NETHERRACK : COBBLESTONE );

				if ( z > topOfPond )
				{
					continue;
				}
				else if ( z < bottomOfPond )
				{
					SetBlockTypeIfLocal( currentBlockWorldMins, wallType );
				}
				else if ( x == pondWorldMins.x || x == pondWorldMaxs.x || y == pondWorldMins.y || y == pondWorldMaxs.y )
				{
					SetBlockTypeIfLocal( currentBlockWorldMins, wallType ); //Perimeter wall.
				}
				else
				{
					BlockType pondContent = ( m_chunkDimension == DIM_NETHER ? LAVA : WATER );
					SetBlockTypeIfLocal( currentBlockWorldMins, pondContent );
				}
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
int Chunk::GetMaxColumnGroundHeightForArea( const GlobalBlockCoords& areaMins, const GlobalBlockCoords& areaMaxs ) const
{
	//To get the bottom of a construct, we have to take the max ground height of all columns in range.
	int currentMaxColumnGroundHeight = -1;
	for ( int y = areaMins.y; y <= areaMaxs.y; y++ )
	{
		for ( int x = areaMins.x; x <= areaMaxs.x; x++ )
		{
			GlobalColumnCoords currentColumn = GlobalColumnCoords( (float)x, (float)y );
			int currentColumnGroundHeight = GetGroundHeightWithPerlinNoiseForColumn( currentColumn );

			if ( currentMaxColumnGroundHeight < currentColumnGroundHeight )
				currentMaxColumnGroundHeight = currentColumnGroundHeight;
		}
	}
	return currentMaxColumnGroundHeight;
}

//--------------------------------------------------------------------------------------------------------------
void Chunk::SetBlockTypeIfLocal( GlobalBlockCoords blockGlobalMins, BlockType newType )
{
	WorldCoordsXY blockWorldMinsXY = WorldCoordsXY( (float)blockGlobalMins.x, (float)blockGlobalMins.y );
	ChunkCoords coordsForChunkContainingWorldPosOfBlock = GetChunkCoordsFromWorldCoordsXY( blockWorldMinsXY );
	if ( coordsForChunkContainingWorldPosOfBlock != m_chunkPosition ) return; //Request was not for a block in this chunk.

	//Now we know the block is in this chunk, so we can transform the global block coords local to this chunk.
	WorldCoordsXY chunkWorldMins = GetChunkMinsInWorldUnits();
	LocalBlockCoords lbc = LocalBlockCoords( blockGlobalMins.x - (int)chunkWorldMins.x, blockGlobalMins.y - (int)chunkWorldMins.y, blockGlobalMins.z );
	LocalBlockIndex lbi = GetLocalBlockIndexFromLocalBlockCoords( lbc );
	m_blocks[ lbi ].SetBlockType( newType );
}


//--------------------------------------------------------------------------------------------------------------
int Chunk::GetGroundHeightWithPerlinNoiseForColumn( GlobalColumnCoords globalColumnCoords ) const
{
	float perlinGridCellSize = GROUND_HEIGHT_PERLIN_GRID_CELL_SIZE;
	int perlinNumOctaves = GROUND_HEIGHT_PERLIN_NUM_OCTAVES;
	float perlinPersistancePercentage = GROUND_HEIGHT_PERLIN_PERSISTANCE_PERCENTAGE;
	float perlinAmplitude = GROUND_HEIGHT_PERLIN_AMPLITUDE;
	int minimumGroundHeight = GROUND_HEIGHT_MINIMUM;

	//Changes for Nether.
	if ( m_chunkDimension == DIM_NETHER )
	{
		perlinGridCellSize *= 2.f;
		perlinNumOctaves += 2;
		perlinPersistancePercentage *= 2.f;
		perlinAmplitude *= 2.f;
		minimumGroundHeight >>= 1; //replaces div by 2.
	}

	float perlinValue = ComputePerlinNoiseValueAtPosition2D(
		globalColumnCoords,
		perlinGridCellSize,
		perlinNumOctaves,
		perlinPersistancePercentage
		);

	int groundHeight = static_cast<int>( perlinAmplitude * perlinValue ) + minimumGroundHeight;

	return groundHeight < 0 ? 0 : groundHeight; //In the case that the Perlin value is negative.
}


//--------------------------------------------------------------------------------------------------------------
int Chunk::GetCeilingHeightWithPerlinNoiseForColumn( GlobalColumnCoords globalColumnCoords ) const
{
	//Made separate if like ground height function above we want these tweaked for other dimensions with ceilings.
	float perlinGridCellSize = CEILING_HEIGHT_PERLIN_GRID_CELL_SIZE;
	int perlinNumOctaves = CEILING_HEIGHT_PERLIN_NUM_OCTAVES;
	float perlinPersistancePercentage = CEILING_HEIGHT_PERLIN_PERSISTANCE_PERCENTAGE;
	float perlinAmplitude = CEILING_HEIGHT_PERLIN_AMPLITUDE;
	int maximumCeilingHeight = CEILING_HEIGHT_OFFSET;

	float perlinValue = ComputePerlinNoiseValueAtPosition2D(
		globalColumnCoords,
		perlinGridCellSize,
		perlinNumOctaves,
		perlinPersistancePercentage
		);

	int ceilingHeight = (int)( perlinAmplitude * perlinValue ) + maximumCeilingHeight;


	return ceilingHeight >= CHUNK_Z_HEIGHT_IN_BLOCKS ? CHUNK_Z_HEIGHT_IN_BLOCKS - 1 : ceilingHeight; //Don't exceed sky either.
}


//--------------------------------------------------------------------------------------------------------------
GlobalColumnCoords Chunk::LookForVillageCenterWithPerlinNoiseAroundColumn( GlobalColumnCoords globalColumnCoords )
{
	//Squirrel suggestion: just check around me in 5x5 chunk area, because I'm checking so much less?

	//Loop over the possible range of a village from where a center could maximally be outside current column.
	float searchMinX = globalColumnCoords.x - ( VILLAGE_RADIUS_X_BLOCKS * VILLAGE_NUM_CHUNKS_TO_LOOK_FOR_VILLAGE );
	float searchMaxX = globalColumnCoords.x + ( VILLAGE_RADIUS_X_BLOCKS * VILLAGE_NUM_CHUNKS_TO_LOOK_FOR_VILLAGE );
	float searchMinY = globalColumnCoords.y - ( VILLAGE_RADIUS_Y_BLOCKS * VILLAGE_NUM_CHUNKS_TO_LOOK_FOR_VILLAGE );
	float searchMaxY = globalColumnCoords.y + ( VILLAGE_RADIUS_Y_BLOCKS * VILLAGE_NUM_CHUNKS_TO_LOOK_FOR_VILLAGE );

	//GOAL: revise this to only check against 5x5 of chunks.
	for ( float y = searchMinY; y <= searchMaxY; y++ )
	{
		for ( float x = searchMinX; x <= searchMaxX; x++ )
		{
			GlobalColumnCoords columnToSearch = GlobalColumnCoords( x, y );
			GlobalColumnCoords northNeighborToCurrentColumn = GlobalColumnCoords( x, y + 1 );
			GlobalColumnCoords southNeighborToCurrentColumn = GlobalColumnCoords( x, y - 1 );
			GlobalColumnCoords eastNeighborToCurrentColumn = GlobalColumnCoords( x + 1, y );
			GlobalColumnCoords westNeighborToCurrentColumn = GlobalColumnCoords( x - 1, y );
// 			GlobalColumnCoords northeastNeighborToCurrentColumn = GlobalColumnCoords( x + 1, y + 1 ); //TODO: Trying to half the calls by halving neighbors checked.
// 			GlobalColumnCoords southeastNeighborToCurrentColumn = GlobalColumnCoords( x + 1, y - 1 );
// 			GlobalColumnCoords northwestNeighborToCurrentColumn = GlobalColumnCoords( x - 1, y + 1 );
// 			GlobalColumnCoords southwestNeighborToCurrentColumn = GlobalColumnCoords( x - 1, y - 1 );

//			float currentVillagePerlinGridValue = 1.f; //Makes it a world of villages if uncommented!

			float currentVillagePerlinGridValue = ComputePerlinNoiseValueAtPosition2D( //TODO: USE [smoothed?] RAW NOISE FUNC
				columnToSearch,
				VILLAGE_PERLIN_GRID_CELL_SIZE,
				VILLAGE_PERLIN_GRID_NUM_OCTAVES,
				VILLAGE_PERLIN_GRID_PERSISTANCE
				);

				if ( currentVillagePerlinGridValue <= ComputePerlinNoiseValueAtPosition2D( northNeighborToCurrentColumn, VILLAGE_PERLIN_GRID_CELL_SIZE, VILLAGE_PERLIN_GRID_NUM_OCTAVES, VILLAGE_PERLIN_GRID_PERSISTANCE ) )
				continue;
			
			if ( currentVillagePerlinGridValue <= ComputePerlinNoiseValueAtPosition2D( southNeighborToCurrentColumn, VILLAGE_PERLIN_GRID_CELL_SIZE, VILLAGE_PERLIN_GRID_NUM_OCTAVES, VILLAGE_PERLIN_GRID_PERSISTANCE ) )
				continue;
			
			if ( currentVillagePerlinGridValue <= ComputePerlinNoiseValueAtPosition2D( eastNeighborToCurrentColumn, VILLAGE_PERLIN_GRID_CELL_SIZE, VILLAGE_PERLIN_GRID_NUM_OCTAVES, VILLAGE_PERLIN_GRID_PERSISTANCE ) )
				continue;
			
			if ( currentVillagePerlinGridValue <= ComputePerlinNoiseValueAtPosition2D( westNeighborToCurrentColumn, VILLAGE_PERLIN_GRID_CELL_SIZE, VILLAGE_PERLIN_GRID_NUM_OCTAVES, VILLAGE_PERLIN_GRID_PERSISTANCE ) )
				continue;

			return columnToSearch; //At this point we know columnToSearch is a local maxima of the Perlin grid, i.e. a village center.
		}
	}

	return GlobalColumnCoords::ZERO;
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::PopulateChunkWithRleString( const std::vector< unsigned char >& rleString )
{
	BlockType currentType;
	int numOfType;
	int currentBlockIndex = 0;
	for ( int byteIndex = 0; byteIndex < (int)rleString.size(); byteIndex += 2 )
	{
		currentType = (BlockType)rleString[ byteIndex ];
		numOfType = (BlockType)rleString[ byteIndex + 1 ];

		for ( int blockIndex = 0; blockIndex < numOfType; blockIndex++ )
		{
			Block& currentBlock = m_blocks[ currentBlockIndex + blockIndex ];
			currentBlock.SetBlockType( currentType );

			if ( BlockDefinition::IsOpaque( currentType ) ) currentBlock.SetBlockToBeOpaque( );
			else currentBlock.SetBlockToNotBeOpaque();

		}
		currentBlockIndex += numOfType;
	}
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::RebuildVertexArray()
{
	std::vector< Vertex3D_PCT > vertexes;
	vertexes.reserve( 10000 );
	PopulateChunkVertexArray( vertexes );
	m_numVertexes = vertexes.size();
	if ( g_renderChunksWithVertexArrays ) m_vertexes = vertexes;
	/*else*/ g_theRenderer->UpdateVbo( m_vboID, vertexes.data(), m_numVertexes * sizeof( Vertex3D_PCT ) );
	m_isVertexArrayDirty = false;
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::Render() const
{
	if ( !m_isVisible ) return;
	if ( g_renderChunksWithVertexArrays ) RenderWithVertexArray();
	else RenderWithVbo();
}


//--------------------------------------------------------------------------------------------------------------
WorldCoords Chunk::GetChunkCenterInWorldUnits() const
{
	WorldCoords cciw;

	cciw.x = static_cast<float>( m_chunkPosition.x * CHUNK_X_LENGTH_IN_BLOCKS );
	cciw.y = static_cast<float>( m_chunkPosition.y * CHUNK_Y_WIDTH_IN_BLOCKS );
	cciw.z = 0.f;

	//Offset to center. Always +='d because chunk grid is mins-based.
	cciw.x += CHUNK_X_LENGTH_IN_BLOCKS * .5f;
	cciw.y += CHUNK_Y_WIDTH_IN_BLOCKS * .5f;

	return cciw;
}


//--------------------------------------------------------------------------------------------------------------
WorldCoordsXY Chunk::GetChunkMinsInWorldUnits() const
{
	WorldCoordsXY cciw;

	cciw.x = (float)( m_chunkPosition.x * CHUNK_X_LENGTH_IN_BLOCKS );
	cciw.y = (float)( m_chunkPosition.y * CHUNK_Y_WIDTH_IN_BLOCKS );

	return cciw;
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::PopulateChunkVertexArray( std::vector< Vertex3D_PCT >& out_vertexArray )
{
	out_vertexArray.clear();
	out_vertexArray.reserve( 10000 );

	for ( LocalBlockIndex blockIndex = 0; blockIndex < NUM_BLOCKS_PER_CHUNK; blockIndex++ )
	{
		Block& block = m_blocks[ blockIndex ];
		if ( block.GetBlockType() != AIR ) //not visible.
		{
			AddBlockToVertexArray( block, blockIndex, out_vertexArray );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
bool Chunk::ShouldFaceRender(BlockFace face, LocalBlockIndex thisBlockIndex )
{
	BlockInfo thisBlock = BlockInfo( this, thisBlockIndex );
	BlockInfo neighborBlock = thisBlock;
	bool successInStepping = false;

	//Detect if neighbor exists via BlockInfo stepping, if not, need to render.
	switch ( face )
	{
		case BOTTOM: successInStepping = neighborBlock.StepDown(); break; //-z, down.
		case TOP: successInStepping = neighborBlock.StepUp(); break; //+z, up.
		case LEFT: successInStepping = neighborBlock.StepNorth(); break; //+y, left.
		case RIGHT: successInStepping = neighborBlock.StepSouth(); break; //-y, right.
		case FRONT: successInStepping = neighborBlock.StepWest(); break; //-x, south because looking down +x.
		case BACK: successInStepping = neighborBlock.StepEast(); break; //+x, north.
	}
	if ( !successInStepping ) return true; //Typically should mean on neighborless chunk edge.

	//If this is below the next check it will not render water right, as water on water will be told it should not render when it should.
	BlockType myBlockType = m_blocks[ thisBlockIndex ].GetBlockType();
	BlockType neighborBlockType = neighborBlock.m_myChunk->GetBlockFromLocalBlockIndex( neighborBlock.m_myBlockIndex )->GetBlockType();
	if ( BlockDefinition::IsOpaque( neighborBlockType ) == false ) 
		return true; //If neighbor is NOT opaque, e.g. air, need to render.

	//If neighbor's type is the same, do not render (return false).
	if ( myBlockType == neighborBlockType ) 
		return false;
	else 
		return true;
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::AddBlockToVertexArray( const Block& block, LocalBlockIndex blockIndex, std::vector< Vertex3D_PCT >& out_vertexArray )
{
	BlockType thisBlockType = block.GetBlockType( );
	Vertex3D_PCT tempVertex;
	Rgba lightModulation = Rgba::WHITE;
	tempVertex.m_color = lightModulation;
	const Vector3& blockSize = Vector3::ONE;
	AABB2 tempTexCoords;

	WorldCoords renderBoundsMins = GetWorldCoordsFromLocalBlockIndex( blockIndex );
	AABB3 bounds = AABB3( renderBoundsMins, renderBoundsMins + blockSize );
	BlockInfo thisBlock = BlockInfo( this, blockIndex );
	BlockInfo currentNeighbor;

	//Handle special rendering cases.
	float heightScaling = 0.0f; //No scale by default.
	float LADDER_OFFSET = .10f; //90% of way to block maxs.
		//TODO: add this selectively based on the orientation of the ladder as set at time of PlaceBlock call.
		//Clarification: right now all ladders will position themselves with facing with the same orientation.
	if ( thisBlockType == LADDER )
	{
		tempTexCoords = ( g_useLightTestingTexture ? g_textureAtlas->GetTexCoordsFromSpriteCoords( 8, 0 ) : BlockDefinition::s_blockDefinitionRegistry[ block.GetBlockType() ].m_texCoordsSides );
		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.maxs.x - LADDER_OFFSET, bounds.maxs.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.maxs.x - LADDER_OFFSET, bounds.mins.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.maxs.x - LADDER_OFFSET, bounds.mins.y, bounds.maxs.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.maxs.x - LADDER_OFFSET, bounds.maxs.y, bounds.maxs.z );
		out_vertexArray.push_back( tempVertex );

		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.maxs.x - LADDER_OFFSET, bounds.maxs.y, bounds.maxs.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.maxs.x - LADDER_OFFSET, bounds.mins.y, bounds.maxs.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.maxs.x - LADDER_OFFSET, bounds.mins.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.maxs.x - LADDER_OFFSET, bounds.maxs.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		return; //Render the sides texture only where orienting bits direct.
	}
	else if ( thisBlockType == STAIRS )
	{
		heightScaling = .5f;
	}

	//HSR: Hidden Surface Removal -- neither of two adjacent blocks' adjacent faces will be seen (they are hidden), if they are the same block type.
	if ( ShouldFaceRender( BOTTOM, blockIndex ) )
	{
		currentNeighbor = thisBlock;
		currentNeighbor.StepDown( );
		lightModulation = GetLightColorForLightLevel( currentNeighbor.GetBlock()->GetLightLevel() );

		tempVertex.m_color = lightModulation;
		tempTexCoords = ( g_useLightTestingTexture ? g_textureAtlas->GetTexCoordsFromSpriteCoords( 8, 0 ) : BlockDefinition::s_blockDefinitionRegistry[ block.GetBlockType( ) ].m_texCoordsBottom );

		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.maxs.x, bounds.maxs.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.maxs.x, bounds.mins.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.mins.x, bounds.mins.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.mins.x, bounds.maxs.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
	}

	if ( ShouldFaceRender( TOP, blockIndex ) )
	{
		currentNeighbor = thisBlock;
		currentNeighbor.StepUp();
		lightModulation = GetLightColorForLightLevel( currentNeighbor.GetBlock()->GetLightLevel() );

		tempVertex.m_color = lightModulation;
		tempTexCoords = ( g_useLightTestingTexture ? g_textureAtlas->GetTexCoordsFromSpriteCoords( 8, 0 ) : BlockDefinition::s_blockDefinitionRegistry[ block.GetBlockType( ) ].m_texCoordsTop );

		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.mins.x, bounds.maxs.y, bounds.maxs.z - heightScaling );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.mins.x, bounds.mins.y, bounds.maxs.z - heightScaling );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.maxs.x, bounds.mins.y, bounds.maxs.z - heightScaling );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z - heightScaling );
		out_vertexArray.push_back( tempVertex );
	}

	//Sides.
	tempTexCoords = ( g_useLightTestingTexture ? g_textureAtlas->GetTexCoordsFromSpriteCoords( 8, 0 ) : BlockDefinition::s_blockDefinitionRegistry[ block.GetBlockType( ) ].m_texCoordsSides );

	if ( ShouldFaceRender( LEFT, blockIndex ) )
	{
		currentNeighbor = thisBlock;
		currentNeighbor.StepNorth(); //+y.
		lightModulation = GetLightColorForLightLevel( currentNeighbor.GetBlock()->GetLightLevel() );

		tempVertex.m_color = lightModulation;
		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.maxs.x, bounds.maxs.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.mins.x, bounds.maxs.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.mins.x, bounds.maxs.y, bounds.maxs.z - heightScaling );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z - heightScaling );
		out_vertexArray.push_back( tempVertex );
	}

	if ( ShouldFaceRender( RIGHT, blockIndex ) )
	{
		currentNeighbor = thisBlock;
		currentNeighbor.StepSouth(); //-y.
		lightModulation = GetLightColorForLightLevel( currentNeighbor.GetBlock()->GetLightLevel() );

		tempVertex.m_color = lightModulation;
		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.mins.x, bounds.mins.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.maxs.x, bounds.mins.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.maxs.x, bounds.mins.y, bounds.maxs.z - heightScaling );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.mins.x, bounds.mins.y, bounds.maxs.z - heightScaling );
		out_vertexArray.push_back( tempVertex );
	}

	if ( ShouldFaceRender( FRONT, blockIndex ) )
	{
		currentNeighbor = thisBlock;
		currentNeighbor.StepWest(); //-x, since cam looks down this axis.
		lightModulation = GetLightColorForLightLevel( currentNeighbor.GetBlock()->GetLightLevel() );

		tempVertex.m_color = lightModulation;
		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.mins.x, bounds.maxs.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.mins.x, bounds.mins.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.mins.x, bounds.mins.y, bounds.maxs.z - heightScaling );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.mins.x, bounds.maxs.y, bounds.maxs.z - heightScaling );
		out_vertexArray.push_back( tempVertex );
	}
	
	if ( ShouldFaceRender( BACK, blockIndex ) )
	{
		currentNeighbor = thisBlock;
		currentNeighbor.StepEast(); //+x.
		lightModulation = GetLightColorForLightLevel( currentNeighbor.GetBlock()->GetLightLevel() );

		tempVertex.m_color = lightModulation;
		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.maxs.x, bounds.mins.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.maxs.y );
		tempVertex.m_position = Vector3( bounds.maxs.x, bounds.maxs.y, bounds.mins.z );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.maxs.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.maxs.x, bounds.maxs.y, bounds.maxs.z - heightScaling );
		out_vertexArray.push_back( tempVertex );
		tempVertex.m_texCoords = Vector2( tempTexCoords.mins.x, tempTexCoords.mins.y );
		tempVertex.m_position = Vector3( bounds.maxs.x, bounds.mins.y, bounds.maxs.z - heightScaling );
		out_vertexArray.push_back( tempVertex );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::RenderWithVbo() const
{
	g_theRenderer->BindTexture( g_textureAtlas->GetAtlasTexture() );
	g_theRenderer->DrawVbo_PCT( m_vboID, m_numVertexes, TheRenderer::AS_QUADS );
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::RenderWithVertexArray() const
{
	g_theRenderer->BindTexture( g_textureAtlas->GetAtlasTexture() );
	g_theRenderer->DrawVertexArray_PCT( TheRenderer::VertexGroupingRule::AS_QUADS, m_vertexes, m_vertexes.size() );
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::RenderWithDrawAABB() const //Mostly to preserve the visualization of the workflow it yields for cube render, from A1-A4. Originally used glBegin.
{
	for ( int blockIndex = 0; blockIndex < NUM_BLOCKS_PER_CHUNK; blockIndex++ )
		RenderBlockWithDrawAABB( m_blocks[ blockIndex ].GetBlockType(), GetWorldCoordsFromLocalBlockIndex( blockIndex ) );
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::RenderBlockWithDrawAABB( BlockType blockType, const WorldCoords& renderBoundsMins, const Vector3& blockSize /*= Vector3( 1.f, 1.f, 1.f )*/ ) const
{
	AABB3 renderBounds = AABB3( renderBoundsMins, renderBoundsMins + blockSize );

	g_theRenderer->EnableDepthTesting( true );
	g_theRenderer->EnableBackfaceCulling( true );

	AABB2 texCoords[ 6 ];
	switch ( blockType )
	{
	case BlockType::AIR:
		texCoords[ 0 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 0 ); //top mins
		texCoords[ 1 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 0 );
		texCoords[ 2 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 0 );
		texCoords[ 3 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 0 ); //collapse 4 sides to 1 of 3 lines -- side mins
		texCoords[ 4 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 0 );
		texCoords[ 5 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 0 ); //bottom mins
		break;
	case BlockType::DIRT:
		texCoords[ 0 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 8 );
		texCoords[ 1 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 8 );
		texCoords[ 2 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 8 );
		texCoords[ 3 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 8 );
		texCoords[ 4 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 8 );
		texCoords[ 5 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 8 );
		break;
	case BlockType::GRASS:
		texCoords[ 0 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 8 );
		texCoords[ 1 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 9, 8 );
		texCoords[ 2 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 8, 8 );
		texCoords[ 3 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 8, 8 );
		texCoords[ 4 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 8, 8 );
		texCoords[ 5 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 8, 8 );
		break;
	case BlockType::STONE:
		texCoords[ 0 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 10 );
		texCoords[ 1 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 10 );
		texCoords[ 2 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 10 );
		texCoords[ 3 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 10 );
		texCoords[ 4 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 10 );
		texCoords[ 5 ] = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 10 );
		break;
	}

	g_theRenderer->DrawAABB( TheRenderer::VertexGroupingRule::AS_QUADS, renderBounds, *g_textureAtlas->GetAtlasTexture( ), texCoords );

	g_theRenderer->EnableDepthTesting( false );
	g_theRenderer->EnableBackfaceCulling( false );
}


//--------------------------------------------------------------------------------------------------------------
GlobalColumnCoords Chunk::GetGlobalColumnCoordsFromChunkColumnIndex( ChunkColumnIndex cci ) const
{
	LocalColumnCoords columnPositionInsideChunk = GetLocalColumnCoordsFromChunkColumnIndex( cci );
	GlobalColumnCoords columnPositionInsideWorld;

	columnPositionInsideWorld.x = (float)( columnPositionInsideChunk.x + ( m_chunkPosition.x * CHUNK_X_LENGTH_IN_BLOCKS ) );
	columnPositionInsideWorld.y = (float)( columnPositionInsideChunk.y + ( m_chunkPosition.y * CHUNK_Y_WIDTH_IN_BLOCKS ) );

	return columnPositionInsideWorld;
}


//--------------------------------------------------------------------------------------------------------------
LocalColumnCoords Chunk::GetLocalColumnCoordsFromChunkColumnIndex( ChunkColumnIndex cci ) const
{
	LocalColumnCoords lcc;

	lcc.x = cci & ( CHUNK_X_LENGTH_IN_BLOCKS - 1 ); //replaces % CHUNK_X_LENGTH_IN_BLOCKS;
	lcc.y = ( cci >> CHUNK_BITS_X ) & ( CHUNK_Y_WIDTH_IN_BLOCKS - 1 ); // replaces ( cci / CHUNK_X_LENGTH_IN_BLOCKS ) % CHUNK_Y_WIDTH_IN_BLOCKS;

	return lcc;
}


//--------------------------------------------------------------------------------------------------------------
WorldCoords Chunk::GetWorldCoordsFromLocalBlockIndex( LocalBlockIndex lbi ) const
{
	LocalBlockCoords blockPositionInsideChunk = GetLocalBlockCoordsFromLocalBlockIndex( lbi );
	WorldCoords blockPositionInsideWorld;

	//No need to scale z-component.
	blockPositionInsideWorld.x = (float)( blockPositionInsideChunk.x + ( m_chunkPosition.x * CHUNK_X_LENGTH_IN_BLOCKS ) );
	blockPositionInsideWorld.y = (float)( blockPositionInsideChunk.y + ( m_chunkPosition.y * CHUNK_Y_WIDTH_IN_BLOCKS ) );
	blockPositionInsideWorld.z = (float)( blockPositionInsideChunk.z );

	return blockPositionInsideWorld;
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::SetSelectedFace( const Vector3& directionOppositeFace )
{
	if ( directionOppositeFace == WORLD_UP ) m_selectedFace = BOTTOM;
	else if ( directionOppositeFace == WORLD_DOWN ) m_selectedFace = TOP;
	else if ( directionOppositeFace == WORLD_LEFT ) m_selectedFace = RIGHT;
	else if ( directionOppositeFace == WORLD_RIGHT ) m_selectedFace = LEFT;
	else if ( directionOppositeFace == WORLD_FORWARD ) m_selectedFace = BACK;
	else if ( directionOppositeFace == WORLD_BACKWARD ) m_selectedFace = FRONT;
}


//--------------------------------------------------------------------------------------------------------------
Rgba Chunk::GetLightColorForLightLevel( int lightLevel ) const
{
	Rgba lightColor;

	if ( g_colorizeLightLevels )
	{
		switch ( lightLevel )
		{
		case 0: return Rgba( .3f, 0.f, 0.f );
		case 1: return Rgba( 0.f, 1.f, 0.f );
		case 2: return Rgba( 0.f, 0.f, 1.f );
		case 3: return Rgba( 1.f, 1.f, 0.f );
		case 4: return Rgba( 0.f, 1.f, 1.f );
		case 5: return Rgba( 1.f, 0.f, 1.f );
		case 6: return Rgba( .75f, 0.f, 0.f );
		case 7: return Rgba( 0.f, .75f, 0.f );
		case 8: return Rgba( 0.f, 0.f, .75f );
		case 9: return Rgba( .75f, .75f, 0.f );
		case 10: return Rgba( 0.f, .75f, .75f );
		case 11: return Rgba( .75f, 0.f, .75f );
		case 12: return Rgba( .75f, .75f, .75f );
		case 13: return Rgba( 0.f, .33f, 0.f );
		case 14: return Rgba( 0.f, 0.f, .33f );
		case 15: return Rgba::WHITE;
		}
	}

	float lightColorValue = (  (float)lightLevel / (float)MAX_LIGHTING_LEVEL ); //Not a speed-critical div as func is for debug.
	lightColorValue = ClampFloat( lightColorValue, LOWEST_LIGHT_RGBA_VALUE, 1.f );
	return Rgba( lightColorValue, lightColorValue, lightColorValue );
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::SetCurrentSkyLightLevel( int clampedNewLightLevel )
{
	ASSERT_OR_DIE( clampedNewLightLevel >= 0 && clampedNewLightLevel <= MAX_LIGHTING_LEVEL, "SetLightLevel Given Argument Beyond Max Level" );
	m_currentSkyLightLevel = clampedNewLightLevel;
}


//--------------------------------------------------------------------------------------------------------------
bool Chunk::IsBlockSolid( LocalBlockIndex lbi ) const
{
	return BlockDefinition::IsSolid( m_blocks[ lbi ].GetBlockType() );
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::BreakBlock( LocalBlockIndex lbi )
{
	m_blocks[ lbi ].SetBlockType( BlockType::AIR );
	m_blocks[ lbi ].SetBlockToNotBeOpaque();
	m_isVertexArrayDirty = true;
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::PlaceBlock( LocalBlockIndex lbi, const Vector3& directionOppositeFace, BlockType typeToPlace, BlockInfo* out_blockPlaced /*= nullptr*/ )
{
	if ( directionOppositeFace == Vector3::ZERO ) //Just place it, means another chunk sent a preconfigured lbi.
	{
		//Note also need to alter the below entry to active HUD element!
		m_blocks[ lbi ].SetBlockType( typeToPlace );
		if ( out_blockPlaced != nullptr )
		{
			out_blockPlaced->m_myChunk = this;
			out_blockPlaced->m_myBlockIndex = lbi;
		}

		if ( BlockDefinition::IsOpaque( m_blocks[ lbi ].GetBlockType() ) ) m_blocks[ lbi ].SetBlockToBeOpaque();
		else m_blocks[ lbi ].SetBlockToNotBeOpaque();

		m_isVertexArrayDirty = true;
		return;
	}

	SetSelectedFace( directionOppositeFace );
	LocalBlockCoords hitBlockLbc = GetLocalBlockCoordsFromLocalBlockIndex( lbi );
	LocalBlockCoords newBlockLbc = hitBlockLbc;
	switch ( m_selectedFace )
	{
		case BlockFace::BOTTOM:
		{
			newBlockLbc.z--;
			if ( newBlockLbc.z < 0 ) return; //Can't place blocks below chunks.
			break;
		}
		case BlockFace::TOP:
		{
			newBlockLbc.z++;
			if ( newBlockLbc.z > CHUNK_Z_HEIGHT_IN_BLOCKS - 1 ) return; //Can't place blocks above chunks.
			break;
		}
		case BlockFace::LEFT:
		{
			newBlockLbc.y++;
			if ( newBlockLbc.y > CHUNK_Y_WIDTH_IN_BLOCKS - 1 )
			{
				if ( this->m_westNeighbor == nullptr ) return; //No neighbor to place into loaded.

				newBlockLbc.y = 0;
				LocalBlockIndex lbiInNeighborChunk = GetLocalBlockIndexFromLocalBlockCoords( newBlockLbc );
				this->m_westNeighbor->PlaceBlock( lbiInNeighborChunk, Vector3::ZERO, typeToPlace, out_blockPlaced );
				return;
			}
			break;
		}
		case BlockFace::RIGHT:
		{
			newBlockLbc.y--;
			if ( newBlockLbc.y < 0 )
			{
				if ( this->m_eastNeighbor == nullptr ) return; //No neighbor to place into loaded.

				newBlockLbc.y = CHUNK_Y_WIDTH_IN_BLOCKS - 1;
				LocalBlockIndex lbiInNeighborChunk = GetLocalBlockIndexFromLocalBlockCoords( newBlockLbc );
				this->m_eastNeighbor->PlaceBlock( lbiInNeighborChunk, Vector3::ZERO, typeToPlace, out_blockPlaced );
				return;
			}
			break;
		}
		case BlockFace::FRONT: //Reversed because looking down +x, so front faces -x.
		{
			newBlockLbc.x--;
			if ( newBlockLbc.x < 0 )
			{
				if ( this->m_southNeighbor == nullptr ) return; //No neighbor to place into loaded.

				newBlockLbc.x = CHUNK_X_LENGTH_IN_BLOCKS - 1;
				LocalBlockIndex lbiInNeighborChunk = GetLocalBlockIndexFromLocalBlockCoords( newBlockLbc );
				this->m_southNeighbor->PlaceBlock( lbiInNeighborChunk, Vector3::ZERO, typeToPlace, out_blockPlaced );
				return;
			}
			break;
		}
		case BlockFace::BACK:
		{
			newBlockLbc.x++;
			if ( newBlockLbc.x > CHUNK_X_LENGTH_IN_BLOCKS - 1 )
			{
				if ( this->m_northNeighbor == nullptr ) return; //No neighbor to place into loaded.

				newBlockLbc.x = 0;
				LocalBlockIndex lbiInNeighborChunk = GetLocalBlockIndexFromLocalBlockCoords( newBlockLbc );
				this->m_northNeighbor->PlaceBlock( lbiInNeighborChunk, Vector3::ZERO, typeToPlace, out_blockPlaced );
				return;
			}
			break;
		}
	}
	LocalBlockIndex newBlockLbi = GetLocalBlockIndexFromLocalBlockCoords( newBlockLbc );
	m_blocks[ newBlockLbi ].SetBlockType( typeToPlace );
	if ( out_blockPlaced != nullptr )
	{
		out_blockPlaced->m_myChunk = this;
		out_blockPlaced->m_myBlockIndex = newBlockLbi;
	}

	m_isVertexArrayDirty = true;
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::HighlightBlock( LocalBlockIndex lbi, Vector3 directionOppositeFace )
{
	m_selectedBlock = lbi;
	SetSelectedFace( directionOppositeFace );
}


//--------------------------------------------------------------------------------------------------------------
Block* Chunk::GetBlockFromLocalBlockIndex( LocalBlockIndex m_myBlockIndex )
{
	if ( m_myBlockIndex < 0 || m_myBlockIndex > NUM_BLOCKS_PER_CHUNK ) return nullptr;
	return &m_blocks[ m_myBlockIndex ];
}


//--------------------------------------------------------------------------------------------------------------
Block* Chunk::GetBlockFromLocalBlockCoords( const LocalBlockCoords& lbc )
{
	if ( lbc.x < 0 || lbc.y < 0 || lbc.y < 0 || lbc.x > CHUNK_X_LENGTH_IN_BLOCKS 
		 || lbc.y > CHUNK_Y_WIDTH_IN_BLOCKS || lbc.z > CHUNK_Z_HEIGHT_IN_BLOCKS  ) return nullptr;
	return &m_blocks[ GetLocalBlockIndexFromLocalBlockCoords( lbc ) ];
}


//--------------------------------------------------------------------------------------------------------------
void Chunk::GetRleString( std::vector< unsigned char >& out_rleBuffer )
{
	BlockType currentType = m_blocks[ 0 ].GetBlockType();
	int numOfTypeSeen = 1;
	for ( int blockIndex = 1; blockIndex < NUM_BLOCKS_PER_CHUNK; blockIndex++ )
	{
		if ( currentType == m_blocks[ blockIndex ].GetBlockType() && numOfTypeSeen < 255) numOfTypeSeen++; //0-255 a pair.
		else
		{
			out_rleBuffer.push_back( currentType );
			out_rleBuffer.push_back( (unsigned char)numOfTypeSeen );

			currentType = m_blocks[ blockIndex ].GetBlockType();
			numOfTypeSeen = 1;
		}
	}
}
