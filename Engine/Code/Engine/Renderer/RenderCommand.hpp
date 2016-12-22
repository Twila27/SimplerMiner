#pragma once


#include "Engine/Renderer/Rgba.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Vector3.hpp"
#include <list>


//-----------------------------------------------------------------------------
struct RenderCommand;
extern std::list< RenderCommand* >* g_theRenderCommands; //Because deleted from at random frequently (expirations).
void RenderAndExpireDebugCommands();
void UpdateDebugCommands( float deltaSeconds );
void ClearDebugCommands();


//-----------------------------------------------------------------------------
enum DepthMode { 
	DEPTH_TEST_ON,
	DEPTH_TEST_OFF,
	DEPTH_TEST_DUAL, //Drawn once on (brighter/bigger), once off (fainter/smaller).
	NUM_DEPTH_MODES
};


//-----------------------------------------------------------------------------
struct RenderCommand
{
	Rgba m_color;
	float m_secondsToLive;
	DepthMode m_depthMode;
	float m_lineThickness;

	RenderCommand( float secondsToLive = 0.f, 
				   DepthMode depthMode = DEPTH_TEST_ON, 
				   const Rgba& color = Rgba::WHITE,
				   float lineThickness = 1.f )
		: m_color( color )
		, m_secondsToLive( secondsToLive )
		, m_depthMode( depthMode ) 
		, m_lineThickness( lineThickness )
	{
	}

	bool IsExpired() { return m_secondsToLive <= 0.f; }
	void Update( float deltaSeconds ) { m_secondsToLive -= deltaSeconds; }
	void UpdateRenderMode();
	void virtual Render() = 0;
};

//-----------------------------------------------------------------------------
struct RenderCommandPoint : public RenderCommand
{
	Vector3 m_position;

	RenderCommandPoint( const Vector3& position, 
						float secondsToLive, DepthMode depthMode, const Rgba& color, float lineThickness )
		: RenderCommand( secondsToLive, depthMode, color, lineThickness )
		, m_position( position ) 
	{
	}

	void Render() override;
};


//-----------------------------------------------------------------------------
struct RenderCommandLine : public RenderCommand
{
	Vector3 m_startPos;
	Vector3 m_endPos;

	RenderCommandLine( const Vector3& startPos,
					   const Vector3& endPos,
					   float secondsToLive, DepthMode depthMode, const Rgba& color, float lineThickness )
		: RenderCommand( secondsToLive, depthMode, color, lineThickness )
		, m_startPos( startPos )
		, m_endPos( endPos ) 
	{
	}

	void Render() override;
};


//-----------------------------------------------------------------------------
struct RenderCommandArrow : public RenderCommand
{
	Vector3 m_startPos;
	Vector3 m_endPos;

	RenderCommandArrow( const Vector3& startPos,
						const Vector3& endPos,
						float secondsToLive, DepthMode depthMode, const Rgba& color, float lineThickness )
		: RenderCommand( secondsToLive, depthMode, color, lineThickness )
		, m_startPos( startPos )
		, m_endPos( endPos ) 
	{
	}

	void Render() override;
};


//-----------------------------------------------------------------------------
struct RenderCommandAABB3 : public RenderCommand
{
	AABB3 m_bounds;

	RenderCommandAABB3( const AABB3& bounds,
						float secondsToLive, DepthMode depthMode, const Rgba& color, float lineThickness )
		: RenderCommand( secondsToLive, depthMode, color, lineThickness )
		, m_bounds( bounds ) 
	{
	}

	void Render() override;
};


//-----------------------------------------------------------------------------
struct RenderCommandSphere : public RenderCommand
{
	Vector3 m_centerPos;
	float m_radius;

	RenderCommandSphere( const Vector3& centerPos,
						 float radius,
						 float secondsToLive, DepthMode depthMode, const Rgba& color, float lineThickness )
		: RenderCommand( secondsToLive, depthMode, color, lineThickness )
		, m_centerPos( centerPos )
		, m_radius( radius )
	{
	}

	void Render() override;
};
