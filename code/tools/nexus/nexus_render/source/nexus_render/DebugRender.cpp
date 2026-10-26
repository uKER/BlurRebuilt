#include <nexus_render/stdafx.h>
#include <nexus_render/Primitive3DVertex.h>
#include <nexus_render/VBWrapper.h>
#include <nexus_render/DebugRender.h>

using namespace NexusRender;

DebugRender::DebugRender()
{
	m_vertexBuffersBuilt = false;
}

DebugRender::~DebugRender()
{
}

Tu::Status DebugRender::Create()
{
	Tu::Status status;

	status = m_lineVertexBuffers3D.Create(D3DManager::GetDevicePtr(), D3DPT_LINELIST);
	if (!status)
	{
		return status;
	}

	status = m_matrixVertexBuffer.Create(D3DManager::GetDevicePtr(), D3DPT_LINELIST);
	if (!status)
	{
		return status;
	}

    // initialise the static vertex buffer for matrix render
	Primitive3DVertex v;

	F32 scale = 0.0015f;

	// z axis 'y' arrowhead
	v.SetPosition( 0.0f, 0.0f, 1.0f * scale);
	v.SetColour(0,0,255,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( 0.0f, 0.2f * scale, (1.0f - 0.3f)  * scale);
	v.SetColour(0,0,255,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( 0.0f, 0.0f, 1.0f * scale);
	v.SetColour(0,0,255,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( 0.0f, -0.2f * scale, (1.0f - 0.3f) * scale );
	v.SetColour(0,0,255,255);
	m_matrixVertexBuffer.AddVertexData(v);

	// z axis 'x' arrowhead
	v.SetPosition( 0.0f, 0.0f, 1.0f * scale );
	v.SetColour(0,0,255,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( 0.2f * scale, 0.0f, (1.0f - 0.3f) * scale );
	v.SetColour(0,0,255,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( 0.0f, 0.0f, 1.0f * scale );
	v.SetColour(0,0,255,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( -0.2f * scale, 0.0f, (1.0f - 0.3f) * scale );
	v.SetColour(0,0,255,255);
	m_matrixVertexBuffer.AddVertexData(v);

 

	//////////
	// x axis
	//////////
	v.SetPosition( 0.0f, 0.0f, 0.0f );
	v.SetColour(255,0,0,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( 1.0f * scale, 0.0f, 0.0f );
	v.SetColour(255,0,0,255);
	m_matrixVertexBuffer.AddVertexData(v);



	//////////
	// y axis
	//////////
	v.SetPosition( 0.0f, 0.0f, 0.0f );
	v.SetColour(0,255,0,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( 0.0f, 1.0f * scale, 0.0f );
	v.SetColour(0,255,0,255);
	m_matrixVertexBuffer.AddVertexData(v);

	//////////
	// y axis
	//////////
	v.SetPosition( 0.0f, 0.0f, 0.0f );
	v.SetColour(0,0,255,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( 0.0f, 0.0f, 1.0f * scale );
	v.SetColour(0,0,255,255);
	m_matrixVertexBuffer.AddVertexData(v);
	

	// y axis 'x' arrowhead
	v.SetPosition( 0.0f, 1.0f * scale, 0.0f );
	v.SetColour(0,255,0,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( 0.2f * scale, (1.0f-0.3f) * scale, 0.0f);
	v.SetColour(0,255,0,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( 0.0f, 1.0f * scale, 0.0f );
	v.SetColour(0,255,0,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( -0.2f * scale, (1.0f - 0.3f) * scale, 0.0f );
	v.SetColour(0,255,0,255);
	m_matrixVertexBuffer.AddVertexData(v);

	// x axis 'x' arrowhead
	v.SetPosition( 1.0f * scale, 0.0f, 0.0f );
	v.SetColour(255,0,0,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( (1.0f-0.3f) * scale, 0.0f , 0.2f * scale);
	v.SetColour(255,0,0,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition( 1.0f * scale, 0.0f, 0.0f );
	v.SetColour(255,0,0,255);
	m_matrixVertexBuffer.AddVertexData(v);

	v.SetPosition((1.0f - 0.3f) * scale, 0.0f , -0.2f * scale );
	v.SetColour(255,0,0,255);
	m_matrixVertexBuffer.AddVertexData(v);
	
	return Tu::Status::SUCCESS;
}

void DebugRender::Destroy()
{
	m_lineVertexBuffers3D.Destroy();
	m_matrixVertexBuffer.Destroy();
}

void DebugRender::Clear()
{
	//Note that the matrix VB is not cleared.

	m_lineVertexBuffers3D.Clear();
}

Tu::Status DebugRender::RebuildVertexBuffers()
{
	Tu::Status status;

	status = m_matrixVertexBuffer.RebuildVertexBuffers();
	if (!status) return status;

	status = m_lineVertexBuffers3D.RebuildVertexBuffers();
	if (!status) return status;

	m_vertexBuffersBuilt = true;
	return Tu::Status::SUCCESS;
}

Tu::Status DebugRender::Draw()
{
	Tu::Status status;

	status = m_lineVertexBuffers3D.Draw();
	if (!status)
	{
		return status;
	}

	return Tu::Status::SUCCESS;
}

Tu::Status DebugRender::OnCameraZoom()
{
	return Tu::Status::SUCCESS;
}

/*
void DebugRender::Draw2dLine(const Position2& start, const Position2& end, const Colour4& colour)
{
   Debug2dVertex* pVertex;

	const U32 noLines = 1;
	const U32 baseVertex = m_DynamicVertexBuffer2D.LockVertices(noLines * 2, pVertex);

    pVertex->SetPos(m_pD3DWindow->Transform2dPoint(start));
	pVertex->SetColour(colour);
	pVertex++;

	pVertex->SetPos(m_DisplayWindowPtr->Transform2dPoint(end));
	pVertex->SetColour(colour);
	pVertex++;    

    m_DynamicVertexBuffer2D.UnlockVertices();

    m_pD3DWindow->GetD3DGetDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer2D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug2dVertex) );
    m_pD3DWindow->GetD3DGetDevicePtr()->SetFVF( ((Debug2dVertex*)NULL)->GetFVF() );
    m_pD3DWindow->GetD3DGetDevicePtr()->DrawPrimitive( D3DPT_LINELIST, baseVertex, noLines );
    m_pD3DWindow->GetD3DGetDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );

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

}

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
void DebugRender::Draw3DLine(const TMathsNM::HVec3& A, const TMathsNM::HVec3& B, const TMathsNM::HVec4& colour)
{
	Primitive3DVertex v;

	v.SetPosition(A);
	v.SetColour(colour);
	m_lineVertexBuffers3D.AddVertexData(v);

	v.SetPosition(B);
	v.SetColour(colour);
	m_lineVertexBuffers3D.AddVertexData(v);
}
/*
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

    m_pD3DWindow->GetD3DGetDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3DWindow->GetD3DGetDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3DWindow->GetD3DGetDevicePtr()->DrawPrimitive( D3DPT_LINELIST, baseVertex, noLines );
    m_pD3DWindow->GetD3DGetDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );
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

    m_pD3DWindow->GetD3DGetDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3DWindow->GetD3DGetDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3DWindow->GetD3DGetDevicePtr()->DrawPrimitive( D3DPT_TRIANGLELIST, baseVertex, noTriangles );
    m_pD3DWindow->GetD3DGetDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );
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

    m_pD3DWindow->GetD3DGetDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3DWindow->GetD3DGetDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3DWindow->GetD3DGetDevicePtr()->DrawPrimitive( D3DPT_TRIANGLELIST, baseVertex, noTriangles );
    m_pD3DWindow->GetD3DGetDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );
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

    m_pD3DWindow->GetD3DGetDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3DWindow->GetD3DGetDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3DWindow->GetD3DGetDevicePtr()->DrawPrimitive( D3DPT_LINELIST, baseVertex, noLines );
    m_pD3DWindow->GetD3DGetDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );
}
*/
void DebugRender::Draw3DCross(const TMathsNM::HVec3& pos, const F32 size, const TMathsNM::HVec4& colour)
{
//    const U32 cull_flags = m_DisplayWindowPtr->m_ViewFrustrum.SphereCull(pos, size);
//	if(!CULL_VISIBLE(cull_flags)) return;

	Primitive3DVertex v;

	const U32 noLines = 4;

    F32 x = pos.X();
	F32 y = pos.Y();
	F32 z = pos.Z();

	v.SetPosition( x + size, y + size, z + size);
	v.SetColour(colour);
	m_lineVertexBuffers3D.AddVertexData(v);

	v.SetPosition( x - size, y - size, z - size );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	v.SetPosition( x - size, y + size, z + size );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	v.SetPosition( x + size, y - size, z - size );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);


	v.SetPosition( x + size, y - size, z + size );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	v.SetPosition( x - size, y + size, z - size );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);


	v.SetPosition( x + size, y + size, z - size );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	v.SetPosition( x - size, y - size, z + size );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);
}

void DebugRender::Draw3DStar(const TMathsNM::HVec3& pos, const F32 size, const TMathsNM::HVec4& colour)
{
//	Draw3dPoint(pos, size, colour);
	Draw3DCross(pos, size, colour);
}
/*
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

    m_pD3DWindow->GetD3DGetDevicePtr()->SetStreamSource( 0, m_DynamicVertexBuffer3D.m_VertexBuffer.m_pVertexBuffer, 0, sizeof(Debug3dVertex) );
    m_pD3DWindow->GetD3DGetDevicePtr()->SetFVF( ((Debug3dVertex*)NULL)->GetFVF() );
    m_pD3DWindow->GetD3DGetDevicePtr()->DrawPrimitive( D3DPT_LINESTRIP, baseVertex, noPoints );
    m_pD3DWindow->GetD3DGetDevicePtr()->SetStreamSource( 0, NULL, 0, 0 );

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
*/

void DebugRender::Draw3DMatrix()
{
	m_matrixVertexBuffer.Draw();
}

void DebugRender::Draw3DMatrix(const TMathsNM::HMat44& m)
{
	D3DManager::GetDevicePtr()->SetTransform(D3DTS_WORLD, (const D3DMATRIX*)&m);
	m_matrixVertexBuffer.Draw();
}

void DebugRender::Draw3DBox(
	const TMathsNM::HVec3& min, 
	const TMathsNM::HVec3& max, 
	const TMathsNM::HVec4& colour)
{

	TMathsNM::HVec3 com = ((min + max) * 0.5f);

	const TMathsNM::HVec3 diag = (max - min);
	const F32 radius = diag.Length() * 0.5f;

	TMathsNM::HVec3 pos[8];

	pos[0] = TMathsNM::HVec3(min.X(), min.Y(), min.Z());
	pos[1] = TMathsNM::HVec3(max.X(), min.Y(), min.Z());
	pos[2] = TMathsNM::HVec3(min.X(), min.Y(), max.Z());
	pos[3] = TMathsNM::HVec3(max.X(), min.Y(), max.Z());

	pos[4] = TMathsNM::HVec3(min.X(), max.Y(), min.Z());
	pos[5] = TMathsNM::HVec3(max.X(), max.Y(), min.Z());
	pos[6] = TMathsNM::HVec3(min.X(), max.Y(), max.Z());
	pos[7] = TMathsNM::HVec3(max.X(), max.Y(), max.Z());

    Primitive3DVertex v;

	const U32 noLines = 12;
	

    // BASE
	v.SetPosition( pos[0] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);
	
	v.SetPosition( pos[1] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	////

	v.SetPosition( pos[2] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);
	
	v.SetPosition( pos[3] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	////

	v.SetPosition( pos[0] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	v.SetPosition( pos[2] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	////

	v.SetPosition( pos[1] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	v.SetPosition( pos[3] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	////
	// TOP
	////

	v.SetPosition( pos[4] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	v.SetPosition( pos[5] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	////

	v.SetPosition( pos[6] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	v.SetPosition( pos[7] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	////

	v.SetPosition( pos[4] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	v.SetPosition( pos[6] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	////

	v.SetPosition( pos[5] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	v.SetPosition( pos[7] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	////
	//	VERTICALS
	////
	
	v.SetPosition( pos[0] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);
	
	v.SetPosition( pos[4] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	////
	
	v.SetPosition( pos[1] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);
	
	v.SetPosition( pos[5] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	////
	v.SetPosition( pos[2] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);
	
	v.SetPosition( pos[6] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);

	////
	v.SetPosition( pos[3] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);
	
	v.SetPosition( pos[7] );
	v.SetColour( colour );
	m_lineVertexBuffers3D.AddVertexData(v);
}
/*
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

}

void DebugRender::Close()
{
    //dbPrint(DBSTR "Close Debug Render");

    m_StaticVertexBuffer3D.Close();
    m_DynamicVertexBuffer3D.Close();
    m_DynamicVertexBuffer2D.Close();

    m_pD3DWindow = NULL;
}


DebugRender::~DebugRender()
{
    if (m_pD3DWindow != NULL)
    {
        //dbWarn(DBSTR "DebugRender not closed");
        Close();
    }
}


*/

