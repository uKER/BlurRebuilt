#ifndef RENDER_IMMEDIATEDEBUGRENDER_H
#define RENDER_IMMEDIATEDEBUGRENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Render/Debug/DebugRender.h"
#include "Render/Debug/DebugRendered.h"
#include "Render/Primitive/PrimitiveRender.h"
//#include "Render/Font/Font.h"

namespace Render
{

namespace Font { class C_FontRenderer; }

namespace Debug
{

class C_ImmediateDebugRender : public DbUtil::I_DebugRender, public Display::I_Renderer, public Render::Debug::I_BeginEndVertices
{
public:

    C_ImmediateDebugRender();
    ~C_ImmediateDebugRender();

	bool OnCreate();
	void OnDestroy();

	// 2d rendering calls...

    void Draw2dLine(const Maths::HVec2& start, const Maths::HVec2& end, const Maths::RGBA colour);
    void Draw2dCircle(const Maths::HVec2& centre, const Maths::HVec2& size, const Maths::RGBA colour);
    void Draw2dArrow(const Maths::HVec2& start, const Maths::HVec2& dir, const F32 arrowheadSize, const Maths::RGBA colour);
    void Draw2dBar(const Maths::HVec2& pos, const Maths::HVec2& size, const Maths::RGBA colour);
	void Draw2dTriangle(const Maths::HVec2& A, const Maths::HVec2& B, const Maths::HVec2& C, const Maths::RGBA colour);

	void Draw2dText(const Maths::HVec2& pos, const Util::C_String& text);
	void Draw2dText(const Maths::HVec2& pos, const Util::C_String& text, const Maths::RGBA colour, const F32 height, const S32 flags=0);

    // 3d rendering calls...

    void Draw3dLine(const Maths::HVec3& start, const Maths::HVec3& end, const Maths::RGBA colour);
	void Draw3dLine(const Maths::HVec3& start, const Maths::HVec3& end, const Maths::RGBA colour, const Maths::HMat43& mat);
    void Draw3dLine(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::RGBA Acolour, const Maths::RGBA Bcolour);
    void Draw3dLines(const Maths::HVec3* startArray, const Maths::HVec3* endArray, const U32 count, const Maths::RGBA colour);

    void Draw3dTriangleOutline(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour);
    void Draw3dTriangle(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour);
	void Draw3dTriangleLit(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA colour);
    void Draw3dTriangle(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA Acolour, const Maths::RGBA Bcolour, const Maths::RGBA Ccolour);
	void Draw3dTriangleLit(const Maths::HVec3& A, const Maths::HVec3& B, const Maths::HVec3& C, const Maths::RGBA Acolour, const Maths::RGBA Bcolour, const Maths::RGBA Ccolour);
	void Draw3dTriangles( const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count);
	void Draw3dTrianglesLit( const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count);

    void Draw3dPoint(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour);
    void Draw3dCross(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour);
    void Draw3dStar(const Maths::HVec3& pos, const F32 size, const Maths::RGBA colour);

    void Draw3dPlanePoint(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour);
    void Draw3dPlaneCross(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour);
    void Draw3dPlaneStar(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA colour);
    void Draw3dPlane(const Maths::C_Plane& plane, const Maths::C_Vertex& referencePoint, const F32 size, const Maths::RGBA colour);

    void Draw3dArrow(const Maths::HVec3& pos, const Maths::HVec3& dir, const F32 lengthScale, const F32 arrowheadSize, const Maths::RGBA colour);

    void Draw3dCircle(const Maths::HVec3& centre, const Maths::HVec3& normal, const F32 radius, const Maths::RGBA colour);
    
    void Draw3dSphere(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA colour);
    void Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::RGBA colour);
    void Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::HMat43& transform, const Maths::RGBA colour);
    
	void Draw3dSphereOutline(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA colour);
    void Draw3dSphereOutline(const Maths::C_Sphere& sphere, const Maths::RGBA colour);

    void Draw3dMatrix(const Maths::HMat43& m, const Maths::HF32& size = 1.0f);

    void Draw3dBox(const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& m, const Maths::RGBA colour);
    void Draw3dBox(const Maths::C_AxisBox& box, const Maths::HMat43& m, const Maths::RGBA colour);
	void Draw3dBoxLit(const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& m, const Maths::RGBA colour);
    void Draw3dBoxLit(const Maths::C_AxisBox& box, const Maths::HMat43& m, const Maths::RGBA colour);
    
    void Draw3dGrid(const Maths::HMat43& matrix);

    void Draw3dText(const Maths::HVec3& pos, const Util::C_String text);
	void Draw3dText(const Maths::HVec3& pos, const F32 size, const Util::C_String text);

	// render states

	void SetDepthEnable(const bool depthEnable);
	void SetLightDirection(const Maths::HVec3& lightDirection);
	void SetBlendMode(const DbUtil::E_BlendMode blendMode);

	//Direct Access
	static C_ImmediateDebugRender* GetInstance();

	//Texture draw calls
	void DrawTextureTriangles2D(const Maths::HVec2* AArray, const Maths::HVec2* BArray, const Maths::HVec2* CArray, const Maths::HVec2* ATArray, const Maths::HVec2* BTArray, const Maths::HVec2* CTArray, const Maths::RGBA* colourArray, const U32 count, Display::C_Texture2D& texture2D);
	void Draw2DTextureBar(const Maths::HVec2& pos, const Maths::HVec2& size, const Maths::RGBA colour, Display::C_Texture2D& texture2D);

	void SetStates(bool b) {m_setStates = b;}

private:
	void LazyCreateFontRenderer();

	//void Begin3D(const Maths::HMat43& worldMatrix);
	//void Begin3DLit(const Maths::HMat43& worldMatrix);
	//void Begin2D();
	
	void Begin3DLit();
	void Begin3D();
	void Begin2D();
	void* BeginVertices(const Display::E_PrimitiveType primitiveType, const U32 vertexCount, const bool is3D, const bool lit = false);
	void EndVertices(void* pLockedMemory);

private:

	Display::C_VertexDeclaration*	m_pVertexDeclaration2D;
	Display::C_VertexDeclaration*	m_pVertexDeclaration3D;
	Display::C_VertexDeclaration*	m_pVertexDeclaration3DLit;
	Display::C_VertexDeclaration*	m_pVertexDeclaration2DTex;
	Display::C_VertexShader			m_VertexShader[Render::Primitive::E_PrimitiveRenderVertexShader_Count];
	Display::C_PixelShader			m_PixelShader[Render::Primitive::E_PrimitiveRenderPixelShader_Count];
	Display::C_VertexShader			m_VertexShader2DTex;
	Display::C_PixelShader			m_PixelShaderTex;


	Display::E_BlendMode			m_BlendMode;
	bool							m_DepthEnable;
	bool							m_setStates;
	
	Render::Font::C_FontRenderer*	m_pFontRenderer;
	//Resource::C_HandlePtr m_DebugFontPtr;
	//Font::C_FontStyle* m_pFontStyleLeft;
	//Font::C_FontStyle* m_pFontStyleCentre;
};

} // namespace Render
} // namespace Debug

#endif // RENDER_IMMEDIATEDEBUGRENDER_H
