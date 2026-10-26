#include "stdafx.h"
#include "Effect/Sky/SkyDomeBase.h"
#include "Effect/Shaders/Constants.h"
#include "Display/Buffer/VertexDeclarations.h"
#include <Display/Context/Context.h>

#include <SceneGraph/View.h>
#include <SceneGraph/Visitor.h>
#include <SceneGraph/Context.h>

namespace Effect
{
namespace Sky
{

Debug::Int		g_debugRingCount	("Graphics.Render.Sky.RingCount",		25,		Debug::Range(1, 50));
Debug::Int		g_debugSegmentCount	("Graphics.Render.Sky.SegmentCount",	25,		Debug::Range(1, 50));
Debug::Bool		g_debugEnabled		("Graphics.Render.Sky.Enabled",			true);
Debug::Bool		g_debugWireframe	("Graphics.Render.Sky.Wireframe",		false);
Debug::Float    g_debugHeight		("Graphics.Render.Sky.Height",			1.0f,	Debug::Range(-1.0f, 1.0f));
Debug::Float    g_debugOffset		("Graphics.Render.Sky.Offset",			0.0f,	Debug::Range(-1.0f, 1.0f));
Debug::Float	g_debugDepth		("Graphics.Render.Sky.Depth",			0.9f,	Debug::Range(0.0f, 1.0f));

C_SkyDomeBase::C_SkyDomeBase() :
	Display::I_Renderer("SkyDome"),
	m_VertexCount(0),
	m_IndexCount(0),
	m_RenderPass(0)
{
	SetVertexShader();
	SetPixelShader();
}

C_SkyDomeBase::~C_SkyDomeBase()
{
	DestroyRenderer();
}

bool C_SkyDomeBase::OnCreate()
{
	if (!BuildSphere())
		return false;

	return true;
}

bool C_SkyDomeBase::OnCreateVolatile()
{
	return true;
}

void C_SkyDomeBase::OnDestroy()
{
	Display::Device()->Destroy(&m_VertexShader);

	Display::Device()->Destroy(&m_PixelShader);
	Display::Device()->Destroy(&m_VertexBuffer);
	Display::Device()->Destroy(&m_IndexBuffer);
}

void C_SkyDomeBase::OnDestroyVolatile()
{
}

void C_SkyDomeBase::OnRender(SceneGraph::C_Context& context) const
{
	Display::C_Context* pDisplay = Display::GetContext(&context);
	context.SetContextFlags(pDisplay->GetFlags());

	Render(pDisplay);
}

void C_SkyDomeBase::OnGather(const SceneGraph::C_View& view) const
{
	view.AddVisibleNode(this, m_RenderPass);
}

void C_SkyDomeBase::Accept(SceneGraph::I_Visitor& visitor) const
{
	visitor.Visit(*this);
}

void C_SkyDomeBase::SetRenderPass(const U32 renderPass)
{
	m_RenderPass = renderPass;
}

void C_SkyDomeBase::Render() const
{
	Display::C_DeviceContext *pContext = new Display::C_DeviceContext;
	Display::C_Context *pDContext = Display::GetContext(pContext);
	pDContext->BeginCommandBuffer(NULL);
	Render(Display::GetContext(pDContext));
	pDContext->EndCommandBuffer();
	delete pContext;
}

void C_SkyDomeBase::Render(Display::C_Context* pContext) const
{
	#ifndef CFG_SPEED
	if (!g_debugEnabled)
		return;
	#endif

	PERF_EVENT("Effect.RenderSkyDome");

	Bind(pContext);

	pContext->SetVertexShader(m_pVertexShader);
	pContext->SetPixelShader(m_pPixelShader);

	pContext->SetVertexDeclaration(Display::Vertex_XYZ_UV::m_pDeclaration);
	pContext->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XYZ_UV));
	pContext->SetIndexBuffer(&m_IndexBuffer);
	pContext->DrawIndexedPrimitive(Display2::E_PrimitiveType_TriangleStrip, 0, 0, m_VertexCount, 0, m_IndexCount);
	
	Unbind(pContext);
}

void C_SkyDomeBase::Bind(Display::C_Context* pContext) const
{
	pContext->SetBlendEnable(Display::E_Bool_False);
	pContext->SetBlendState(Display::E_Blend_One, Display::E_Blend_Zero, Display::E_BlendOp_Add);
	pContext->SetAlphaTestState(Display::E_Bool_False, 0);
	pContext->SetAlphaToMaskEnable(Display::E_Bool_False);
	pContext->SetCullMode(Display::E_CullMode_Default);
	pContext->SetDepthWriteEnable(Display::E_Bool_False);
	pContext->SetDepthEnable(Display::E_Bool_True);

	#ifndef CFG_SPEED
	if (g_debugWireframe)
		pContext->SetFillMode(Display::E_FillMode_Wireframe);
	#endif

	// TODO: need a thread-safe way to get the far-clip distance in case it changes per-view
	const Display::C_ViewSettings3D& viewSettings3D = pContext->GetView()->GetViewSettings3D();
	const Display::C_Float4 vsSkySettings(viewSettings3D.GetFarClip() * g_debugDepth, g_debugHeight, g_debugOffset, 0.f);
	pContext->SetVertexShaderConstantF(CV_RENDER_SETTINGS, (const F32*)&vsSkySettings, 1);
}

void C_SkyDomeBase::Unbind(Display::C_Context* pContext) const
{
	#ifndef CFG_SPEED
	if (g_debugWireframe)
		pContext->SetFillMode(Display::E_FillMode_Solid);
	#endif

	pContext->SetDepthWriteEnable(Display::E_Bool_True);
}

bool C_SkyDomeBase::BuildSphere()
{
	m_VertexCount	= (g_debugRingCount + 1) * (g_debugSegmentCount + 1);
	m_IndexCount	= (g_debugRingCount * (g_debugSegmentCount + 1)) * 2;
	
	if (!Display::Device()->CreateVertexBuffer(m_VertexCount * sizeof(Display::Vertex_XYZ_UV), 0, &m_VertexBuffer))
		return false;
	if (!Display::Device()->CreateIndexBuffer(m_IndexCount * sizeof(U16), 0, Display::E_Format_Index16, &m_IndexBuffer))
		return false;

	Display::Vertex_XYZ_UV* pVertex = (Display::Vertex_XYZ_UV*) m_VertexBuffer.Lock();
	if (!pVertex)
		return false;

	U16* pIndex = (U16*) m_IndexBuffer.Lock();
	if (!pIndex)
		return false;

	const F32 deltaRingAngle	= Maths::Pi / (F32) g_debugRingCount;
	const F32 deltaSegmentAngle	= 2.0f * Maths::Pi / (F32) g_debugSegmentCount;
	U16 baseIndex = 0;

	for (S32 ringNo = 0; ringNo <= g_debugRingCount; ++ringNo)
	{
		const F32 r = sinf(ringNo * deltaRingAngle);
		const F32 y = cosf(ringNo * deltaRingAngle);
		for (S32 segmentNo = 0; segmentNo <= g_debugSegmentCount; ++segmentNo)
		{
			F32 fSegmentAngle = segmentNo * deltaSegmentAngle;
			if(segmentNo == g_debugSegmentCount)
				fSegmentAngle += 0.001f;
			pVertex->x = r * sinf(fSegmentAngle);
			pVertex->y = y;
			pVertex->z = r * cosf(fSegmentAngle);
			pVertex->u = fSegmentAngle / (deltaSegmentAngle  * (F32) g_debugSegmentCount);
			pVertex->v = (F32) ringNo * 2.0f / (F32) g_debugRingCount;
			++pVertex;

			if (ringNo != g_debugRingCount)
			{
				*pIndex++ = baseIndex + (U16)(g_debugSegmentCount + 1);
				*pIndex++ = baseIndex;
				++baseIndex;
			}
		}
	}

	m_VertexBuffer.Unlock();
	m_IndexBuffer.Unlock();
	
	return true;
}

bool C_SkyDomeBase::BuildHemisphere()
{
	m_VertexCount	= (g_debugRingCount + 1) * (g_debugSegmentCount + 1);
	m_IndexCount	= (g_debugRingCount * (g_debugSegmentCount + 1)) * 2;
	
	if (!Display::Device()->CreateVertexBuffer(m_VertexCount * sizeof(Display::Vertex_XYZ_UV), 0, &m_VertexBuffer))
		return false;
	if (!Display::Device()->CreateIndexBuffer(m_IndexCount * sizeof(U16), 0, Display::E_Format_Index16, &m_IndexBuffer))
		return false;

	Display::Vertex_XYZ_UV* pVertex = (Display::Vertex_XYZ_UV*) m_VertexBuffer.Lock();
	if (!pVertex)
		return false;

	U16* pIndex = (U16*) m_IndexBuffer.Lock();
	if (!pIndex)
		return false;

	const F32 deltaRingAngle	= 0.5f * Maths::Pi / (F32) g_debugRingCount;
	const F32 deltaSegmentAngle	= 2.0f * Maths::Pi / (F32) g_debugSegmentCount;
	U16 baseIndex = 0;

	for (S32 ringNo = 0; ringNo <= g_debugRingCount; ++ringNo)
	{
		const F32 r = sinf(ringNo * deltaRingAngle);
		const F32 y = cosf(ringNo * deltaRingAngle);
		for (S32 segmentNo = 0; segmentNo <= g_debugSegmentCount; ++segmentNo)
		{
			pVertex->x = r * sinf(segmentNo * deltaSegmentAngle);
			pVertex->y = y;
			pVertex->z = r * cosf(segmentNo * deltaSegmentAngle);
			pVertex->u = (F32) segmentNo / (F32) g_debugSegmentCount;
			pVertex->v = (F32) ringNo * 2.0f / (F32) g_debugRingCount;
			++pVertex;

			if (ringNo != g_debugRingCount)
			{
				*pIndex++ = baseIndex + (U16)(g_debugSegmentCount + 1);
				*pIndex++ = baseIndex;
				++baseIndex;
			}
		}
	}

	m_VertexBuffer.Unlock();
	m_IndexBuffer.Unlock();

	return true;
}

}; // namespace Sky
}; // namespace Effect
