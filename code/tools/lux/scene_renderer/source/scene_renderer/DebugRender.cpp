#include "stdafx.h"

#include <scene_renderer/D3dWindow.h>

#include <scene_renderer/Common.h>
#include <scene_renderer/SinCos.h>
#include <scene_renderer/VertexBuffer.h>
#include <scene_renderer/DebugVB.h>
#include <scene_renderer/DebugRender.h>

#define D3DFVF_DEBUG3DVERTEX (D3DFVF_XYZ)
//#define D3DFVF_DEBUG3DVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE2(0))

using namespace SRD;

class Debug3dVertex
{
public:
    U32                                 GetFVF() {return D3DFVF_DEBUG3DVERTEX;}

	void                                SetPos(const Position3& pos) { m_X = pos.X(); m_Y = pos.Y(); m_Z = pos.Z(); }
	void                                SetPos(F32 vx, F32 vy, F32 vz) { m_X = vx; m_Y = vy; m_Z = vz; }

    void                                SetColour(Colour4 rgba) { /*m_R = rgba.GetX(), m_G = rgba.GetY(), m_B = rgba.GetZ(), m_A = rgba.GetW();*/ }
    void                                SetUV(F32 u, F32 v) {/* m_U = u; m_V = v; */}

public:
    F32                                 m_X, m_Y, m_Z;      // The untransformed, 3D position for the vertex
//    F32									m_R;				// The vertex color
    //F32									m_G;				// The vertex color
    //F32									m_B;				// The vertex color
    //F32									m_A;				// The vertex color
//    F32                                 m_U, m_V;
};

#define D3DFVF_DEBUG2DVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE2(0))

class Debug2dVertex
{
public:
    U32                                 GetFVF() {return D3DFVF_DEBUG2DVERTEX;}

	void                                SetPos(const Position3& pos) 
	{ 
		m_X = pos.X(); 
		m_Y = pos.Y(); 
		m_Z = pos.Z();
		m_RHW = 1.0f;
	}
	void                                SetPos(F32 vx, F32 vy, F32 vz, F32 vrhw) { m_X = vx; m_Y = vy; m_Z = vz; m_Z = vrhw; }

    void                                SetColour(const Colour4& rgba) { m_Colour = rgba; }
    void                                SetUV(F32 u, F32 v) { m_U = u; m_V = v; }

public:
    F32                                 m_X, m_Y, m_Z, m_RHW;      // The untransformed, 3D position for the vertex
    Colour4								m_Colour;      // The vertex color
    F32                                 m_U, m_V;
};

DebugRender::DebugRender()
{
    m_pD3dWindow = NULL;
}

void DebugRender::Init(D3dWindow* pD3dWindow)
{
    //dbPrint(DBSTR "Initialising Debug Render");
    m_pD3dWindow = pD3dWindow;

    m_DynamicVertexBuffer3D.Init(m_pD3dWindow, 200, 1);
    m_StaticVertexBuffer3D.Init(m_pD3dWindow, 18, 0);

    m_DynamicVertexBuffer2D.Init(m_pD3dWindow, 100, 1);


    // initialise the static vertex buffer for matrix render
	U32 no_lines = 9;

    Debug3dVertex* vertex;
	m_MatrixBaseVertex = m_StaticVertexBuffer3D.LockVertices(no_lines * 2, vertex);

	vertex->SetPos( 0.0f, 0.0f, 0.0f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	vertex->SetPos( 0.0f, 0.0f, 1.0f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	// z axis 'y' arrowhead
	vertex->SetPos( 0.0f, 0.0f, 1.0f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	vertex->SetPos( 0.0f, 0.2f, 1.0f - 0.3f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	vertex->SetPos( 0.0f, 0.0f, 1.0f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	vertex->SetPos( 0.0f, -0.2f, 1.0f - 0.3f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	// z axis 'x' arrowhead
	vertex->SetPos( 0.0f, 0.0f, 1.0f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	vertex->SetPos( 0.2f, 0.0f, 1.0f - 0.3f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	vertex->SetPos( 0.0f, 0.0f, 1.0f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	vertex->SetPos( -0.2f, 0.0f, 1.0f - 0.3f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

 

	//////////
	// x axis
	//////////
	vertex->SetPos( 0.0f, 0.0f, 0.0f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	vertex->SetPos( 1.0f, 0.0f, 0.0f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;



	//////////
	// y axis
	//////////
	vertex->SetPos( 0.0f, 0.0f, 0.0f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	vertex->SetPos( 0.0f, 1.0f, 0.0f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;


	// y axis 'x' arrowhead
	vertex->SetPos( 0.0f, 1.0f, 0.0f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	vertex->SetPos( 0.2f, 1.0f-0.3f, 0.0f);
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	vertex->SetPos( 0.0f, 1.0f, 0.0f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

	vertex->SetPos( -0.2f, 1.0f - 0.3f, 0.0f );
	vertex->SetColour( Colour4(0.0f,0.0f,0.0f,1.0f) );
	vertex++;

    m_StaticVertexBuffer3D.UnlockVertices();
}

void DebugRender::Draw2dLine(const Position2& start, const Position2& end, const Colour4& colour)
{
 /*   Debug2dVertex* pVertex;

	const U32 noLines = 1;
	const U32 baseVertex = m_DynamicVertexBuffer2D.LockVertices(noLines * 2, pVertex);

    pVertex->SetPos(m_pD3dWindow->Transform2dPoint(start));
	pVertex->SetColour(colour);
	pVertex++;

	pVertex->SetPos(m_DisplayWindowPtr->Transform2dPoint(end));
	pVertex->SetColour(colour);
	pVertex++;    

    m_DynamicVertexBuffer2D.UnlockVertices();

    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer2D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug2dVertex) );
    m_pD3dWindow->GetD3dDevicePtr()->SetFVF( ((Debug2dVertex*)NULL)->GetFVF() );
    m_pD3dWindow->GetD3dDevicePtr()->DrawPrimitive( D3DPT_LINELIST, baseVertex, noLines );
    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );
	*/
}

void DebugRender::Draw2dCircle(const Position2& centre, const F32 radius, const Colour4& colour)
{
/*    const S32 noSteps = 32;

    // OPT : use linestrip

    F32 deltaAngle = PI * 2.0f / (F32)noSteps;
    F32 currAngle = -deltaAngle;

    Position2 prevPos;

    {
        SinCosAlpha sc_angle( currAngle );

        prevPos.X( centre.GetX() + sc_angle.SinAlpha() * radius );
        prevPos.Y( centre.GetY() + sc_angle.CosAlpha() * radius );
    }

    S32 stepNo;
    for(stepNo=0; stepNo<noSteps; stepNo++)
    {
        currAngle = currAngle + deltaAngle;
        
        SinCosAlpha sc_angle( currAngle );

        Position2 currPos;

        currPos.X( centre.GetX() + sc_angle.SinAlpha() * radius);
        currPos.Y( centre.GetY() + sc_angle.CosAlpha() * radius);
    
        Draw2dLine( prevPos, currPos, colour);

        prevPos = currPos;
    }
	*/
}
/*
void DebugRender::Draw2dArrow(const Position2& start, const Position2& dir, const F32 arrowheadSize, const BizMaths::CVector4 colour)
{
    /*const Position2 end = start + dir;

    Draw2dLine(start, end, colour);

    // TODO : replace this with Position2 function
    //Position2 normDir = dir.Normalised();

    F32 len = dir.GetX() * dir.GetX() + dir.GetY() * dir.GetY();
    if(len > 0.0f) len = sqrtf(len); // TODO : replace with official sqrt?
    if(len > 0.0f) len = 1.0f / len;
    const Position2 normDir = dir * len;

    const Position2 perpDir(-normDir.GetY(), normDir.GetX());

    const Position2 arrowEndA = end - normDir * arrowheadSize + perpDir * arrowheadSize;
    Draw2dLine(end, arrowEndA, colour);

    const Position2 arrowEndB = end - normDir * arrowheadSize - perpDir * arrowheadSize;
    Draw2dLine(end, arrowEndB, colour);
}
*/

void DebugRender::Draw3dLine(const SRD::Position3& A, const SRD::Position3& B, const SRD::Colour4& colour)
{
    Debug3dVertex* pVertex;

	const U32 noLines = 1;
	const U32 baseVertex = m_DynamicVertexBuffer3D.LockVertices(noLines * 2, pVertex);

	pVertex->SetPos(A);
	pVertex->SetColour(colour);
	pVertex++;

	pVertex->SetPos(B);
	pVertex->SetColour(colour);
	pVertex++;    

    m_DynamicVertexBuffer3D.UnlockVertices();

    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3dWindow->GetD3dDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3dWindow->GetD3dDevicePtr()->DrawPrimitive( D3DPT_LINELIST, baseVertex, noLines );
    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );
}

void DebugRender::Draw3dLine(const Position3& A, const Position3& B, const Colour4& Acolour, const Colour4& Bcolour)
{
    Debug3dVertex* pVertex;

	const U32 noLines = 1;
	const U32 baseVertex = m_DynamicVertexBuffer3D.LockVertices(noLines * 2, pVertex);

	pVertex->SetPos(A);
	pVertex->SetColour(Acolour);
	pVertex++;

	pVertex->SetPos(B);
	pVertex->SetColour(Bcolour);
	pVertex++;    

    m_DynamicVertexBuffer3D.UnlockVertices();

    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3dWindow->GetD3dDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3dWindow->GetD3dDevicePtr()->DrawPrimitive( D3DPT_LINELIST, baseVertex, noLines );
    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );
}

void DebugRender::Draw3dTriangle(const Position3& A, const Position3& B, const Position3& C, const Colour4& colour)
{
    Debug3dVertex* pVertex;

	const U32 noTriangles = 1;
	const U32 baseVertex = m_DynamicVertexBuffer3D.LockVertices(noTriangles * 3, pVertex);

	pVertex->SetPos(A);
	pVertex->SetColour(colour);
	pVertex++;

	pVertex->SetPos(B);
	pVertex->SetColour(colour);
	pVertex++;    

	pVertex->SetPos(C);
	pVertex->SetColour(colour);
	pVertex++;    

    m_DynamicVertexBuffer3D.UnlockVertices();

    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3dWindow->GetD3dDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3dWindow->GetD3dDevicePtr()->DrawPrimitive( D3DPT_TRIANGLELIST, baseVertex, noTriangles );
    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );
}

void DebugRender::Draw3dTriangle(const Position3& A, const Position3& B, const Position3& C, const Colour4& Acolour, const Colour4& Bcolour, const Colour4& Ccolour)
{
    Debug3dVertex* pVertex;

	const U32 noTriangles = 1;
	const U32 baseVertex = m_DynamicVertexBuffer3D.LockVertices(noTriangles * 3, pVertex);

	pVertex->SetPos(A);
	pVertex->SetColour(Acolour);
	pVertex++;

	pVertex->SetPos(B);
	pVertex->SetColour(Bcolour);
	pVertex++;    

	pVertex->SetPos(C);
	pVertex->SetColour(Ccolour);
	pVertex++;    

    m_DynamicVertexBuffer3D.UnlockVertices();

    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3dWindow->GetD3dDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3dWindow->GetD3dDevicePtr()->DrawPrimitive( D3DPT_TRIANGLELIST, baseVertex, noTriangles );
    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );
}

void DebugRender::Draw3dPoint(const Position3& pos, const F32 size, const Colour4& colour)
{
//   const U32 cull_flags = m_DisplayWindowPtr->m_ViewFrustrum.SphereCull(pos, size);
//	if(!CULL_VISIBLE(cull_flags)) return;

    Debug3dVertex* pVertex;

	const U32 noLines = 3;
	const U32 baseVertex = m_DynamicVertexBuffer3D.LockVertices(noLines * 2, pVertex);

    F32 x = pos.X();
	F32 y = pos.Y();
	F32 z = pos.Z();

	pVertex->SetPos( x, y, z + size );
	pVertex->SetColour( colour );
	pVertex++;    

	pVertex->SetPos( x, y, z - size );
	pVertex->SetColour( colour );
	pVertex++;    


	pVertex->SetPos( x, y + size, z );
	pVertex->SetColour( colour );
	pVertex++;    

	pVertex->SetPos( x, y - size, z );
	pVertex->SetColour( colour );
	pVertex++;    


	pVertex->SetPos( x + size, y, z );
	pVertex->SetColour( colour );
	pVertex++;    

	pVertex->SetPos( x - size, y, z );
	pVertex->SetColour( colour );
	pVertex++;    

    m_DynamicVertexBuffer3D.UnlockVertices();

    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3dWindow->GetD3dDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3dWindow->GetD3dDevicePtr()->DrawPrimitive( D3DPT_LINELIST, baseVertex, noLines );
    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );
}

void DebugRender::Draw3dCross(const Position3& pos, const F32 size, const Colour4& colour)
{
//    const U32 cull_flags = m_DisplayWindowPtr->m_ViewFrustrum.SphereCull(pos, size);
//	if(!CULL_VISIBLE(cull_flags)) return;

    Debug3dVertex* pVertex;

	const U32 noLines = 4;
	const U32 baseVertex = m_DynamicVertexBuffer3D.LockVertices(noLines * 2, pVertex);

    F32 x = pos.X();
	F32 y = pos.Y();
	F32 z = pos.Z();

	pVertex->SetPos( x + size, y + size, z + size );
	pVertex->SetColour( colour );
	pVertex++;    

	pVertex->SetPos( x - size, y - size, z - size );
	pVertex->SetColour( colour );
	pVertex++;    


	pVertex->SetPos( x - size, y + size, z + size );
	pVertex->SetColour( colour );
	pVertex++;    

	pVertex->SetPos( x + size, y - size, z - size );
	pVertex->SetColour( colour );
	pVertex++;    


	pVertex->SetPos( x + size, y - size, z + size );
	pVertex->SetColour( colour );
	pVertex++;    

	pVertex->SetPos( x - size, y + size, z - size );
	pVertex->SetColour( colour );
	pVertex++;    


	pVertex->SetPos( x + size, y + size, z - size );
	pVertex->SetColour( colour );
	pVertex++;    

	pVertex->SetPos( x - size, y - size, z + size );
	pVertex->SetColour( colour );
	pVertex++;    

    m_DynamicVertexBuffer3D.UnlockVertices();

    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3dWindow->GetD3dDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3dWindow->GetD3dDevicePtr()->DrawPrimitive( D3DPT_LINELIST, baseVertex, noLines );
    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );
}

void DebugRender::Draw3dStar(const Position3& pos, const F32 size, const Colour4& colour)
{
	Draw3dPoint(pos, size, colour);
	Draw3dCross(pos, size * 0.57735f, colour);
}

void DebugRender::Draw3dPlanePoint(const Position3& pos, const Normal3& normal, const F32 size, const Colour4& colour)
{
    // TODO : implement Draw3dPlanePoint
    //dbWarn(DBSTR "Draw3dPlanePoint not implemented");
}

void DebugRender::Draw3dPlaneCross(const Position3& pos, const Normal3& normal, const F32 size, const Colour4& colour)
{
    // TODO : implement Draw3dPlaneCross
    //dbWarn(DBSTR "Draw3dPlaneCross not implemented");
}

void DebugRender::Draw3dPlaneStar(const Position3& pos, const Normal3& normal, const F32 size, const Colour4& colour)
{
    // TODO : implement Draw3dPlaneStar
    //dbWarn(DBSTR "Draw3dPlaneStar not implemented");
}

void DebugRender::Draw3dArrow(const Position3& pos, const Position3& dir, const F32 length_scale, const F32 arrowhead_size, const Colour4& colour)
{
    // TODO : implement Draw3dArrow
    //dbWarn(DBSTR "Draw3dArrow not implemented");
}

void DebugRender::Draw3dCircle(const Position3& centre, const Normal3& normal, const F32 radius, const Colour4& colour)
{
	/*
//    const U32 cull_flags = m_DisplayWindowPtr->m_ViewFrustrum.SphereCull(centre, radius);
//	if(!CULL_VISIBLE(cull_flags)) return;

	S32 noPoints = 32; 

    // TODO : implement PerpendicularAxes()
	//normal.PerpendicularAxes( u, v );
    Position3 u(1,0,0);
	Position3 v(0,1,0);
    
	F32 deltaTheta = (PI * 2.0f) / (F32)noPoints;

    Debug3dVertex* pVertex;
	const U32 baseVertex = m_DynamicVertexBuffer3D.LockVertices(noPoints + 1, pVertex);

	for ( S32 i=0; i<=noPoints; i++ )
	{
		F32 theta = i * deltaTheta;
        
		SinCosAlpha angles( theta );
        
        pVertex->SetPos(centre + (u * angles.SinAlpha() + v * angles.CosAlpha()) * radius);
        pVertex->SetColour(colour);
        pVertex++;
	}
    
    m_DynamicVertexBuffer3D.UnlockVertices();

    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3dWindow->GetD3dDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3dWindow->GetD3dDevicePtr()->DrawPrimitive( D3DPT_LINESTRIP, baseVertex, noPoints );
    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );
	*/
}

void DebugRender::Draw3dSphere(const Position3& centre, const F32 radius, const Colour4& colour)
{
//    const U32 cull_flags = m_DisplayWindowPtr->m_ViewFrustrum.SphereCull(centre, radius);
//	if(!CULL_VISIBLE(cull_flags)) return;

//    Draw3dCircle(centre, Position3(1,0,0), radius, colour);
  //  Draw3dCircle(centre, Position3(0,1,0), radius, colour);
	//Draw3dCircle(centre, Position3(0,0,1), radius, colour);
}

void DebugRender::Draw3dSphereOutline(const Position3& centre, const F32 radius, const Colour4& colour)
{
    // TODO : implement Draw3dSphereOutline
    //dbWarn(DBSTR "Draw3dSphereOutline not implemented");
}
/*
void DebugRender::Draw3dMatrix(const Matrix44& m)
{
//    const U32 cull_flags = m_DisplayWindowPtr->m_ViewFrustrum.SphereCull(m.AW(), 1.0f);
//	if(!CULL_VISIBLE(cull_flags)) return;

	const U32 noLines = 9;

	m_pD3dWindow->GetD3dDevicePtr()->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&m);

    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, m_StaticVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3dWindow->GetD3dDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3dWindow->GetD3dDevicePtr()->DrawPrimitive( D3DPT_LINELIST, m_MatrixBaseVertex, noLines );
    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );

	Matrix44 id_m;
	id_m.SetIdentity();
	m_pD3dWindow->GetD3dDevicePtr()->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&id_m);
}
*/
void DebugRender::Draw3dBox(const Position3& min, const Position3& max, const Matrix44& m, const Colour4& colour)
{
	/*
	Position4 com = ((min + max) * 0.5f);
	com = com * m;
	const Vector4F32 diag = (max - min);
	const F32 radius = UtMaths::Magnitude(diag) * 0.5f;

//    const U32 cull_flags = m_DisplayWindowPtr->m_ViewFrustrum.SphereCull(com, radius);
//	if(!CULL_VISIBLE(cull_flags)) return;

	Position3 pos[8];

	pos[0] = Position3(min.X(), min.Y(), min.Z());
	pos[1] = Position3(max.X(), min.Y(), min.Z());
	pos[2] = Position3(min.X(), min.Y(), max.Z());
	pos[3] = Position3(max.X(), min.Y(), max.Z());

	pos[4] = Position3(min.X(), max.Y(), min.Z());
	pos[5] = Position3(max.X(), max.Y(), min.Z());
	pos[6] = Position3(min.X(), max.Y(), max.Z());
	pos[7] = Position3(max.X(), max.Y(), max.Z());

    Debug3dVertex* pVertex;

	const U32 noLines = 12;
	const U32 baseVertex = m_DynamicVertexBuffer3D.LockVertices(noLines * 2, pVertex);

    // BASE
	
	pVertex->SetPos( pos[0] );
	pVertex->SetColour( colour );
	pVertex++;
	
	pVertex->SetPos( pos[1] );
	pVertex->SetColour( colour );
	pVertex++;

	////

	pVertex->SetPos( pos[2] );
	pVertex->SetColour( colour );
	pVertex++;
	
	pVertex->SetPos( pos[3] );
	pVertex->SetColour( colour );
	pVertex++;

	////

	pVertex->SetPos( pos[0] );
	pVertex->SetColour( colour );
	pVertex++;

	pVertex->SetPos( pos[2] );
	pVertex->SetColour( colour );
	pVertex++;

	////

	pVertex->SetPos( pos[1] );
	pVertex->SetColour( colour );
	pVertex++;

	pVertex->SetPos( pos[3] );
	pVertex->SetColour( colour );
	pVertex++;

	////
	// TOP
	////

	pVertex->SetPos( pos[4] );
	pVertex->SetColour( colour );
	pVertex++;

	pVertex->SetPos( pos[5] );
	pVertex->SetColour( colour );
	pVertex++;

	////

	pVertex->SetPos( pos[6] );
	pVertex->SetColour( colour );
	pVertex++;

	pVertex->SetPos( pos[7] );
	pVertex->SetColour( colour );
	pVertex++;

	////

	pVertex->SetPos( pos[4] );
	pVertex->SetColour( colour );
	pVertex++;

	pVertex->SetPos( pos[6] );
	pVertex->SetColour( colour );
	pVertex++;

	////

	pVertex->SetPos( pos[5] );
	pVertex->SetColour( colour );
	pVertex++;

	pVertex->SetPos( pos[7] );
	pVertex->SetColour( colour );
	pVertex++;

	////
	//	VERTICALS
	////
	
	pVertex->SetPos( pos[0] );
	pVertex->SetColour( colour );
	pVertex++;
	
	pVertex->SetPos( pos[4] );
	pVertex->SetColour( colour );
	pVertex++;

	////
	
	pVertex->SetPos( pos[1] );
	pVertex->SetColour( colour );
	pVertex++;
	
	pVertex->SetPos( pos[5] );
	pVertex->SetColour( colour );
	pVertex++;

	////
	
	pVertex->SetPos( pos[2] );
	pVertex->SetColour( colour );
	pVertex++;
	
	pVertex->SetPos( pos[6] );
	pVertex->SetColour( colour );
	pVertex++;

	////
	
	pVertex->SetPos( pos[3] );
	pVertex->SetColour( colour );
	pVertex++;
	
	pVertex->SetPos( pos[7] );
	pVertex->SetColour( colour );
	pVertex++;

    m_DynamicVertexBuffer3D.UnlockVertices();

	m_pD3dWindow->GetD3dDevicePtr()->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&m);

    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3dWindow->GetD3dDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3dWindow->GetD3dDevicePtr()->DrawPrimitive( D3DPT_LINELIST, baseVertex, noLines );
    m_pD3dWindow->GetD3dDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );

    Matrix44 id_m;
	UtMaths::SetIdentity(&id_m);

	m_pD3dWindow->GetD3dDevicePtr()->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&id_m);
	*/
}

void DebugRender::Draw3dGrid(const Matrix44& m)
{
//    const U32 cull_flags = m_DisplayWindowPtr->m_ViewFrustrum.SphereCull(m.AW(), 7.071f);
//	if(!CULL_VISIBLE(cull_flags)) return;
/*
	F32 x;

	for(x=-10; x<=10; x++)
	{
		if(x == 0.0f)
		{
			Draw3dLine( 
				Position4(x, 0.0f, -10.0f, 1.0f) * m, 
				Position4(x, 0.0f, 10.0f, 1.0f) * m, 
				Colour4(0.0f, 0.0f, 0.0f, 1.0f) );

			Draw3dLine( 
				Position4(-10.0f, 0.0f, x, 1.0f) * m, 
				Position4(10.0f, 0.0f, x, 1.0f) * m, 
				Colour4(0.0f, 0.0f, 0.0f, 1.0f) );
		}
		else
		{
			Draw3dLine( 
				Position4(x, 0.0f, -10.0f, 1.0f) * m,
				Position4(x, 0.0f, 10.0f, 1.0f) * m, 
				Colour4(0.0f, 0.0f, 0.0f, 1.0f) );

			Draw3dLine( 
				Position4(-10.0f, 0.0f, x, 1.0f) * m,
				Position4(10.0f, 0.0f, x, 1.0f) * m, 
				Colour4(0.0f, 0.0f, 0.0f, 1.0f) );
		}
	}    
	*/
}

void DebugRender::Close()
{
    //dbPrint(DBSTR "Close Debug Render");

    m_StaticVertexBuffer3D.Close();
    m_DynamicVertexBuffer3D.Close();
    m_DynamicVertexBuffer2D.Close();

    m_pD3dWindow = NULL;
}


DebugRender::~DebugRender()
{
    if (m_pD3dWindow != NULL)
    {
        //dbWarn(DBSTR "DebugRender not closed");
        Close();
    }
}


