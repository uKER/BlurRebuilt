#include <lighting and pvs application/stdafx.h>
#include <lighting and pvs application/MainWindow.h>
#include <lighting and pvs application/D3DUtils.h>

#include <scene_renderer/SceneRendererHeaders.h>

#include <lighting and pvs application/D3DBaseCode.h>
#include <lighting and pvs application/GeneralUtils.h>
#include <lighting and pvs application/LPVSApplication.h>

#include <common/BizMaths.h>
//#include <utility/maths.h>
//#include <common/CVector3Wrapper.h>

#include <lighting and pvs application/D3DPrimitives3D.h>

using namespace LPVS;
using namespace BizMaths;

void Draw3dLineSetup()
{
	#ifdef _DEBUG
//	if(!gDisplay.InSceneRender()) gDebug.Error( "Draw3dLineSetup() called in update - use gDebugRender" );
	#endif

//	gStates.default_3d_solid.Set();
	LPVSAPP.GetD3DDevice()->SetTexture(0, NULL);
}


void Draw3dLineNext( const CVec4 A, const CVec4 B, CVec4 rgbaColour )
{
	
	#ifdef _DEBUG
//	if(!gDisplay.InSceneRender()) gDebug.Error( "Draw3dLineNext() called in update - use gDebugRender" );
	#endif
    
//	LPVSAPP.GetD3DDevice()->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );
/*
	LPVSAPP.GetD3DDevice()->DrawPrimitive(D3DPT_LINELIST, 0, 1);
	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , A.GetX(), A.GetY(), A.GetZ(), 1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , B.GetX(), B.GetY(), B.GetZ(), 1.0f );
*/
}
/*
void Draw3dLine(const CVec4 A, const CVec4 B, CVec4 rgbaColour)
{
	#ifdef _DEBUG
	//if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dLine() called in update - use gDebugRender");
	#endif

	LPVSAPP.GetD3DDevice()->SetTexture(0, NULL);

	//gStates.default_3d_solid.Set();

	LPVSAPP.GetD3DDevice()->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );

	LPVSAPP.GetD3DDevice()->Begin( D3DPT_LINELIST );
	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , A.GetX(), A.GetY(), A.GetZ(), 1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , B.GetX(), B.GetY(), B.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->End();
}

void Draw3dLine(const CVec4 A, const CVec4 B, RGBA Acolour, RGBA Bcolour)
{
	#ifdef _DEBUG
	//if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dLine() called in update - use gDebugRender");
	#endif

//	gStates.default_3d_solid.Set();
	LPVSAPP.GetD3DDevice()->SetTexture(0, NULL);


	LPVSAPP.GetD3DDevice()->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );

	LPVSAPP.GetD3DDevice()->Begin( D3DPT_LINELIST );

	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, Acolour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , A.GetX(), A.GetY(), A.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, Bcolour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , B.GetX(), B.GetY(), B.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->End();
}

void Draw3dTriangle(const CVec4 A, const CVec4 B, const CVec4 C, RGBA Acolour, RGBA Bcolour, RGBA Ccolour)
{
	#ifdef _DEBUG
//	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dTriangle() called in update - use gDebugRender");
	#endif

	LPVSAPP.GetD3DDevice()->SetTexture(0, NULL);

//	gStates.default_3d_solid.Set();

	LPVSAPP.GetD3DDevice()->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );

	LPVSAPP.GetD3DDevice()->Begin( D3DPT_TRIANGLELIST );

	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, Acolour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , A.GetX(), A.GetY(), A.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, Bcolour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , B.GetX(), B.GetY(), B.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, Ccolour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , C.GetX(), C.GetY(), C.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->End();
}

void Draw3dTriangle(const CVec4 A, const CVec4 B, const CVec4 C, RGBA colour)
{
	#ifdef _DEBUG
//	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dTriangle() called in update - use gDebugRender");
	#endif

	LPVSAPP.GetD3DDevice()->SetTexture(0, NULL);


//	gStates.default_3d_solid.Set();

	LPVSAPP.GetD3DDevice()->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );

	LPVSAPP.GetD3DDevice()->Begin( D3DPT_TRIANGLELIST );
	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , A.GetX(), A.GetY(), A.GetZ(), 1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , B.GetX(), B.GetY(), B.GetZ(), 1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , C.GetX(), C.GetY(), C.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->End();
}




void Draw3dMatrix(const CMat44& m)
{
	#ifdef _DEBUG
//	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dMatrix() called in update - use gDebugRender");
	#endif

	CVec4 pos = m.GetAW();
	U32 cull_flags = gSceneSettings.view_volume.SphereCull(pos, 1.732f);
	if(!CULL_VISIBLE(cull_flags)) return;

	LPVSAPP.GetD3DDevice()->SetTexture(0, NULL);

//	gStates.default_3d_solid.Set();

	// Set the matrix
	LPVSAPP.GetD3DDevice()->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&m);

	LPVSAPP.GetD3DDevice()->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );

	LPVSAPP.GetD3DDevice()->Begin( D3DPT_LINELIST );

	RGBA x_col(0xff, 0x00, 0x00, 0x80);
	RGBA y_col(0x00, 0xff, 0x00, 0x80);
	RGBA z_col(0x00, 0x00, 0xff, 0x80);

	// z axis

	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, z_col.rgba );

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.0f, 0.0f, 0.0f,		1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.0f, 0.0f, 1.0f,		1.0f );

	// z axis 'y' arrowhead
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.0f, 0.0f, 1.0f,		1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.0f, 0.2f, 0.7f,		1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.0f, 0.0f, 1.0f,		1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.0f,-0.2f, 0.7f,		1.0f );

	// z axis 'x' arrowhead
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.0f, 0.0f, 1.0f,		1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.2f, 0.0f, 0.7f,		1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.0f, 0.0f, 1.0f,		1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	-0.2f, 0.0f, 0.7f,		1.0f );


	// x axis

	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, x_col.rgba );

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.0f, 0.0f, 0.0f,		1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	1.0f, 0.0f, 0.0f,		1.0f );

	// y axis

	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, y_col.rgba );
	
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.0f, 0.0f, 0.0f,		1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.0f, 1.0f, 0.0f,		1.0f );

	// y axis 'x' arrowhead

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.0f, 1.0f, 0.0f,		1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.3f, 0.7f, 0.0f,		1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	0.0f, 1.0f, 0.0f,		1.0f );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX ,	-0.3f, 0.7f, 0.0f,		1.0f );

	LPVSAPP.GetD3DDevice()->End();

	// restore identity matrix
	LPVSAPP.GetD3DDevice()->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&gSceneSettings.id_m);
}

void Draw3dBox(const CVec4 min, const CVec4 max, const CMat44& m, RGBA colour)
{
	#ifdef _DEBUG
//	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dBox() called in update - use gDebugRender");
	#endif

	CVec4 com = (min + max) * 0.5f;
	CVec4 diag = (max - min);

	F32 radius = diag.MagnitudeXYZ() * 0.5f;

	com.ApplyRotTransMatrixTo(m);
	U32 cull_flags = gSceneSettings.view_volume.SphereCull(com, radius);
	if(!CULL_VISIBLE(cull_flags)) return;

	LPVSAPP.GetD3DDevice()->SetTexture(0, NULL);

	gStates.default_3d_solid.Set();

	// Set the matrix
	LPVSAPP.GetD3DDevice()->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&m);

	LPVSAPP.GetD3DDevice()->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );

	LPVSAPP.GetD3DDevice()->Begin( D3DPT_LINELIST );

	// BASE 

	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , min.GetX(), min.GetY(), min.GetZ(), 1.0f );
	
	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , max.GetX(), min.GetY(), min.GetZ(), 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , min.GetX(), min.GetY(), max.GetZ(), 1.0f );
	
	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , max.GetX(), min.GetY(), max.GetZ(), 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , min.GetX(), min.GetY(), min.GetZ(), 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , min.GetX(), min.GetY(), max.GetZ(), 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , max.GetX(), min.GetY(), min.GetZ(), 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , max.GetX(), min.GetY(), max.GetZ(), 1.0f );

	// TOP 

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , min.GetX(), max.GetY(), min.GetZ(), 1.0f );
	
	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , max.GetX(), max.GetY(), min.GetZ(), 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , min.GetX(), max.GetY(), max.GetZ(), 1.0f );
	
	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , max.GetX(), max.GetY(), max.GetZ(), 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , min.GetX(), max.GetY(), min.GetZ(), 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , min.GetX(), max.GetY(), max.GetZ(), 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , max.GetX(), max.GetY(), min.GetZ(), 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , max.GetX(), max.GetY(), max.GetZ(), 1.0f );

	// VERTICALS

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , min.GetX(), min.GetY(), min.GetZ(), 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , min.GetX(), max.GetY(), min.GetZ(), 1.0f );

	//
	
	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , max.GetX(), min.GetY(), min.GetZ(), 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , max.GetX(), max.GetY(), min.GetZ(), 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , min.GetX(), min.GetY(), max.GetZ(), 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , min.GetX(), max.GetY(), max.GetZ(), 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , max.GetX(), min.GetY(), max.GetZ(), 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , max.GetX(), max.GetY(), max.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->End();

	// restore identity matrix
	LPVSAPP.GetD3DDevice()->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&gSceneSettings.id_m);
}


void Draw3dCross(const CVec4 pos, F32 size, RGBA colour)
{
	#ifdef _DEBUG
	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dCross() called in update - use gDebugRender");
	#endif

	U32 cull_flags = gSceneSettings.view_volume.SphereCull(pos, size * 1.732f);
	if(!CULL_VISIBLE(cull_flags)) return;

	LPVSAPP.GetD3DDevice()->SetTexture(0, NULL);

	gStates.default_3d_solid.Set();

	LPVSAPP.GetD3DDevice()->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );

	LPVSAPP.GetD3DDevice()->Begin( D3DPT_LINELIST );

	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX() + size, pos.GetY() + size, pos.GetZ() + size, 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX() - size, pos.GetY() - size, pos.GetZ() - size, 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX() - size, pos.GetY() + size, pos.GetZ() + size, 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX() + size, pos.GetY() - size, pos.GetZ() - size, 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX() + size, pos.GetY() - size, pos.GetZ() + size, 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX() - size, pos.GetY() + size, pos.GetZ() - size, 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX() + size, pos.GetY() + size, pos.GetZ() - size, 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX() - size, pos.GetY() - size, pos.GetZ() + size, 1.0f );

	LPVSAPP.GetD3DDevice()->End();
}


void Draw3dPoint(const CVec4 pos, F32 size, RGBA colour)
{
	#ifdef _DEBUG
	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dPoint() called in update - use gDebugRender");
	#endif

	U32 cull_flags = gSceneSettings.view_volume.SphereCull(pos, size);
	if(!CULL_VISIBLE(cull_flags)) return;

	LPVSAPP.GetD3DDevice()->SetTexture(0, NULL);

	gStates.default_3d_solid.Set();

	LPVSAPP.GetD3DDevice()->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );

	LPVSAPP.GetD3DDevice()->Begin( D3DPT_LINELIST );

	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX() + size, pos.GetY(), pos.GetZ(), 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX() - size, pos.GetY(), pos.GetZ(), 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX(), pos.GetY() + size, pos.GetZ(), 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX(), pos.GetY() - size, pos.GetZ(), 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX(), pos.GetY(), pos.GetZ() + size, 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX(), pos.GetY(), pos.GetZ() - size, 1.0f );

	//

	LPVSAPP.GetD3DDevice()->End();
}

void Draw3dStar(const CVec4 pos, F32 size, RGBA colour)
{
	#ifdef _DEBUG
	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dStar() called in update - use gDebugRender");
	#endif

	Draw3dPoint(pos, size, colour);
	Draw3dCross(pos, size * 0.57735f, colour);
}

void Draw3dArrow(const CVec4 pos, const CVec4 dir, F32 length_scale, F32 arrowhead_size, RGBA colour)
{
	#ifdef _DEBUG
	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dArrow() called in update - use gDebugRender");
	#endif

	CVec4 norm_dir = dir;
	norm_dir.NormaliseXYZ();
	
	CVec4 arrow_end = pos + dir * length_scale;
	CVector4 up(0.0f, 1.0f, 0.0f, 0.0f);
	CVec4 perp;

	if(norm_dir.MagnitudeXZ() < FLT_EPSILON)
	{
		up = CVector4(1.0f, 0.0f, 0.0f);
	}

	perp = norm_dir.CrossNormaliseXYZ( up );

	CVec4 arrow_tip_A = arrow_end - norm_dir * arrowhead_size + perp * arrowhead_size;
	CVec4 arrow_tip_B = arrow_end - norm_dir * arrowhead_size - perp * arrowhead_size;

	LPVSAPP.GetD3DDevice()->SetTexture(0, NULL);

	gStates.default_3d_solid.Set();

	LPVSAPP.GetD3DDevice()->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );

	LPVSAPP.GetD3DDevice()->Begin( D3DPT_LINELIST );

	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX(), pos.GetY(), pos.GetZ(), 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , arrow_end.GetX(), arrow_end.GetY(), arrow_end.GetZ(), 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , arrow_end.GetX(), arrow_end.GetY(), arrow_end.GetZ(), 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , arrow_tip_A.GetX(), arrow_tip_A.GetY(), arrow_tip_A.GetZ(), 1.0f );

	//

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , arrow_end.GetX(), arrow_end.GetY(), arrow_end.GetZ(), 1.0f );

	//LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );
	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , arrow_tip_B.GetX(), arrow_tip_B.GetY(), arrow_tip_B.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->End();
}

void Draw3dCircle(const CVec4 centre, const CVec4 normal, float radius, RGBA colour)
{
	#ifdef _DEBUG
	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dCircle() called in update - use gDebugRender");
	#endif
    
	U32 cull_flags = gSceneSettings.view_volume.SphereCull(centre, radius);
	if(!CULL_VISIBLE(cull_flags)) return;
    
	S32 no_points = 32; // 8

	LPVSAPP.GetD3DDevice()->SetTexture(0, NULL);

	gStates.default_3d_solid.Set();

	LPVSAPP.GetD3DDevice()->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );

	LPVSAPP.GetD3DDevice()->Begin( D3DPT_LINELOOP );
	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );

	CVec4 u, v;
    
	normal.PerpendicularAxes( u, v );
    
	F32 delta_theta = (PI * 2.0f) / (F32)no_points;
    
	for ( int i=0; i<no_points; i++ )
	{
		F32 theta = i * delta_theta;
        
		SinCosAlpha angles( theta );
        
		CVec4 pos = centre + (u * angles.SinAlpha() + v * angles.CosAlpha()) * radius;

		LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , pos.GetX(), pos.GetY(), pos.GetZ(), 1.0f );

	}

	LPVSAPP.GetD3DDevice()->End();
}

void Draw3dSphereOutline(const CVec4 centre, float radius, RGBA colour)
{
	#ifdef _DEBUG
	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dPlane() called in update - use gDebugRender");
	#endif

	U32 cull_flags = gSceneSettings.view_volume.SphereCull(centre, radius);
	if(!CULL_VISIBLE(cull_flags)) return;

	float dist;

    float sintheta;
    float costheta;
	CVec4 cam_offset;

	cam_offset = centre - gSceneSettings.camera.world_matrix.GetAW();
    
	dist = cam_offset.NormaliseGetMagnitudeXYZ();

    if(dist < radius)
        return;
    
    sintheta = radius / dist;
    costheta = fastsqrt( 1.0f - (sintheta * sintheta));

    float hcl = radius * costheta;
    float backdist = radius * sintheta;
    
    CVec4 pos;
    
    pos = centre + cam_offset * -backdist;
    Draw3dCircle(pos, cam_offset, hcl, colour);
}

void Draw3dSphere(const CVec4 centre, float radius, RGBA colour)
{
	#ifdef _DEBUG
	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dSphere() called in update - use gDebugRender");
	#endif

	U32 cull_flags = gSceneSettings.view_volume.SphereCull(centre, radius);
	if(!CULL_VISIBLE(cull_flags)) return;

    //CVec4 n;
    
    //n = CVector4(1.0f, 0.0f, 0.0f);
    Draw3dCircle(centre, CVector4UnitAxisX(), radius, colour);
    
    //n = CVector4(0.0f, 1.0f, 0.0f);
    Draw3dCircle(centre, CVector4UnitAxisY(), radius, colour);
    
    //n = CVector4(0.0f, 0.0f, 1.0f);
    Draw3dCircle(centre, CVector4UnitAxisZ(), radius, colour);
}

void Draw3dPlanePoint(const CVec4 pos, const CVec4 normal, F32 size, RGBA colour)
{
	#ifdef _DEBUG
	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dPlanePoint() called in update - use gDebugRender");
	#endif

	U32 cull_flags = gSceneSettings.view_volume.SphereCull(pos, size);
	if(!CULL_VISIBLE(cull_flags)) return;

	CVec4 u, v;
	normal.PerpendicularAxes( u, v );

	LPVSAPP.GetD3DDevice()->SetTexture(0, NULL);

	gStates.default_3d_solid.Set();

	LPVSAPP.GetD3DDevice()->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );

	LPVSAPP.GetD3DDevice()->Begin( D3DPT_LINELIST );
	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );

	CVec4 A, B;

	A = pos + u * size;
	B = pos - u * size;

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , A.GetX(), A.GetY(), A.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , B.GetX(), B.GetY(), B.GetZ(), 1.0f );

	A = pos + v * size;
	B = pos - v * size;

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , A.GetX(), A.GetY(), A.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , B.GetX(), B.GetY(), B.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->End();
}

void Draw3dPlaneCross(const CVec4 pos, const CVec4 normal, F32 size, RGBA colour)
{
	#ifdef _DEBUG
	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dPlanePoint() called in update - use gDebugRender");
	#endif

	U32 cull_flags = gSceneSettings.view_volume.SphereCull(pos, size * 1.415f);
	if(!CULL_VISIBLE(cull_flags)) return;

	CVec4 u, v;
	normal.PerpendicularAxes( u, v );

	LPVSAPP.GetD3DDevice()->SetTexture(0, NULL);

	gStates.default_3d_solid.Set();

	LPVSAPP.GetD3DDevice()->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );

	LPVSAPP.GetD3DDevice()->Begin( D3DPT_LINELIST );
	LPVSAPP.GetD3DDevice()->SetVertexDataColor( D3DVSDE_DIFFUSE, colour.rgba );

	CVec4 A, B;

	A = pos + u * size + v * size;
	B = pos - u * size - v * size;

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , A.GetX(), A.GetY(), A.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , B.GetX(), B.GetY(), B.GetZ(), 1.0f );

	A = pos + u * size - v * size;
	B = pos - u * size + v * size;

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , A.GetX(), A.GetY(), A.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->SetVertexData4f( D3DVSDE_VERTEX , B.GetX(), B.GetY(), B.GetZ(), 1.0f );

	LPVSAPP.GetD3DDevice()->End();
}

void Draw3dPlaneStar(const CVec4 pos, const CVec4 normal, F32 size, RGBA colour)
{
	#ifdef _DEBUG
	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dPlane() called in update - use gDebugRender");
	#endif

	Draw3dPlanePoint(pos, normal, size, colour);	
	Draw3dPlaneCross(pos, normal, size * 0.7071f, colour);	
}


void Draw3dPlane(const CPlane& plane, const CMat44& m, F32 plane_size, F32 normal_length, RGBA colour)
{
	#ifdef _DEBUG
	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dPlane() called in update - use gDebugRender");
	#endif

	CVec4 normal = plane.GetNormal();
	CVec4 pos = normal * plane.GetD();

	pos.ApplyRotTransMatrixTo(m);
	normal.ApplyRotMatrixTo(m);

	if(plane_size > 0.0f)
		Draw3dPlaneStar(pos, normal, plane_size, colour);

	if(normal_length > 0.0f)
		Draw3dArrow(pos, normal, normal_length, normal_length * 0.2f, colour);
}

void Draw3dPlane(const CPlane& plane, const CVec4 closest_pos, const CMat44& m, F32 plane_size, F32 normal_length, RGBA colour)
{
	#ifdef _DEBUG
	if(!gDisplay.InSceneRender()) gDebug.Error("Draw3dPlane() called in update - use gDebugRender");
	#endif

	CVec4 pos;
	CVec4 normal = plane.GetNormal();

	// find closest point on plane
	F32 dist_to_plane = plane.DistanceTo(closest_pos);

	pos = closest_pos + normal * (-dist_to_plane);

	pos.ApplyRotTransMatrixTo(m);
	normal.ApplyRotMatrixTo(m);

	if(plane_size > 0.0f)
		Draw3dPlaneStar(pos, normal, plane_size, colour);

	if(normal_length > 0.0f)
		Draw3dArrow(pos, normal, normal_length, normal_length * 0.2f, colour);
}
 */