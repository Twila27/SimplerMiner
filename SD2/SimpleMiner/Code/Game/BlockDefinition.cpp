#include "Game/BlockDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


STATIC std::map< BlockType, BlockDefinition > BlockDefinition::s_blockDefinitionRegistry;
STATIC float BlockDefinition::m_secondsSinceLastDigSound = 0.f;

//--------------------------------------------------------------------------------------------------------------
void BlockDefinition::InitializeBlockDefinitions()
{
	BlockDefinition tempDefinition;

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 0 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 0 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 0 );
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 0.f;
	tempDefinition.m_isSolid = false;
	tempDefinition.m_isOpaque = false;
	s_blockDefinitionRegistry[ BlockType::AIR ] = tempDefinition;

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 15, 11 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 15, 11 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 15, 11 );
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 0.f;
	tempDefinition.m_isSolid = false;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/splash1.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/splash2.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/splash1.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/splash2.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::WATER ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 12 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 12 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 12 );
	tempDefinition.m_emittedLightLevel = GLOWSTONE_LIGHT_LEVEL;
	tempDefinition.m_toughness = 0.f;
	tempDefinition.m_isSolid = false;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/splash1.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/splash2.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/splash1.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/splash2.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::LAVA ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 9, 8 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 8, 8 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 8 );
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 1.5f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = false;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass1.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass2.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass3.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass4.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass5.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass6.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digGrass.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digGrass.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digGrass.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::GRASS ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 9 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 8, 9 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 8 );
	tempDefinition.m_emittedLightLevel = 6; ///////////////////////////////////////////////////////////////////////
	tempDefinition.m_toughness = 1.5f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass1.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass2.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass3.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass4.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass5.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass6.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digGrass.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digGrass.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digGrass.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::MYCELIUM ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();


	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 8 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 8 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 8 );
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 1.5f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass1.ogg" ) ); //TODO -- use 1.8.json in %appdata%/.minecraft/assets/indexes to get the hash for .ogg's
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass2.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass3.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass4.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass5.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass6.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digGrass.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digGrass.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digGrass.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::DIRT ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 12 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 12 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 12 );
	tempDefinition.m_emittedLightLevel = 5;
	tempDefinition.m_toughness = 1.5f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass1.ogg" ) ); //TODO -- use 1.8.json in %appdata%/.minecraft/assets/indexes to get the hash for .ogg's
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass2.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass3.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass4.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass5.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass6.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digGrass.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digGrass.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digGrass.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::NETHERRACK ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 1, 8 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 1, 8 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 1, 8 );
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 1.5f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepSand1.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digSand.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digSand.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digSand.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::SAND ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 11 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 11 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 11 );
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 1.5f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepSand1.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digSand.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digSand.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digSand.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::RED_SAND ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 10 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 10 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 10 );
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 15.f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone1.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone2.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone3.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone4.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone5.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone6.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::STONE ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 9 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 9 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 9 );
	tempDefinition.m_emittedLightLevel = 5;
	tempDefinition.m_toughness = 15.f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone1.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone2.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone3.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone4.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone5.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone6.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::BROWNSTONE ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 3, 10 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 3, 10 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 3, 10 );
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 15.f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone1.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone2.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone3.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone4.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone5.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone6.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::COBBLESTONE ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 4, 11 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 4, 11 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 4, 11 );
	tempDefinition.m_emittedLightLevel = GLOWSTONE_LIGHT_LEVEL;
	tempDefinition.m_toughness = 0.75f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone1.ogg" ) ); //TODO -- use 1.8.json in %appdata%/.minecraft/assets/indexes to get the hash for .ogg's
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone2.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone3.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone4.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone5.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone6.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::GLOWSTONE ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 6, 11 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 6, 11 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 6, 11 );
	tempDefinition.m_emittedLightLevel = GLOWSTONE_LIGHT_LEVEL;
	tempDefinition.m_toughness = 0.75f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone1.ogg" ) ); //TODO -- use 1.8.json in %appdata%/.minecraft/assets/indexes to get the hash for .ogg's
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone2.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone3.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone4.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone5.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone6.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::GOLD_BRICK ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 10 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 10 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 10 );
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 0.75f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepSand1.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digSand.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digSand.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digSand.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::GRAVEL ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 5, 11 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 5, 11 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 5, 11 );
	tempDefinition.m_emittedLightLevel = GLOWSTONE_LIGHT_LEVEL;
	tempDefinition.m_toughness = 0.75f;
	tempDefinition.m_isSolid = false;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone1.ogg" ) ); //TODO -- use 1.8.json in %appdata%/.minecraft/assets/indexes to get the hash for .ogg's
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone2.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone3.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone4.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone5.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone6.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::PORTAL ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 13, 10 ); //Top and front is 1,2.
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 13, 10 ); //L-shape is 0,2.
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 13, 10 ); //Back and bottom are just full square, 13,10.
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 7.5f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = false;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone1.ogg" ) ); //TODO -- use 1.8.json in %appdata%/.minecraft/assets/indexes to get the hash for .ogg's
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone2.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone3.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone4.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone5.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone6.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::STAIRS ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 0 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 13 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 0 );
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 7.5f;
	tempDefinition.m_isSolid = false;
	tempDefinition.m_isOpaque = false;
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone1.ogg" ) ); //TODO -- use 1.8.json in %appdata%/.minecraft/assets/indexes to get the hash for .ogg's
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone2.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone3.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone4.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone5.ogg" ) );
	tempDefinition.m_walkingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/stepStone6.ogg" ) );
	tempDefinition.m_diggingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_breakingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	tempDefinition.m_placingSounds.push_back( g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" ) );
	s_blockDefinitionRegistry[ BlockType::LADDER ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

}


//--------------------------------------------------------------------------------------------------------------
int BlockDefinition::GetLightLevel( BlockType type )
{
	return s_blockDefinitionRegistry[ type ].m_emittedLightLevel;
}


//--------------------------------------------------------------------------------------------------------------
void BlockDefinition::PlayBreakingSound( BlockType blockTypeBroken )
{
	std::vector< SoundID >& soundVector = s_blockDefinitionRegistry[ blockTypeBroken ].m_breakingSounds;
	g_theAudio->PlaySound( soundVector[ GetRandomIntInRange( 0.f, (int)soundVector.size() - 1.f ) ], VOLUME_ADJUST );
}


//--------------------------------------------------------------------------------------------------------------
void BlockDefinition::PlayPlacingSound( BlockType blockTypePlaced )
{
	std::vector< SoundID >& soundVector = s_blockDefinitionRegistry[ blockTypePlaced ].m_placingSounds;
	g_theAudio->PlaySound( soundVector[ GetRandomIntInRange( 0.f, (int)soundVector.size() - 1.f ) ], VOLUME_ADJUST );
}


//--------------------------------------------------------------------------------------------------------------
void BlockDefinition::PlayDiggingSound( BlockType blockTypeDug, float deltaSeconds )
{
	m_secondsSinceLastDigSound += deltaSeconds;
	if ( m_secondsSinceLastDigSound < SECONDS_BETWEEN_DIG_SOUNDS ) return;
	else m_secondsSinceLastDigSound = 0.f;

	std::vector< SoundID >& soundVector = s_blockDefinitionRegistry[ blockTypeDug ].m_diggingSounds;
	g_theAudio->PlaySound( soundVector[ GetRandomIntInRange( 0.f, (int)soundVector.size() - 1.f ) ], VOLUME_ADJUST );
}


//--------------------------------------------------------------------------------------------------------------
void BlockDefinition::PlayWalkingSound( BlockType randomResult )
{
	std::vector< SoundID >& soundVector = s_blockDefinitionRegistry[ randomResult ].m_walkingSounds;
	g_theAudio->PlaySound( soundVector[ GetRandomIntInRange( 0.f, (int)soundVector.size() - 1.f ) ], VOLUME_ADJUST );
}
