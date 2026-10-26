#include "stdafx.h"
#include "Deferred/PrePass/DynamicLights.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Btl/Containers/Radix.h"
#include "Core/Maths/HVec/New/IncludeMinimal.h"
#include "Core/Maths/HVec/New/IncludeMinimal.inl"
#include "Display/Buffer/VertexDeclarations.h"

EXTERN_SHADER(vsDynamicLightsPrepass);
EXTERN_SHADER(psDynamicLightsPrepass1);
EXTERN_SHADER(psDynamicLightsPrepass2);
EXTERN_SHADER(psDynamicLightsPrepass3);
EXTERN_SHADER(psDynamicLightsPrepass4);
EXTERN_SHADER(psDynamicLightsPrepass5);
EXTERN_SHADER(psDynamicLightsPrepass6);
EXTERN_SHADER(psDynamicLightsPrepass7);
EXTERN_SHADER(psDynamicLightsPrepass8);
EXTERN_SHADER(psDynamicLightsPrepass2xMSAA1);
EXTERN_SHADER(psDynamicLightsPrepass2xMSAA2);
EXTERN_SHADER(psDynamicLightsPrepass2xMSAA3);
EXTERN_SHADER(psDynamicLightsPrepass2xMSAA4);
EXTERN_SHADER(psDynamicLightsPrepass2xMSAA5);
EXTERN_SHADER(psDynamicLightsPrepass2xMSAA6);
EXTERN_SHADER(psDynamicLightsPrepass2xMSAA7);
EXTERN_SHADER(psDynamicLightsPrepass2xMSAA8);

namespace Deferred
{

extern Debug::Bool g_debugPrepassDepthBounds;

C_DynamicLights::C_DynamicLights() : Display::I_Renderer("DeferredDynamicLights")
{
	m_pDepthTexture = 0;
	m_pNormalTexture = 0;

	m_HorizontalTiles = 8;
	m_VerticalTiles = 4;
}

C_DynamicLights::~C_DynamicLights()
{
	DestroyRenderer();
}

bool C_DynamicLights::OnCreate()
{
	RESOURCE_MEMORY_ID("DynamicLights");

	if (!Display::CreateVertexShaderFromFunc("Shader/vsDynamicLightsPrepass", 0, &m_VertexShader, REF_SHADER(vsDynamicLightsPrepass)))
		return false;

	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass1", 0, &m_PixelShader[E_DynamicLightShader_1], REF_SHADER(psDynamicLightsPrepass1)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass2", 0, &m_PixelShader[E_DynamicLightShader_2], REF_SHADER(psDynamicLightsPrepass2)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass3", 0, &m_PixelShader[E_DynamicLightShader_3], REF_SHADER(psDynamicLightsPrepass3)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass4", 0, &m_PixelShader[E_DynamicLightShader_4], REF_SHADER(psDynamicLightsPrepass4)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass5", 0, &m_PixelShader[E_DynamicLightShader_5], REF_SHADER(psDynamicLightsPrepass5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass6", 0, &m_PixelShader[E_DynamicLightShader_6], REF_SHADER(psDynamicLightsPrepass6)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass7", 0, &m_PixelShader[E_DynamicLightShader_7], REF_SHADER(psDynamicLightsPrepass7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass8", 0, &m_PixelShader[E_DynamicLightShader_8], REF_SHADER(psDynamicLightsPrepass8)))
		return false;

	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass2xMSAA1", 0, &m_PixelShader[E_DynamicLightShader_2xMSAA_1], REF_SHADER(psDynamicLightsPrepass2xMSAA1)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass2xMSAA2", 0, &m_PixelShader[E_DynamicLightShader_2xMSAA_2], REF_SHADER(psDynamicLightsPrepass2xMSAA2)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass2xMSAA3", 0, &m_PixelShader[E_DynamicLightShader_2xMSAA_3], REF_SHADER(psDynamicLightsPrepass2xMSAA3)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass2xMSAA4", 0, &m_PixelShader[E_DynamicLightShader_2xMSAA_4], REF_SHADER(psDynamicLightsPrepass2xMSAA4)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass2xMSAA5", 0, &m_PixelShader[E_DynamicLightShader_2xMSAA_5], REF_SHADER(psDynamicLightsPrepass2xMSAA5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass2xMSAA6", 0, &m_PixelShader[E_DynamicLightShader_2xMSAA_6], REF_SHADER(psDynamicLightsPrepass2xMSAA6)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass2xMSAA7", 0, &m_PixelShader[E_DynamicLightShader_2xMSAA_7], REF_SHADER(psDynamicLightsPrepass2xMSAA7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLightsPrepass2xMSAA8", 0, &m_PixelShader[E_DynamicLightShader_2xMSAA_8], REF_SHADER(psDynamicLightsPrepass2xMSAA8)))
		return false;

	for (U32 shaderNo = 0; shaderNo < E_DynamicLightShader_Count; ++shaderNo)
	{
		m_PositionParameter[shaderNo]	= m_PixelShader[shaderNo].GetParameterArray(CP_RENDER_SETTINGS, "g_vPosition", 8);
		m_ColourParameter[shaderNo]		= m_PixelShader[shaderNo].GetParameterArray(CP_RENDER_SETTINGS_8, "g_vColour", 8);
		m_DepthParameter[shaderNo]		= m_PixelShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_16, "g_vDepth");
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

void C_DynamicLights::OnDestroy()
{
	for (U32 shaderNo = 0; shaderNo < E_DynamicLightShader_Count; ++shaderNo)
	{
		Display::Device()->Destroy(&m_PixelShader[shaderNo]);
	}
	Display::Device()->Destroy(&m_VertexShader);

	Display::Device()->Destroy(&m_VertexBuffer);
}

void C_DynamicLights::Render() const
{
	PERF_EVENT("RenderDynamicLights");

	const U32 iRenderTargetWidth = Display::Device()->GetRenderTarget().GetColourSurface()->GetWidth();
	const U32 iRenderTargetHeight = Display::Device()->GetRenderTarget().GetColourSurface()->GetHeight();
	const U32 iTextureWidth = m_pNormalTexture->GetWidth();
	const U32 iTextureHeight = m_pNormalTexture->GetHeight();

	const bool bMSAAx2 = (iTextureWidth / iRenderTargetWidth == 2) ? true : false;

	VisibleDynamicLights visibleLights;
	GatherVisibleLights(visibleLights);
	
	Display::Device()->SetBlendMode(Display::E_BlendMode_Additive);

	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);

	Display::Device()->SetVertexShader(&m_VertexShader);

	Display::Device()->SetTexture(0, m_pDepthTexture);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

	Display::Device()->SetTexture(1, m_pNormalTexture);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_False);

	Display::Device()->SetVertexDeclaration(Display::Vertex_XY::m_pDeclaration);

	Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XY));

	const Maths::HMat44 mDeviceToCamera = Display::Device()->GetViewSettings3D().GetCameraToDevice().Inverse();

	MathsStorage::Float4 vDepthParameter = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	vDepthParameter.y = vDepthParameter.y / Display::Device()->GetViewSettings3D().GetFarClip();

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

	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS, (F32*)vsSettings, 4);
	
#if defined(PLATFORM_PC)
	const Display::C_RenderTarget& renderTarget = Display::Device()->GetRenderTarget();
	const F32 fOffsetX = +1.0f / (F32)iRenderTargetWidth;
	const F32 fOffsetY = +1.0f / (F32)iRenderTargetHeight;
#else
	const F32 fOffsetX = 0.0f;
	const F32 fOffsetY = 0.0f;
#endif

	Display::C_Float4 vsUVScale(0.5f, -0.5f, 0.5f, -0.5f);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_4, (F32*)&vsUVScale, 1);

	Display::C_Float4 vsUVOffset(0.5f + 0.5f * fOffsetX, 0.5f + 0.5f * fOffsetY, 0.5f + 1.5f * fOffsetX, 0.5f + 0.5f * fOffsetY);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_5, (F32*)&vsUVOffset, 1);

#if defined(PLATFORM_PS3)
	if (g_debugPrepassDepthBounds)
		cellGcmSetDepthBoundsTestEnable(CELL_GCM_TRUE);
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

			U32 lightCount = 0;
			Display::C_Float4 vLightPositions[c_MaximumLights];
			Display::C_Float4 vLightColours[c_MaximumLights];
			
			F32 fNear = 1.0f, fFar = 0.0f;
			GatherVisibleParameters(visibleLights, rectangle, lightCount, vLightPositions, vLightColours, fNear, fFar);

			if ( lightCount > 0)
			{
				const U32 shader = bMSAAx2 ? lightCount * 2 - 1 : lightCount - 1;

				#if defined(PLATFORM_PS3)
				cellGcmSetDepthBounds(fNear, fFar);
				#endif

				Display::Device()->SetPixelShaderParameterArrayF(m_PositionParameter[shader], (F32*)&vLightPositions);
				Display::Device()->SetPixelShaderParameterArrayF(m_ColourParameter[shader], (F32*)&vLightColours);
				Display::Device()->SetPixelShaderParameterF(m_DepthParameter[shader], (F32*)&vDepthParameter);
				Display::Device()->SetPixelShaderParameters(&m_PixelShader[shader]);

				Display::Device()->SetPixelShader(&m_PixelShader[shader]);
 	   
			 	Display::Device()->DrawPrimitive(Display::E_PrimitiveType_TriangleStrip, (y * m_HorizontalTiles + x) * 4, 4);

			}
		}
	}

#if defined(PLATFORM_PS3)
	cellGcmSetDepthBoundsTestEnable(CELL_GCM_FALSE);
#endif

	Display::Device()->SetTexture(0, 0);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_Default);
	
	Display::Device()->SetTexture(1, 0);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_Default);

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthEnable(Display::E_Bool_Default);

	m_DynamicLights.Clear();
}

void C_DynamicLights::GatherVisibleLights(VisibleDynamicLights& visibleLights) const
{
	const Maths::VF32 vZero			= Maths::Const<Maths::VF32>::Zero();
	const Maths::VF32 vOne			= Maths::Const<Maths::VF32>::One();
	const Maths::VF32 vMinSize		= Maths::MakeVF32(1.0f / 480.0f);
	const Maths::VF32 vMaxDistance	= Maths::MakeVF32(500.0f);

	const Maths::Mat44 mView = Maths::Load<Maths::HMat44>(&Display::Device()->GetViewSettings3D().GetViewMatrix());
	const Maths::Mat44 mProj = Maths::Load<Maths::HMat44>(&Display::Device()->GetViewSettings3D().GetProjMatrix());
	for (S32 lightNo = 0; lightNo < m_DynamicLights.Size(); ++lightNo)
	{
		const Maths::Vec4F32	vPositionRadius		= Maths::Load<Maths::Vec4F32>((MathsStorage::Float4*) &m_DynamicLights[lightNo].m_vPosition);
		const Maths::Vec4F32	vColour				= Maths::Load<Maths::Vec4F32>((MathsStorage::Float3*) &m_DynamicLights[lightNo].m_vColour);

		const Maths::Vec4F32	vViewPosition		= Maths::MakeVec4F32(Maths::X(vPositionRadius), Maths::Y(vPositionRadius), Maths::Z(vPositionRadius), vOne);
		const Maths::VF32		fRadius				= Maths::W(vPositionRadius);
		const Maths::Vec4F32	vViewExtents0		= Maths::MakeVec4F32(-fRadius, -fRadius, -fRadius, vZero);
		const Maths::Vec4F32	vViewExtents1		= Maths::MakeVec4F32(-fRadius, -fRadius, +fRadius, vZero);
		const Maths::Vec4F32	vViewExtents2		= Maths::MakeVec4F32(+fRadius, +fRadius, -fRadius, vZero);
		const Maths::Vec4F32	vViewExtents3		= Maths::MakeVec4F32(+fRadius, +fRadius, +fRadius, vZero);

		const Maths::Vec4F32	vViewPosition0		= vViewPosition + vViewExtents0;
		const Maths::Vec4F32	vViewPosition1		= vViewPosition + vViewExtents1;
		const Maths::Vec4F32	vViewPosition2		= vViewPosition + vViewExtents2;
		const Maths::Vec4F32	vViewPosition3		= vViewPosition + vViewExtents3;

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
				
				F32 fDistance;
				Maths::Store(Maths::W(vProjPosition3), &fDistance);

				F32 fNear, fFar;
				Maths::Store(Maths::Z(vScreenMin), &fNear);
				Maths::Store(Maths::Z(vScreenMax), &fFar);

				C_VisibleDynamicLight visibleLight;
				visibleLight.m_rectangle	= Maths::C_Rect(vMin, vMax);
				visibleLight.m_sortKey		= (U32)(fDistance * (4294967295.0f / 5000.0f));
				visibleLight.m_near			= fNear;
				visibleLight.m_far			= fFar;
				visibleLight.m_pLight		= &m_DynamicLights[lightNo];
			
				visibleLights.PushBack(visibleLight);
			}
		}
	}

	Btl::RadixSorter<VisibleDynamicLights> radixSorter;
	radixSorter.Sort(visibleLights);
}

void C_DynamicLights::GatherVisibleParameters(const VisibleDynamicLights& visibleLights, const Maths::C_Rect& rectangle, U32& lightCount, Display::C_Float4* pPositionFalloffs, Display::C_Float4* pColours, F32& fNear, F32& fFar) const
{
	lightCount = 0;
	for (S32 i = 0; i < visibleLights.Size(); ++i)
	{
		if (Maths::Intersect(rectangle, visibleLights[i].m_rectangle))
		{
			const C_DynamicLight* const pLight = visibleLights[i].m_pLight;

			pPositionFalloffs[lightCount]	= MathsStorage::Float4(pLight->m_vPosition.x, pLight->m_vPosition.y, pLight->m_vPosition.z, 1.0f / (pLight->m_fRadius * pLight->m_fRadius));
			pColours[lightCount]			= MathsStorage::Float4(pLight->m_vColour.x, pLight->m_vColour.y, pLight->m_vColour.z, 1.0f);

			fNear							= Maths::Min(fNear, visibleLights[i].m_near);
			fFar							= Maths::Max(fFar, visibleLights[i].m_far);

			++lightCount;
		}
		if (c_MaximumLights <= lightCount)
			break;
	}
}

void C_DynamicLights::AddLight(const MathsStorage::Float3& vPosition, const F32 fRadius, const MathsStorage::Float3& vColour)
{
	m_DynamicLights.PushBack();

	m_DynamicLights.Back().m_vPosition = vPosition;
	m_DynamicLights.Back().m_fRadius = fRadius;
	m_DynamicLights.Back().m_vColour = vColour;
}

}; // namespace Deferred
