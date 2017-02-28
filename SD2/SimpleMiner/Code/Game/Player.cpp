#include "Game/Player.hpp"


#include "Game/GameCommon.hpp"
#include "Engine/Renderer/TheRenderer.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC const float Player::COLLIDER_NUM_SLICES = 20.f;
STATIC const float Player::COLLIDER_NUM_SIDES_PER_SLICE = 20.f;
STATIC const Vector3 Player::CENTER_OFFSET = Vector3( PLAYER_WIDTH, PLAYER_WIDTH, PLAYER_HEIGHT ) * 0.5f;


//--------------------------------------------------------------------------------------------------------------
Player::Player( const Vector3& worldPosition, const Vector3& velocity /*= Vector3::ZERO */ )
	: m_worldPosition( worldPosition )
	, m_velocity( velocity )
	, m_activeColliderType( BOX )
	, m_secondsSpentDigging( 0.f )
	, m_isDigging( false )
{
}


//--------------------------------------------------------------------------------------------------------------
void Player::Render()
{
	if ( g_renderPlayerCollider )
	{
		if ( m_activeColliderType == PILL )
		{
			m_pillBounds.Render( COLLIDER_NUM_SLICES, COLLIDER_NUM_SIDES_PER_SLICE );
		}
		else if ( m_activeColliderType == CAN )
		{
			m_canBounds.Render( COLLIDER_NUM_SLICES, COLLIDER_NUM_SIDES_PER_SLICE );
		}
		else if ( m_activeColliderType == BOX )
		{
			g_theRenderer->DrawAABB( TheRenderer::VertexGroupingRule::AS_LINE_LOOP, m_boxBounds, Rgba::BLUE );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
void Player::UpdateCollidersAndDigTime( float deltaSeconds )
{
	//The m_worldPosition that gets updated is to be treated as the center of our bounding volume.
	m_boxBounds.mins = m_worldPosition - CENTER_OFFSET;
	m_boxBounds.maxs = m_worldPosition + CENTER_OFFSET;

	if ( m_isDigging )
		m_secondsSpentDigging += deltaSeconds;
}


//--------------------------------------------------------------------------------------------------------------
WorldCoords Player::GetFeetPos()
{
	WorldCoords feetPos = m_worldPosition;

	feetPos.z -= PLAYER_HALF_HEIGHT;

	return feetPos;
}


//--------------------------------------------------------------------------------------------------------------
WorldCoords Player::GetHeadPos()
{
	WorldCoords headPos = m_worldPosition;

	headPos.z += PLAYER_HALF_HEIGHT;

	return headPos;
}


//--------------------------------------------------------------------------------------------------------------
inline void Player::PlayerCanCollider::Render( float numSlices, float numSidesPerSlice )
{
	g_theRenderer->DrawCylinder( TheRenderer::VertexGroupingRule::AS_LINE_LOOP, m_center, m_radius, m_height, numSlices, numSidesPerSlice );
}


//--------------------------------------------------------------------------------------------------------------
inline void Player::PlayerPillCollider::Render( float numSlices, float numSidesPerSlice )
{
	m_top.Render( numSlices, numSidesPerSlice );
	m_bottom.Render( numSlices, numSidesPerSlice );
	m_middle.Render( numSlices, numSidesPerSlice );
}


//--------------------------------------------------------------------------------------------------------------
inline void Player::PlayerBallCollider::Render( float numSlices, float numSidesPerSlice )
{
	g_theRenderer->DrawSphere( TheRenderer::VertexGroupingRule::AS_LINE_LOOP, m_center, m_radius, numSlices, numSidesPerSlice );
}