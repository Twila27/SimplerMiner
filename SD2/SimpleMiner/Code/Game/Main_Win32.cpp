#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include <stdlib.h>
#include <time.h>
#include <list>
#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library

#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Audio/TheAudio.hpp"
#include "Engine/Input/TheInput.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Time/Time.hpp"
#include "Game/TheApp.hpp"
#include "Game/TheGame.hpp"
#include "Game/GameCommon.hpp"

//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);


//-----------------------------------------------------------------------------------------------
//Noting that this sets (0,0) is in the bottom-left until SetOrtho is called later on.
const int OFFSET_FROM_WINDOWS_DESKTOP = 50;
const int WINDOW_PHYSICAL_WIDTH = 1600;
const int WINDOW_PHYSICAL_HEIGHT = 900;
const float VIEW_LEFT = 0.0;
const float VIEW_RIGHT = 1600.0;
const float VIEW_BOTTOM = 0.0;
const float VIEW_TOP = VIEW_RIGHT * static_cast<float>( WINDOW_PHYSICAL_HEIGHT ) / static_cast<float>( WINDOW_PHYSICAL_WIDTH );

float g_spinCounter;
float g_bKeyStopwatch;
bool g_isQuitting = false;
HWND g_hWnd = nullptr;
HDC g_displayDeviceContext = nullptr;
HGLRC g_openGLRenderingContext = nullptr;


//-----------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	unsigned char asKey = (unsigned char)wParam;
	switch ( wmMessageCode )
	{
	case WM_CLOSE:
	case WM_DESTROY:
	case WM_QUIT:
		g_isQuitting = true;
		return 0;

	case WM_KEYDOWN:
			g_theInput->SetKeyDownStatus( asKey, true );
		if ( asKey == VK_ESCAPE )
		{
			g_isQuitting = true;
			return 0;
		}
		break;

	case WM_KEYUP:
		g_theInput->SetKeyDownStatus( asKey, false );
		break;

	case WM_SETFOCUS:
		if ( g_theInput != nullptr) g_theInput->OnGainedFocus();
		break;

	case WM_KILLFOCUS:
		if ( g_theInput != nullptr ) g_theInput->OnLostFocus( );
		break;

	case WM_LBUTTONDOWN:
		g_theInput->SetMouseDownStatus( VK_LBUTTON, true );
		break;


	case WM_LBUTTONUP:
		g_theInput->SetMouseDownStatus( VK_LBUTTON, false );
		break;


	case WM_RBUTTONDOWN:
		g_theInput->SetMouseDownStatus( VK_RBUTTON, true );
		break;


	case WM_RBUTTONUP:
		g_theInput->SetMouseDownStatus( VK_RBUTTON, false );
		break;

	case WM_MOUSEWHEEL:
		g_theInput->SetMouseWheelDelta( GET_WHEEL_DELTA_WPARAM(wParam) );
		break;
	}

	return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
}


//-----------------------------------------------------------------------------------------------
void CreateOpenGLWindow( HINSTANCE applicationInstanceHandle )
{
	// Define a window class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>( WindowsMessageHandlingProcedure ); // Assign a win32 message-handling function
	windowClassDescription.hInstance = GetModuleHandle( NULL );
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT( "Simple Window Class" );
	RegisterClassEx( &windowClassDescription );

	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow( );
	GetClientRect( desktopWindowHandle, &desktopRect );

	RECT windowRect = { OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_WIDTH, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_HEIGHT };
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[ 1024 ];
	MultiByteToWideChar( GetACP( ), 0, g_appName, -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[ 0 ] ) );
	g_hWnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		applicationInstanceHandle,
		NULL );

	ShowWindow( g_hWnd, SW_SHOW );
	SetForegroundWindow( g_hWnd );
	SetFocus( g_hWnd );

	g_displayDeviceContext = GetDC( g_hWnd );

	HCURSOR cursor = LoadCursor( NULL, IDC_ARROW );
	SetCursor( cursor );

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset( &pixelFormatDescriptor, 0, sizeof( pixelFormatDescriptor ) );
	pixelFormatDescriptor.nSize = sizeof( pixelFormatDescriptor );
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	int pixelFormatCode = ChoosePixelFormat( g_displayDeviceContext, &pixelFormatDescriptor );
	SetPixelFormat( g_displayDeviceContext, pixelFormatCode, &pixelFormatDescriptor );
	g_openGLRenderingContext = wglCreateContext( g_displayDeviceContext );
	wglMakeCurrent( g_displayDeviceContext, g_openGLRenderingContext );

	g_theRenderer->SetOrtho( Vector2( VIEW_LEFT, VIEW_BOTTOM ), Vector2( VIEW_RIGHT, VIEW_TOP ) );
	g_theRenderer->SetRenderFlag( GL_BLEND );
	g_theRenderer->SetBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	g_theRenderer->SetAlphaFunc( GL_GREATER, 0.1f );
	g_theRenderer->SetLineWidth( 2.f );
	g_theRenderer->SetRenderFlag( GL_LINE_SMOOTH );
}


//-----------------------------------------------------------------------------------------------
void RunMessagePump( )
{
	MSG queuedMessage;
	for ( ;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if ( !wasMessagePresent )
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage );
	}
}


//-----------------------------------------------------------------------------------------------
void Update( float deltaSeconds )
{
	g_theGame->Update( deltaSeconds ); //Can pass in 0 to freeze, or others to rewind/slow/etc.
}


//-----------------------------------------------------------------------------------------------
void Render()
{
	g_theGame->Render();
	SwapBuffers( g_displayDeviceContext );
}



//-----------------------------------------------------------------------------------------------
float CalcDeltaSeconds()
{
	static double s_timeLastFrameBegan = GetCurrentTimeSeconds(); //Called only once due static.
	double timeThisFrameBegan = GetCurrentTimeSeconds();
	float deltaSeconds = static_cast<float>( timeThisFrameBegan - s_timeLastFrameBegan );
	s_timeLastFrameBegan = timeThisFrameBegan;

	return deltaSeconds;
}


//-----------------------------------------------------------------------------------------------
AudioChannelHandle g_bgMusicChannel;
void RunFrame()
{
	g_theInput->Update(); //Resets keys, putting this here validates WasKeyJustChanged.
	
	g_theAudio->Update();

	RunMessagePump( ); //Can rename this to just doInput, because it handles updating key callback.

	float deltaSeconds = CalcDeltaSeconds();

	Update( deltaSeconds );

	if ( g_theInput->WasKeyPressedOnce( 'X' ) ) 
		g_theAudio->StopChannel( g_bgMusicChannel );

	Render();
}


//-----------------------------------------------------------------------------------------------
void Initialize( HINSTANCE applicationInstanceHandle )
{
	srand( static_cast<unsigned int>( time( NULL ) ) ); //TODO: BE SMARTER

	SetProcessDPIAware();
	CreateOpenGLWindow( applicationInstanceHandle );
	g_theApp = new TheApp( VIEW_RIGHT, VIEW_TOP );

	g_theAudio = new AudioSystem();
	//SoundID musicID;
	//musicID = g_theAudio->CreateOrGetSound( "Data/Audio/Yume Nikki mega mix (SD).mp3" );
	//g_bgMusicChannel = g_theAudio->PlaySound( musicID );

	g_theGame = new TheGame();
	
	g_theInput = new TheInput();
	IntVector2 screenCenter = IntVector2( (int)( g_theApp->GetScreenWidth() / 2.0 ), (int)( g_theApp->GetScreenHeight() / 2.0 ) );
	g_theInput->SetCursorSnapToPos( screenCenter );
	g_theInput->HideCursor();
	
	g_theRenderer = new TheRenderer();
	g_theRenderCommands = new std::list< RenderCommand* >();
}


//-----------------------------------------------------------------------------------------------
void Shutdown( )
{
	delete g_theApp;
	delete g_theGame;
	delete g_theInput;
	delete g_theRenderer;
	ClearDebugCommands();
	delete g_theRenderCommands;

	g_theApp = nullptr;
	g_theGame = nullptr;
	g_theInput = nullptr;
	g_theRenderer = nullptr;
	g_theRenderCommands = nullptr;
}


//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED( commandLineString );
	Initialize( applicationInstanceHandle );

	while ( !g_isQuitting )
	{
		RunFrame( );
	}

	Shutdown( );
	return 0;
}