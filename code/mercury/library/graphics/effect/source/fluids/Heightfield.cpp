#include "stdafx.h"
#include "Effect/Fluids/Heightfield.h"
#include "Display/Buffer/VertexDeclarations.h"
#include "Display/Render/Render.h"
#include "Render/Shaders/Constants.h"
#include "System/Scheduler.h"

EXTERN_SHADER(psHeightfieldModify);
EXTERN_SHADER(psHeightfieldSolve);
EXTERN_SHADER(psHeightfieldSolveTop);
EXTERN_SHADER(psHeightfieldSolveBottom);
EXTERN_SHADER(psHeightfieldSolveLeft);
EXTERN_SHADER(psHeightfieldSolveRight);

namespace Effect
{

Debug::Float	g_debugHeightfieldDamping	("Graphics.Effect.Heightfield.Damping",	0.99f,	Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugHeightfieldSpeed		("Graphics.Effect.Heightfield.Speed",	0.10f,	Debug::Range(0.0f, 1.0f));

C_HeightfieldSurface::C_HeightfieldSurface()
{
	m_fSurfaceSize = 1.0f;
	m_iTextureSize = 128;
}

C_HeightfieldSurface::~C_HeightfieldSurface()
{
}

bool C_HeightfieldSurface::CreateVolatile()
{
	for (U32 textureNo = 0; textureNo <= c_HeightfieldIterations; ++textureNo)
	{
		if (!Display::Device()->CreateRenderTexture2D(m_iTextureSize, m_iTextureSize, 1, Display::E_Usage_System, Display::E_Format_R32F, Display::E_Multisample_None, 0, &m_HeightTextures[textureNo]))
			return false;

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetRenderTexture2DName(&m_HeightTextures[textureNo], "_HeightfieldSurfaceRT");
		#endif
	}

	m_pPreviousTexture	= &m_HeightTextures[0];
	m_pCurrentTexture	= &m_HeightTextures[1];
	m_pNextTexture		= &m_HeightTextures[2];

	return true;
}

void C_HeightfieldSurface::DestroyVolatile()
{
	for (U32 textureNo = 0; textureNo <= c_HeightfieldIterations; ++textureNo)
	{
		Display::Device()->Destroy(&m_HeightTextures[textureNo]);
	}
}

C_HeightfieldCpuSolver::C_HeightfieldCpuSolver() : m_hSolveJob(Scheduler::JQ_GROUP_NONE), m_hModifyJob(Scheduler::JQ_GROUP_NONE)
{
}

C_HeightfieldCpuSolver::~C_HeightfieldCpuSolver()
{
}

bool C_HeightfieldCpuSolver::Create()
{
	BIZ_ASSERT(m_hSolveJob == Scheduler::JQ_GROUP_NONE);
	m_hSolveJob = Scheduler::CreateGroup();
	m_hModifyJob = Scheduler::CreateGroup();

	return true;
}

void C_HeightfieldCpuSolver::Destroy()
{
	Scheduler::DestroyGroup(m_hModifyJob);
	Scheduler::DestroyGroup(m_hSolveJob);

	m_hModifyJob = m_hSolveJob = Scheduler::JQ_GROUP_NONE;
}

void C_HeightfieldCpuSolver::ClearSurface(C_HeightfieldSurface* pSurface) const
{
	for (U32 i = 0; i <= c_HeightfieldIterations; ++i)
	{
		Display::C_RenderTexture2D* pRenderTexture = &pSurface->m_HeightTextures[i];
		Display::C_Surface* pColourSurface = &pRenderTexture->GetSurface();
		
		#if PLATFORM_PS3
		U8* const pAddress = pColourSurface->GetAddress();
		U32 const iSize = pColourSurface->GetSize();
		memset(pAddress, 0, iSize);
		#endif
	}
}

void C_HeightfieldCpuSolver::ModifySurface(C_HeightfieldSurface* pSurface, Display::C_Texture2D* pTexture, const Display::C_Float4& vTransformU, const Display::C_Float4& vTransformV, const F32 fHeight)
{
#if PLATFORM_PS3
	m_ModifyData.m_pNext			= pSurface->m_pNextTexture->GetTexture().GetAddress();
	m_ModifyData.m_pCurrent			= pSurface->m_pCurrentTexture->GetTexture().GetAddress();
	m_ModifyData.m_iSurfaceWidth	= pSurface->m_iTextureSize;
	m_ModifyData.m_iSurfaceHeight	= pSurface->m_iTextureSize;
	m_ModifyData.m_iSurfacePitch	= pSurface->m_iTextureSize * sizeof(U32);
	m_ModifyData.m_pModify			= pTexture->GetAddress();
	m_ModifyData.m_iModifyWidth		= pTexture->GetWidth();
	m_ModifyData.m_iModifyHeight	= pTexture->GetHeight();
	m_ModifyData.m_iModifyPitch		= pTexture->GetWidth() * sizeof(U32);
	m_ModifyData.m_fScale			= fHeight;
	m_ModifyData.m_vTransformU		= vTransformU;
	m_ModifyData.m_vTransformV		= vTransformV;
	
	Scheduler::AddTask(HeightfieldModifyModule, 0, &m_ModifyData, m_hModifyJob, m_hPolicy, 0, 0);
	while (Scheduler::jqGetQueuedBatchCount(m_hModifyJob) + Scheduler::jqGetExecutingBatchCount(m_hModifyJob) > 0);

	Display::C_RenderTexture2D* pRenderTexture = pSurface->m_pCurrentTexture;
	pSurface->m_pCurrentTexture = pSurface->m_pNextTexture;
	pSurface->m_pNextTexture = pRenderTexture;
#endif
}

void C_HeightfieldCpuSolver::SolveSurface(C_HeightfieldSurface* pSurface, const F32 fTimeStep)
{
	SolveSurface(pSurface, 0, 0, 0, 0, fTimeStep);
}

void C_HeightfieldCpuSolver::SolveSurface(C_HeightfieldSurface* pSurface, Display::C_Texture2D* pTop, Display::C_Texture2D* pBottom, Display::C_Texture2D* pLeft, Display::C_Texture2D* pRight, const F32 fTimeStep)
{
#if PLATFORM_PS3
	m_SolveData.m_pNext		= pSurface->m_pNextTexture->GetTexture().GetAddress();
	m_SolveData.m_pCurrent	= pSurface->m_pCurrentTexture->GetTexture().GetAddress();
	m_SolveData.m_pPrevious	= pSurface->m_pPreviousTexture->GetTexture().GetAddress();
	m_SolveData.m_pTop		= pTop ? pTop->GetAddress() : 0;
	m_SolveData.m_pBottom	= pBottom ? pBottom->GetAddress() : 0;
	m_SolveData.m_pLeft		= pLeft ? pLeft->GetAddress() : 0;
	m_SolveData.m_pRight	= pRight ? pRight->GetAddress() : 0;
	m_SolveData.m_iWidth	= pSurface->m_iTextureSize;
	m_SolveData.m_iHeight	= pSurface->m_iTextureSize;
	m_SolveData.m_iPitch	= pSurface->m_iTextureSize * sizeof(U32);
	
	const F32 fPixelSize	= 1.0f / (F32) pSurface->m_iTextureSize;
	const F32 fH			= pSurface->m_fSurfaceSize * fPixelSize;
	const F32 fC			= g_debugHeightfieldSpeed;
	const F32 fT			= (fTimeStep < (fH / fC)) ? fTimeStep : (fH / fC) - 0.000001f;
	m_SolveData.m_fScale	= 4.0f * fC * fC * fT * fT / (fH * fH);
	m_SolveData.m_fDamping	= g_debugHeightfieldDamping;

	Scheduler::AddTask(HeightfieldSolveModule, 0, &m_SolveData, m_hSolveJob, m_hPolicy, 0, 0);

	Display::C_RenderTexture2D* pRenderTexture = pSurface->m_pPreviousTexture;
	pSurface->m_pPreviousTexture = pSurface->m_pCurrentTexture;
	pSurface->m_pCurrentTexture = pSurface->m_pNextTexture;
	pSurface->m_pNextTexture = pRenderTexture;
#endif
}

void C_HeightfieldCpuSolver::ModifyWait() const
{
	while (Scheduler::jqGetQueuedBatchCount(m_hModifyJob) + Scheduler::jqGetExecutingBatchCount(m_hModifyJob) > 0);
}

void C_HeightfieldCpuSolver::SolveWait() const
{
	while (Scheduler::jqGetQueuedBatchCount(m_hSolveJob) + Scheduler::jqGetExecutingBatchCount(m_hSolveJob) > 0);
}

C_HeightfieldGpuSolver::C_HeightfieldGpuSolver() : Display::I_Renderer("HeightfieldGpuSolver")
{
}

C_HeightfieldGpuSolver::~C_HeightfieldGpuSolver()
{
	DestroyRenderer();
}

bool C_HeightfieldGpuSolver::OnCreate()
{
	if (!Display::CreatePixelShaderFromFunc("Shaders/psHeightfieldModify", 0, &m_ModifyPixelShader, REF_SHADER(psHeightfieldModify)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psHeightfieldSolve", 0, &m_SolvePixelShader[E_Shader_Inside], REF_SHADER(psHeightfieldSolve)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psHeightfieldSolveTop", 0, &m_SolvePixelShader[E_Shader_Top], REF_SHADER(psHeightfieldSolveTop)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psHeightfieldSolveBottom", 0, &m_SolvePixelShader[E_Shader_Bottom], REF_SHADER(psHeightfieldSolveBottom)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psHeightfieldSolveLeft", 0, &m_SolvePixelShader[E_Shader_Left], REF_SHADER(psHeightfieldSolveLeft)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psHeightfieldSolveRight", 0, &m_SolvePixelShader[E_Shader_Right], REF_SHADER(psHeightfieldSolveRight)))
		return false;

	m_ModifyParameter = m_ModifyPixelShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_ModifyUParameter = m_ModifyPixelShader.GetParameter(CP_RENDER_SETTINGS_1, "g_vTransformU");
	m_ModifyVParameter = m_ModifyPixelShader.GetParameter(CP_RENDER_SETTINGS_2, "g_vTransformV");
	
	for (U32 shaderNo = 0; shaderNo < E_Shader_Count; ++shaderNo)
	{
		m_SolveParameter[shaderNo] = m_SolvePixelShader[shaderNo].GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	}

	return true;
}

void C_HeightfieldGpuSolver::OnDestroy()
{
	for (U32 shaderNo = 0; shaderNo < E_Shader_Count; ++shaderNo)
	{
		Display::Device()->Destroy(&m_SolvePixelShader[shaderNo]);
	}
	Display::Device()->Destroy(&m_ModifyPixelShader);
}

void C_HeightfieldGpuSolver::ClearSurface(const C_HeightfieldSurface* pSurface) const
{
	const Display::C_RenderTarget& renderTarget = Display::Device()->GetRenderTarget();

	for (U32 i = 0; i <= c_HeightfieldIterations; ++i)
	{
		const Display::C_RenderTexture2D* pRenderTexture = &pSurface->m_HeightTextures[i];
		const Display::C_Surface* pColourSurface = &pRenderTexture->GetSurface();
		Display::Device()->SetColourDepthStencilSurface(pColourSurface, 0);
		Display::Device()->Clear(Display::E_Clear_Colour, Maths::RGBA(0x00000000));
		pRenderTexture->Resolve();
	}

	Display::Device()->SetColourDepthStencilSurface(renderTarget.GetColourSurface(), renderTarget.GetDepthStencilSurface());
}

void C_HeightfieldGpuSolver::ModifySurface(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pTexture, const Display::C_Float4& vTransformU, const Display::C_Float4& vTransformV, const F32 fHeight) const
{
	const Display::C_Surface* pNextSurface		= &pSurface->m_pNextTexture->GetSurface();
	const Display::C_Texture* pCurrentTexture	= &pSurface->m_pCurrentTexture->GetTexture();

	const Display::C_RenderTarget& renderTarget = Display::Device()->GetRenderTarget();
	Display::Device()->SetColourDepthStencilSurface(pNextSurface, 0);
	
	Display::C_Viewport viewport;
	viewport.m_W = pNextSurface->GetWidth();
	viewport.m_H = pNextSurface->GetHeight();
	Display::Device()->SetViewport(viewport);

	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
	Display::Device()->SetTexture(0, pCurrentTexture);
	Display::Device()->SetTexture(1, pTexture);

	const Display::C_Float4 settings(fHeight, 0.0f, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_ModifyParameter, (F32*)&settings);
	Display::Device()->SetPixelShaderParameterF(m_ModifyUParameter, (F32*)&vTransformU);
	Display::Device()->SetPixelShaderParameterF(m_ModifyVParameter, (F32*)&vTransformV);
	Display::Device()->SetPixelShader(&m_ModifyPixelShader);

	Display::Quad::DrawFullScreen();
	pSurface->m_pNextTexture->Resolve();

	Display::Device()->SetTexture(0, 0);
	Display::Device()->SetTexture(1, 0);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);

	Display::Device()->SetColourDepthStencilSurface(renderTarget.GetColourSurface(), renderTarget.GetDepthStencilSurface());

	Display::C_RenderTexture2D* pRenderTexture = pSurface->m_pCurrentTexture;
	pSurface->m_pCurrentTexture = pSurface->m_pNextTexture;
	pSurface->m_pNextTexture = pRenderTexture;

	Display::Device()->SetColourDepthStencilSurface(renderTarget.GetColourSurface(), renderTarget.GetDepthStencilSurface());
}

void C_HeightfieldGpuSolver::BindSolver(const C_HeightfieldSurface* pSurface) const
{
	const Display::C_Surface* pNextSurface		= &pSurface->m_pNextTexture->GetSurface();
	const Display::C_Texture* pCurrentTexture	= &pSurface->m_pCurrentTexture->GetTexture();
	const Display::C_Texture* pPreviousTexture	= &pSurface->m_pPreviousTexture->GetTexture();

	m_SavedRenderTarget = Display::Device()->GetRenderTarget();
	Display::Device()->SetColourDepthStencilSurface(pNextSurface, 0);
	
	Display::C_Viewport viewport;
	viewport.m_W = pNextSurface->GetWidth();
	viewport.m_H = pNextSurface->GetHeight();
	Display::Device()->SetViewport(viewport);

	Display::Device()->SetTexture(0, pCurrentTexture);
	Display::Device()->SetTexture(1, pPreviousTexture);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Point);

	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	Display::Device()->SetCullMode(Display::E_CullMode_None);
}

void C_HeightfieldGpuSolver::UnbindSolver(const C_HeightfieldSurface* pSurface) const
{
	pSurface->m_pNextTexture->Resolve();

	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);

	Display::Device()->SetTexture(0, 0);
	Display::Device()->SetTexture(1, 0);
	Display::Device()->SetTexture(2, 0);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Default);

	Display::Device()->SetColourDepthStencilSurface(m_SavedRenderTarget.GetColourSurface(), m_SavedRenderTarget.GetDepthStencilSurface());

	Display::C_RenderTexture2D* pRenderTexture = pSurface->m_pPreviousTexture;
	pSurface->m_pPreviousTexture = pSurface->m_pCurrentTexture;
	pSurface->m_pCurrentTexture = pSurface->m_pNextTexture;
	pSurface->m_pNextTexture = pRenderTexture;
}

void C_HeightfieldGpuSolver::SolveSurface(const C_HeightfieldSurface* pSurface, const F32 fTimeStep)
{
	SolveSurface(pSurface, 0, 0, 0, 0, fTimeStep);
}

void C_HeightfieldGpuSolver::SolveSurface(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pTop, const Display::C_Texture2D* pBottom, const Display::C_Texture2D* pLeft, const Display::C_Texture2D* pRight, const F32 fTimeStep)
{
	BindSolver(pSurface);
	SolveInterior(pSurface, fTimeStep);
	if (pTop)
		SolveTop(pSurface, pTop, fTimeStep);
	if (pBottom)
		SolveBottom(pSurface, pBottom, fTimeStep);
	if (pLeft)
		SolveLeft(pSurface, pLeft, fTimeStep);
	if (pRight)
		SolveRight(pSurface, pRight, fTimeStep);
	UnbindSolver(pSurface);
}

void C_HeightfieldGpuSolver::SolveInterior(const C_HeightfieldSurface* pSurface, const F32 fTimeStep) const
{
	PERF_EVENT("Heightfield::SolveInterior");

	const F32 fPixelSize	= 1.0f / (F32) pSurface->m_iTextureSize;
	const F32 fH			= pSurface->m_fSurfaceSize * fPixelSize;
	const F32 fC			= g_debugHeightfieldSpeed;
	const F32 fT			= (fTimeStep < (fH / fC)) ? fTimeStep : (fH / fC) - 0.000001f;
	const F32 fScale		= 4.0f * fC * fC * fT * fT / (fH * fH);
	const F32 fDamping		= g_debugHeightfieldDamping;
	const Display::C_Float4 settings(fScale, fDamping, fPixelSize, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_SolveParameter[E_Shader_Inside], (F32*)&settings);
	Display::Device()->SetPixelShader(&m_SolvePixelShader[E_Shader_Inside]);

	F32 u = 0.0f, v = 0.0f, du = 1.0f, dv = 1.0f;
	F32 x0 = -1.0f;
	F32 y0 = 1.0f;
	F32 dx = 2.0f;
	F32 dy = -2.0f;
	#ifdef PLATFORM_PC
	x0 -= fPixelSize;
	y0 += fPixelSize;
	#endif

	// always use this vertex shader
	Display::Device()->SetVertexShader(&Display::VertexShader_XY::m_VertexShader);
		
	Display::Vertex_XY_UV* pVertex = (Display::Vertex_XY_UV*)Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleList, 6, sizeof(Display::Vertex_XY_UV), Display::Vertex_XY_UV::m_pDeclaration);
	if(pVertex)
	{
		// tri 1
		pVertex->x = x0;		pVertex->y = y0;		pVertex->u = u;			pVertex->v = v;			pVertex++;
		pVertex->x = x0 + dx;	pVertex->y = y0;		pVertex->u = u + du;	pVertex->v = v;			pVertex++;
		pVertex->x = x0;		pVertex->y = y0 + dy;	pVertex->u = u;			pVertex->v = v + dv;	pVertex++;

		// tri 2
		pVertex->x = x0;		pVertex->y = y0 + dy;	pVertex->u = u;			pVertex->v = v + dv;	pVertex++;
		pVertex->x = x0 + dx;	pVertex->y = y0;		pVertex->u = u + du;	pVertex->v = v;			pVertex++;
		pVertex->x = x0 + dx;	pVertex->y = y0 + dy;	pVertex->u = u + du;	pVertex->v = v + dv;	pVertex++;

		Display::Device()->EndVertices(pVertex);
	}
}

void C_HeightfieldGpuSolver::SolveTop(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pTop, const F32 fTimeStep) const
{
	const F32 fPixelSize = 1.0f / (F32) pSurface->m_iTextureSize;
	SolveEdge(pSurface, pTop, E_Shader_Top, -1.0f, +1.0f - 2.0f * fPixelSize, +1.0f, +1.0f - 2.0f * fPixelSize, 0.0f, -fPixelSize, fTimeStep);
}

void C_HeightfieldGpuSolver::SolveBottom(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pBottom, const F32 fTimeStep) const
{
	const F32 fPixelSize = 1.0f / (F32) pSurface->m_iTextureSize;
	SolveEdge(pSurface, pBottom, E_Shader_Bottom, -1.0f, -1.0f, +1.0f, -1.0f, 0.0f, -fPixelSize, fTimeStep);
}

void C_HeightfieldGpuSolver::SolveLeft(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pLeft, const F32 fTimeStep) const
{
	const F32 fPixelSize = 1.0f / (F32) pSurface->m_iTextureSize;
	SolveEdge(pSurface, pLeft, E_Shader_Left, -1.0f, +1.0f, -1.0f, -1.0f, 0.0f, 0.0f, fTimeStep);
}

void C_HeightfieldGpuSolver::SolveRight(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pRight, const F32 fTimeStep) const
{
	const F32 fPixelSize = 1.0f / (F32) pSurface->m_iTextureSize;
	SolveEdge(pSurface, pRight, E_Shader_Right, +1.0f - 2.0f * fPixelSize, +1.0f, +1.0f - 2.0f * fPixelSize, -1.0f, 0.0f, 0.0f, fTimeStep);
}

void C_HeightfieldGpuSolver::SolveEdge(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pEdge, const E_Shader eShader, const F32 x1, const F32 y1, const F32 x2, const F32 y2, const F32 du, const F32 dv, const F32 fTimeStep) const
{
	PERF_EVENT("Heightfield::SolveEdge");

	Display::Device()->SetTexture(2, pEdge);

	const F32 fPixelSize	= 1.0f / (F32) pSurface->m_iTextureSize;
	const F32 fH			= pSurface->m_fSurfaceSize * fPixelSize;
	const F32 fC			= g_debugHeightfieldSpeed;
	const F32 fT			= (fTimeStep < (fH / fC)) ? fTimeStep : (fH / fC) - 0.000001f;
	const F32 fScale		= 4.0f * fC * fC * fT * fT / (fH * fH);
	const F32 fDamping		= g_debugHeightfieldDamping;
	const Display::C_Float4 settings(fScale, fDamping, fPixelSize, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_SolveParameter[eShader], (F32*)&settings);
	Display::Device()->SetPixelShader(&m_SolvePixelShader[eShader]);

	Display::Device()->SetVertexShader(&Display::VertexShader_XY::m_VertexShader);

	Display::Vertex_XY_UV* pVertex = (Display::Vertex_XY_UV*)Display::Device()->BeginVertices(Display::E_PrimitiveType_LineList, 2, sizeof(Display::Vertex_XY_UV), Display::Vertex_XY_UV::m_pDeclaration);
	if (pVertex)
	{
		pVertex->x = x1;
		pVertex->y = y1;
		pVertex->u = x1 * +0.5f + 0.5f + du;
		pVertex->v = y1 * -0.5f + 0.5f + dv;
		pVertex++;

		pVertex->x = x2;
		pVertex->y = y2;
		pVertex->u = x2 * +0.5f + 0.5f + du;
		pVertex->v = y2 * -0.5f + 0.5f + dv;
		pVertex++;

		Display::Device()->EndVertices();
	}
}

}; // namespace Effect
