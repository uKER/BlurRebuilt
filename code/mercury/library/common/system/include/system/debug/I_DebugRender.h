#ifndef I_DEBUGRENDER_H
#define I_DEBUGRENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\Maths.h>
#include <core\containers\String.h>

namespace DbUtil
{

class I_DebugRender;
typedef I_DebugRender* I_DebugRenderPtr;

class I_DebugRenderSys;
typedef I_DebugRenderSys* I_DebugRenderSysPtr;

enum E_BlendMode
{
	E_BlendMode_Solid = 0,
	E_BlendMode_SolidAlphaTest,
	E_BlendMode_SolidAlphaMask,
	E_BlendMode_Alpha,
	E_BlendMode_AlphaPreMultiply,
	E_BlendMode_Additive,
	E_BlendMode_AlphaAdditive,
	E_BlendMode_AlphaSubtractive,
	E_BlendMode_Subtractive,
	E_BlendMode_Multiply,
	E_BlendMode_AdditiveSmooth,
	E_BlendMode_Count,
	E_BlendMode_Default,
	E_BlendMode_Unknown,
};



class I_DebugRender
{
public:

	virtual ~I_DebugRender() {}

    // 2d rendering calls...

    virtual void    Draw2dLine(const Maths::HVec2& start, const Maths::HVec2& end, const Maths::RGBA colour) = 0;
    virtual void    Draw2dCircle(const Maths::HVec2& centre, const Maths::HVec2& size, const Maths::RGBA colour) = 0;
    virtual void    Draw2dArrow(const Maths::HVec2& start, const Maths::HVec2& dir, const F32 arrowheadSize, const Maths::RGBA colour) = 0;
    virtual void    Draw2dBar(const Maths::HVec2& pos, const Maths::HVec2& size, const Maths::RGBA colour) = 0;
    virtual void    Draw2dTriangle(const Maths::HVec2& A, const Maths::HVec2& B, const Maths::HVec2& C, const Maths::RGBA colour) = 0;
	
	virtual void    Draw2dText(const Maths::HVec2& pos, const Util::C_String& text) = 0;
	virtual void    Draw2dText(const Maths::HVec2& pos, const Util::C_String& text, const Maths::RGBA colour, const F32 height, const S32 flags=0) = 0;

    // 3d rendering calls...

    virtual void    Draw3dLine(const Maths::HVec3& start, const Maths::HVec3& end, const Maths::RGBA colour) = 0;
    virtual void    Draw3dLine(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::RGBA Acolour, const Maths::RGBA Bcolour) = 0;
    virtual void    Draw3dLines( const Maths::HVec3* startArray, const Maths::HVec3* endArray, const U32 count, const Maths::RGBA colour) = 0;

    virtual void    Draw3dTriangleOutline(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour) = 0;
    virtual void    Draw3dTriangle(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour) = 0;
	virtual void    Draw3dTriangleLit(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour) {Draw3dTriangle(A,B,C,colour);}
    virtual void    Draw3dTriangle(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA Acolour, const Maths::RGBA Bcolour, const Maths::RGBA Ccolour) = 0;
    virtual void    Draw3dTriangleLit(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA Acolour, const Maths::RGBA Bcolour, const Maths::RGBA Ccolour) {Draw3dTriangle(A,B,C,Acolour,Bcolour,Ccolour);}
    virtual void    Draw3dTriangles( const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count) = 0;
	virtual void    Draw3dTrianglesLit( const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count) {Draw3dTriangles(AArray, BArray, CArray, colourArray, count);}

    virtual void    Draw3dPoint(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour) = 0;
    virtual void    Draw3dCross(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour) = 0;
    virtual void    Draw3dStar(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour) = 0;

    virtual void    Draw3dPlanePoint(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour) = 0;
    virtual void    Draw3dPlaneCross(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour) = 0;
    virtual void    Draw3dPlaneStar(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour) = 0;
    virtual void    Draw3dPlane(const Maths::C_Plane& plane, const Maths::C_Vertex& referencePoint, const F32 size, const Maths::RGBA colour) = 0;
    
    virtual void    Draw3dArrow(const Maths::HVec3& pos, const Maths::HVec3& dir, const F32 lengthScale, const F32 arrowheadSize, const Maths::RGBA colour) = 0;
    
    virtual void    Draw3dCircle(const Maths::HVec3& centre, const Maths::HVec3& normal, const F32 radius, const Maths::RGBA colour) = 0;

    virtual void    Draw3dSphere(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA colour) = 0;
    virtual void    Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::RGBA colour) = 0;
    virtual void    Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::HMat43& transform, const Maths::RGBA colour) = 0;

	virtual void    Draw3dSphereOutline(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA colour) = 0;
    virtual void    Draw3dSphereOutline(const Maths::C_Sphere& sphere, const Maths::RGBA colour) = 0;

    virtual void    Draw3dMatrix(const Maths::HMat43& m, const Maths::HF32& size = 1.0f ) = 0;

    virtual void    Draw3dBox(const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& m, const Maths::RGBA colour) = 0;
    virtual void    Draw3dBox(const Maths::C_AxisBox& box, const Maths::HMat43& m, const Maths::RGBA colour) = 0;
	virtual void    Draw3dBoxLit(const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& m, const Maths::RGBA colour) {Draw3dBox(min,max,m,colour);}
	virtual void    Draw3dBoxLit(const Maths::C_AxisBox& box, const Maths::HMat43& m, const Maths::RGBA colour) {Draw3dBox(box,m,colour);}
    
    virtual void    Draw3dGrid(const Maths::HMat43& m) = 0;

    virtual void	Draw3dText(const Maths::HVec3& pos, const Util::C_String text) = 0;
    virtual void	Draw3dText(const Maths::HVec3& pos, const F32 size, const Util::C_String text) = 0;

	virtual void	SetLightDirection(const Maths::HVec3&) {};
	virtual void	SetBlendMode(const E_BlendMode) {};
	virtual void	SetDepthEnable(const bool depthEnable) = 0;

	virtual void	SetStates(bool b) {} // turn on/off setting of render states
};

class I_DebugRenderSys
{
public:

	virtual ~I_DebugRenderSys() {}

    /// Return an I_DebugRender which draws directly to the device.
    virtual I_DebugRenderPtr    Immediate() = 0;

    /// Return an I_DebugRender which will defer render calls until RenderDeferred is called.
    virtual I_DebugRenderPtr    Deferred() = 0;

    /// Return an I_DebugRender which will add render calls to a circular buffer which is rendered but not cleared when RenderDeferred is called.
    virtual I_DebugRenderPtr    Persistent() = 0;

	virtual I_DebugRenderPtr	Primitive()
	{
		return NULL;
	}
	virtual void PrimitiveEndFrame()
	{
	}

    
	// flush deferred render
	virtual void                RenderDeferred() = 0;
	//reset deferred render
	virtual void				ResetDeferred()
	{
	}
	// render primitive render
	virtual void                RenderPrimitive() = 0;    
	/// Clear any items from the persistent or deferred renderer
	virtual void                ResetPersistent() = 0; // made not pure virtual until implemented in game = 0;
};

/// A do nothing implementation of I_DebugRender.
I_DebugRenderPtr        NullDebugRender();

/// A do nothing implementation of I_DebugRenderSys.
I_DebugRenderSysPtr     NullDebugRenderSys();

/// The default implementation of I_DebugRenderSys.
I_DebugRenderSysPtr     DefaultDebugRenderSys();

/// Set the default implementation of I_DebugRenderSys.
void                    DefaultDebugRenderSys(I_DebugRenderSysPtr defaultRenderSysPtr);

}; // namespace DbUtil

#endif // DbUtil
