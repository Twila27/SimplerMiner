#include "Game/BlockDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


STATIC std::map< BlockType, BlockDefinition > BlockDefinition::s_blockDefinitionRegistry;
STATIC float BlockDefinition::m_secondsSinceLastDigSound = 0.f;

//--------------------------------------------------------------------------------------------------------------
void BlockDefinition::InitializeBlockDefinitions()
{
	//Possible speedup: use 1.8.json file in %appdata%/.minecraft/assets/indexes to get .ogg files' hashes.
	//However, for the time being -- not that many blocks plus an only-once process -- no need to optimize prematurely.

	BlockDefinition tempDefinition;
	SoundID splashSound1 = g_theAudio->CreateOrGetSound( "Data/Audio/splash1.ogg" );
	SoundID splashSound2 = g_theAudio->CreateOrGetSound( "Data/Audio/splash2.ogg" );
	SoundID digGrassSound = g_theAudio->CreateOrGetSound( "Data/Audio/digGrass.ogg" );
	SoundID digSandSound = g_theAudio->CreateOrGetSound( "Data/Audio/digSand.ogg" );
	SoundID digStoneSound = g_theAudio->CreateOrGetSound( "Data/Audio/digStone.ogg" );
	SoundID stepGrassSound1 = g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass1.ogg" );
	SoundID stepGrassSound2 = g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass2.ogg" );
	SoundID stepGrassSound3 = g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass3.ogg" );
	SoundID stepGrassSound4 = g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass4.ogg" );
	SoundID stepGrassSound5 = g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass5.ogg" );
	SoundID stepGrassSound6 = g_theAudio->CreateOrGetSound( "Data/Audio/stepGrass6.ogg" );
	SoundID stepSandSound1 = g_theAudio->CreateOrGetSound( "Data/Audio/stepSand1.ogg" );
	SoundID stepStoneSound1 = g_theAudio->CreateOrGetSound( "Data/Audio/stepStone1.ogg" );
	SoundID stepStoneSound2 = g_theAudio->CreateOrGetSound( "Data/Audio/stepStone2.ogg" );
	SoundID stepStoneSound3 = g_theAudio->CreateOrGetSound( "Data/Audio/stepStone3.ogg" );
	SoundID stepStoneSound4 = g_theAudio->CreateOrGetSound( "Data/Audio/stepStone4.ogg" );
	SoundID stepStoneSound5 = g_theAudio->CreateOrGetSound( "Data/Audio/stepStone5.ogg" );
	SoundID stepStoneSound6 = g_theAudio->CreateOrGetSound( "Data/Audio/stepStone6.ogg" );

	AABB2 texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 0 );
	tempDefinition.m_texCoordsTop = texCoords;
	tempDefinition.m_texCoordsSides = texCoords;
	tempDefinition.m_texCoordsBottom = texCoords;
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 0.f;
	tempDefinition.m_isSolid = false;
	tempDefinition.m_isOpaque = false;
	s_blockDefinitionRegistry[ BlockType::AIR ] = tempDefinition;

	texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 15, 11 );
	tempDefinition.m_texCoordsTop = texCoords;
	tempDefinition.m_texCoordsSides = texCoords;
	tempDefinition.m_texCoordsBottom = texCoords;
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 0.f;
	tempDefinition.m_isSolid = false;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( splashSound1 );
	tempDefinition.m_walkingSounds.push_back( splashSound2 );
	tempDefinition.m_placingSounds.push_back( splashSound1 );
	tempDefinition.m_placingSounds.push_back( splashSound2 );
	s_blockDefinitionRegistry[ BlockType::WATER ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();

	texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 12 );
	tempDefinition.m_texCoordsTop = texCoords;
	tempDefinition.m_texCoordsSides = texCoords;
	tempDefinition.m_texCoordsBottom = texCoords;
	tempDefinition.m_emittedLightLevel = GLOWSTONE_LIGHT_LEVEL;
	tempDefinition.m_toughness = 0.f;
	tempDefinition.m_isSolid = false;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( splashSound1 );
	tempDefinition.m_walkingSounds.push_back( splashSound2 );
	tempDefinition.m_placingSounds.push_back( splashSound1 );
	tempDefinition.m_placingSounds.push_back( splashSound2 );
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
	tempDefinition.m_walkingSounds.push_back( stepGrassSound1 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound2 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound3 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound4 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound5 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound6 );
	tempDefinition.m_diggingSounds.push_back( digGrassSound );
	tempDefinition.m_breakingSounds.push_back( digGrassSound );
	tempDefinition.m_placingSounds.push_back( digGrassSound );
	s_blockDefinitionRegistry[ BlockType::GRASS ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	tempDefinition.m_texCoordsTop = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 9 );
	tempDefinition.m_texCoordsSides = g_textureAtlas->GetTexCoordsFromSpriteCoords( 8, 9 );
	tempDefinition.m_texCoordsBottom = g_textureAtlas->GetTexCoordsFromSpriteCoords( 7, 8 );
	tempDefinition.m_emittedLightLevel = 6;
	tempDefinition.m_toughness = 1.5f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( stepGrassSound1 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound2 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound3 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound4 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound5 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound6 );
	tempDefinition.m_diggingSounds.push_back( digGrassSound );
	tempDefinition.m_breakingSounds.push_back( digGrassSound );
	tempDefinition.m_placingSounds.push_back( digGrassSound );
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
	tempDefinition.m_walkingSounds.push_back( stepGrassSound1 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound2 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound3 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound4 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound5 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound6 );
	tempDefinition.m_diggingSounds.push_back( digGrassSound );
	tempDefinition.m_breakingSounds.push_back( digGrassSound );
	tempDefinition.m_placingSounds.push_back( digGrassSound );
	s_blockDefinitionRegistry[ BlockType::DIRT ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 12 );
	tempDefinition.m_texCoordsTop = texCoords;
	tempDefinition.m_texCoordsSides = texCoords;
	tempDefinition.m_texCoordsBottom = texCoords;
	tempDefinition.m_emittedLightLevel = 5;
	tempDefinition.m_toughness = 1.5f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( stepGrassSound1 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound2 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound3 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound4 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound5 );
	tempDefinition.m_walkingSounds.push_back( stepGrassSound6 );
	tempDefinition.m_diggingSounds.push_back( digGrassSound );
	tempDefinition.m_breakingSounds.push_back( digGrassSound );
	tempDefinition.m_placingSounds.push_back( digGrassSound );
	s_blockDefinitionRegistry[ BlockType::NETHERRACK ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 1, 8 );
	tempDefinition.m_texCoordsTop = texCoords;
	tempDefinition.m_texCoordsSides = texCoords;
	tempDefinition.m_texCoordsBottom = texCoords;
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 1.5f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( stepSandSound1 );
	tempDefinition.m_diggingSounds.push_back( digSandSound );
	tempDefinition.m_breakingSounds.push_back( digSandSound );
	tempDefinition.m_placingSounds.push_back( digSandSound );
	s_blockDefinitionRegistry[ BlockType::SAND ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 11 );
	tempDefinition.m_texCoordsTop = texCoords;
	tempDefinition.m_texCoordsSides = texCoords;
	tempDefinition.m_texCoordsBottom = texCoords;
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 1.5f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( stepSandSound1 );
	tempDefinition.m_diggingSounds.push_back( digSandSound );
	tempDefinition.m_breakingSounds.push_back( digSandSound );
	tempDefinition.m_placingSounds.push_back( digSandSound );
	s_blockDefinitionRegistry[ BlockType::RED_SAND ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 10 );
	tempDefinition.m_texCoordsTop = texCoords;
	tempDefinition.m_texCoordsSides = texCoords;
	tempDefinition.m_texCoordsBottom = texCoords;
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 15.f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( stepStoneSound1 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound2 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound3 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound4 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound5 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound6 );
	tempDefinition.m_diggingSounds.push_back( digStoneSound );
	tempDefinition.m_breakingSounds.push_back( digStoneSound );
	tempDefinition.m_placingSounds.push_back( digStoneSound );
	s_blockDefinitionRegistry[ BlockType::STONE ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 2, 9 );
	tempDefinition.m_texCoordsTop = texCoords;
	tempDefinition.m_texCoordsSides = texCoords;
	tempDefinition.m_texCoordsBottom = texCoords;
	tempDefinition.m_emittedLightLevel = 5;
	tempDefinition.m_toughness = 15.f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( stepStoneSound1 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound2 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound3 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound4 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound5 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound6 );
	tempDefinition.m_diggingSounds.push_back( digStoneSound );
	tempDefinition.m_breakingSounds.push_back( digStoneSound );
	tempDefinition.m_placingSounds.push_back( digStoneSound );
	s_blockDefinitionRegistry[ BlockType::BROWNSTONE ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 3, 10 );
	tempDefinition.m_texCoordsTop = texCoords;
	tempDefinition.m_texCoordsSides = texCoords;
	tempDefinition.m_texCoordsBottom = texCoords;
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 15.f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( stepStoneSound1 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound2 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound3 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound4 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound5 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound6 );
	tempDefinition.m_diggingSounds.push_back( digStoneSound );
	tempDefinition.m_breakingSounds.push_back( digStoneSound );
	tempDefinition.m_placingSounds.push_back( digStoneSound );
	s_blockDefinitionRegistry[ BlockType::COBBLESTONE ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 4, 11 );
	tempDefinition.m_texCoordsTop = texCoords;
	tempDefinition.m_texCoordsSides = texCoords;
	tempDefinition.m_texCoordsBottom = texCoords;
	tempDefinition.m_emittedLightLevel = GLOWSTONE_LIGHT_LEVEL;
	tempDefinition.m_toughness = 0.75f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( stepStoneSound1 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound2 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound3 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound4 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound5 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound6 );
	tempDefinition.m_diggingSounds.push_back( digStoneSound );
	tempDefinition.m_breakingSounds.push_back( digStoneSound );
	tempDefinition.m_placingSounds.push_back( digStoneSound );
	s_blockDefinitionRegistry[ BlockType::GLOWSTONE ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 6, 11 );
	tempDefinition.m_texCoordsTop = texCoords;
	tempDefinition.m_texCoordsSides = texCoords;
	tempDefinition.m_texCoordsBottom = texCoords;
	tempDefinition.m_emittedLightLevel = GLOWSTONE_LIGHT_LEVEL;
	tempDefinition.m_toughness = 0.75f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( stepStoneSound1 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound2 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound3 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound4 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound5 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound6 );
	tempDefinition.m_diggingSounds.push_back( digStoneSound );
	tempDefinition.m_breakingSounds.push_back( digStoneSound );
	tempDefinition.m_placingSounds.push_back( digStoneSound );
	s_blockDefinitionRegistry[ BlockType::GOLD_BRICK ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 0, 10 );
	tempDefinition.m_texCoordsTop = texCoords;
	tempDefinition.m_texCoordsSides = texCoords;
	tempDefinition.m_texCoordsBottom = texCoords;
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 0.75f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( stepSandSound1 );
	tempDefinition.m_diggingSounds.push_back( digSandSound );
	tempDefinition.m_breakingSounds.push_back( digSandSound );
	tempDefinition.m_placingSounds.push_back( digSandSound );
	s_blockDefinitionRegistry[ BlockType::GRAVEL ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 5, 11 );
	tempDefinition.m_texCoordsTop = texCoords;
	tempDefinition.m_texCoordsSides = texCoords;
	tempDefinition.m_texCoordsBottom = texCoords;
	tempDefinition.m_emittedLightLevel = GLOWSTONE_LIGHT_LEVEL;
	tempDefinition.m_toughness = 0.75f;
	tempDefinition.m_isSolid = false;
	tempDefinition.m_isOpaque = true;
	tempDefinition.m_walkingSounds.push_back( stepStoneSound1 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound2 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound3 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound4 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound5 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound6 );
	tempDefinition.m_diggingSounds.push_back( digStoneSound );
	tempDefinition.m_breakingSounds.push_back( digStoneSound );
	tempDefinition.m_placingSounds.push_back( digStoneSound );
	s_blockDefinitionRegistry[ BlockType::PORTAL ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

	texCoords = g_textureAtlas->GetTexCoordsFromSpriteCoords( 13, 10 );
	tempDefinition.m_texCoordsTop = texCoords; //Future reference: Top and front stairs image is 1,2.
	tempDefinition.m_texCoordsSides = texCoords; //Future reference: L-shape stairs image is 0,2.
	tempDefinition.m_texCoordsBottom = texCoords; //And back/bottom are just full square, 13,10.
	tempDefinition.m_emittedLightLevel = 0;
	tempDefinition.m_toughness = 7.5f;
	tempDefinition.m_isSolid = true;
	tempDefinition.m_isOpaque = false;
	tempDefinition.m_walkingSounds.push_back( stepStoneSound1 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound2 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound3 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound4 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound5 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound6 );
	tempDefinition.m_diggingSounds.push_back( digStoneSound );
	tempDefinition.m_breakingSounds.push_back( digStoneSound );
	tempDefinition.m_placingSounds.push_back( digStoneSound );
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
	tempDefinition.m_walkingSounds.push_back( stepStoneSound1 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound2 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound3 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound4 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound5 );
	tempDefinition.m_walkingSounds.push_back( stepStoneSound6 );
	tempDefinition.m_diggingSounds.push_back( digStoneSound );
	tempDefinition.m_breakingSounds.push_back( digStoneSound );
	tempDefinition.m_placingSounds.push_back( digStoneSound );
	s_blockDefinitionRegistry[ BlockType::LADDER ] = tempDefinition;
	tempDefinition.m_walkingSounds.clear();
	tempDefinition.m_placingSounds.clear();
	tempDefinition.m_diggingSounds.clear();
	tempDefinition.m_breakingSounds.clear();

}


//--------------------------------------------------------------------------------------------------------------
void BlockDefinition::PlayBreakingSound( BlockType blockTypeBroken )
{
	std::vector< SoundID >& soundVector = s_blockDefinitionRegistry[ blockTypeBroken ].m_breakingSounds;
	int randomSoundIndex = GetRandomIntInRange( 0.f, (int)soundVector.size() - 1.f );
	g_theAudio->PlaySound( soundVector[ randomSoundIndex ], VOLUME_ADJUST );
}


//--------------------------------------------------------------------------------------------------------------
void BlockDefinition::PlayPlacingSound( BlockType blockTypePlaced )
{
	std::vector< SoundID >& soundVector = s_blockDefinitionRegistry[ blockTypePlaced ].m_placingSounds;
	int randomSoundIndex = GetRandomIntInRange( 0.f, (int)soundVector.size() - 1.f );
	g_theAudio->PlaySound( soundVector[ randomSoundIndex ], VOLUME_ADJUST );
}


//--------------------------------------------------------------------------------------------------------------
void BlockDefinition::PlayDiggingSound( BlockType blockTypeDug, float deltaSeconds )
{
	m_secondsSinceLastDigSound += deltaSeconds;
	if ( m_secondsSinceLastDigSound < SECONDS_BETWEEN_DIG_SOUNDS ) 
		return;
	else 
		m_secondsSinceLastDigSound = 0.f;

	std::vector< SoundID >& soundVector = s_blockDefinitionRegistry[ blockTypeDug ].m_diggingSounds;
	int randomSoundIndex = GetRandomIntInRange( 0.f, (int)soundVector.size() - 1.f );
	g_theAudio->PlaySound( soundVector[ randomSoundIndex ], VOLUME_ADJUST );
}


//--------------------------------------------------------------------------------------------------------------
void BlockDefinition::PlayWalkingSound( BlockType randomResult )
{
	std::vector< SoundID >& soundVector = s_blockDefinitionRegistry[ randomResult ].m_walkingSounds;
	int randomSoundIndex = GetRandomIntInRange( 0.f, (int)soundVector.size() - 1.f );
	g_theAudio->PlaySound( soundVector[ randomSoundIndex ], VOLUME_ADJUST );
}
