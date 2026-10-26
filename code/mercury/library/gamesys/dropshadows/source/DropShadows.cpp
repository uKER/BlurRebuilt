#include "stdafx.h"
#include "DropShadows/DropShadows.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Core/Maths/HVec/New/IncludeMinimal.h"
#include "Core/Maths/HVec/New/IncludeMinimal.inl"
#include "Display/Buffer/VertexDeclarations.h"
#include "System/debug/I_DebugRender.h"
#include <scenegraph/Context.h>
#include <Display/Context/Context.h>
#include <SceneGraph/View.h>
#include <SceneGraph/Visitor.h>
#include <SceneGraph/Context.h>
#include "Deferred/Manager.h"

EXTERN_SHADER(vsDropShadowsViewNormal);
EXTERN_SHADER(psDropShadowsViewNormal1);
EXTERN_SHADER(psDropShadowsViewNormal2);
EXTERN_SHADER(psDropShadowsViewNormal3);
EXTERN_SHADER(psDropShadowsViewNormal4);
EXTERN_SHADER(psDropShadowsViewNormal5);
EXTERN_SHADER(psDropShadowsViewNormal6);
EXTERN_SHADER(psDropShadowsViewNormal7);
EXTERN_SHADER(psDropShadowsViewNormal8);
EXTERN_SHADER(psDropShadowsViewNormal1HDR);
EXTERN_SHADER(psDropShadowsViewNormal2HDR);
EXTERN_SHADER(psDropShadowsViewNormal3HDR);
EXTERN_SHADER(psDropShadowsViewNormal4HDR);
EXTERN_SHADER(psDropShadowsViewNormal5HDR);
EXTERN_SHADER(psDropShadowsViewNormal6HDR);
EXTERN_SHADER(psDropShadowsViewNormal7HDR);
EXTERN_SHADER(psDropShadowsViewNormal8HDR);

Debug::Bool gDebugShadowDropShadowsEnable("Graphics.Render.Deferred.DropShadows.Enable", true);
Debug::Bool gDebugShadowDropShadows("Graphics.Render.Deferred.DropShadows.ShowDropShadows", false);
Debug::Bool gDebugDisableBlendInShadowDropShadows("Graphics.Render.Deferred.DropShadows.DisableBlendInDropShadows", false);
Debug::Bool gDebugShadowDropShadowTiles("Graphics.Render.Deferred.DropShadows.ShowDropShadowTiles", false);

Debug::Float gDebugShadowDropShadowsMinSize("Graphics.Render.Deferred.DropShadows.DropShadowsMinSize", 0.00f);
Debug::Float gDebugShadowDropShadowsMaxDistant("Graphics.Render.Deferred.DropShadows.DropShadowsMaxDistant", 100.f);

namespace DropShadows
{

const U32 c_MaximumShadows = 8;

C_DropShadows::C_DropShadows(Deferred::C_Manager* pDeferredManager, U32 visibilityFlags) : Display::I_Renderer("DeferredDropShadows")
{
	BIZ_ASSERT(pDeferredManager);
	
	m_pDeferredManager = pDeferredManager;
	m_VisibilityFlags = visibilityFlags;

	m_HorizontalTiles = 8;
	m_VerticalTiles = 4;

#ifdef PLATFORM_PS3
	m_CachedShader = 0xFFFFFFFF;
#endif
}

C_DropShadows::~C_DropShadows()
{
	DestroyRenderer();
}

bool C_DropShadows::OnCreate()
{
	RESOURCE_MEMORY_ID("DropShadows");

	if (!Display::CreateVertexShaderFromFunc("Shader/vsDropShadowsViewNormal", 0, &m_VertexShader, REF_SHADER(vsDropShadowsViewNormal)))
		return false;

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal1HDR", 0, &m_PixelShader[E_DropShadowShader_1], REF_SHADER(psDropShadowsViewNormal1HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal2HDR", 0, &m_PixelShader[E_DropShadowShader_2], REF_SHADER(psDropShadowsViewNormal2HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal3HDR", 0, &m_PixelShader[E_DropShadowShader_3], REF_SHADER(psDropShadowsViewNormal3HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal4HDR", 0, &m_PixelShader[E_DropShadowShader_4], REF_SHADER(psDropShadowsViewNormal4HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal5HDR", 0, &m_PixelShader[E_DropShadowShader_5], REF_SHADER(psDropShadowsViewNormal5HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal6HDR", 0, &m_PixelShader[E_DropShadowShader_6], REF_SHADER(psDropShadowsViewNormal6HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal7HDR", 0, &m_PixelShader[E_DropShadowShader_7], REF_SHADER(psDropShadowsViewNormal7HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal8HDR", 0, &m_PixelShader[E_DropShadowShader_8], REF_SHADER(psDropShadowsViewNormal8HDR)))
			return false;
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal1", 0, &m_PixelShader[E_DropShadowShader_1], REF_SHADER(psDropShadowsViewNormal1)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal2", 0, &m_PixelShader[E_DropShadowShader_2], REF_SHADER(psDropShadowsViewNormal2)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal3", 0, &m_PixelShader[E_DropShadowShader_3], REF_SHADER(psDropShadowsViewNormal3)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal4", 0, &m_PixelShader[E_DropShadowShader_4], REF_SHADER(psDropShadowsViewNormal4)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal5", 0, &m_PixelShader[E_DropShadowShader_5], REF_SHADER(psDropShadowsViewNormal5)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal6", 0, &m_PixelShader[E_DropShadowShader_6], REF_SHADER(psDropShadowsViewNormal6)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal7", 0, &m_PixelShader[E_DropShadowShader_7], REF_SHADER(psDropShadowsViewNormal7)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadowsViewNormal8", 0, &m_PixelShader[E_DropShadowShader_8], REF_SHADER(psDropShadowsViewNormal8)))
			return false;
	}

	for (U32 shaderNo = 0; shaderNo < E_DropShadowShader_Count; ++shaderNo)
	{
		m_PositionParameter[shaderNo]	= m_PixelShader[shaderNo].GetParameterArray(CP_RENDER_SETTINGS, "g_vPosition", shaderNo + 1);
		m_IntensityParameter[shaderNo]	= m_PixelShader[shaderNo].GetParameterArray(CP_RENDER_SETTINGS_8, "g_vIntensity", shaderNo + 1);
		m_ViewUpVectorParameter[shaderNo]	= m_PixelShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_20,	"g_vViewUpVector");
		m_DepthParameter[shaderNo]	= m_PixelShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_21,	"g_vDepth");
	}

	if (!Display::Device()->CreateVertexBuffer(sizeof(Display::Vertex_XY) * m_VerticalTiles * m_HorizontalTiles * 4, 0, &m_VertexBuffer))
		return false;

	Display::Vertex_XY* pVertex = (Display::Vertex_XY*)m_VertexBuffer.Lock();
	if (pVertex)
	{
		const F32 fDX = 2.0f / (F32) m_HorizontalTiles;
		const F32 fDY = 2.0f / (F32) m_VerticalTiles;
		for (U32 y = 0; y < m_VerticalTiles; ++y)
		{
			for (U32 x = 0; x < m_HorizontalTiles; ++x)
			{
				const F32 fX = x * fDX - 1.0f;
				const F32 fY = y * fDY - 1.0f;

				pVertex->x = fX;
				pVertex->y = fY;
				pVertex++;

				pVertex->x = fX;
				pVertex->y = fY + fDY;
				pVertex++;

				pVertex->x = fX + fDX;
				pVertex->y = fY;
				pVertex++;

				pVertex->x = fX + fDX;
				pVertex->y = fY + fDY;
				pVertex++;
			}
		}

		m_VertexBuffer.Unlock();
	}

	return true;
}

bool C_DropShadows::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("DropShadows");

	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
	
	const U32 iWidth = backBuffer.GetWidth();
	const U32 iHeight = backBuffer.GetHeight();
	const Display::E_Format eFormat = backBuffer.GetColourFormat();

	if (!Display::Device()->CreateRenderTexture2D(iWidth, iHeight, 1, 0, eFormat, Display::E_Multisample_None, 0, &m_RenderTexture))
		return false;
	
	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_RenderTexture, "_DropShadowsRT");
	#endif

	return true;
}

void C_DropShadows::OnDestroy()
{
	for (U32 shaderNo = 0; shaderNo < E_DropShadowShader_Count; ++shaderNo)
	{
		Display::Device()->Destroy(&m_PixelShader[shaderNo]);
	}
	Display::Device()->Destroy(&m_VertexShader);

	Display::Device()->Destroy(&m_VertexBuffer);
}

void C_DropShadows::OnDestroyVolatile()
{
	Display::Device()->Destroy(&m_RenderTexture);
}

void C_DropShadows::OnRender(SceneGraph::C_Context& context) const
{
	Display::C_Context* pDisplay = Display::GetContext(&context);
	context.SetContextFlags(pDisplay->GetFlags());

	Render(pDisplay);
}

void C_DropShadows::OnGather(const SceneGraph::C_View& view) const
{
	if (m_VisibilityFlags & view.GetTypeFlag())
		view.AddVisibleNode(this, m_RenderPass);
}

void C_DropShadows::Accept(SceneGraph::I_Visitor& visitor) const
{
	visitor.Visit(*this);
}

void C_DropShadows::SetRenderPass(const U32 renderPass)
{
	m_RenderPass = renderPass;
}

void C_DropShadows::Render() const
{
	Display::C_DeviceContext *pContext = new Display::C_DeviceContext;
	Display::C_Context *pDContext = Display::GetContext(pContext);
	pDContext->BeginCommandBuffer(NULL);
	Render(Display::GetContext(pDContext));
	pDContext->EndCommandBuffer();
	delete pContext;
}

void C_DropShadows::Render(Display::C_Context* pContext) const
{
	PERF_EVENT("RenderDropShadows");

	if (!gDebugShadowDropShadowsEnable)
		return;

	S32 viewIndex = pContext->GetView()->GetIndex();

	if (m_DropShadows[viewIndex].Size() == 0)
		return;

	VisibleDropShadows visibleShadows;
	GatherVisibleShadows(viewIndex, visibleShadows, pContext->GetView()->GetViewSettings3D());
	
	if (visibleShadows.Size() == 0)
	{
		m_DropShadows[viewIndex].Clear();
		return;
	}

	if (gDebugDisableBlendInShadowDropShadows)
		pContext->SetBlendMode(Display::E_BlendMode_Solid);
	else
		pContext->SetBlendMode(Display::E_BlendMode_Multiply);
	pContext->SetCullMode(Display::E_CullMode_None);
	pContext->SetDepthEnable(Display::E_Bool_False);
	pContext->SetDepthWriteEnable(Display::E_Bool_False);

	pContext->SetVertexShader(&m_VertexShader);

	pContext->SetTexture(0, pContext->GetDepthStencilTexture()->GetGPUTexture());
	pContext->SetTexture(1, m_pDeferredManager->GetColourTexture(viewIndex, 0)->GetGPUTexture());

	Display::C_SamplerUsageFlags flags;
	flags.SetEnabled(0, 0);
	flags.SetEnabled(0, 1);
	pContext->SetSamplerUsageFlags(&flags);

	Display::C_SamplerState depthSampleState;
	depthSampleState.SetDefaultStates();
	depthSampleState.SetTextureAddress(Display2::E_TextureAddress_Clamp);
	depthSampleState.SetTextureFilter(Display2::E_TextureFilter_Point);
	//depthSampleState.SetTextureGamma(Display2::E_Bool_False);
	pContext->SetSamplerState(0, &depthSampleState);

	Display::C_SamplerState normalSampleState;
	normalSampleState.SetDefaultStates();
	normalSampleState.SetTextureAddress(Display2::E_TextureAddress_Clamp);
	normalSampleState.SetTextureFilter(Display2::E_TextureFilter_Point);
	//normalSampleState.SetTextureGamma(Display2::E_Bool_False);
	pContext->SetSamplerState(1, &normalSampleState);

#if PLATFORM_PS3
	pContext->m_Context.SetTextureControl(0, Display2::E_Bool_True, 0, 1<<8, CELL_GCM_TEXTURE_MAX_ANISO_1);
#endif

	pContext->SetVertexDeclaration(Display::Vertex_XY::m_pDeclaration);
	pContext->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XY));

	Maths::HMat44 mTexture;
	{
		Display::C_Matrix mTemp;
		Display::MatrixSetIdentity(&mTemp);
		mTemp._11 = +2.0f;
		mTemp._22 = -2.0f;
		mTemp._41 = -1.0f;
		mTemp._42 = +1.0f;
		mTexture = Maths::Load<Maths::HMat44>(&mTemp);
	}
	const Maths::HMat44 mDeviceToWorld = pContext->GetView()->GetViewSettings3D().GetWorldToDevice().Inverse();
	const Maths::HMat44 mDeviceToCamera = pContext->GetView()->GetViewSettings3D().GetCameraToDevice().Inverse();

	MathsStorage::Float4 vDepthParameter = Render::C_RenderState::Instance().GetDepthScaleShaderParameter(pContext->GetView()->GetViewSettings3D().GetNearClip(), pContext->GetView()->GetViewSettings3D().GetFarClip());
	vDepthParameter.y = vDepthParameter.y / pContext->GetView()->GetViewSettings3D().GetFarClip();

	Maths::HVec4 vFar0;
	Maths::HVec4 vFar1;
	Maths::HVec4 vFar2;
	Maths::HVec4 vFar3;
	if (Display::Device()->GetDepthFlip())
	{
		vFar0 = Maths::HVec4(-1.0f, -1.0f, 0.0f, 1.0f) * mDeviceToCamera;
		vFar1 = Maths::HVec4(-1.0f, +1.0f, 0.0f, 1.0f) * mDeviceToCamera;
		vFar2 = Maths::HVec4(+1.0f, +1.0f, 0.0f, 1.0f) * mDeviceToCamera;
		vFar3 = Maths::HVec4(+1.0f, -1.0f, 0.0f, 1.0f) * mDeviceToCamera;
	}
	else
	{
		vFar0 = Maths::HVec4(-1.0f, -1.0f, 1.0f, 1.0f) * mDeviceToCamera;
		vFar1 = Maths::HVec4(-1.0f, +1.0f, 1.0f, 1.0f) * mDeviceToCamera;
		vFar2 = Maths::HVec4(+1.0f, +1.0f, 1.0f, 1.0f) * mDeviceToCamera;
		vFar3 = Maths::HVec4(+1.0f, -1.0f, 1.0f, 1.0f) * mDeviceToCamera;
	}
	vFar0 = vFar0 * vFar0.W().Rcp();
	vFar1 = vFar1 * vFar1.W().Rcp();
	vFar2 = vFar2 * vFar2.W().Rcp();
	vFar3 = vFar3 * vFar3.W().Rcp();
	vFar0 = vFar0 * vDepthParameter.y;
	vFar1 = vFar1 * vDepthParameter.y;
	vFar2 = vFar2 * vDepthParameter.y;
	vFar3 = vFar3 * vDepthParameter.y;

	Display::C_Float4 vsSettings[4];
	Maths::Store(vFar0, &vsSettings[0]);
	Maths::Store(vFar1, &vsSettings[1]);
	Maths::Store(vFar2, &vsSettings[2]);
	Maths::Store(vFar3, &vsSettings[3]);

	pContext->SetVertexShaderConstantF(CV_RENDER_SETTINGS, (F32*)vsSettings, 4);

	//UpVector
	Display::C_Float4 vViewUpVector;
	vViewUpVector = Display::C_Float4(pContext->GetView()->GetViewSettings3D().GetCameraToWorld().AX().Y(), pContext->GetView()->GetViewSettings3D().GetCameraToWorld().AY().Y(), pContext->GetView()->GetViewSettings3D().GetCameraToWorld().AZ().Y(), 1.f);

#if defined(PLATFORM_PC)
	const F32 fOffsetX = +1.0f / (F32) pContext->GetDepthStencilTexture()->GetWidth();
	const F32 fOffsetY = +1.0f / (F32) pContext->GetDepthStencilTexture()->GetHeight();
#else
	const F32 fOffsetX = 0.0f;
	const F32 fOffsetY = 0.0f;
#endif

	Display::C_Float4 vsUVScale(0.5f, -0.5f, 0.5f + 0.5f * fOffsetX, 0.5f + 0.5f * fOffsetY);
	pContext->SetVertexShaderConstantF(CV_RENDER_SETTINGS_4, (F32*)&vsUVScale, 1);


#ifdef PLATFORM_PS3
	//Clear cache
	for (U32 i = 0; i < E_DropShadowShader_Count; i++)
	{
		m_ParamsCache[i].m_Patched = false;
	}
	m_CachedShader = 0xFFFFFFFF;
#endif

	for (U32 y = 0; y < m_VerticalTiles; ++y)
	{
		for (U32 x = 0; x < m_HorizontalTiles; ++x)
		{
			const F32 fDX = 2.0f / (F32) m_HorizontalTiles;
			const F32 fDY = 2.0f / (F32) m_VerticalTiles;
			const F32 fX = x * fDX - 1.0f + fOffsetX;
			const F32 fY = y * fDY - 1.0f + fOffsetY;

			const Maths::C_Rect rectangle(Maths::HVec2(fX, fY), Maths::HVec2(fX + fDX, fY + fDY));

			U32 shadowCount = 0;
			U32	vShadowIndexes[c_MaximumShadows];
			Display::C_Float4 vShadowPositions[c_MaximumShadows];
			Display::C_Float4 vShadowIntensities[c_MaximumShadows];


			U32 renderCount = 0;

			U32 firstVisibleShadow = 0;
			bool endProcessing = false;
			do 
			{
				endProcessing = GatherVisibleParameters(visibleShadows, rectangle, firstVisibleShadow, shadowCount, vShadowIndexes);

				if (shadowCount > 0)
				{
					const U32 shader = shadowCount - 1;

					for (U32 i = 0; i < shadowCount; i++)
					{
						const C_DropShadow* const pShadow	= visibleShadows[vShadowIndexes[i]].m_pShadow;

						//Convert position to viewspace
						Maths::HVec4 shadowPosition = Maths::Load<Maths::HVec3>(&pShadow->m_vPosition) || Maths::HVecW(1.f);
						Maths::HVec3 shadowViewPosition = Maths::HVec3(shadowPosition * pContext->GetView()->GetViewSettings3D().GetWorldToCamera());
						vShadowPositions[i]	= MathsStorage::Float4(shadowViewPosition.X(), shadowViewPosition.Y(), shadowViewPosition.Z(), 1.0f / (pShadow->m_fRadius * pShadow->m_fRadius));
						vShadowIntensities[i]	= MathsStorage::Float4(pShadow->m_fIntensity, 0.0f, 0.0f, 0.0f);
					}
#ifdef PLATFORM_PS3
					//Check cache values
					bool NeedUpdateTile = false;
					
					for (U32 i = 0; i < shadowCount; i++)
					{
						if (vShadowIndexes[i] != m_ParamsCache[shader].m_ShadowsIndex[i])
						{
							NeedUpdateTile = true;
							m_ParamsCache[shader].m_ShadowsIndex[i] = vShadowIndexes[i];
						}
					}
					bool NeedUpdateParameters = false;
					if (NeedUpdateTile || !m_ParamsCache[shader].m_Patched)
					{
						pContext->SetPixelShaderParameterArrayF(m_PositionParameter[shader], (F32*)&vShadowPositions);
						pContext->SetPixelShaderParameterArrayF(m_IntensityParameter[shader], (F32*)&vShadowIntensities);
						NeedUpdateParameters = true;
					}
					if (!m_ParamsCache[shader].m_Patched)
					{
						pContext->SetPixelShaderParameterF(m_ViewUpVectorParameter[shader], (F32*)&vViewUpVector);
						pContext->SetPixelShaderParameterF(m_DepthParameter[shader], (F32*)&vDepthParameter);
						m_ParamsCache[shader].m_Patched = true;
						NeedUpdateParameters = true;
					}

					if (shader == m_CachedShader)
					{
						if (NeedUpdateParameters)
							pContext->SetPixelShaderParameters(&m_PixelShader[shader]);
					}
					else
					{
						m_CachedShader = shader;
						pContext->SetPixelShader(&m_PixelShader[shader]);
					}
#else
					pContext->SetPixelShaderParameterArrayF(m_PositionParameter[shader], (F32*)&vShadowPositions);
					pContext->SetPixelShaderParameterArrayF(m_IntensityParameter[shader], (F32*)&vShadowIntensities);
					pContext->SetPixelShaderParameterF(m_ViewUpVectorParameter[shader], (F32*)&vViewUpVector);
					pContext->SetPixelShaderParameterF(m_DepthParameter[shader], (F32*)&vDepthParameter);
					
					pContext->SetPixelShader(&m_PixelShader[shader]);
#endif
					pContext->DrawPrimitive(Display2::E_PrimitiveType_TriangleStrip, (y * m_HorizontalTiles + x) * 4, 4);
					renderCount++;
				}

			} while (!endProcessing);

			if (gDebugShadowDropShadowTiles)
			{
				//Tile pos
				F32 posX, posY;
				posX = 1024.f * ((F32(x) + 0.5f) / F32(m_HorizontalTiles));
				posY = 768.f * (1.f - ((F32(y) + 0.5f) / F32(m_VerticalTiles)));
				Util::C_String msg = Util::C_String::Format("%i",renderCount);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(posX,posY), msg);	
			}
		}
	}

	Display::C_SamplerState defaultSample;
	defaultSample.SetDefaultStates();

	pContext->SetTexture(0, 0);
	pContext->SetSamplerState(0, &defaultSample);
	pContext->SetTexture(1, 0);
	pContext->SetSamplerState(1, &defaultSample);

	pContext->SetBlendMode(Display::E_BlendMode_Solid);
	pContext->SetCullMode(Display::E_CullMode_Default);
	pContext->SetDepthEnable(Display::E_Bool_True);
	pContext->SetDepthWriteEnable(Display::E_Bool_True);


	m_DropShadows[viewIndex].Clear();
}

void C_DropShadows::GatherVisibleShadows(const U32 viewIndex, VisibleDropShadows& visibleShadows, const Display::C_ViewSettings3D& viewSettings3D) const
{
	const Maths::VF32 vZero	= Maths::Const<Maths::VF32>::Zero();
	const Maths::VF32 vOne	= Maths::Const<Maths::VF32>::One();

	const Maths::Mat44 mView = Maths::Load<Maths::HMat44>(&viewSettings3D.GetViewMatrix());
	const Maths::Mat44 mProj = Maths::Load<Maths::HMat44>(&viewSettings3D.GetProjMatrix());
	
	const Maths::VF32 vUpCamera = vOne - Maths::Clamp(viewSettings3D.GetCameraToWorld().AY().Y(), vZero, vOne);

	const Maths::VF32 vMinSize		= Maths::MakeVF32(gDebugShadowDropShadowsMinSize);
	const Maths::VF32 vMaxDistance	= Maths::MakeVF32(gDebugShadowDropShadowsMaxDistant);

	const Maths::C_Rect screenRectangle(Maths::HVec2(-0.5f, -0.5f), Maths::HVec2(0.5f, 0.5f));

	DropShadows& dropShadows = m_DropShadows[viewIndex];

	for (S32 shadowNo = 0; shadowNo < dropShadows.Size(); ++shadowNo)
	{

		const Maths::Vec4F32	vPositionRadius		= Maths::Load<Maths::Vec4F32>((MathsStorage::Float4*) &dropShadows[shadowNo].m_vPosition);

		const Maths::Vec4F32	vWorldPosition		= Maths::MakeVec4F32(Maths::X(vPositionRadius), Maths::Y(vPositionRadius), Maths::Z(vPositionRadius), vOne);
		const Maths::VF32		fRadius				= Maths::W(vPositionRadius);
		const Maths::Vec4F32	vViewExtents0		= Maths::MakeVec4F32(-fRadius, -fRadius, -fRadius, vZero);
		const Maths::Vec4F32	vViewExtents1		= Maths::MakeVec4F32(-fRadius, -fRadius, +fRadius , vZero);
		const Maths::Vec4F32	vViewExtents2		= Maths::MakeVec4F32(+fRadius, fRadius*vUpCamera, -fRadius, vZero);
		const Maths::Vec4F32	vViewExtents3		= Maths::MakeVec4F32(+fRadius, fRadius*vUpCamera, +fRadius , vZero);

		const Maths::Vec4F32	vViewPosition		= vWorldPosition * mView;
		const Maths::Vec4F32	vViewPosition0		= vViewPosition + vViewExtents0;
		const Maths::Vec4F32	vViewPosition1		= vViewPosition + vViewExtents1;
		const Maths::Vec4F32	vViewPosition2		= vViewPosition + vViewExtents2;
		const Maths::Vec4F32	vViewPosition3		= vViewPosition + vViewExtents3;

		const Maths::Vec4F32	vProjPosition		= vViewPosition * mProj;
		const Maths::Vec4F32	vProjPosition0		= vViewPosition0 * mProj;
		const Maths::Vec4F32	vProjPosition1		= vViewPosition1 * mProj;
		const Maths::Vec4F32	vProjPosition2		= vViewPosition2 * mProj;
		const Maths::Vec4F32	vProjPosition3		= vViewPosition3 * mProj;

		if (vZero < Maths::W(vProjPosition3) && Maths::W(vProjPosition3) < vMaxDistance)
		{
			Maths::Vec4F32	vScreenPosition0	= vProjPosition0 * Maths::Rcp(Maths::W(vProjPosition0));
			Maths::Vec4F32	vScreenPosition1	= vProjPosition1 * Maths::Rcp(Maths::W(vProjPosition1));
			Maths::Vec4F32	vScreenPosition2	= vProjPosition2 * Maths::Rcp(Maths::W(vProjPosition2));
			Maths::Vec4F32	vScreenPosition3	= vProjPosition3 * Maths::Rcp(Maths::W(vProjPosition3));

			vScreenPosition0 = Maths::Select(Maths::W(vProjPosition0) < vZero, Maths::Sign(vProjPosition0), vScreenPosition0);
			vScreenPosition1 = Maths::Select(Maths::W(vProjPosition1) < vZero, Maths::Sign(vProjPosition1), vScreenPosition1);
			vScreenPosition2 = Maths::Select(Maths::W(vProjPosition2) < vZero, Maths::Sign(vProjPosition2), vScreenPosition2);
			vScreenPosition3 = Maths::Select(Maths::W(vProjPosition3) < vZero, Maths::Sign(vProjPosition3), vScreenPosition3);

			const Maths::Vec4F32	vScreenMin			= Maths::Min(vScreenPosition0, Maths::Min(vScreenPosition1, Maths::Min(vScreenPosition2, vScreenPosition3)));
			const Maths::Vec4F32	vScreenMax			= Maths::Max(vScreenPosition0, Maths::Max(vScreenPosition1, Maths::Max(vScreenPosition2, vScreenPosition3)));
			const Maths::Vec4F32	vScreenSize			= vScreenMax - vScreenMin;

			if (vMinSize < Maths::X(vScreenSize) && vMinSize < Maths::Y(vScreenSize))
			{
				const Maths::HVec2 vMin(Maths::X(vScreenMin), Maths::Y(vScreenMin));
				const Maths::HVec2 vMax(Maths::X(vScreenMax), Maths::Y(vScreenMax));

				C_VisibleDropShadow visibleShadow;
				visibleShadow.m_rectangle	= Maths::C_Rect(vMin, vMax);
				visibleShadow.m_pShadow		= &dropShadows[shadowNo];
				
				if (gDebugShadowDropShadows)
				{
					Maths::HVec3 vPosition(dropShadows[shadowNo].m_vPosition.x, dropShadows[shadowNo].m_vPosition.y, dropShadows[shadowNo].m_vPosition.z);
					
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(vPosition, dropShadows[shadowNo].m_fRadius, Maths::RGBA(200,200,200,255));
					Util::C_String msg = Util::C_String::Format("R: %f/nI: %f",dropShadows[shadowNo].m_fRadius, dropShadows[shadowNo].m_fIntensity);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(vPosition, 1, msg);	
				}

				visibleShadows.PushBack(visibleShadow);
			}
		}
	}
}

bool C_DropShadows::GatherVisibleParameters(const VisibleDropShadows& visibleShadows, const Maths::C_Rect& rectangle, U32& firstProcessedShadow, U32& shadowCount, U32* pShadowsIndexes) const
{
	shadowCount = 0;
	for (S32 i = firstProcessedShadow; i < visibleShadows.Size(); ++i)
	{
		if (Maths::Intersect(rectangle, visibleShadows[i].m_rectangle))
		{
			const C_DropShadow* const pShadow	= visibleShadows[i].m_pShadow;
			pShadowsIndexes[shadowCount] = i;
			++shadowCount;
		}
		if (c_MaximumShadows <= shadowCount)
		{
			firstProcessedShadow = i + 1;
			return false;
		}
	}
	return true;
}

void C_DropShadows::AddShadow(const U32 viewIndex, const MathsStorage::Float3& vPosition, const F32 fRadius, const F32 fIntensity)
{
	BIZ_ASSERT(viewIndex < 16);

	m_DropShadows[viewIndex].PushBack();

	m_DropShadows[viewIndex].Back().m_vPosition = vPosition;
	m_DropShadows[viewIndex].Back().m_fRadius = fRadius;
	m_DropShadows[viewIndex].Back().m_fIntensity = fIntensity;
}

}; // namespace DropShadows
