#ifndef RENDER_DEBUGRENDERED_H
#define RENDER_DEBUGRENDERED_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Debug
{

class I_BeginEndVertices
{
public:
	virtual void Begin3D() = 0;
	virtual void Begin3DLit() = 0;
	virtual void Begin2D() = 0;
	virtual void* BeginVertices(const Display::E_PrimitiveType primitiveType, const U32 vertexCount, const bool is3D = false, const bool lit = false) = 0;
	virtual void EndVertices(void* pLockedMemory) = 0;
};


class C_DebugRendered
{
public:

   // 2d rendering calls...

    static void Draw2dLine(I_BeginEndVertices* pRendered, const Maths::HVec2& start, const Maths::HVec2& end, const Maths::RGBA colour);
    static void Draw2dCircle(I_BeginEndVertices* pRendered, const Maths::HVec2& centre, const Maths::HVec2& size, const Maths::RGBA colour);
    static void Draw2dArrow(I_BeginEndVertices* pRendered, const Maths::HVec2& start, const Maths::HVec2& dir, const F32 arrowheadSize, const Maths::RGBA colour);
    static void Draw2dBar(I_BeginEndVertices* pRendered, const Maths::HVec2& pos, const Maths::HVec2& size, const Maths::RGBA colour);
	static void Draw2dTriangle(I_BeginEndVertices* pRendered, const Maths::HVec2& A, const Maths::HVec2& B, const Maths::HVec2& C, const Maths::RGBA colour);

	static void Draw2dText(I_BeginEndVertices* pRendered, const Maths::HVec2& pos, const Util::C_String& text);
	static void Draw2dText(I_BeginEndVertices* pRendered, const Maths::HVec2& pos, const Util::C_String& text, const Maths::RGBA colour, const F32 height, const S32 flags=0);

    // 3d rendering calls...

    static void Draw3dLine(I_BeginEndVertices* pRendered, const Maths::HVec3& start, const Maths::HVec3& end, const Maths::RGBA colour);
	static void Draw3dLine(I_BeginEndVertices* pRendered, const Maths::HVec3& start, const Maths::HVec3& end, const Maths::RGBA colour, const Maths::HMat43& mat);
    static void Draw3dLine(I_BeginEndVertices* pRendered, const Maths::HVec3& A, const Maths::HVec3& B, const Maths::RGBA Acolour, const Maths::RGBA Bcolour);
    static void Draw3dLines(I_BeginEndVertices* pRendered, const Maths::HVec3* startArray, const Maths::HVec3* endArray, const U32 count, const Maths::RGBA colour);

    static void Draw3dTriangleOutline(I_BeginEndVertices* pRendered, const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour);
    static void Draw3dTriangle(I_BeginEndVertices* pRendered, const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour);
	static void Draw3dTriangleLit(I_BeginEndVertices* pRendered, const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour);
    static void Draw3dTriangle(I_BeginEndVertices* pRendered, const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA Acolour, const Maths::RGBA Bcolour, const Maths::RGBA Ccolour);
	static void Draw3dTriangleLit(I_BeginEndVertices* pRendered, const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA Acolour, const Maths::RGBA Bcolour, const Maths::RGBA Ccolour);
	static void Draw3dTriangles(I_BeginEndVertices* pRendered,  const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count);
	static void Draw3dTrianglesLit(I_BeginEndVertices* pRendered,  const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count);

    static void Draw3dPoint(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour);
    static void Draw3dCross(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour);
    static void Draw3dStar(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour);

    static void Draw3dPlanePoint(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour);
    static void Draw3dPlaneCross(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour);
    static void Draw3dPlaneStar(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour);
    static void Draw3dPlane(I_BeginEndVertices* pRendered, const Maths::C_Plane& plane, const Maths::C_Vertex& referencePoint, const F32 size, const Maths::RGBA colour);

    static void Draw3dArrow(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const Maths::HVec3& dir, const F32 lengthScale, const F32 arrowheadSize, const Maths::RGBA colour);

    static void Draw3dCircle(I_BeginEndVertices* pRendered, const Maths::HVec3& centre, const Maths::HVec3& normal, const F32 radius, const Maths::RGBA colour);
    
    static void Draw3dSphere(I_BeginEndVertices* pRendered, const Maths::HVec3& centre, const F32 radius, const Maths::RGBA colour);
    static void Draw3dSphere(I_BeginEndVertices* pRendered, const Maths::C_Sphere& sphere, const Maths::RGBA colour);
    static void Draw3dSphere(I_BeginEndVertices* pRendered, const Maths::C_Sphere& sphere, const Maths::HMat43& transform, const Maths::RGBA colour);
    
	static void Draw3dSphereOutline(I_BeginEndVertices* pRendered, const Maths::HVec3& centre, const F32 radius, const Maths::RGBA colour);
    static void Draw3dSphereOutline(I_BeginEndVertices* pRendered, const Maths::C_Sphere& sphere, const Maths::RGBA colour);

    static void Draw3dMatrix(I_BeginEndVertices* pRendered, const Maths::HMat43& m, const Maths::HF32& size = 1.0f);

    static void Draw3dBox(I_BeginEndVertices* pRendered, const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& m, const Maths::RGBA colour, bool setStates = true);
    static void Draw3dBox(I_BeginEndVertices* pRendered, const Maths::C_AxisBox& box, const Maths::HMat43& m, const Maths::RGBA colour, bool setStates = true);
	static void Draw3dBoxLit(I_BeginEndVertices* pRendered, const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& m, const Maths::RGBA colour);
    static void Draw3dBoxLit(I_BeginEndVertices* pRendered, const Maths::C_AxisBox& box, const Maths::HMat43& m, const Maths::RGBA colour);
    
    static void Draw3dGrid(I_BeginEndVertices* pRendered, const Maths::HMat43& matrix);

    static void Draw3dText(I_BeginEndVertices* pRendered, const Maths::HVec3& pos, const Util::C_String text);
};


} // namespace Debug
} // namespace Render

#endif // RENDER_DEBUGRENDERED_H
