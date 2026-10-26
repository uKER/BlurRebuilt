#include "StdAfx.h"
#include "Samples/Section/SectionGeomCulling.h"
#include <Display/Buffer/GCM/GCMUserFifo.h>
#include <Render/Render/GridRender.h>
#include <Render/Mesh/MeshLoader.h>
#include <Render/Edge/Edge.h>
#include <Render/Edge/OutputBuffer.h>

Debug::Bool g_debugUseGeomCullingSPU("Display.SPU.UseGeomCulling", true);



namespace Samples
{

C_SectionGeomCulling::C_SectionGeomCulling(const Util::C_String& name) : C_Section(name)
{
	Display::C_VertexFormat vertexFormat;
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
	vertexFormat.AddElement(0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
	m_pVertexDeclaration = Display::Device()->CreateVertexDeclaration(vertexFormat);
	m_VertexStride = vertexFormat.GetStride(0);
}

C_SectionGeomCulling::~C_SectionGeomCulling()
{
}

bool C_SectionGeomCulling::OnCreate()
{
	Render::Edge::Culler::Create();

	// Create vertex and index buffers

	struct C_Vertex
	{
		F32 x, y, z;
		U32 colour;
	};

	const U32 vertexCount = 8;
	if (!Display::Device()->CreateVertexBuffer(sizeof(C_Vertex) * vertexCount, 0, &m_VertexBuffer))
		return false;

	C_Vertex* pVertex = (C_Vertex*)m_VertexBuffer.Lock();

	pVertex->x = -1.0f;	pVertex->y = -1.0f;	pVertex->z = -1.0f;	pVertex->colour = 0xFFFFFFFF; pVertex++;
    pVertex->x = -1.0f;	pVertex->y = -1.0f;	pVertex->z =  1.0f;	pVertex->colour = 0xFFFFFFFF; pVertex++;
    pVertex->x = -1.0f;	pVertex->y =  1.0f;	pVertex->z = -1.0f;	pVertex->colour = 0xFFFFFFFF; pVertex++;
    pVertex->x = -1.0f;	pVertex->y =  1.0f;	pVertex->z =  1.0f;	pVertex->colour = 0xFFFFFFFF; pVertex++;
	pVertex->x =  1.0f;	pVertex->y = -1.0f;	pVertex->z = -1.0f;	pVertex->colour = 0xFFFFFFFF; pVertex++;
    pVertex->x =  1.0f;	pVertex->y = -1.0f;	pVertex->z =  1.0f;	pVertex->colour = 0xFFFFFFFF; pVertex++;
    pVertex->x =  1.0f;	pVertex->y =  1.0f;	pVertex->z = -1.0f;	pVertex->colour = 0xFFFFFFFF; pVertex++;
    pVertex->x =  1.0f;	pVertex->y =  1.0f;	pVertex->z =  1.0f;	pVertex->colour = 0xFFFFFFFF; pVertex++;

	m_VertexBuffer.Unlock();

	m_VertexBuffer.CreateCommandBuffer(m_VertexStride, m_pVertexDeclaration);

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
	
	// Create mesh

	if(!Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		return false;
	m_Mesh.AddInstance();

	// Return success

	return true;
}

void C_SectionGeomCulling::OnDestroy()
{
	Render::Edge::Culler::Destroy();

	// destroy buffers
	Display::Device()->Destroy(&m_VertexBuffer);
	Display::Device()->Destroy(&m_IndexBuffer);

	// destroy mesh
	m_Mesh.Destroy();
}

void C_SectionGeomCulling::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionGeomCulling");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	Render::Edge::Culler::Culler()->Reset();
	Render::Edge::Culler::Culler()->StartUsing();

	m_Mesh.Render();
	Render::DrawGrid();

	// bind shader
	Maths::HMat44 hmatWorld;
	hmatWorld.AW( Maths::HVec4(0.0f, 4.0f, 0.0f, 1.0f) );

	MathsStorage::Matrix44 matWorld;
	Maths::Store(hmatWorld, &matWorld);
	
	Display::BasicRender()->BindColoured3D(&matWorld);

	if(g_debugUseGeomCullingSPU)
	{
		// set the command buffer up
		m_VertexBuffer.SetCommandBuffer();
		Display::Device()->m_SetVertexDeclaration = false;

		Render::Edge::Culler::Culler()->SetWorldMatrix(hmatWorld);
		Render::Edge::Culler::Culler()->SetVertexBuffer(&m_VertexBuffer, m_VertexStride);
		Render::Edge::Culler::Culler()->SetIndexBuffer(&m_IndexBuffer);
		Render::Edge::Culler::Culler()->SetVertexDeclaration(m_pVertexDeclaration);
		Render::Edge::Culler::Culler()->DrawIndexedPrimitive(Display::E_PrimitiveType_TriangleList, 8, 36);
	}
	else
	{
		Display::Device()->SetIndexBuffer(&m_IndexBuffer);

		m_VertexBuffer.SetCommandBuffer();
		Display::Device()->m_SetVertexDeclaration = false;

		Display::Device()->DrawIndexedPrimitive(Display::E_PrimitiveType_TriangleList, 0, 0, 12 * 3);
	}

	Render::Edge::Culler::Culler()->StopUsing();

	Util::C_String str;
	str.Printf("Geom Culling Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
