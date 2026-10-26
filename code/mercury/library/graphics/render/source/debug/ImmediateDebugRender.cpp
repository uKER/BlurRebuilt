#include "stdafx.h"
#include "Render/Debug/ImmediateDebugRender.h"
#include "Render/Shaders/Constants.h"

// use display's basic font rendering
#include "Display/Render/FontRender.h"
#include "Render/Debug/DebugRendered.h"
#include "Render/Font/Font.h"

EXTERN_SHADER(vsPrimitive2D);
EXTERN_SHADER(vsPrimitive3D);
EXTERN_SHADER(vsPrimitive3DLit);
EXTERN_SHADER(psPrimitive);
EXTERN_SHADER(vsPrimitiveTex2D);
EXTERN_SHADER(psPrimitiveTex);

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/Debug/PS3/vsPrimitive2D.shader"
#include "Render/Shaders/Debug/PS3/vsPrimitiveTex2D.shader"
#include "Render/Shaders/Debug/PS3/vsPrimitive3D.shader"
#include "Render/Shaders/Debug/PS3/psPrimitive.shader"
#include "Render/Shaders/Debug/PS3/psPrimitiveTex.shader"
#endif
#endif

using namespace Maths;
using namespace Render::Debug;
using namespace Render::Primitive;


struct C_TexturedVertex2D
{
	F32 px, py;
    
	U32 c;

	F32 tu, tv;

	void Pos(const F32 x, const F32 y) { px = x; py = y; }

	void Pos(const Maths::HVec2& p) { px = p.X(); py = p.Y(); }

	void Col(const Maths::RGBA col) { c = Display::GPU_RGBA(col); }

	void Tex(const F32 u, const F32 v) { tu = u; tv = v; }

	void Tex(const Maths::HVec2& t) { tu = t.X(); tv = t.Y(); }
};



C_ImmediateDebugRender* pImmediateDebugRender = NULL;

C_ImmediateDebugRender::C_ImmediateDebugRender() : Display::I_Renderer("ImmediateDebugRender"), m_pVertexDeclaration2D(NULL), m_pVertexDeclaration3D(NULL), m_pVertexDeclaration3DLit(NULL), m_pVertexDeclaration2DTex(NULL), m_BlendMode(Display::E_BlendMode_Solid), m_pFontRenderer(NULL), m_DepthEnable(true)
{
	pImmediateDebugRender = this;
	m_setStates = true;
}

C_ImmediateDebugRender::~C_ImmediateDebugRender()
{
	DestroyRenderer();
	delete m_pFontRenderer;
}

void C_ImmediateDebugRender::LazyCreateFontRenderer()
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

bool C_ImmediateDebugRender::OnCreate()
{
	RESOURCE_MEMORY_ID("ImmediateDebugRender");

	if(!m_pVertexDeclaration2D)
	{
		Display::C_VertexFormat vertexFormat2D;
		vertexFormat2D.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_Position, 0);
		vertexFormat2D.AddElement(0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
		m_pVertexDeclaration2D = Display::Device()->CreateVertexDeclaration(vertexFormat2D);
	}

	if(!m_pVertexDeclaration3D)
	{
		Display::C_VertexFormat vertexFormat3D;
		vertexFormat3D.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
		vertexFormat3D.AddElement(0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
		m_pVertexDeclaration3D = Display::Device()->CreateVertexDeclaration(vertexFormat3D);
	}

	if(!m_pVertexDeclaration3DLit)
	{
		Display::C_VertexFormat vertexFormat3D;
		vertexFormat3D.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
		vertexFormat3D.AddElement(0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
		vertexFormat3D.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_TexCoord, 0);
		m_pVertexDeclaration3DLit = Display::Device()->CreateVertexDeclaration(vertexFormat3D);
	}
	if(!m_pVertexDeclaration2DTex)
	{
		Display::C_VertexFormat vertexFormat2DTex;
		vertexFormat2DTex.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_Position, 0);
		vertexFormat2DTex.AddElement(0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
		vertexFormat2DTex.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0);
		m_pVertexDeclaration2DTex = Display::Device()->CreateVertexDeclaration(vertexFormat2DTex);
	}

	if (!Display::CreateVertexShaderFromFunc("Shaders/vsPrimitive2D", 0, &m_VertexShader[E_PrimitiveRenderVertexShader_2D], REF_SHADER(vsPrimitive2D)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsPrimitive3D", 0, &m_VertexShader[E_PrimitiveRenderVertexShader_3D], REF_SHADER(vsPrimitive3D)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsPrimitive3D", 0, &m_VertexShader[E_PrimitiveRenderVertexShader_3DLit], REF_SHADER(vsPrimitive3DLit)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psPrimitive", 0, &m_PixelShader[E_PrimitiveRenderPixelShader_Default], REF_SHADER(psPrimitive)))
		return false;

	if (!Display::CreateVertexShaderFromFunc("Shaders/vsPrimitiveTex2D", 0, &m_VertexShader2DTex, REF_SHADER(vsPrimitiveTex2D)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psPrimitiveTex", 0, &m_PixelShaderTex, REF_SHADER(psPrimitiveTex)))
		return false;


	return true;
}

void C_ImmediateDebugRender::OnDestroy()
{
	Display::Device()->Destroy(&m_VertexShader[E_PrimitiveRenderVertexShader_2D]);
	Display::Device()->Destroy(&m_VertexShader[E_PrimitiveRenderVertexShader_3D]);
	Display::Device()->Destroy(&m_VertexShader[E_PrimitiveRenderVertexShader_3DLit]);
	Display::Device()->Destroy(&m_PixelShader[E_PrimitiveRenderPixelShader_Default]);
	Display::Device()->Destroy(&m_VertexShader2DTex);
	Display::Device()->Destroy(&m_PixelShaderTex);
}


void C_ImmediateDebugRender::Draw2dLine(const Maths::HVec2& posA, const Maths::HVec2& posB, const Maths::RGBA col)
{
	C_DebugRendered::Draw2dLine(this, posA, posB, col);
}

void C_ImmediateDebugRender::Draw2dCircle(const Maths::HVec2& centre, const Maths::HVec2& size, const Maths::RGBA col)
{
	C_DebugRendered::Draw2dCircle(this, centre, size, col);
}

void C_ImmediateDebugRender::Draw2dArrow(const Maths::HVec2& start, const Maths::HVec2& dir, const F32 arrowheadSize, const Maths::RGBA col)
{
	C_DebugRendered::Draw2dArrow(this, start, dir, arrowheadSize, col);
}

void C_ImmediateDebugRender::Draw2dTriangle(const Maths::HVec2& A, const Maths::HVec2& B, const Maths::HVec2& C, const Maths::RGBA colour)
{
	C_DebugRendered::Draw2dTriangle(this, A, B, C, colour);
}

void C_ImmediateDebugRender::Draw2dBar(const Maths::HVec2& pos, const Maths::HVec2& size, const Maths::RGBA col)
{
	C_DebugRendered::Draw2dBar(this, pos, size, col);
}

void C_ImmediateDebugRender::Draw2dText(const Maths::HVec2& pos, const Util::C_String& text)
{
	LazyCreateFontRenderer();
	m_pFontRenderer->Print( pos, text.Chars() );
	//Display::FontRender()->Draw(text.Chars(), pos.X(), pos.Y(), 20, 20, Maths::RGBA(255, 255, 255, 255));
}

void C_ImmediateDebugRender::Draw2dText(const Maths::HVec2& pos, const Util::C_String& text, const Maths::RGBA colour, const F32 height, const S32 flags)
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

void C_ImmediateDebugRender::Draw3dLine(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dLine(this, posA, posB, col);
}

void C_ImmediateDebugRender::Draw3dLines(const Maths::HVec3* posA, const Maths::HVec3* posB, const U32 lineCount, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dLines(this, posA, posB, lineCount, col);
}

void C_ImmediateDebugRender::Draw3dLine(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::RGBA colA, const Maths::RGBA colB)
{
	C_DebugRendered::Draw3dLine(this, posA, posB, colA, colB);
}

void C_ImmediateDebugRender::Draw3dTriangleOutline(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dTriangleOutline(this, posA, posB, posC, col);
}

void C_ImmediateDebugRender::Draw3dTriangle(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dTriangle(this, posA, posB, posC, col);
}

void C_ImmediateDebugRender::Draw3dTriangleLit(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dTriangleLit(this, posA, posB, posC, col);
}

void C_ImmediateDebugRender::Draw3dTriangle(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA colA, const Maths::RGBA colB, const Maths::RGBA colC)
{
	C_DebugRendered::Draw3dTriangle(this, posA, posB, posC, colA, colB, colC);
}

void C_ImmediateDebugRender::Draw3dTriangleLit(const Maths::HVec3& posA, const Maths::HVec3& posB, const Maths::HVec3& posC, const Maths::RGBA colA, const Maths::RGBA colB, const Maths::RGBA colC)
{
	C_DebugRendered::Draw3dTriangleLit(this, posA, posB, posC, colA, colB, colC);
}

void C_ImmediateDebugRender::Draw3dTriangles( const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count)
{
	C_DebugRendered::Draw3dTriangles(this, AArray, BArray, CArray, colourArray, count);
}

void C_ImmediateDebugRender::Draw3dTrianglesLit( const Maths::HVec3* AArray, const Maths::HVec3* BArray, const Maths::HVec3* CArray, const Maths::RGBA* colourArray, const U32 count)
{
	C_DebugRendered::Draw3dTrianglesLit(this, AArray, BArray, CArray, colourArray, count);
}


void C_ImmediateDebugRender::Draw3dPoint(const Maths::HVec3& pos, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dPoint(this, pos, size, col);
}

void C_ImmediateDebugRender::Draw3dCross(const Maths::HVec3& pos, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dCross(this, pos, size, col);
}

void C_ImmediateDebugRender::Draw3dStar(const Maths::HVec3& pos, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dStar(this, pos, size, col);
}

void C_ImmediateDebugRender::Draw3dPlanePoint(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dPlanePoint(this, pos, normal, size, col);
}

void C_ImmediateDebugRender::Draw3dPlaneCross(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dPlaneCross(this, pos, normal, size, col);
}

void C_ImmediateDebugRender::Draw3dPlaneStar(const Maths::HVec3& pos, const Maths::HVec3& normal, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dPlaneStar(this, pos, normal, size, col);
}

void C_ImmediateDebugRender::Draw3dPlane(const Maths::C_Plane& plane, const Maths::C_Vertex& referencePoint, const F32 size, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dPlane(this, plane, referencePoint, size, col);
}

void C_ImmediateDebugRender::Draw3dArrow(const Maths::HVec3& pos, const Maths::HVec3& dir, const F32 lengthScale, const F32 arrowheadSize, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dArrow(this, pos, dir, lengthScale, arrowheadSize, col);
}

void C_ImmediateDebugRender::Draw3dCircle(const Maths::HVec3& centre, const Maths::HVec3& normal, const F32 radius, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dCircle(this, centre, normal, radius, col);
}

void C_ImmediateDebugRender::Draw3dSphere(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dSphere(this, centre, radius, col);
}

void C_ImmediateDebugRender::Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dSphere(this, sphere, col);
}

void C_ImmediateDebugRender::Draw3dSphere(const Maths::C_Sphere& sphere, const Maths::HMat43& transform, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dSphere(this, sphere, transform, col);
}

void C_ImmediateDebugRender::Draw3dSphereOutline(const Maths::HVec3& centre, const F32 radius, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dSphereOutline(this, centre, radius, col);
}

void C_ImmediateDebugRender::Draw3dSphereOutline(const Maths::C_Sphere& sphere, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dSphereOutline(this, sphere, col);
}

void C_ImmediateDebugRender::Draw3dMatrix(const Maths::HMat43& mat, const Maths::HF32& size)
{
	C_DebugRendered::Draw3dMatrix(this, mat, size);
}

void C_ImmediateDebugRender::Draw3dBox(const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& mat, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dBox(this, min, max, mat, col, m_setStates);
}

void C_ImmediateDebugRender::Draw3dBox(const Maths::C_AxisBox& box, const Maths::HMat43& mat, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dBox(this, box, mat, col, m_setStates);
}

void C_ImmediateDebugRender::Draw3dBoxLit(const Maths::HVec3& min, const Maths::HVec3& max, const Maths::HMat43& mat, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dBoxLit(this, min, max, mat, col);
}

void C_ImmediateDebugRender::Draw3dBoxLit(const Maths::C_AxisBox& box, const Maths::HMat43& mat, const Maths::RGBA col)
{
	C_DebugRendered::Draw3dBoxLit(this, box, mat, col);
}

void C_ImmediateDebugRender::Draw3dGrid(const Maths::HMat43& mat)
{
	C_DebugRendered::Draw3dGrid(this, mat);
}

void C_ImmediateDebugRender::Draw3dText(const Maths::HVec3& pos, const Util::C_String text)
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

void C_ImmediateDebugRender::Draw3dText(const Maths::HVec3& pos, const F32 size, const Util::C_String text)
{
	if (text.Length() > 0)
	{
		LazyCreateFontRenderer();
		m_pFontRenderer->Print3D( pos, size, text.Chars(), Render::Font::E_Justification_Centre );
	}
}

void C_ImmediateDebugRender::SetDepthEnable(const bool depthEnable)
{
	m_DepthEnable = depthEnable;
}

void C_ImmediateDebugRender::SetLightDirection(const Maths::HVec3& lightDirection)
{
	MathsStorage::Float4 light;
	Maths::Store(Maths::HVec4(lightDirection), &light);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_LIGHT_DIRECTION, (F32*)&light, 1);
}

void C_ImmediateDebugRender::SetBlendMode(const DbUtil::E_BlendMode blendMode)
{
	m_BlendMode = (Display::E_BlendMode)blendMode;
}

void C_ImmediateDebugRender::Begin3D()
{
	Display::Device()->SetBlendMode(m_BlendMode);

	if(m_DepthEnable)
		Display::Device()->SetDepthEnable(Display::E_Bool_True);
	else
		Display::Device()->SetDepthEnable(Display::E_Bool_False);

	Display::Device()->SetVertexShader(&m_VertexShader[E_PrimitiveRenderVertexShader_3D]);
	Display::Device()->SetPixelShader(&m_PixelShader[E_PrimitiveRenderPixelShader_Default]);

	Display::Device()->SetViewShaderConstants3D();

	Display::C_Matrix worldMatrix;
	Display::MatrixSetIdentity(&worldMatrix);
	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);	
}

void C_ImmediateDebugRender::Begin3DLit()
{
	Display::Device()->SetBlendMode(m_BlendMode);

	if(m_DepthEnable)
		Display::Device()->SetDepthEnable(Display::E_Bool_True);
	else
		Display::Device()->SetDepthEnable(Display::E_Bool_False);

	Display::Device()->SetVertexShader(&m_VertexShader[E_PrimitiveRenderVertexShader_3DLit]);
	Display::Device()->SetPixelShader(&m_PixelShader[E_PrimitiveRenderPixelShader_Default]);

	Display::Device()->SetViewShaderConstants3D();

	Display::C_Matrix worldMatrix;
	Display::MatrixSetIdentity(&worldMatrix);
	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);
}

void C_ImmediateDebugRender::Begin2D()
{
	Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);

	if(m_DepthEnable)
		Display::Device()->SetDepthEnable(Display::E_Bool_True);
	else
		Display::Device()->SetDepthEnable(Display::E_Bool_False);

	Display::Device()->SetVertexShader(&m_VertexShader[E_PrimitiveRenderVertexShader_2D]);
	Display::Device()->SetPixelShader(&m_PixelShader[E_PrimitiveRenderPixelShader_Default]);

	Display::Device()->SetViewShaderConstants2D();

	Display::C_Matrix worldMatrix;
	Display::MatrixSetIdentity(&worldMatrix);
	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);
}

void* C_ImmediateDebugRender::BeginVertices( const Display::E_PrimitiveType primitiveType, const U32 vertexCount, const bool is3D, const bool lit)
{
	if (is3D)
	{
		if (lit)
		{
			return Display::Device()->BeginVertices(primitiveType, vertexCount, sizeof(Vertex3DLit), m_pVertexDeclaration3DLit, true); // This can fail	
		}
		else
		{
			return Display::Device()->BeginVertices(primitiveType, vertexCount, sizeof(Vertex3D), m_pVertexDeclaration3D, true); // This can fail	
		}
	}
	else
	{
		return Display::Device()->BeginVertices(primitiveType, vertexCount, sizeof(Vertex2D), m_pVertexDeclaration2D, true); // This can fail	
	}
}

void C_ImmediateDebugRender::EndVertices( void* pLockedMemory )
{
	Display::Device()->EndVertices(pLockedMemory);
}


C_ImmediateDebugRender* Render::Debug::C_ImmediateDebugRender::GetInstance()
{
	return pImmediateDebugRender;
}

void C_ImmediateDebugRender::DrawTextureTriangles2D( const Maths::HVec2* AArray, const Maths::HVec2* BArray, const Maths::HVec2* CArray, const Maths::HVec2* ATArray, const Maths::HVec2* BTArray, const Maths::HVec2* CTArray, const Maths::RGBA* colourArray, const U32 count, Display::C_Texture2D& texture2D )
{
	//Set States		
	Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);

	if(m_DepthEnable)
		Display::Device()->SetDepthEnable(Display::E_Bool_True);
	else
		Display::Device()->SetDepthEnable(Display::E_Bool_False);

	Display::Device()->SetVertexShader(&m_VertexShader2DTex);
	Display::Device()->SetPixelShader(&m_PixelShaderTex);

	Display::C_Matrix worldMatrix;
	Display::MatrixSetIdentity(&worldMatrix);
	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);

	//Set Texture
	Display::Device()->SetTexture(0, &texture2D);
	Display::Device()->SetTextureFilterMagMinMip(0, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);

	//Fill Vertices
	C_TexturedVertex2D* pTextureVertex =  (C_TexturedVertex2D*) Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleList, count * 3, sizeof(C_TexturedVertex2D), m_pVertexDeclaration2DTex);
	if (pTextureVertex)
	{
		for (U32 i = 0; i < count; i++)
		{
			pTextureVertex->Pos(AArray[i]);
			pTextureVertex->Col(colourArray[i]);
			pTextureVertex->Tex(ATArray[i]);
			pTextureVertex++;

			pTextureVertex->Pos(BArray[i]);
			pTextureVertex->Col(colourArray[i]);
			pTextureVertex->Tex(BTArray[i]);
			pTextureVertex++;

			pTextureVertex->Pos(CArray[i]);
			pTextureVertex->Col(colourArray[i]);
			pTextureVertex->Tex(CTArray[i]);
			pTextureVertex++;
		}

		Display::Device()->EndVertices(pTextureVertex);
	}

	Display::Device()->SetVertexShader(NULL);
	Display::Device()->SetPixelShader(NULL);
	Display::Device()->SetTexture(0, NULL);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
}

void C_ImmediateDebugRender::Draw2DTextureBar(const Maths::HVec2& pos, const Maths::HVec2& size, const Maths::RGBA colour, Display::C_Texture2D& texture2D)
{
	//Set States		
	Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);

	if(m_DepthEnable)
		Display::Device()->SetDepthEnable(Display::E_Bool_True);
	else
		Display::Device()->SetDepthEnable(Display::E_Bool_False);

	Display::Device()->SetVertexShader(&m_VertexShader2DTex);
	Display::Device()->SetPixelShader(&m_PixelShaderTex);

	Display::C_Matrix worldMatrix;
	Display::MatrixSetIdentity(&worldMatrix);
	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);

	//Set Texture
	Display::Device()->SetTexture(0, &texture2D);
	Display::Device()->SetTextureFilterMagMinMip(0, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);

	//Fill Vertices
	C_TexturedVertex2D* pTextureVertex =  (C_TexturedVertex2D*) Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleStrip, 4, sizeof(C_TexturedVertex2D), m_pVertexDeclaration2DTex);
	if (pTextureVertex)
	{
		
		pTextureVertex->Pos(Maths::HVec2(pos.X(), pos.Y()));
		pTextureVertex->Col(colour);
		pTextureVertex->Tex(Maths::HVec2(0.f,0.f));
		pTextureVertex++;

		pTextureVertex->Pos(Maths::HVec2(pos.X() + size.X(), pos.Y()));
		pTextureVertex->Col(colour);
		pTextureVertex->Tex(Maths::HVec2(1.f,0.f));
		pTextureVertex++;

		pTextureVertex->Pos(Maths::HVec2(pos.X(), pos.Y() + size.Y()));
		pTextureVertex->Col(colour);
		pTextureVertex->Tex(Maths::HVec2(0.f,1.f));
		pTextureVertex++;

		pTextureVertex->Pos(Maths::HVec2(pos.X() + size.X(), pos.Y() + size.Y()));
		pTextureVertex->Col(colour);
		pTextureVertex->Tex(Maths::HVec2(1.f,1.f));
		pTextureVertex++;

		Display::Device()->EndVertices(pTextureVertex);
	}

	Display::Device()->SetVertexShader(NULL);
	Display::Device()->SetPixelShader(NULL);
	Display::Device()->SetTexture(0, NULL);
}
