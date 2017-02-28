#include "Game/Block.hpp"
#include "Game/BlockDefinition.hpp"

void Block::SetBlockType( BlockType type )
{
	m_type = type;

	if ( BlockDefinition::IsOpaque( type ) )
		this->SetBlockToBeOpaque();
	else
		this->SetBlockToNotBeOpaque();
}
