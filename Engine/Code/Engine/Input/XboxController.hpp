#pragma once


#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/PolarCoords.hpp"


//-----------------------------------------------------------------------------
struct ButtonState
{
	bool m_isButtonDown = false;
	bool m_didButtonJustChange = false;
};


//-----------------------------------------------------------------------------
class XboxController
{
public:
	XboxController();
	XboxController( int controllerNumber );

	void Update();

	inline Vector2 GetLeftStickPosition() const { return m_correctedLeftStick; }
	inline float GetLeftStickPositionAsRadius() const { return m_correctedLeftStickInPolar.radius; }
	inline float GetLeftStickPositionAsAngleInRadians() const { return m_correctedLeftStickInPolar.thetaRadians; }
		
	inline Vector2 GetRightStickPosition() const { return m_correctedRightStick; }
	inline float GetRightStickPositionAsRadius() const { return m_correctedRightStickInPolar.radius; }
	inline float GetRightStickPositionAsAngleInRadians() const { return m_correctedRightStickInPolar.thetaRadians; }
		
	inline float GetLeftTrigger() const { return m_leftTrigger; }
	inline float GetRightTrigger() const { return m_rightTrigger; }
		
	inline bool IsAButtonDown() const { return m_aButton.m_isButtonDown; }
	inline bool IsBButtonDown( ) const { return m_bButton.m_isButtonDown; }
	inline bool IsXButtonDown( ) const { return m_xButton.m_isButtonDown; }
	inline bool IsYButtonDown( ) const { return m_yButton.m_isButtonDown; }
	inline bool IsLeftShoulderButtonDown( ) const { return m_leftShoulderButton.m_isButtonDown; }
	inline bool IsRightShoulderButtonDown( ) const { return m_rightShoulderButton.m_isButtonDown; }
	inline bool IsBackButtonDown() const { return m_backButton.m_isButtonDown; }
	inline bool IsStartButtonDown() const { return m_startButton.m_isButtonDown; }
	inline bool IsLeftStickClickedIn() const { return m_leftStickClick.m_isButtonDown; }
	inline bool IsRightStickClickedIn() const { return m_rightStickClick.m_isButtonDown; }

	inline bool WasAButtonJustPressed() const { return m_aButton.m_didButtonJustChange; }
	inline bool WasBButtonJustPressed() const { return m_bButton.m_didButtonJustChange; }
	inline bool WasXButtonJustPressed() const { return m_xButton.m_didButtonJustChange; }
	inline bool WasYButtonJustPressed() const { return m_yButton.m_didButtonJustChange; }
	inline bool WasLeftShoulderButtonJustPressed() const { return m_leftShoulderButton.m_didButtonJustChange; }
	inline bool WasRigtShoulderButtonJustPressed( ) const { return m_rightShoulderButton.m_didButtonJustChange; }
	inline bool WasBackButtonJustPressed() const { return m_backButton.m_didButtonJustChange; }
	inline bool WasStartButtonJustPressed() const { return m_startButton.m_didButtonJustChange; }
	inline bool WasLeftStickClickJustPressed() const { return m_leftStickClick.m_didButtonJustChange; }
	inline bool WasRightStickClickJustPressed() const { return m_rightStickClick.m_didButtonJustChange; }

private:
	int m_controllerNumber;

	ButtonState m_aButton;
	ButtonState m_bButton;
	ButtonState m_xButton;
	ButtonState m_yButton;
	ButtonState m_leftShoulderButton;
	ButtonState m_rightShoulderButton;
	ButtonState m_startButton;
	ButtonState m_backButton;
	ButtonState m_leftStickClick;
	ButtonState m_rightStickClick;

	float m_leftTrigger;
	float m_rightTrigger;

	Vector2 m_correctedLeftStick;
	Vector2 m_correctedRightStick;
	PolarCoords m_correctedLeftStickInPolar;
	PolarCoords m_correctedRightStickInPolar;

	void UpdateButtonStates( unsigned short wButtons );
	void MapControlSticks( short sThumbLX, short sThumbLY, short sThumbRX, short sThumbRY );
};