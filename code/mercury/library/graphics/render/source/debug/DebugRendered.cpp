#include "stdafx.h"
#include <Render/Debug/Debug.h>
#include <Render/Debug/DebugRender.h>
#include <Render/Primitive/PrimitiveRender.h>
#include <Render/debug/DebugRendered.h>
#include <core/maths/hvec/New/IncludeMinimal.h>
#include <core/maths/hvec/New/IncludeMinimal.inl>

using namespace DbUtil;
using namespace Maths;
using namespace Render::Debug;


void C_DebugRendered::Draw2dLine(I_BeginEndVertices* pRendered, const Maths::HVec2& posA, const Maths::HVec2& posB, const Maths::RGBA col)
{
	pRendered->Begin2D();

	const U32 lineCount = 1;
	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex2D* pVertex = (Render::Primitive::Vertex2D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount); // This can fail
	
	if (pVertex)
	{
		pVertex->Pos(posA);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(posB);
		pVertex->Col(col);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw2dTriangle(I_BeginEndVertices* pRendered, const Maths::HVec2& posA, const Maths::HVec2& posB, const Maths::HVec2& posC, const Maths::RGBA col)
{
	pRendered->Begin2D();

	const U32 vertCount = 3;
	Render::Primitive::Vertex2D* pVertex = (Render::Primitive::Vertex2D*)pRendered->BeginVertices(Display::E_PrimitiveType_TriangleList, vertCount, false, false); // This can fail

	if (pVertex)
	{
		pVertex->Pos(posA);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(posB);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(posC);
		pVertex->Col(col);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw2dCircle(I_BeginEndVertices* pRendered, const Maths::HVec2& centre, const Maths::HVec2& size, const Maths::RGBA col)
{
	pRendered->Begin2D();

	const S32 stepCount = 32;
	const F32 deltaAngle = Maths::Pi * 2.0f / (F32)stepCount;
    F32 currAngle = 0;
	const U32 lineCount = stepCount;
	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex2D* pVertex = (Render::Primitive::Vertex2D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount); // This can fail
	
	if (pVertex)
	{
		for (S32 i = 0; i < stepCount; i++)
		{        
			SinCosAlpha sc_angle(currAngle);

			HVec2 currPos;

			currPos.X(centre.X() + sc_angle.SinAlpha() * size.X());
			currPos.Y(centre.Y() + sc_angle.CosAlpha() * size.Y());
	    
			pVertex->Pos(currPos);
			pVertex->Col(col);
			pVertex++;

			currAngle = currAngle + deltaAngle;

			SinCosAlpha sc_angle2(currAngle);

			currPos.X(centre.X() + sc_angle2.SinAlpha() * size.X());
			currPos.Y(centre.Y() + sc_angle2.CosAlpha() * size.Y());
	    
			pVertex->Pos(currPos);
			pVertex->Col(col);
			pVertex++;
		}

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw2dArrow(I_BeginEndVertices* pRendered, const Maths::HVec2& start, const Maths::HVec2& dir, const F32 arrowheadSize, const Maths::RGBA col)
{
	pRendered->Begin2D();

	const HVec2 end = start + dir;
	Draw2dLine(pRendered, start, end, col);

    HVec2 normDir = dir.Normalised();
    const HVec2 perpDir(-normDir.Y(), normDir.X());
    const HVec2 arrowEndA = end - normDir * arrowheadSize + perpDir * arrowheadSize;
  	Draw2dLine(pRendered, end, arrowEndA, col);

    const HVec2 arrowEndB = end - normDir * arrowheadSize - perpDir * arrowheadSize;
  	Draw2dLine(pRendered, end, arrowEndB, col);
}

void C_DebugRendered::Draw2dBar(I_BeginEndVertices* pRendered, const Maths::HVec2& pos, const Maths::HVec2& size, const Maths::RGBA col)
{
	pRendered->Begin2D();

	const U32 faceCount = 2;
	const U32 vertCount = 3 * faceCount;
	Render::Primitive::Vertex2D* pVertex = (Render::Primitive::Vertex2D*)pRendered->BeginVertices(Display::E_PrimitiveType_TriangleList, vertCount); // This can fail
	
	if (pVertex)
	{
		pVertex->Pos(pos);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos.X() + size.X(), pos.Y());
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos.X(), pos.Y() + size.Y());
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos.X(), pos.Y() + size.Y());
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos.X() + size.X(), pos.Y());
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos + size);
		pVertex->Col(col);
		pVertex++;


		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw2dText(I_BeginEndVertices* pRendered, const Maths::HVec2& pos, const Util::C_String& text)
{
	Display::FontRender()->Draw(text.Chars(), pos.X(), pos.Y(), 20, 20, Maths::RGBA(255, 255, 255, 255));
}

void C_DebugRendered::Draw2dText(I_BeginEndVertices* pRendered, const Maths::HVec2& pos, const Util::C_String& text, const Maths::RGBA colour, const F32 height, const S32 flags)
{
	Display::FontRender()->Draw(text.Chars(), pos.X(), pos.Y(), height, height, colour);
}

void C_DebugRendered::Draw3dLine(I_BeginEndVertices* pRendered, const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::RGBA col)
{
	pRendered->Begin3D();
	
	const U32 lineCount = 1;
	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount, true, false);
	
	if (pVertex)
	{
		pVertex->Pos(posA);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(posB);
		pVertex->Col(col);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dLine(I_BeginEndVertices* pRendered, const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::RGBA col, const Maths::HMat43& mat)
{
	pRendered->Begin3D();

	const U32 lineCount = 1;
	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount, true); // This can fail
	
	if (pVertex)
	{
		pVertex->Pos(/* mat * */posA);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(/* mat * */posB);
		pVertex->Col(col);
		pVertex++;
		
		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dLines(I_BeginEndVertices* pRendered, const Maths::HVec3* posA, const Maths::HVec3* posB, const U32 lineCount, const Maths::RGBA col)
{
	pRendered->Begin3D();

	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount, true); // This can fail
	
	if (pVertex)
	{
		for (U32 i = 0; i < lineCount; i++)
		{
			pVertex->Pos(posA[i]);
			pVertex->Col(col);
			pVertex++;
			
			pVertex->Pos(posB[i]);
			pVertex->Col(col);
			pVertex++;
		}

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dLine(I_BeginEndVertices* pRendered, const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::RGBA colA, const Maths::RGBA colB)
{
	pRendered->Begin3D();

	const U32 lineCount = 1;
	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount, true); // This can fail

	if (pVertex)
	{
		pVertex->Pos(posA);
		pVertex->Col(colA);
		pVertex++;
		
		pVertex->Pos(posB);
		pVertex->Col(colB);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dTriangleOutline(I_BeginEndVertices* pRendered, const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA col)
{
	pRendered->Begin3D();

	const U32 lineCount = 3;
	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount, true); // This can fail

	if (pVertex)
	{
		pVertex->Pos(posA);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(posB);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(posC);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(posA);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(posB);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(posC);
		pVertex->Col(col);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dTriangle(I_BeginEndVertices* pRendered, const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA col)
{
	pRendered->Begin3D();

	const U32 vertCount = 3;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_TriangleList, vertCount, true); // This can fail

	if (pVertex)
	{
		pVertex->Pos(posA);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(posB);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(posC);
		pVertex->Col(col);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dTriangleLit(I_BeginEndVertices* pRendered, const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA col)
{
	pRendered->Begin3DLit();

	const U32 vertCount = 3;
	Render::Primitive::Vertex3DLit* pVertex = (Render::Primitive::Vertex3DLit*)pRendered->BeginVertices(Display::E_PrimitiveType_TriangleList, vertCount, true, true); // This can fail

	if (pVertex)
	{
		Maths::HVec3 v0 = posC - posA;
		Maths::HVec3 v1 = posB - posA;
		Maths::HVec3 normal = v1 ^ v0;
		normal.Normalise();

		pVertex->Pos(posA);
		pVertex->Col(col);
		pVertex->Norm(normal);
		pVertex++;
		
		pVertex->Pos(posB);
		pVertex->Col(col);
		pVertex->Norm(normal);
		pVertex++;

		pVertex->Pos(posC);
		pVertex->Col(col);
		pVertex->Norm(normal);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dTriangle(I_BeginEndVertices* pRendered, const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA colA, const Maths::RGBA colB, const Maths::RGBA colC)
{
	pRendered->Begin3D();

	const U32 vertCount = 3;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_TriangleList, vertCount, true, false); // This can fail

	if (pVertex)
	{
		pVertex->Pos(posA);
		pVertex->Col(colA);
		pVertex++;
		
		pVertex->Pos(posB);
		pVertex->Col(colB);
		pVertex++;

		pVertex->Pos(posC);
		pVertex->Col(colC);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dTriangleLit(I_BeginEndVertices* pRendered, const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA colA, const Maths::RGBA colB, const Maths::RGBA colC)
{
	pRendered->Begin3DLit();

	const U32 vertCount = 3;
	Render::Primitive::Vertex3DLit* pVertex = (Render::Primitive::Vertex3DLit*)pRendered->BeginVertices(Display::E_PrimitiveType_TriangleList, vertCount, true, true); // This can fail

	Maths::HVec3 v0 = posC - posA;
	Maths::HVec3 v1 = posB - posA;
	Maths::HVec3 normal = v1 ^ v0;
	normal.Normalise();
		
	if (pVertex)
	{
		pVertex->Pos(posA);
		pVertex->Col(colA);
		pVertex->Norm(normal);
		pVertex++;
		
		pVertex->Pos(posB);
		pVertex->Col(colB);
		pVertex->Norm(normal);
		pVertex++;

		pVertex->Pos(posC);
		pVertex->Col(colC);
		pVertex->Norm(normal);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dTriangles(I_BeginEndVertices* pRendered,  const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count)
{
	pRendered->Begin3D();

	const U32 vertCount = 3 * count;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_TriangleList, vertCount, true); // This can fail

	if (pVertex)
	{
		for (U32 i = 0; i < count; i++)
		{
			pVertex->Pos(AArray[i]);
			pVertex->Col(colourArray[i]);
			pVertex++;
			
			pVertex->Pos(BArray[i]);
			pVertex->Col(colourArray[i]);
			pVertex++;

			pVertex->Pos(CArray[i]);
			pVertex->Col(colourArray[i]);
			pVertex++;
		}

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dTrianglesLit(I_BeginEndVertices* pRendered,  const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count)
{
	pRendered->Begin3DLit();

	const U32 vertCount = 3 * count;
	Render::Primitive::Vertex3DLit* pVertex = (Render::Primitive::Vertex3DLit*)pRendered->BeginVertices(Display::E_PrimitiveType_TriangleList, vertCount, true, true); // This can fail

	if (pVertex)
	{
		for (U32 i = 0; i < count; i++)
		{
			Maths::HVec3 v0 = CArray[i] - AArray[i];
			Maths::HVec3 v1 = BArray[i] - AArray[i];
			Maths::HVec3 normal = v1 ^ v0;
			normal.Normalise();

			pVertex->Pos(AArray[i]);
			pVertex->Col(colourArray[i]);
			pVertex->Norm(normal);
			pVertex++;
			
			pVertex->Pos(BArray[i]);
			pVertex->Col(colourArray[i]);
			pVertex->Norm(normal);
			pVertex++;

			pVertex->Pos(CArray[i]);
			pVertex->Col(colourArray[i]);
			pVertex->Norm(normal);
			pVertex++;
		}

		pRendered->EndVertices(pVertex);
	}
}


void C_DebugRendered::Draw3dPoint(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const F32 size, const Maths::RGBA col)
{
	pRendered->Begin3D();

	const U32 lineCount = 3;
	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount, true); // This can fail

	if (pVertex)
	{
		pVertex->Pos(pos.X(), pos.Y(), pos.Z() + size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos.X(), pos.Y(), pos.Z() - size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos.X(), pos.Y() + size, pos.Z());
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos.X(), pos.Y() - size, pos.Z());
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos.X() + size, pos.Y(), pos.Z());
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos.X() - size, pos.Y(), pos.Z());
		pVertex->Col(col);
		pVertex++;
		
		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dCross(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const F32 size, const Maths::RGBA col)
{
	pRendered->Begin3D();

	const U32 lineCount = 4;
	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount, true); // This can fail

	if (pVertex)
	{
		pVertex->Pos(pos.X() + size, pos.Y() + size, pos.Z() + size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos.X() - size, pos.Y() - size, pos.Z() - size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos.X() - size, pos.Y() + size, pos.Z() + size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos.X() + size, pos.Y() - size, pos.Z() - size);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos.X() + size, pos.Y() - size, pos.Z() + size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos.X() - size, pos.Y() + size, pos.Z() - size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos.X() + size, pos.Y() + size, pos.Z() - size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos.X() - size, pos.Y() - size, pos.Z() + size);
		pVertex->Col(col);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dStar(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const F32 size, const Maths::RGBA col)
{
	Draw3dPoint(pRendered, pos, size, col);
	Draw3dCross(pRendered, pos, size * 0.57735f, col);
}

void C_DebugRendered::Draw3dPlanePoint(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA col)
{
	pRendered->Begin3D();

	HVec3 u, v;
	normal.PerpendicularAxes(u, v);

	const U32 lineCount = 2;
	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount, true); // This can fail

	if (pVertex)
	{
		pVertex->Pos(pos + u * size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos - u * size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos + v * size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos - v * size);
		pVertex->Col(col);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dPlaneCross(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA col)
{
	pRendered->Begin3D();

	HVec3 u, v;
	normal.PerpendicularAxes(u, v);

	const U32 lineCount = 2;
	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount, true); // This can fail

	if (pVertex)
	{
		pVertex->Pos(pos + u * size + v * size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos - u * size - v * size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos - u * size + v * size);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos + u * size - v * size);
		pVertex->Col(col);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dPlaneStar(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA col)
{
	Draw3dPlanePoint(pRendered, pos, normal, size, col);
	Draw3dPlaneCross(pRendered, pos, normal, size * 0.7071f, col);
}

void C_DebugRendered::Draw3dPlane(I_BeginEndVertices* pRendered, const Maths::C_Plane& plane, const Maths::C_Vertex& referencePoint, const F32 size, const Maths::RGBA col)
{
	const C_Vertex p(plane.OnPlane(referencePoint));
    const C_Normal n(plane.Normal());
    Draw3dPlaneCross(pRendered, p, n, size * 0.2f, col);
    Draw3dArrow(pRendered, p, n, size, size * 0.1f, col);
}

void C_DebugRendered::Draw3dArrow(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const Maths::HVec3& dir, const F32 lengthScale, const F32 arrowheadSize, const Maths::RGBA col)
{
	if ( !_finite( lengthScale ) || !_finite( arrowheadSize ) || (bool)( !dir.Finite() | Maths::Length(dir).LT( 0.0001f ) ) )
        return;

	pRendered->Begin3D();

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
    
	const HVec3 cameraPos = viewSettings3D.GetCameraPosition();
	const HVec3 arrowEnd = pos + dir * lengthScale;
	const HVec3 normDir = dir.Normalised().Normal();
	const HVec3 camDir = (arrowEnd - cameraPos).Normalised().Normal();
    const HVec3 perp = (normDir ^ camDir).SafeNormalised().Normal();
	const HVec3 arrowTipA = arrowEnd - normDir * arrowheadSize + perp * arrowheadSize;
	const HVec3 arrowTipB = arrowEnd - normDir * arrowheadSize - perp * arrowheadSize;

	const U32 lineCount = 3;
	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount, true); // This can fail

	if (pVertex)
	{
		pVertex->Pos(pos);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(arrowEnd);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(arrowEnd);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(arrowTipA);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(arrowEnd);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(arrowTipB);
		pVertex->Col(col);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dCircle(I_BeginEndVertices* pRendered, const Maths::HVec3& centre, const Maths::HVec3& normal, const F32 radius, const Maths::RGBA col)
{
    using namespace Maths;
	pRendered->Begin3D();

	HVec3 u(Maths::UNDEFINED), v(Maths::UNDEFINED);
	normal.PerpendicularAxes(u, v);
	
//#ifdef USER_KSTOREY // JASOND: The USER_XXX define no longer exists!
//	const S32 pointCount = 8;
//#else
	const S32 pointCount = 32;
//#endif
    
	const S32 vertCount = 2 * pointCount;
	//const F32 deltaTheta = (Maths::Pi * 2.f) / (F32)pointCount;
	Maths::HF32 deltaTheta = (Maths::Constants<Maths::HF32>::Pi() * Maths::Constants<Maths::HF32>::Two())/Maths::HF32((F32)pointCount);
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount, true); // This can fail

	if (pVertex)
	{
		#if defined( _DEBUG )
	    
	    const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		Maths::HF32 fi = Maths::Constants<Maths::HF32>::Zero();
		for (S32 i = 0; i < pointCount; i++, fi += one)
		{
			{
				const Maths::HF32 theta = fi * deltaTheta;
				Maths::HF32 sinAlpha(sinf(theta));//Maths::Sin(theta)); // PERF: Stdlib cos+sin similar speed in release, 10x faster in debug!
				Maths::HF32 cosAlpha(cosf(theta));//Maths::Cos(theta));

				const HVec3 pos = centre + (u * sinAlpha + v * cosAlpha) * radius;
		        
				pVertex->Pos(pos);
				pVertex->Col(col);
				pVertex++;
			}
			{
				const Maths::HF32 theta = (fi + 1) * deltaTheta;
				Maths::HF32 sinAlpha(sinf(theta));//Maths::Sin(theta));
				Maths::HF32 cosAlpha(cosf(theta));//Maths::Cos(theta));
				
				const HVec3 pos = centre + (u * sinAlpha + v * cosAlpha) * radius;
		        
				pVertex->Pos(pos);
				pVertex->Col(col);
				pVertex++;
			}
		}
		
		#else // JASOND: Optimised for release & speed builds.
		
		//const VF32    zero( Const<VF32>::Zero() );
		//const VF32    one( Const<VF32>::One() );
		const VF32    dt( deltaTheta );
		const VF32    r( MakeVF32( radius ) );
		const Point3  c( centre );
		const Vector3 ur( (Vector3)u * r );
		const Vector3 vr( (Vector3)v * r );
		HVec3 p1( c + vr );
		VF32 fi = dt;
		for ( S32 i = 0; i < pointCount; i++, fi += dt )
		{
		    VF32 sinAlpha, cosAlpha;
		    SinCos( fi, &sinAlpha, &cosAlpha );
		    const HVec3 p2( MulAdd( vr, cosAlpha, MulAdd( ur, sinAlpha, c ) ) );
		    
			pVertex->Pos( p1 );
			pVertex->Col( col );
			pVertex++;
			
			pVertex->Pos( p2 );
			pVertex->Col( col );
			pVertex++;
			
			p1 = p2;
		}
		
		#endif

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dSphere(I_BeginEndVertices* pRendered, const Maths::HVec3& centre, const F32 radius, const Maths::RGBA col)
{
	Draw3dCircle(pRendered, centre, HVecX(1.f), radius, col);
    Draw3dCircle(pRendered, centre, HVecY(1.f), radius, col);
    Draw3dCircle(pRendered, centre, HVecZ(1.f), radius, col);
}

void C_DebugRendered::Draw3dSphere(I_BeginEndVertices* pRendered, const Maths::C_Sphere& sphere, const Maths::RGBA col)
{
	Draw3dCircle(pRendered, sphere.Centre(), HVecX(1.0f), sphere.Radius(), col);
    Draw3dCircle(pRendered, sphere.Centre(), HVecY(1.0f), sphere.Radius(), col);
    Draw3dCircle(pRendered, sphere.Centre(), HVecZ(1.0f), sphere.Radius(), col);
}

void C_DebugRendered::Draw3dSphere(I_BeginEndVertices* pRendered, const Maths::C_Sphere& sphere, const Maths::HMat43& transform, const Maths::RGBA col)
{
	const C_Sphere transformedSphere(sphere * transform);

	Draw3dCircle(pRendered, transformedSphere.Centre(), (HVec3)transform.AX().Normalised().Normal(), transformedSphere.Radius(), col);
	Draw3dCircle(pRendered, transformedSphere.Centre(), (HVec3)transform.AY().Normalised().Normal(), transformedSphere.Radius(), col);
	Draw3dCircle(pRendered, transformedSphere.Centre(), (HVec3)transform.AZ().Normalised().Normal(), transformedSphere.Radius(), col);
}

void C_DebugRendered::Draw3dSphereOutline(I_BeginEndVertices* pRendered, const Maths::HVec3& centre, const F32 radius, const Maths::RGBA col)
{
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	Draw3dCircle(pRendered, centre, viewSettings3D.GetCameraToWorld().AZ(), radius, col);
}

void C_DebugRendered::Draw3dSphereOutline(I_BeginEndVertices* pRendered, const Maths::C_Sphere& sphere, const Maths::RGBA col)
{
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	Draw3dCircle(pRendered, sphere.Centre(), viewSettings3D.GetCameraToWorld().AZ(), sphere.Radius(), col);
}

void C_DebugRendered::Draw3dMatrix(I_BeginEndVertices* pRendered, const Maths::HMat43& mat, const Maths::HF32& size)
{
	pRendered->Begin3D();

	const U32 lineCount = 9;
	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount, true); // This can fail

	if (pVertex)
	{
		pVertex->Pos( HVec3(0.0f, 0.0f, 0.f) * mat);
		pVertex->Col(Maths::RGBA(0, 0, 255, 255));
		pVertex++;

		pVertex->Pos(HVec3(0.0f, 0.0f, 1.0f * size) * mat);
		pVertex->Col(Maths::RGBA(0, 0, 255, 255));
		pVertex++;

		// z axis 'y' arrowhead

		pVertex->Pos(HVec3(0.0f, 0.0f, 1.0f * size) * mat);
		pVertex->Col(Maths::RGBA(0, 0, 255, 255));
		pVertex++;

		pVertex->Pos(HVec3(0.0f, 0.2f * size, (1.0f - 0.3f) * size) * mat);
		pVertex->Col(Maths::RGBA(0, 0, 255, 255));
		pVertex++;

		pVertex->Pos(HVec3(0.0f, 0.0f, 1.0f * size) * mat);
		pVertex->Col(Maths::RGBA(0, 0, 255, 255));
		pVertex++;

		pVertex->Pos(HVec3(0.0f, -0.2f * size, (1.0f - 0.3f) * size) * mat);
		pVertex->Col(Maths::RGBA(0, 0, 255, 255));
		pVertex++;

		// z axis 'x' arrowhead

		pVertex->Pos(HVec3(0.0f, 0.0f, 1.0f * size) * mat);
		pVertex->Col(Maths::RGBA(0, 0, 255, 255));
		pVertex++;

		pVertex->Pos(HVec3(0.2f * size, 0.0f, (1.0f - 0.3f) * size) * mat);
		pVertex->Col(Maths::RGBA(0, 0, 255, 255));
		pVertex++;

		pVertex->Pos(HVec3(0.0f, 0.0f, 1.0f * size) * mat);
		pVertex->Col(Maths::RGBA(0, 0, 255, 255));
		pVertex++;

		pVertex->Pos(HVec3(-0.2f * size, 0.0f, (1.0f - 0.3f) * size) * mat);
		pVertex->Col(Maths::RGBA(0, 0, 255, 255));
		pVertex++;

		// x axis

		pVertex->Pos(HVec3(0.0f, 0.0f, 0.0f) * mat);
		pVertex->Col(Maths::RGBA(255, 0, 0, 255));
		pVertex++;

		pVertex->Pos(HVec3(1.0f * size, 0.0f, 0.0f) * mat);
		pVertex->Col(Maths::RGBA(255, 0, 0, 255));
		pVertex++;

		// y axis

		pVertex->Pos(HVec3(0.0f, 0.0f, 0.0f) * mat);
		pVertex->Col(Maths::RGBA(0, 255, 0, 255));
		pVertex++;

		pVertex->Pos(HVec3(0.0f, 1.0f * size, 0.0f) * mat);
		pVertex->Col(Maths::RGBA(0, 255, 0, 255));
		pVertex++;

		// y axis 'x' arrowhead

		pVertex->Pos(HVec3(0.0f, 1.0f * size, 0.0f) * mat);
		pVertex->Col(Maths::RGBA(0, 255, 0, 255));
		pVertex++;

		pVertex->Pos(HVec3(0.2f * size, (1.0f - 0.3f) * size, 0.0f) * mat);
		pVertex->Col(Maths::RGBA(0, 255, 0, 255));
		pVertex++;

		pVertex->Pos(HVec3(0.0f, 1.0f * size, 0.0f) * mat);
		pVertex->Col(Maths::RGBA(0, 255, 0, 255));
		pVertex++;

		pVertex->Pos(HVec3(-0.2f * size, (1.0f - 0.3f) * size, 0.0f) * mat);
		pVertex->Col(Maths::RGBA(0, 255, 0, 255));
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dBox(I_BeginEndVertices* pRendered, const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& mat, const Maths::RGBA col, bool setStates)
{
	Draw3dBox(pRendered, C_AxisBox(min, max), mat, col, setStates);
}

void C_DebugRendered::Draw3dBox(I_BeginEndVertices* pRendered, const Maths::C_AxisBox& box, const Maths::HMat43& mat, const Maths::RGBA col, bool setStates)
{
	if (setStates)
		pRendered->Begin3D();

	const U32 lineCount = 12;
	const U32 vertCount = 2 * lineCount;
	Render::Primitive::Vertex3D* pVertex = (Render::Primitive::Vertex3D*)pRendered->BeginVertices(Display::E_PrimitiveType_LineList, vertCount, true); // This can fail

	if (pVertex)
	{
		HVec3 pos[8];
		box.GetVertexArray( pos );

		// base

		pVertex->Pos(pos[0] * mat);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos[1] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[2] * mat);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos[3] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[0] * mat);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos[2] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[1] * mat);
		pVertex->Col(col);
		pVertex++;
		
		pVertex->Pos(pos[3] * mat);
		pVertex->Col(col);
		pVertex++;

		// top

		pVertex->Pos(pos[4] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[5] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[6] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[7] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[4] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[6] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[5] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[7] * mat);
		pVertex->Col(col);
		pVertex++;

		// verticals

		pVertex->Pos(pos[0] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[4] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[1] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[5] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[2] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[6] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[3] * mat);
		pVertex->Col(col);
		pVertex++;

		pVertex->Pos(pos[7] * mat);
		pVertex->Col(col);
		pVertex++;

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dBoxLit(I_BeginEndVertices* pRendered, const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& mat, const Maths::RGBA col)
{
	Draw3dBoxLit(pRendered, C_AxisBox(min, max), mat, col);
}

void C_DebugRendered::Draw3dBoxLit(I_BeginEndVertices* pRendered, const Maths::C_AxisBox& box, const Maths::HMat43& mat, const Maths::RGBA col)
{
	pRendered->Begin3DLit();

#ifndef PLATFORM_PC
	U32 numPrimitives = 6;
	U32 numVertices = 4;
	Display::E_PrimitiveType	primitiveType = Display::E_PrimitiveType_QuadList;
#else
	U32 numPrimitives = 12;
	U32 numVertices = 3;
	Display::E_PrimitiveType	primitiveType = Display::E_PrimitiveType_TriangleList;
#endif

	HVec3 pos[8];
	box.GetVertexArray( pos );

	U32 indices[] = {	0, 1, 3, 2,
						4, 0, 2, 6,
						5, 4, 6, 7,
						1, 5, 7, 3,
						6, 2, 3, 7,
						4, 5, 1, 0 };
	
	Render::Primitive::Vertex3DLit* pVertex = (Render::Primitive::Vertex3DLit*)pRendered->BeginVertices(primitiveType, numPrimitives * numVertices, true, true); // This can fail

	if (pVertex)
	{
		HVec3 pos[8];
		box.GetVertexArray( pos );

		for(S32 i = 0; i <= 20; i += 4)
		{
			// reload appropriate vectors
			Maths::HVec3 position0(pos[indices[i]] * mat);
			Maths::HVec3 position1(pos[indices[i+1]] * mat);
			Maths::HVec3 position2(pos[indices[i+2]] * mat);
			Maths::HVec3 position3(pos[indices[i+3]] * mat);

			Maths::HVec3 v0 = position3 - position0;
			Maths::HVec3 v1 = position1 - position0;
			Maths::HVec3 normal = v1 ^ v0;
			normal.Normalise();
			
			// 0
			Maths::Store<Maths::HVec3>(position0, (MathsStorage::Float3*)&pVertex->px);
			pVertex->c = col.Colour();
			Maths::Store<Maths::HVec3>(normal, (MathsStorage::Float3*)&pVertex->nx);
			++pVertex;
			// 1
			Maths::Store<Maths::HVec3>(position1, (MathsStorage::Float3*)&pVertex->px);
			pVertex->c = col.Colour();
			Maths::Store<Maths::HVec3>(normal, (MathsStorage::Float3*)&pVertex->nx);
			++pVertex;
			// 2
			Maths::Store<Maths::HVec3>(position2, (MathsStorage::Float3*)&pVertex->px);
			pVertex->c = col.Colour();
			Maths::Store<Maths::HVec3>(normal, (MathsStorage::Float3*)&pVertex->nx);
			++pVertex;
#ifndef PLATFORM_PC
			// 3
			Maths::Store<Maths::HVec3>(position3, (MathsStorage::Float3*)&pVertex->px);
			pVertex->c = col.Colour();
			Maths::Store<Maths::HVec3>(normal, (MathsStorage::Float3*)&pVertex->nx);
			++pVertex;	
#else
			// 2
			Maths::Store<Maths::HVec3>(position2, (MathsStorage::Float3*)&pVertex->px);
			pVertex->c = col.Colour();
			Maths::Store<Maths::HVec3>(normal, (MathsStorage::Float3*)&pVertex->nx);
			++pVertex;
			// 3
			Maths::Store<Maths::HVec3>(position3, (MathsStorage::Float3*)&pVertex->px);
			pVertex->c = col.Colour();
			Maths::Store<Maths::HVec3>(normal, (MathsStorage::Float3*)&pVertex->nx);
			++pVertex;
			// 0
			Maths::Store<Maths::HVec3>(position0, (MathsStorage::Float3*)&pVertex->px);
			pVertex->c = col.Colour();
			Maths::Store<Maths::HVec3>(normal, (MathsStorage::Float3*)&pVertex->nx);
			++pVertex;
#endif
		}

		pRendered->EndVertices(pVertex);
	}
}

void C_DebugRendered::Draw3dGrid(I_BeginEndVertices* pRendered, const Maths::HMat43& mat)
{
	for (S32 x = -10; x <= 10; x++)
	{
		if (x == 0)
		{
			Draw3dLine(pRendered, HVec3(x, 0.f, -10.f), HVec3(x, 0.f, 10.f), RGBA(0xff, 0xff, 0xff, 0xff), mat);
			Draw3dLine(pRendered, HVec3(-10.f, 0.f, x), HVec3(10.f, 0.f, x), RGBA(0xff, 0xff, 0xff, 0xff), mat);
		}
		else
		{
			Draw3dLine(pRendered, HVec3(x, 0.f, -10.f), HVec3(x, 0.f, 10.f), RGBA(0x00, 0x80, 0x80, 0xff), mat);
			Draw3dLine(pRendered, HVec3(-10.f, 0.f, x), HVec3(10.f, 0.f, x), RGBA(0x00, 0x80, 0x80, 0xff), mat);
		}
	}    
}
