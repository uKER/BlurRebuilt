#include <nexus_render/stdafx.h>
#include <nexus_render/Primitives.h>

using namespace NexusRender;

void Primitives::DrawLine(
	D3DVertexPosCol* pVertex,
	const Maths::HMat44& m,
	const Maths::HVec3& A, 
	const Maths::HVec3& B,
	const Maths::HVec4& colour)
{
	pVertex->SetPosition(A*m);
	pVertex->SetColour(colour);
	pVertex++;

	pVertex->SetPosition(B*m);
	pVertex->SetColour(colour);
}

void Primitives::DrawStar(
	D3DVertexPosCol* pVertex,
	const Maths::HMat44& m,
	const F32 radius,
	const Maths::HVec4& colour)
{
	pVertex->SetPosition(Maths::HVec3(radius, radius, radius) * m);
	pVertex->SetColour(colour);
	pVertex++;

	pVertex->SetPosition(Maths::HVec3(-radius, -radius, -radius) * m);
	pVertex->SetColour(colour);
	pVertex++;

	pVertex->SetPosition(Maths::HVec3(-radius, radius, radius) * m);
	pVertex->SetColour(colour);
	pVertex++;

	pVertex->SetPosition(Maths::HVec3(radius, -radius, -radius) * m);
	pVertex->SetColour(colour);
	pVertex++;

	pVertex->SetPosition(Maths::HVec3(radius, -radius, radius) * m);
	pVertex->SetColour(colour);
	pVertex++;

	pVertex->SetPosition(Maths::HVec3(-radius, radius, -radius) * m);
	pVertex->SetColour(colour);
	pVertex++;

	pVertex->SetPosition(Maths::HVec3(radius, radius, -radius) * m);
	pVertex->SetColour(colour);
	pVertex++;

	pVertex->SetPosition(Maths::HVec3(-radius, -radius, radius) * m);
	pVertex->SetColour(colour);
}

void Primitives::DrawCircle(
	D3DVertexPosCol* pVertex,
	const Maths::HMat44& m,
	const F32 radius,
	const Maths::HVec4& colour)
{
	Maths::HVec3 u(1,0,0);
	Maths::HVec3 v(0,1,0);
    
	//The number of radians each segment turns thru.
	F32 deltaTheta = (Maths::Pi * 2.0f) / (F32)NEXUS_RENDER_PRIMITIVES_POINTS_PER_CIRCLE;

	for (U32 i = 0; i < NEXUS_RENDER_PRIMITIVES_POINTS_PER_CIRCLE; i++)
	{
		F32 theta = i * deltaTheta;
		pVertex->SetPosition(Maths::HVec3((u * sinf(theta) + v * cosf(theta)) * radius) * m);
        pVertex->SetColour(colour);
        pVertex++;

		theta = (i+1) * deltaTheta;
		pVertex->SetPosition(Maths::HVec3((u * sinf(theta) + v * cosf(theta)) * radius) * m);
        pVertex->SetColour(colour);
        pVertex++;
	}
}

void Primitives::DrawCone(
	D3DVertexPosCol* pVertex,
	const Maths::HMat44& m,
	const F32 radius,
	const F32 height,
	const Maths::HVec4& colour)
{
	DrawCircle(
		pVertex, 
		m,
		radius,
		colour);
	pVertex += NEXUS_RENDER_PRIMITIVES_CIRCLE_VERTS;

	Maths::HVec3 u(1,0,0);
	Maths::HVec3 v(0,1,0);
	Maths::HVec3 top(0,0,height);
    
	F32 deltaTheta = (Maths::Pi * 2.0f) / (F32)NEXUS_RENDER_PRIMITIVES_POINTS_PER_CONE_BODY;

	for (U32 i = 0; i < NEXUS_RENDER_PRIMITIVES_POINTS_PER_CONE_BODY; i++)
	{
		F32 theta = i * deltaTheta;
		DrawLine(
			pVertex,
			m,
			top,
            Maths::HVec3((u * sinf(theta) + v * cosf(theta)) * radius),
			colour);
		pVertex += NEXUS_RENDER_PRIMITIVES_LINE_VERTS;
	}
}

void Primitives::DrawBox(
	D3DVertexPosCol* pVertex,
	const Maths::HMat44& m,
	const Maths::HVec3& min, 
	const Maths::HVec3& max, 
	const Maths::HVec4& colour)
{
	Maths::HVec3 com = ((min + max) * 0.5f);

	const Maths::HVec3 diag = (max - min);
	const F32 radius = diag.Length() * 0.5f;

	Maths::HVec3 pos[8];

	pos[0] = Maths::HVec3(min.X(), min.Y(), min.Z());
	pos[1] = Maths::HVec3(max.X(), min.Y(), min.Z());
	pos[2] = Maths::HVec3(min.X(), min.Y(), max.Z());
	pos[3] = Maths::HVec3(max.X(), min.Y(), max.Z());

	pos[4] = Maths::HVec3(min.X(), max.Y(), min.Z());
	pos[5] = Maths::HVec3(max.X(), max.Y(), min.Z());
	pos[6] = Maths::HVec3(min.X(), max.Y(), max.Z());
	pos[7] = Maths::HVec3(max.X(), max.Y(), max.Z());

	const U32 noLines = 12;
	

    // BASE
	pVertex->SetPosition( pos[0] );
	pVertex->SetColour( colour );
	pVertex++;
	
	pVertex->SetPosition( pos[1] );
	pVertex->SetColour( colour );
	pVertex++;

	////

	pVertex->SetPosition( pos[2] );
	pVertex->SetColour( colour );
	pVertex++;
	
	pVertex->SetPosition( pos[3] );
	pVertex->SetColour( colour );
	pVertex++;

	////

	pVertex->SetPosition( pos[0] );
	pVertex->SetColour( colour );
	pVertex++;

	pVertex->SetPosition( pos[2] );
	pVertex->SetColour( colour );
	pVertex++;

	////

	pVertex->SetPosition( pos[1] );
	pVertex->SetColour( colour );
	pVertex++;

	pVertex->SetPosition( pos[3] );
	pVertex->SetColour( colour );
	pVertex++;

	////
	// TOP
	////

	pVertex->SetPosition( pos[4] );
	pVertex->SetColour( colour );
	pVertex++;

	pVertex->SetPosition( pos[5] );
	pVertex->SetColour( colour );
	pVertex++;

	////

	pVertex->SetPosition( pos[6] );
	pVertex->SetColour( colour );
	pVertex++;

	pVertex->SetPosition( pos[7] );
	pVertex->SetColour( colour );
	pVertex++;

	////

	pVertex->SetPosition( pos[4] );
	pVertex->SetColour( colour );
	pVertex++;

	pVertex->SetPosition( pos[6] );
	pVertex->SetColour( colour );
	pVertex++;

	////

	pVertex->SetPosition( pos[5] );
	pVertex->SetColour( colour );
	pVertex++;

	pVertex->SetPosition( pos[7] );
	pVertex->SetColour( colour );
	pVertex++;

	////
	//	VERTICALS
	////
	
	pVertex->SetPosition( pos[0] );
	pVertex->SetColour( colour );
	pVertex++;
	
	pVertex->SetPosition( pos[4] );
	pVertex->SetColour( colour );
	pVertex++;

	////
	
	pVertex->SetPosition( pos[1] );
	pVertex->SetColour( colour );
	pVertex++;
	
	pVertex->SetPosition( pos[5] );
	pVertex->SetColour( colour );
	pVertex++;

	////
	pVertex->SetPosition( pos[2] );
	pVertex->SetColour( colour );
	pVertex++;
	
	pVertex->SetPosition( pos[6] );
	pVertex->SetColour( colour );
	pVertex++;

	////
	pVertex->SetPosition( pos[3] );
	pVertex->SetColour( colour );
	pVertex++;
	
	pVertex->SetPosition( pos[7] );
	pVertex->SetColour( colour );
	pVertex++;
}

