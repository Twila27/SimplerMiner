#pragma once


#include <map>
#include "Game/GameCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Audio/TheAudio.hpp"


struct BlockDefinition
{
	static std::map< BlockType, BlockDefinition > s_blockDefinitionRegistry;

	AABB2 m_texCoordsTop;
	AABB2 m_texCoordsSides;
	AABB2 m_texCoordsBottom;
	int m_emittedLightLevel;
	float m_toughness;
	bool m_isSolid;
	bool m_isOpaque;

	std::vector< SoundID > m_walkingSounds;
	std::vector< SoundID > m_breakingSounds;
	std::vector< SoundID > m_placingSounds;
	std::vector< SoundID > m_diggingSounds;

	static void InitializeBlockDefinitions();
	static bool IsSolid( BlockType type ) { return s_blockDefinitionRegistry[ type ].m_isSolid; } //Things that aren't solid can't be selected by raycast or collided with.
	static bool IsOpaque( BlockType type ) { return s_blockDefinitionRegistry[ type ].m_isOpaque; } //Things that aren't opaque don't occlude faces in HSR and end lighting column descents.
	static int GetLightLevel( BlockType type );
	static float GetSecondsToBreak( BlockType type ) { return s_blockDefinitionRegistry[ type ].m_toughness; }
	static AABB2 GetSideTexCoords( BlockType type ) { return s_blockDefinitionRegistry[ type ].m_texCoordsSides; }
	static void PlayBreakingSound( BlockType blockTypeBroken );
	static void PlayPlacingSound( BlockType blockTypePlaced );
	static void PlayDiggingSound( BlockType blockTypeDug, float deltaSeconds );
	static void PlayWalkingSound( BlockType randomResult );

	static float m_secondsSinceLastDigSound;
};