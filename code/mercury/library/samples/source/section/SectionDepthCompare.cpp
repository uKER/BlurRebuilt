#include "StdAfx.h"
#include "Samples/Section/SectionDepthCompare.h"
#include "Render/Utils/Globals.h"

#if defined(DEVICE_PS3GCM)
#include "Shaders/PS3/vsOutputDepth.shader"
#include "Shaders/PS3/psOutputDepth.shader"
#endif

namespace Samples
{



Debug::Int g_debugDepthCompareMode("Section.DepthCompare.Mode", 0, Debug::Range(0, 1));



C_SectionDepthCompare::C_SectionDepthCompare(const Util::C_String& name) : C_Section(name)
{
}

C_SectionDepthCompare::~C_SectionDepthCompare()
{
}

bool C_SectionDepthCompare::OnCreate()
{
	Display::C_VertexFormat vertexFormat;
	vertexFormat.AddElement( 0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0 );
	m_VertexStride = vertexFormat.GetStride(0);
	m_pVertexDeclaration = Display::Device()->CreateVertexDeclaration(vertexFormat);

	if ( !Display::CreateVertexShaderFromFunc("Shaders/vsOutputDepth", 0, &m_VertexShader, sizeof(vsOutputDepth), vsOutputDepth) )
		return false;
	if ( !Display::CreatePixelShaderFromFunc("Shaders/psOutputDepth", 0, &m_PixelShader, sizeof(psOutputDepth), psOutputDepth) )
		return false;

	return true;
}

bool C_SectionDepthCompare::OnCreateVolatile()
{
	if (!Render::CreateGlobalGraphicsBuffers())
		return false;
	
	return true;
}

void C_SectionDepthCompare::OnDestroyVolatile()
{
	Render::DestroyGlobalGraphicsBuffers();
}

void C_SectionDepthCompare::OnDestroy()
{
	Display::Device()->Destroy(&m_VertexShader);
	Display::Device()->Destroy(&m_PixelShader);
}

void C_SectionDepthCompare::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionDepthCompare");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Display::WHITE);

	//m_SkyDome.Render();
	
	Display::Device()->SetVertexShader(&m_VertexShader);
	Display::Device()->SetPixelShader(&m_PixelShader);

	struct C_Vertex
	{
		F32 x, y, z;
	};

	Maths::HVec3 pos(0.0f, 0.0f, 0.0f);
	Maths::HVec3 dim(12.0f, 8.0f, 32.0f);
	C_Vertex vertex[8] = 
	{
		{pos.X() - dim.X(), pos.Y() - dim.Y(), pos.Z() - dim.Z()},
		{pos.X() - dim.X(), pos.Y() - dim.Y(), pos.Z() + dim.Z()},
		{pos.X() - dim.X(), pos.Y() + dim.Y(), pos.Z() - dim.Z()},
		{pos.X() - dim.X(), pos.Y() + dim.Y(), pos.Z() + dim.Z()},
		{pos.X() + dim.X(), pos.Y() - dim.Y(), pos.Z() - dim.Z()},
		{pos.X() + dim.X(), pos.Y() - dim.Y(), pos.Z() + dim.Z()},
		{pos.X() + dim.X(), pos.Y() + dim.Y(), pos.Z() - dim.Z()},
		{pos.X() + dim.X(), pos.Y() + dim.Y(), pos.Z() + dim.Z()},
	};
	
	const U32 vertexCount = 36;
	C_Vertex *pVertex = (C_Vertex *) Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleList, vertexCount, m_VertexStride, m_pVertexDeclaration);

	// top
	*(pVertex++) = vertex[2]; *(pVertex++) = vertex[3]; *(pVertex++) = vertex[7];
	*(pVertex++) = vertex[2]; *(pVertex++) = vertex[7]; *(pVertex++) = vertex[6];
	// bottom
	*(pVertex++) = vertex[0]; *(pVertex++) = vertex[5]; *(pVertex++) = vertex[1];
	*(pVertex++) = vertex[0]; *(pVertex++) = vertex[4]; *(pVertex++) = vertex[5];
	// left
	*(pVertex++) = vertex[1]; *(pVertex++) = vertex[3]; *(pVertex++) = vertex[2];
	*(pVertex++) = vertex[1]; *(pVertex++) = vertex[2]; *(pVertex++) = vertex[0];
	// right
	*(pVertex++) = vertex[4]; *(pVertex++) = vertex[6]; *(pVertex++) = vertex[5];
	*(pVertex++) = vertex[5]; *(pVertex++) = vertex[6]; *(pVertex++) = vertex[7];
	// front
	*(pVertex++) = vertex[2]; *(pVertex++) = vertex[6]; *(pVertex++) = vertex[0];
	*(pVertex++) = vertex[0]; *(pVertex++) = vertex[6]; *(pVertex++) = vertex[4];
	// back
	*(pVertex++) = vertex[1]; *(pVertex++) = vertex[7]; *(pVertex++) = vertex[3];
	*(pVertex++) = vertex[1]; *(pVertex++) = vertex[5]; *(pVertex++) = vertex[7];

	Display::Device()->EndVertices();

	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
	#ifdef GRAPHICS_GCM
	Display::Device()->ResolveDepthStencilSurface(backBuffer.GetDepthStencilSurface(), &Render::g_DepthStencilBuffer);
	#endif

	if (g_debugDepthCompareMode == 1)
	{
		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		Display::BasicRender()->DrawTextured2DRect(0.0f, 0.0f, 1024.0f, 768.0f, Display::WHITE, &Render::g_DepthStencilBuffer.m_Texture);
	}

	Util::C_String str;
	str.Printf("Render Section Depth Compare: %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
