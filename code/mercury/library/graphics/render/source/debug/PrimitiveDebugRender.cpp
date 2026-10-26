#include "stdafx.h"
#include "Render/Debug/PrimitiveDebugRender.h"
#include "Render/Shaders/Constants.h"

// use display's basic font rendering
#include "Display/Render/FontRender.h"
#include "Render/Debug/DebugRendered.h"

#include "Render/Font/Font.h"

EXTERN_SHADER(vsDebug2D);
EXTERN_SHADER(vsDebug3D);
EXTERN_SHADER(vsDebug3DLit);
EXTERN_SHADER(psDebug);

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/Debug/PS3/vsDebug2D.shader"
#include "Render/Shaders/Debug/PS3/vsDebug3D.shader"
#include "Render/Shaders/Debug/PS3/psDebug.shader"
#endif
#endif

using namespace Maths;

using namespace Render::Debug;


C_PrimitiveDebugRender::C_PrimitiveDebugRender() : Display::I_Renderer("ImmediateDebugRender"), m_pFontRenderer(NULL)
{
}

C_PrimitiveDebugRender::~C_PrimitiveDebugRender()
{
	delete m_pFontRenderer;
}

void C_PrimitiveDebugRender::LazyCreateFontRenderer()
{
	if (m_pFontRenderer) return;

	if (Render::Font::FontSystem())
	{
		m_pFontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
		if (m_pFontRenderer)
		{
			m_pFontRenderer->SetScaleX(24.0f);
			m_pFontRenderer->SetScaleY(32.0f);
			m_pFontRenderer->SetShadowEnable( true );
		}
	}
}


bool C_PrimitiveDebugRender::OnCreate()
{
	return true;
}

void C_PrimitiveDebugRender::OnDestroy()
{

}


void C_PrimitiveDebugRender::Draw2dLine(const Maths::HVec2& posA, const Maths::HVec2& posB, const Maths::RGBA col)
{
	C_DebugRendered::Draw2dLine(this, posA, posB, col);
}

void C_PrimitiveDebugRender::Draw2dCircle(const Maths::HVec2& centre, const Maths::HVec2& size, const Maths::RGBA col)
{
	C_DebugRendered::Draw2dCircle(this, centre, size, col);
}

void C_PrimitiveDebugRender::Draw2dTriangle(const Maths::HVec2& A, const Maths::HVec2& B, const Maths::HVec2& C, const Maths::RGBA colour)
{
	//C_DebugRendered::Draw2dTriangle(this, A, B, C, col);
}

void C_PrimitiveDebugRender::Draw2dArrow(const Maths::HVec2& start, const Maths::HVec2& dir, const F32 arrowheadSize, const Maths::RGBA col)
{
	C_DebugRendered::Draw2dArrow(this, start, dir, arrowheadSize, col);
}

void C_PrimitiveDebugRender::Draw2dBar(const Maths::HVec2& pos, const Maths::HVec2& size, const Maths::RGBA col)
{
	C_DebugRendered::Draw2dBar(this, pos, size, col);
}

void C_PrimitiveDebugRender::Draw2dText(const Maths::HVec2& pos, const Util::C_String& text)
{
	LazyCreateFontRenderer();
	m_pFontRenderer->Print( pos, text.Chars() );
	//Display::FontRender()->Draw(text.Chars(), pos.X(), pos.Y(), 20, 20, Maths::RGBA(255, 255, 255, 255));
}

void C_PrimitiveDebugRender::Draw2dText(const Maths::HVec2& pos, const Util::C_String& text, const Maths::RGBA colour, const F32 height, const S32 flags)
{
	LazyCreateFontRenderer();
	m_pFontRenderer->SetColourTop( colour );
	m_pFontRenderer->SetColourBottom( Maths::C_RGBA_F32(colour) * 0.75f );
	m_pFontRenderer->SetScaleX( height * 0.75f );
	m_pFontRenderer->SetScaleY( height );

	m_pFontRenderer->Print( pos, text.Chars() );

	m_pFontRenderer->SetColourTop();
	m_pFontRenderer->SetColourBottom();
	m_pFontRenderer->SetScaleX(24.0f);
	m_pFontRenderer->SetScaleY(32.0f);

	//Display::FontRender()->Draw(text.Chars(), pos.X(), pos.Y(), height, height, colour);
}

void C_PrimitiveDebugRender::Draw3dLine(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dLine(this, posA, posB, col);
}

void C_PrimitiveDebugRender::Draw3dLines(const Maths::HVec3* posA, const Maths::HVec3* posB, const U32 lineCount, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dLines(this, posA, posB, lineCount, col);
}

void C_PrimitiveDebugRender::Draw3dLine(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::RGBA colA, const Maths::RGBA colB)
{
	C_DebugRendered::Draw3dLine(this, posA, posB, colA, colB);
}

void C_PrimitiveDebugRender::Draw3dTriangleOutline(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dTriangleOutline(this, posA, posB, posC, col);
}

void C_PrimitiveDebugRender::Draw3dTriangle(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dTriangle(this, posA, posB, posC, col);
}

void C_PrimitiveDebugRender::Draw3dTriangleLit(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dTriangleLit(this, posA, posB, posC, col);
}

void C_PrimitiveDebugRender::Draw3dTriangle(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA colA, const Maths::RGBA colB, const Maths::RGBA colC)
{
	C_DebugRendered::Draw3dTriangle(this, posA, posB, posC, colA, colB, colC);
}

void C_PrimitiveDebugRender::Draw3dTriangleLit(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA colA, const Maths::RGBA colB, const Maths::RGBA colC)
{
	C_DebugRendered::Draw3dTriangleLit(this, posA, posB, posC, colA, colB, colC);
}

void C_PrimitiveDebugRender::Draw3dTriangles( const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count)
{
	C_DebugRendered::Draw3dTriangles(this, AArray, BArray, CArray, colourArray, count);
}

void C_PrimitiveDebugRender::Draw3dTrianglesLit( const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count)
{
	C_DebugRendered::Draw3dTrianglesLit(this, AArray, BArray, CArray, colourArray, count);
}


void C_PrimitiveDebugRender::Draw3dPoint(const Maths::HVec3& pos, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dPoint(this, pos, size, col);
}

void C_PrimitiveDebugRender::Draw3dCross(const Maths::HVec3& pos, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dCross(this, pos, size, col);
}

void C_PrimitiveDebugRender::Draw3dStar(const Maths::HVec3& pos, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dStar(this, pos, size, col);
}

void C_PrimitiveDebugRender::Draw3dPlanePoint(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dPlanePoint(this, pos, normal, size, col);
}

void C_PrimitiveDebugRender::Draw3dPlaneCross(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dPlaneCross(this, pos, normal, size, col);
}

void C_PrimitiveDebugRender::Draw3dPlaneStar(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dPlaneStar(this, pos, normal, size, col);
}

void C_PrimitiveDebugRender::Draw3dPlane(const Maths::C_Plane& plane, const Maths::C_Vertex& referencePoint, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dPlane(this, plane, referencePoint, size, col);
}

void C_PrimitiveDebugRender::Draw3dArrow(const Maths::HVec3& pos, const Maths::HVec3& dir, const F32 lengthScale, const F32 arrowheadSize, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dArrow(this, pos, dir, lengthScale, arrowheadSize, col);
}

void C_PrimitiveDebugRender::Draw3dCircle(const Maths::HVec3& centre, const Maths::HVec3& normal, const F32 radius, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dCircle(this, centre, normal, radius, col);
}

void C_PrimitiveDebugRender::Draw3dSphere(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dSphere(this, centre, radius, col);
}

void C_PrimitiveDebugRender::Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dSphere(this, sphere, col);
}

void C_PrimitiveDebugRender::Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::HMat43& transform, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dSphere(this, sphere, transform, col);
}

void C_PrimitiveDebugRender::Draw3dSphereOutline(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dSphereOutline(this, centre, radius, col);
}

void C_PrimitiveDebugRender::Draw3dSphereOutline(const Maths::C_Sphere& sphere, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dSphereOutline(this, sphere, col);
}

void C_PrimitiveDebugRender::Draw3dMatrix(const Maths::HMat43& mat, const Maths::HF32& size)
{
	C_DebugRendered::Draw3dMatrix(this, mat, size);
}

void C_PrimitiveDebugRender::Draw3dBox(const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& mat, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dBox(this, min, max, mat, col);
}

void C_PrimitiveDebugRender::Draw3dBox(const Maths::C_AxisBox& box, const Maths::HMat43& mat, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dBox(this, box, mat, col);
}

void C_PrimitiveDebugRender::Draw3dBoxLit(const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& mat, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dBoxLit(this, min, max, mat, col);
}

void C_PrimitiveDebugRender::Draw3dBoxLit(const Maths::C_AxisBox& box, const Maths::HMat43& mat, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dBoxLit(this, box, mat, col);
}

void C_PrimitiveDebugRender::Draw3dGrid(const Maths::HMat43& mat)
{
	C_DebugRendered::Draw3dGrid(this, mat);
}

void C_PrimitiveDebugRender::Draw3dText(const Maths::HVec3& pos, const Util::C_String text)
{
	LazyCreateFontRenderer();
	m_pFontRenderer->Print3D( pos, text.Chars(), Render::Font::E_Justification_Centre );

	/*
	// get view settings
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	// get projection matrix
	Maths::HMat44 mViewProj(Maths::Load<Maths::HMat44>(&viewSettings3D.GetViewProjMatrix()));
	// construct HVec4 from HVec3 which was 1.0 in W
	Maths::HVec4 position(pos);
	position.W(Maths::Constants<Maths::HF32>::One());

	// project position and do W divide
	position = position * mViewProj;
	if( position.W() < 0.0f )
	return;
	position /= position.W();

	Maths::HVec2 multiply(Maths::Constants<Maths::HF32>::Half(), -Maths::Constants<Maths::HF32>::Half());
	Maths::HVec2 add(Maths::Constants<Maths::HF32>::Half(), Maths::Constants<Maths::HF32>::Half());

	// scale -1 to 1 result into DeviceCoords
	Maths::HVec2 screenPosition(position.X(), position.Y());
	screenPosition = screenPosition * multiply + add;
	screenPosition *= Display::C_ViewSetup::GetDeviceCoords().Max(); 

	Draw2dText(screenPosition, text);
	*/
}

void C_PrimitiveDebugRender::Draw3dText(const Maths::HVec3& pos, const F32 size, const Util::C_String text)
{
	LazyCreateFontRenderer();
	m_pFontRenderer->Print3D( pos, size, text.Chars(), Render::Font::E_Justification_Centre );
}

void C_PrimitiveDebugRender::SetDepthEnable(const bool depthEnable)
{
	m_PrimitiveRender.SetDepthEnable(depthEnable);
}

void C_PrimitiveDebugRender::SetLightDirection(const Maths::HVec3& lightDirection)
{
	m_PrimitiveRender.SetLightDirection(lightDirection);
}

void Render::Debug::C_PrimitiveDebugRender::SetBlendMode( const DbUtil::E_BlendMode blendMode )
{
	m_PrimitiveRender.SetBlendMode(Display::E_BlendMode(blendMode));
}

void C_PrimitiveDebugRender::Begin3D()
{
	m_PrimitiveRender.Begin3D();
}

void C_PrimitiveDebugRender::Begin3DLit()
{
	m_PrimitiveRender.Begin3D();
}

void C_PrimitiveDebugRender::Begin2D()
{
	m_PrimitiveRender.Begin2D();
}

void* C_PrimitiveDebugRender::BeginVertices( const Display::E_PrimitiveType primitiveType, const U32 vertexCount, const bool is3D, const bool lit)
{
	if (is3D)
		m_PrimitiveRender.Begin3D();
	else
		m_PrimitiveRender.Begin2D();

	return m_PrimitiveRender.BeginVertices(primitiveType, vertexCount, lit);
}

void C_PrimitiveDebugRender::EndVertices( void* pLockedMemory )
{
	m_PrimitiveRender.EndVertices(pLockedMemory);
}

void Render::Debug::C_PrimitiveDebugRender::Render()
{
	m_PrimitiveRender.Render();
}

void Render::Debug::C_PrimitiveDebugRender::EndFrame()
{
	m_PrimitiveRender.EndFrame();
}
