#pragma once


#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/String/StringUtils.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------
class Block
{
public:

	Block( BlockType type = AIR ) 
		: m_type( type )
		, m_bitFlags( 0 ) 
	{
	}
	
	inline int GetLightLevel() const;
	inline void SetLightLevel( int clampedNewLightLevel );

	inline bool IsSky() const;
	inline void SetBlockToBeSky();
	inline void SetBlockToNotBeSky();

	inline bool IsLightingDirty() const;
	inline void SetLightingDirty();
	inline void SetLightingNotDirty();

	inline bool IsOpaque() const;
	inline void SetBlockToBeOpaque();
	inline void SetBlockToNotBeOpaque();

	inline BlockType GetBlockType() const { return m_type; }
	void SetBlockType( BlockType type );

private:
	inline void ClearLightLevelBits();
	inline void ClearSkyBit();
	inline void ClearDirtyLightingBit();
	inline void ClearOpaqueBit();

	unsigned char m_bitFlags; //See accessors.
	BlockType m_type;
};


//--------------------------------------------------------------------------------------------------------------
inline int Block::GetLightLevel() const 
{
	return m_bitFlags & BLOCKFLAGS_LIGHT_LEVEL_BITMASK;
}


//--------------------------------------------------------------------------------------------------------------
inline void Block::ClearLightLevelBits() 
{
	m_bitFlags &= ~BLOCKFLAGS_LIGHT_LEVEL_BITMASK;
}


//--------------------------------------------------------------------------------------------------------------
inline void Block::SetLightLevel( int clampedNewLightLevel )
{
	const std::string lightMaxExceededStr = Stringf( "SetLightLevel Given Argument Beyond Max Level %i", MAX_LIGHTING_LEVEL );
	ASSERT_OR_DIE( ( clampedNewLightLevel >= 0 ) && ( clampedNewLightLevel <= MAX_LIGHTING_LEVEL ), lightMaxExceededStr );

	ClearLightLevelBits();
	m_bitFlags |= clampedNewLightLevel;
}


//--------------------------------------------------------------------------------------------------------------
inline bool Block::IsSky() const 
{
	return ( m_bitFlags & BLOCKFLAGS_IS_SKY_BITMASK ) != 0;
}


//--------------------------------------------------------------------------------------------------------------
inline void Block::SetBlockToBeSky()
{
	m_bitFlags |= BLOCKFLAGS_IS_SKY_BITMASK;
}


//--------------------------------------------------------------------------------------------------------------
inline void Block::SetBlockToNotBeSky()
{
	ClearSkyBit();
}


//--------------------------------------------------------------------------------------------------------------
inline void Block::ClearSkyBit()
{
	m_bitFlags &= ~BLOCKFLAGS_IS_SKY_BITMASK;
}


//--------------------------------------------------------------------------------------------------------------
inline bool Block::IsLightingDirty() const 
{
	return ( m_bitFlags & BLOCKFLAGS_IS_LIGHTING_DIRTY_BITMASK ) != 0;
}


//--------------------------------------------------------------------------------------------------------------
inline void Block::SetLightingDirty( )
{
	m_bitFlags |= BLOCKFLAGS_IS_LIGHTING_DIRTY_BITMASK;
}


//--------------------------------------------------------------------------------------------------------------
inline void Block::SetLightingNotDirty( )
{
	ClearDirtyLightingBit();
}


//--------------------------------------------------------------------------------------------------------------
inline void Block::ClearDirtyLightingBit()
{
	m_bitFlags &= ~BLOCKFLAGS_IS_LIGHTING_DIRTY_BITMASK;
}


//--------------------------------------------------------------------------------------------------------------
inline bool Block::IsOpaque() const 
{
	return ( m_bitFlags & BLOCKFLAGS_IS_OPAQUE_BITMASK ) != 0;
}


//--------------------------------------------------------------------------------------------------------------
inline void Block::SetBlockToBeOpaque()
{
	m_bitFlags |= BLOCKFLAGS_IS_OPAQUE_BITMASK;
}


//--------------------------------------------------------------------------------------------------------------
inline void Block::SetBlockToNotBeOpaque()
{
	ClearOpaqueBit();
}


//--------------------------------------------------------------------------------------------------------------
inline void Block::ClearOpaqueBit()
{
	m_bitFlags &= ~BLOCKFLAGS_IS_OPAQUE_BITMASK;
}