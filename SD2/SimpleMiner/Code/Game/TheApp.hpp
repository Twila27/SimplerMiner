#pragma once

//-----------------------------------------------------------------------------
class TheApp;


//-----------------------------------------------------------------------------
extern TheApp* g_theApp;


//-----------------------------------------------------------------------------
class TheApp
{
public:
	TheApp( const double screenWidth, const double screenHeight );
	inline double GetScreenWidth() const { return m_screenWidth; }
	inline double GetScreenHeight() const { return m_screenHeight; }


private:
	double m_screenWidth;
	double m_screenHeight;
};
