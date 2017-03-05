#pragma once


#include <vector>
#include "Engine/Math/Vector3.hpp"


//-----------------------------------------------------------------------------
class TheGame;
class Player;
class Camera3D;
class World;
class SpriteSheet;
class AABB2;
class AABB3;
enum BlockFace;


//-----------------------------------------------------------------------------
extern TheGame* g_theGame;


//-----------------------------------------------------------------------------
class TheGame
{
public:
	TheGame();
	~TheGame();
	void Update( float deltaSeconds );
	void Render();


private:
	void SetUpPerspectiveProjection();
	void ApplyCameraTransform();
	void SetupView3D();
	void Render3D();
	void RenderDebug3D();
	void SetupView2D();
	void Render2D();
	void RenderDebug2D();

	void RenderBlockHotbar();
	void RenderReticle();
	void RenderRightSideDebug2D();
	void RenderLeftSideDebug2D();

	void Render3DOverlayWithSprite( const AABB2& overlayTexCoords, bool onAllSides, bool enableBackfaceCulling = false, bool enableDepthTesting = false );
	void GetSelectedBlockBounds( AABB3* out_selectedBlockBounds );
	BlockFace GetSelectedBlockFace();

	Camera3D* m_playerCamera;
	Player* m_player;
	World* m_world;
};
