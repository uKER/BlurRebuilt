#include "StdAfx.h"
#include "Samples/Section/SectionFragmentPatch.h"
#include <Render/Render/GridRender.h>

#ifdef PLATFORM_PS3
#include <Display/Buffer/GCM/GCMUserFifo.h>
#endif

#include "Render/Shaders/vsConstants.h"
#include "Render/Shaders/psConstants.h"

#if defined(DEVICE_PS3GCM)
#include "Shaders/PS3/vsFragmentPatch.shader"
#include "Shaders/PS3/psFragmentPatch.shader"
#endif



namespace Samples
{



Debug::Colour g_debugFragmentPatchColour("Samples.FragmentPatch.Colour", 1.0f, 0.0f, 0.0f, 1.0f, Debug::Gamma(2.2f));



C_SectionFragmentPatch::C_SectionFragmentPatch(const Util::C_String& name) : C_Section(name)
{
	Display::C_VertexFormat vertexFormat;
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
	m_pVertexDeclaration = Display::Device()->CreateVertexDeclaration(vertexFormat);
	m_VertexStride = vertexFormat.GetStride(0);
}

C_SectionFragmentPatch::~C_SectionFragmentPatch()
{
}

bool C_SectionFragmentPatch::OnCreate()
{
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

	C_Vertex* pVertex = (C_Vertex *) m_VertexBuffer.Lock();

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

	// Create shader double buffer
	m_DoubleBuffer.Create(1 * 1024 * 1024, 128);

	// Return success	

	return true;
}

void C_SectionFragmentPatch::OnDestroy()
{
	m_DoubleBuffer.Destroy();

	Display::Device()->Destroy(&m_IndexBuffer);
	Display::Device()->Destroy(&m_VertexBuffer);

	Display::Device()->Destroy(&m_VertexShader);
	Display::Device()->Destroy(&m_PixelShader);
}

void C_SectionFragmentPatch::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionFragmentPatch");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	// draw the grid

	Render::DrawGrid();

	// begin using shader buffer
	m_Patcher.BeginUsing();
	m_DoubleBuffer.BeginUsing();

	// draw the cube

	Maths::HMat44 hWorldMatrix;
	MathsStorage::Matrix44 worldMatrix;
	Maths::Store(hWorldMatrix, &worldMatrix);
	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32 *) &worldMatrix, 4);
	Display::Device()->SetVertexShader(&m_VertexShader);

#ifdef PLATFORM_PS3
	U8 * gcmAddress = m_DoubleBuffer.Allocate(&m_PixelShader);
	if (gcmAddress)
	{
		m_Patcher.SetParameter(m_psColour, (F32 *) &g_debugFragmentPatchColour.Value());
		m_Patcher.Patch(&m_PixelShader, gcmAddress);
		
		cellGcmSetInvalidateTextureCache(CELL_GCM_INVALIDATE_TEXTURE);
		Display::Device()->SetPixelShader(&m_PixelShader, gcmAddress);
	}
	else
#endif // PLATFORM_PS3
	{
		Display::Device()->SetPixelShaderParameterF(m_psColour, (F32 *) &g_debugFragmentPatchColour.Value());
		Display::Device()->SetPixelShader(&m_PixelShader);
	}

	Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, m_VertexStride);
	Display::Device()->SetIndexBuffer(&m_IndexBuffer);
	Display::Device()->SetVertexDeclaration(m_pVertexDeclaration);
	Display::Device()->DrawIndexedPrimitive(Display::E_PrimitiveType_TriangleList, 0, 0, 36);

	// end using shader buffer
	m_DoubleBuffer.EndUsing();
	m_Patcher.EndUsing();

	// draw FPS

	Util::C_String str;
	str.Printf("Fragment Patch Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
