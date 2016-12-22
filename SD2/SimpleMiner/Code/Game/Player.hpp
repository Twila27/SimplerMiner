#pragma once


#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Game/GameCommon.hpp"


class Player
{
private:

	typedef AABB3 PlayerBoxCollider;

	struct PlayerCanCollider
	{
		Vector3 m_center;
		float m_radius;
		float m_height;
		inline void Render( float numSlices, float numSidesPerSlice );
	};

	struct PlayerBallCollider
	{
		Vector3 m_center;
		float m_radius;
		inline void Render( float numSlices, float numSidesPerSlice );
	};

	struct PlayerPillCollider
	{
		PlayerBallCollider m_top;
		PlayerBallCollider m_bottom;
		PlayerCanCollider m_middle;
		inline void Render( float numSlices, float numSidesPerSlice );
	};

public:

	Player( const Vector3& worldPosition, const Vector3& velocity = Vector3::ZERO );

	Vector3 m_worldPosition; //Tie with the bounding volumes in Update().
	Vector3 m_velocity;

	//Update all three bounding volumes, but only use the active one.
	enum PlayerColliderType { BOX, CAN, PILL };
	PlayerColliderType m_activeColliderType;
	PlayerBoxCollider m_boxBounds;
	PlayerCanCollider m_canBounds;
	PlayerPillCollider m_pillBounds;

	void Render();
	void UpdateCollidersAndDigTime( float deltaSeconds );

	bool IsDigging() const { return m_isDigging; }
	float GetSecondsSpentDigging() const { return m_secondsSpentDigging; }
	void StartDigging() { m_isDigging = true; }
	void StopDigging() { m_isDigging = false; m_secondsSpentDigging = 0.f; }

	WorldCoords GetFeetPos();
	WorldCoords GetHeadPos();

private:

	static const float COLLIDER_NUM_SLICES;
	static const float COLLIDER_NUM_SIDES_PER_SLICE;
	static const Vector3 CENTER_OFFSET;

	bool m_isDigging; //Flagged by world.
	float m_secondsSpentDigging;
	
};
