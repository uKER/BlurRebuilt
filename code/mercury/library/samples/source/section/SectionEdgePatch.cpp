#include "StdAfx.h"
#include "Samples/Section/SectionEdgePatch.h"
#include <Render/Render/GridRender.h>
#include <Render/Edge/Edge.h>

#include "Render/Shaders/vsConstants.h"
#include "Render/Shaders/psConstants.h"

#if defined(DEVICE_PS3GCM)
#include "Shaders/PS3/vsFragmentPatch.shader"
#include "Shaders/PS3/psFragmentPatch.shader"
#endif



namespace Samples
{



Debug::Colour g_debugEdgePatchColour("Samples.EdgePatch.Colour", 1.0f, 0.0f, 0.0f, 1.0f, Debug::Gamma(2.2f));



C_SectionEdgePatch::C_SectionEdgePatch(const Util::C_String& name) : C_Section(name)
{
	Display::C_VertexFormat vertexFormat;
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
	m_pVertexDeclaration = Display::Device()->CreateVertexDeclaration(vertexFormat);
	m_VertexStride = vertexFormat.GetStride(0);
}

C_SectionEdgePatch::~C_SectionEdgePatch()
{
}

bool C_SectionEdgePatch::OnCreate()
{
	if (!Render::Edge::Fragment::Create())
		return false;

	// Create shaders
	if ( !Display::CreateVertexShaderFromFunc("Shaders/vsFragmentPatch", 0, &m_VertexShader, sizeof(vsFragmentPatch), vsFragmentPatch) )
		return false;
	if ( !Display::CreatePixelShaderFromFunc("Shaders/psFragmentPatch", 0, &m_PixelShader, sizeof(psFragmentPatch), psFragmentPatch) )
		return false;

	m_psColour = m_PixelShader.GetParameter(CP_RENDER_SETTINGS, "g_vColour");

	// Build vertex and index buffers

	struct C_Vertex
	{
		F32 x, y, z;
	};

	const U32 vertexCount = 8;
	if (!Display::Device()->CreateVertexBuffer(sizeof(C_Vertex) * vertexCount, 0, &m_VertexBuffer))
		return false;

	C_Vertex* pVertex = (C_Vertex*)m_VertexBuffer.Lock();

	pVertex->x = -1.0f;	pVertex->y = -1.0f;	pVertex->z = -1.0f; pVertex++;
    pVertex->x = -1.0f;	pVertex->y = -1.0f;	pVertex->z =  1.0f; pVertex++;
    pVertex->x = -1.0f;	pVertex->y =  1.0f;	pVertex->z = -1.0f; pVertex++;
    pVertex->x = -1.0f;	pVertex->y =  1.0f;	pVertex->z =  1.0f; pVertex++;
	pVertex->x =  1.0f;	pVertex->y = -1.0f;	pVertex->z = -1.0f; pVertex++;
    pVertex->x =  1.0f;	pVertex->y = -1.0f;	pVertex->z =  1.0f; pVertex++;
    pVertex->x =  1.0f;	pVertex->y =  1.0f;	pVertex->z = -1.0f; pVertex++;
    pVertex->x =  1.0f;	pVertex->y =  1.0f;	pVertex->z =  1.0f; pVertex++;

	m_VertexBuffer.Unlock();

	const U32 indexCount = 6 * 2 * 3;
	if (!Display::Device()->CreateIndexBuffer(sizeof(U16) * indexCount, 0, Display::E_Format_Index16, &m_IndexBuffer))
		return false;

	U16* pIndex = (U16*)m_IndexBuffer.Lock();
	
	// top
	*(pIndex++) = 2; *(pIndex++) = 7; *(pIndex++) = 3; 
	*(pIndex++) = 2; *(pIndex++) = 6; *(pIndex++) = 7; 
	// bottom
	*(pIndex++) = 0; *(pIndex++) = 1; *(pIndex++) = 5; 
	*(pIndex++) = 0; *(pIndex++) = 5; *(pIndex++) = 4; 					 
	// left
	*(pIndex++) = 1; *(pIndex++) = 2; *(pIndex++) = 3; 
	*(pIndex++) = 1; *(pIndex++) = 0; *(pIndex++) = 2; 						 
	// right
	*(pIndex++) = 4; *(pIndex++) = 5; *(pIndex++) = 6; 
	*(pIndex++) = 5; *(pIndex++) = 7; *(pIndex++) = 6; 						 
	// front
	*(pIndex++) = 2; *(pIndex++) = 0; *(pIndex++) = 6; 
	*(pIndex++) = 0; *(pIndex++) = 4; *(pIndex++) = 6; 							 
	// back
	*(pIndex++) = 1; *(pIndex++) = 3; *(pIndex++) = 7; 
	*(pIndex++) = 1; *(pIndex++) = 7; *(pIndex++) = 5;

	m_IndexBuffer.Unlock();

	// Return success	

	return true;
}

void C_SectionEdgePatch::OnDestroy()
{
	Display::Device()->Destroy(&m_IndexBuffer);
	Display::Device()->Destroy(&m_VertexBuffer);

	Display::Device()->Destroy(&m_VertexShader);
	Display::Device()->Destroy(&m_PixelShader);

	if (Render::Edge::Fragment::Exists())
		Render::Edge::Fragment::Destroy();
}

void C_SectionEdgePatch::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionFragmentPatch");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	// draw the grid

	Render::DrawGrid();

	// draw the cube

	Maths::HMat44 hWorldMatrix;

	MathsStorage::Matrix44 worldMatrix;
	Maths::Store(hWorldMatrix, &worldMatrix);

	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32 *) &worldMatrix, 4);

	Display::Device()->SetVertexShader(&m_VertexShader);

	Render::Edge::Fragment::Patcher()->Begin(&m_PixelShader);
	Render::Edge::Fragment::Patcher()->SetParameter(m_psColour, (F32 *) &g_debugEdgePatchColour.Value());
	Render::Edge::Fragment::Patcher()->SetVertexDeclaration(m_pVertexDeclaration);
	Render::Edge::Fragment::Patcher()->SetVertexBuffer(&m_VertexBuffer, m_VertexStride);
	Render::Edge::Fragment::Patcher()->SetIndexBuffer(&m_IndexBuffer);
	Render::Edge::Fragment::Patcher()->DrawIndexedPrimitive(Display::E_PrimitiveType_TriangleList, 36);
	Render::Edge::Fragment::Patcher()->End();

	// draw FPS

	Util::C_String str;
	str.Printf("Fragment Patch Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
