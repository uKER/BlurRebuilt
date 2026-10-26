#include "SystemPch.h"

#include <system/debug/I_DebugRender.h>

namespace DbUtil
{

class C_NullDebugRender : public I_DebugRender
{
public:

    // 2D Rendering Calls

    virtual void    Draw2dLine(const Maths::HVec2& start, const Maths::HVec2& end, const Maths::RGBA colour) {}
    virtual void    Draw2dCircle(const Maths::HVec2& centre, const Maths::HVec2& size, const Maths::RGBA colour) {}
    virtual void    Draw2dArrow(const Maths::HVec2& start, const Maths::HVec2& dir, const F32 arrowheadSize, const Maths::RGBA colour) {}
    virtual void    Draw2dBar(const Maths::HVec2& pos, const Maths::HVec2& size, const Maths::RGBA colour) {}
	virtual void Draw2dTriangle(const Maths::HVec2& A, const Maths::HVec2& B, const Maths::HVec2& C, const Maths::RGBA colour) {}

	virtual void    Draw2dText(const Maths::HVec2& pos, const Util::C_String& text) {}
	virtual void    Draw2dText(const Maths::HVec2& pos, const Util::C_String& text, const Maths::RGBA colour, const F32 height, const S32 flags=0) {};

    // 3D Rendering Calls

    virtual void    Draw3dLine(const Maths::HVec3& start, const Maths::HVec3& end, const Maths::RGBA colour) {}
    virtual void    Draw3dLine(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::RGBA Acolour, const Maths::RGBA Bcolour) {}
    virtual void    Draw3dLines(const Maths::HVec3* startArray, const Maths::HVec3* endArray, const U32 count, const Maths::RGBA colour) {};

    virtual void    Draw3dTriangleOutline( const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour) {}
    virtual void    Draw3dTriangle(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour) {}
    virtual void    Draw3dTriangle(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA Acolour, const Maths::RGBA Bcolour, const Maths::RGBA Ccolour) {}
	virtual void	Draw3dTriangles( const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count) {}

    virtual void    Draw3dPoint(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour) {}
    virtual void    Draw3dCross(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour) {}
    virtual void    Draw3dStar(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour) {}

    virtual void    Draw3dPlanePoint(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour) {}
    virtual void    Draw3dPlaneCross(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour) {}
    virtual void    Draw3dPlaneStar(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour) {}
    virtual void    Draw3dPlane(const Maths::C_Plane& plane, const Maths::C_Vertex& referencePoint, const F32 size, const Maths::RGBA colour ) {}

    virtual void    Draw3dArrow(const Maths::HVec3& pos, const Maths::HVec3& dir, const F32 length_scale, const F32 arrowhead_size, const Maths::RGBA colour) {}

    virtual void    Draw3dCircle(const Maths::HVec3& centre, const Maths::HVec3& normal, const F32 radius, const Maths::RGBA colour) {}

    virtual void    Draw3dSphere(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA colour) {}
    virtual void    Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::RGBA colour) {}
    virtual void    Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::HMat43& transform, const Maths::RGBA colour) {};

	virtual void    Draw3dSphereOutline(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA colour) {}
    virtual void    Draw3dSphereOutline(const Maths::C_Sphere& sphere, const Maths::RGBA colour) {}

    virtual void    Draw3dMatrix(const Maths::HMat43& m, const Maths::HF32& size = 1.0f) {}

    virtual void    Draw3dBox( const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& m, const Maths::RGBA colour ) {}
    virtual void    Draw3dBox( const Maths::C_AxisBox& box, const Maths::HMat43& m, const Maths::RGBA colour ) {}
    
    virtual void    Draw3dGrid(const Maths::HMat43& m) {}

    virtual void	Draw3dText(const Maths::HVec3& pos, const Util::C_String text) {}
    virtual void	Draw3dText(const Maths::HVec3& pos, const F32 size, const Util::C_String text) {}

	virtual void	SetDepthEnable(const bool depthEnable) {}
};

class C_NullDebugRenderSys : public I_DebugRenderSys
{
public:
	C_NullDebugRenderSys()
	{
		m_pNull = new C_NullDebugRender();
	}
	~C_NullDebugRenderSys()
	{
		delete m_pNull;
		m_pNull = 0;
	}

    virtual I_DebugRenderPtr    Immediate()     { return m_pNull; }
    virtual I_DebugRenderPtr    Deferred()      { return m_pNull; }
    virtual I_DebugRenderPtr    Persistent()    { return m_pNull; }

    virtual void                RenderDeferred()    {}
	virtual void                ResetPersistent() {}
	virtual void                RenderPrimitive() {}

private:
	C_NullDebugRender* m_pNull;
};

I_DebugRenderPtr NullDebugRender()
{
    return new C_NullDebugRender();
}

I_DebugRenderSysPtr NullDebugRenderSys()
{
    return new C_NullDebugRenderSys();
}

C_NullDebugRenderSys g_DefaultDebugRenderSys;

I_DebugRenderSysPtr g_DefaultRenderSys = &g_DefaultDebugRenderSys;

I_DebugRenderSysPtr DefaultDebugRenderSys()
{
    return g_DefaultRenderSys;
}

void DefaultDebugRenderSys(I_DebugRenderSysPtr defaultRenderSysPtr)
{
    g_DefaultRenderSys = defaultRenderSysPtr;
}

} // namespace DbUtil
