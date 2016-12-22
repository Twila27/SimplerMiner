#include "Game/TheApp.hpp"

//---------------------------------------------------------------------
TheApp* g_theApp = nullptr;


//---------------------------------------------------------------------
TheApp::TheApp( const double screenWidth, const double screenHeight )
	: m_screenWidth(screenWidth)
	, m_screenHeight(screenHeight)
{
}