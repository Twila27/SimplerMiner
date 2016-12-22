#include "Game/Camera3D.hpp"
#include "Engine/Math/MathUtils.hpp"


//--------------------------------------------------------------------------------------------------------------
Camera3D::Camera3D( const Vector3& worldPosition, const EulerAngles& orientation /*= EulerAngles::ZERO*/ )
	: m_worldPosition( worldPosition )
	, m_orientation( orientation )
{
}


//--------------------------------------------------------------------------------------------------------------
void Camera3D::FixAndClampAngles()
{
	m_orientation.m_rollDegreesAboutX = ClampFloat( m_orientation.m_rollDegreesAboutX, -89.f, 89.f );
	m_orientation.m_pitchDegreesAboutY = ClampFloat( m_orientation.m_pitchDegreesAboutY, -89.f, 89.f );
	m_orientation.m_yawDegreesAboutZ = WrapNumberWithinCircularRange( m_orientation.m_yawDegreesAboutZ, 0.f, 360.f );
}


//--------------------------------------------------------------------------------------------------------------
Vector3 Camera3D::GetForwardXYZ() const //Same heading cos(yaw), sin(yaw), scaled by cos(pitch) as in polar form.
{
	float theta = m_orientation.m_yawDegreesAboutZ;
	float phi = m_orientation.m_pitchDegreesAboutY;
	
	float radiusScaleAlongXY = CosDegrees( phi );
	return Vector3( radiusScaleAlongXY * CosDegrees( theta ), radiusScaleAlongXY * SinDegrees( theta ), -SinDegrees( phi ) );
}


//--------------------------------------------------------------------------------------------------------------
Vector3 Camera3D::GetForwardXY() const //Just a 2D heading with zero z-component.
{
	float cosYaw = CosDegrees( m_orientation.m_yawDegreesAboutZ );
	float sinYaw = SinDegrees( m_orientation.m_yawDegreesAboutZ );
	return Vector3( cosYaw, sinYaw, 0.f );
}


//--------------------------------------------------------------------------------------------------------------
Vector3 Camera3D::GetLeftXY() const //Just a 2D heading with zero z-component, negated by our backward y-axis.
{
	Vector3 forwardXY = GetForwardXY();
	return Vector3( -forwardXY.y, forwardXY.x, 0.f );
}
