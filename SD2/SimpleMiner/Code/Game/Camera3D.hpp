#pragma once


#include "Game/GameCommon.hpp"
#include "Engine/Math/EulerAngles.hpp"


class Camera3D
{
public:
	Camera3D( const Vector3& worldPosition, const EulerAngles& orientation = EulerAngles::ZERO );
	void FixAndClampAngles();
	Vector3 GetForwardXYZ() const;
	Vector3 GetForwardXY() const;
	Vector3 GetLeftXY() const;

public:
	WorldCoords m_worldPosition;
	EulerAngles m_orientation; //Euler angles best solve mouse-look and clamping.
};