#pragma once


#include "Game/GameCommon.hpp"
#include "Game/Block.hpp"


//--------------------------------------------------------------------------------------------------------------
class Chunk;


//--------------------------------------------------------------------------------------------------------------
struct BlockInfo //WARNING: DO NOT HOLD ONTO, DATA IS LIKELY TO BECOME OBSOLETE.
{
	Chunk* m_myChunk;
	LocalBlockIndex m_myBlockIndex;

	BlockInfo() : m_myChunk( nullptr ), m_myBlockIndex( 0 ) {}
	BlockInfo( Chunk* chunkOfBlock, LocalBlockIndex indexOfBlockInThatChunk );
	bool StepSouth();
	bool StepNorth();
	bool StepEast();
	bool StepWest();
	bool StepUp();
	bool StepDown();
	Block* GetBlock() const;
	inline bool operator==( const BlockInfo& other ) const;
	inline bool operator!=( const BlockInfo& other ) const;
};


//--------------------------------------------------------------------------------------------------------------
inline bool BlockInfo::operator==( const BlockInfo& other ) const 
{
	return ( m_myBlockIndex == other.m_myBlockIndex ) && ( m_myChunk == other.m_myChunk );
}


//--------------------------------------------------------------------------------------------------------------
inline bool BlockInfo::operator!=( const BlockInfo& other ) const
{
	return !(*this == other);
}