#include "Game/BlockInfo.hpp"


#include "Game/Chunk.hpp"


//--------------------------------------------------------------------------------------------------------------
BlockInfo::BlockInfo( Chunk* chunkOfBlock, LocalBlockIndex indexOfBlockInThatChunk )
	: m_myChunk( chunkOfBlock )
	, m_myBlockIndex( indexOfBlockInThatChunk )
{
}


//--------------------------------------------------------------------------------------------------------------
bool BlockInfo::StepSouth() //-y.
{
	bool onEastBoundary = false;
	LocalBlockCoords lbc = GetLocalBlockCoordsFromLocalBlockIndex( m_myBlockIndex );
	if ( lbc.y == 0 ) onEastBoundary = true; //0-based to 1-based.


	if ( onEastBoundary )
	{
		if ( m_myChunk->m_eastNeighbor == nullptr ) return false;
		m_myChunk = m_myChunk->m_eastNeighbor;

		m_myBlockIndex = GetLocalBlockIndexFromLocalBlockCoords( LocalBlockCoords( lbc.x, CHUNK_Y_WIDTH_IN_BLOCKS - 1, lbc.z ) );
	}
	else m_myBlockIndex -= CHUNK_Y_WIDTH_IN_BLOCKS;

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool BlockInfo::StepNorth() //+y.
{
	bool onWestBoundary = false;
	LocalBlockCoords lbc = GetLocalBlockCoordsFromLocalBlockIndex( m_myBlockIndex );
	if ( lbc.y == CHUNK_Y_WIDTH_IN_BLOCKS - 1 ) onWestBoundary = true; //0-based to 1-based.


	if ( onWestBoundary )
	{
		if ( m_myChunk->m_westNeighbor == nullptr ) return false;
		m_myChunk = m_myChunk->m_westNeighbor;

		m_myBlockIndex = GetLocalBlockIndexFromLocalBlockCoords( LocalBlockCoords( lbc.x, 0, lbc.z ) );
	}
	else m_myBlockIndex += CHUNK_Y_WIDTH_IN_BLOCKS;

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool BlockInfo::StepEast() //+x. Corresponds to back (NOT FRONT) faces, however, because we look down +x.
{
	bool onNorthBoundary = false;
	LocalBlockCoords lbc = GetLocalBlockCoordsFromLocalBlockIndex( m_myBlockIndex );
	if ( lbc.x == CHUNK_X_LENGTH_IN_BLOCKS - 1 ) onNorthBoundary = true; //0-based to 1-based.


	if ( onNorthBoundary )
	{
		if ( m_myChunk->m_northNeighbor == nullptr ) return false;
		m_myChunk = m_myChunk->m_northNeighbor;

		m_myBlockIndex = GetLocalBlockIndexFromLocalBlockCoords( LocalBlockCoords( 0, lbc.y, lbc.z ) );
	}
	else m_myBlockIndex++;

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool BlockInfo::StepWest() //-x. Corresponds to FRONT (NOT BACK) faces, however, because we look down +x.
{
	bool onSouthBoundary = false;
	LocalBlockCoords lbc = GetLocalBlockCoordsFromLocalBlockIndex( m_myBlockIndex );
	if ( lbc.x == 0 ) onSouthBoundary = true; //0-based to 1-based.


	if ( onSouthBoundary )
	{
		if ( m_myChunk->m_southNeighbor == nullptr ) return false;
		m_myChunk = m_myChunk->m_southNeighbor;

		m_myBlockIndex = GetLocalBlockIndexFromLocalBlockCoords( LocalBlockCoords( CHUNK_X_LENGTH_IN_BLOCKS - 1, lbc.y, lbc.z ) );
	}
	else m_myBlockIndex--;

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool BlockInfo::StepUp() //+z.
{
	bool onSkyBoundary = false;
	LocalBlockCoords lbc = GetLocalBlockCoordsFromLocalBlockIndex( m_myBlockIndex );
	if ( lbc.z == CHUNK_Z_HEIGHT_IN_BLOCKS - 1 ) onSkyBoundary = true; //0-based to 1-based.


	if ( onSkyBoundary ) return false;
	else m_myBlockIndex += NUM_COLUMNS_PER_CHUNK;

	return true;
}


//--------------------------------------------------------------------------------------------------------------
bool BlockInfo::StepDown() //-z.
{
	bool onGroundBoundary = false;
	LocalBlockCoords lbc = GetLocalBlockCoordsFromLocalBlockIndex( m_myBlockIndex );
	if ( lbc.z == 0 ) onGroundBoundary = true; //0-based to 1-based.


	if ( onGroundBoundary ) return false;
	else m_myBlockIndex -= NUM_COLUMNS_PER_CHUNK;

	return true;
}


//--------------------------------------------------------------------------------------------------------------
Block* BlockInfo::GetBlock() const
{
	return m_myChunk->GetBlockFromLocalBlockIndex( m_myBlockIndex );
}
