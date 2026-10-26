#include "stdafx.h"
#include "Graphics/Lighting/PointLights.h"

#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Core/Maths/HVec/New/IncludeMinimal.h"
#include "Core/Maths/HVec/New/IncludeMinimal.inl"
#include "Display/Buffer/VertexDeclarations.h"

EXTERN_SHADER(vsPointLights);
EXTERN_SHADER(psPointLights1);
EXTERN_SHADER(psPointLights2);
EXTERN_SHADER(psPointLights3);
EXTERN_SHADER(psPointLights4);
EXTERN_SHADER(psPointLights5);
EXTERN_SHADER(psPointLights6);
EXTERN_SHADER(psPointLights7);
EXTERN_SHADER(psPointLights8);
EXTERN_SHADER(psPointLights9);
EXTERN_SHADER(psPointLights10);
EXTERN_SHADER(psPointLights11);
EXTERN_SHADER(psPointLights12);

namespace Deferred
{
	extern Debug::Bool g_debugPrepassDepthBounds;
};

C_PointLights::C_PointLights() : Display::I_Renderer("DeferredPointLights")
{
	m_pNormalDepthTexture = 0;

	m_HorizontalTiles = 8;
	m_VerticalTiles = 4;
}

C_PointLights::~C_PointLights()
{
	DestroyRenderer();
}

bool C_PointLights::OnCreate()
{
	RESOURCE_MEMORY_ID("PointLights");

	if (!Display::CreateVertexShaderFromFunc("Shader/vsPointLights", 0, &m_VertexShader, REF_SHADER(vsPointLights)))
		return false;

	if (!Display::CreatePixelShaderFromFunc("Shader/psPointLights1", 0, &m_PixelShader[E_PointLightShader_1], REF_SHADER(psPointLights1)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psPointLights2", 0, &m_PixelShader[E_PointLightShader_2], REF_SHADER(psPointLights2)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psPointLights3", 0, &m_PixelShader[E_PointLightShader_3], REF_SHADER(psPointLights3)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psPointLights4", 0, &m_PixelShader[E_PointLightShader_4], REF_SHADER(psPointLights4)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psPointLights5", 0, &m_PixelShader[E_PointLightShader_5], REF_SHADER(psPointLights5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psPointLights6", 0, &m_PixelShader[E_PointLightShader_6], REF_SHADER(psPointLights6)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psPointLights7", 0, &m_PixelShader[E_PointLightShader_7], REF_SHADER(psPointLights7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psPointLights8", 0, &m_PixelShader[E_PointLightShader_8], REF_SHADER(psPointLights8)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psPointLights9", 0, &m_PixelShader[E_PointLightShader_9], REF_SHADER(psPointLights9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psPointLights10", 0, &m_PixelShader[E_PointLightShader_10], REF_SHADER(psPointLights10)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psPointLights11", 0, &m_PixelShader[E_PointLightShader_11], REF_SHADER(psPointLights11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psPointLights12", 0, &m_PixelShader[E_PointLightShader_12], REF_SHADER(psPointLights12)))
		return false;

	for (U32 shaderNo = 0; shaderNo < E_PointLightShader_Count; ++shaderNo)
	{
		m_PositionParameter[shaderNo]	= m_PixelShader[shaderNo].GetParameterArray(CP_RENDER_SETTINGS, "g_vPosition", 12);
		m_ColourParameter[shaderNo]		= m_PixelShader[shaderNo].GetParameterArray(CP_RENDER_SETTINGS_12, "g_vColour", 12);
	}

	return true;
}

bool C_PointLights::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("PointLights");

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

void C_PointLights::OnDestroy()
{
	for (U32 shaderNo = 0; shaderNo < E_PointLightShader_Count; ++shaderNo)
	{
		Display::Device()->Destroy(&m_PixelShader[shaderNo]);
	}
	Display::Device()->Destroy(&m_VertexShader);
}

void C_PointLights::OnDestroyVolatile()
{
	Display::Device()->Destroy(&m_VertexBuffer);
}

void C_PointLights::Render() const
{
	PERF_EVENT("RenderPointLights");

	m_VisibleLights.Clear();
	GatherVisibleLights(m_VisibleLights);

	Display::Device()->SetBlendMode(Display::E_BlendMode_Additive);

	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);

	Display::Device()->SetVertexShader(&m_VertexShader);

	Display::Device()->SetTexture(0, m_pNormalDepthTexture);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

	Display::Device()->SetVertexDeclaration(Display::Vertex_XY::m_pDeclaration);
	Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XY));

	const Maths::HMat44 mDeviceToCamera = Display::Device()->GetViewSettings3D().GetCameraToDevice().Inverse();

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

	vFar0 = vFar0 * vFar0.W().Rcp() * m_FarClip / Display::Device()->GetViewSettings3D().GetFarClip();
	vFar1 = vFar1 * vFar1.W().Rcp() * m_FarClip / Display::Device()->GetViewSettings3D().GetFarClip();
	vFar2 = vFar2 * vFar2.W().Rcp() * m_FarClip / Display::Device()->GetViewSettings3D().GetFarClip();
	vFar3 = vFar3 * vFar3.W().Rcp() * m_FarClip / Display::Device()->GetViewSettings3D().GetFarClip();

	Display::C_Float4 vsSettings[4];
	Maths::Store(vFar0, &vsSettings[0]);
	Maths::Store(vFar1, &vsSettings[1]);
	Maths::Store(vFar2, &vsSettings[2]);
	Maths::Store(vFar3, &vsSettings[3]);

	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS, (F32*)vsSettings, 4);

#if defined(PLATFORM_PC)
	const Display::C_RenderTarget& renderTarget = Display::Device()->GetRenderTarget();
	F32 fOffsetX = +1.0f / (F32) renderTarget.GetColourSurface()->GetWidth();
	F32 fOffsetY = +1.0f / (F32) renderTarget.GetColourSurface()->GetHeight();
#else
	F32 fOffsetX = 0.0f;
	F32 fOffsetY = 0.0f;
#endif

	Display::C_Float4 vsUVScale(0.5f, -0.5f, 0.5f + 0.5f * fOffsetX, 0.5f + 0.5f * fOffsetY);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_4, (F32*)&vsUVScale, 1);

	fOffsetX -= 1.0f;
	fOffsetY -= 1.0f;

#if defined(PLATFORM_PS3)
	if (Deferred::g_debugPrepassDepthBounds)
		cellGcmSetDepthBoundsTestEnable(CELL_GCM_TRUE);
#endif

	const F32 fDX = 2.0f / (F32) m_HorizontalTiles;
	const F32 fDY = 2.0f / (F32) m_VerticalTiles;

	Display::C_Float4 vLightPositions[c_MaximumLights];
	Display::C_Float4 vLightColours[c_MaximumLights];

	F32 fY = fOffsetY;
	for (U32 y = 0; y < m_VerticalTiles; ++y)
	{
		F32 fX = fOffsetX;
		for (U32 x = 0; x < m_HorizontalTiles; ++x)
		{
			const Maths::C_Rect rectangle(Maths::HVec2(fX, fY), Maths::HVec2(fX + fDX, fY + fDY));

			U32 lightCount = 0;
			F32 fNear = 1.0f, fFar = 0.0f;
			
			for (S32 i = 0; i < m_VisibleLights.Size(); ++i)
			{
				if (Maths::Intersect(rectangle, m_VisibleLights[i].m_rectangle))
				{
					const C_PointLight* const pLight = m_VisibleLights[i].m_pLight;

					vLightPositions[lightCount].x = pLight->m_vPosition.x;
					vLightPositions[lightCount].y = pLight->m_vPosition.y;
					vLightPositions[lightCount].z = pLight->m_vPosition.z;
					vLightPositions[lightCount].w = pLight->m_fRcpRadiusSq;
					vLightColours[lightCount].x	= pLight->m_vColour.x;
					vLightColours[lightCount].y	= pLight->m_vColour.y;
					vLightColours[lightCount].z	= pLight->m_vColour.z;
					vLightColours[lightCount].w	= 1.0f;
				
					fNear = Maths::Min(fNear, m_VisibleLights[i].m_near);
					fFar = Maths::Max(fFar, m_VisibleLights[i].m_far);
					++lightCount;
				}
				if (c_MaximumLights <= lightCount)
					break;
			}

			if (lightCount > 0)
			{
				const U32 shader = lightCount - 1;

#if defined(PLATFORM_PS3)
				cellGcmSetDepthBounds(fNear, fFar);
#endif

				Display::Device()->SetPixelShaderParameterArrayF(m_PositionParameter[shader], (F32*)&vLightPositions);
				Display::Device()->SetPixelShaderParameterArrayF(m_ColourParameter[shader], (F32*)&vLightColours);
				Display::Device()->SetPixelShaderParameters(&m_PixelShader[shader]);

				Display::Device()->SetPixelShader(&m_PixelShader[shader]);

				Display::Device()->DrawPrimitive(Display::E_PrimitiveType_TriangleStrip, (y * m_HorizontalTiles + x) * 4, 4);
			}

			fX += fDX;
		}

		fY += fDY;
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

	m_PointLights.Clear();
}

void C_PointLights::GatherVisibleLights(VisiblePointLights& visibleLights) const
{
	const Maths::VF32 vZero			= Maths::Const<Maths::VF32>::Zero();
	const Maths::VF32 vOne			= Maths::Const<Maths::VF32>::One();
	const Maths::VF32 vMinSize		= Maths::MakeVF32(1.0f / 480.0f);
	const Maths::VF32 vMaxDistance	= Maths::MakeVF32(500.0f);

	const Maths::Mat44 mView = Maths::Load<Maths::HMat44>(&Display::Device()->GetViewSettings3D().GetViewMatrix());
	const Maths::Mat44 mProj = Maths::Load<Maths::HMat44>(&Display::Device()->GetViewSettings3D().GetProjMatrix());
	for (S32 lightNo = 0; lightNo < m_PointLights.Size(); ++lightNo)
	{
		const Maths::Vec4F32	vPositionRadius		= Maths::Load<Maths::Vec4F32>((MathsStorage::Float4*) &m_PointLights[lightNo].m_vPosition);
		const Maths::Vec4F32	vColour				= Maths::Load<Maths::Vec4F32>((MathsStorage::Float3*) &m_PointLights[lightNo].m_vColour);

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

				C_VisiblePointLight visibleLight;
				visibleLight.m_rectangle	= Maths::C_Rect(vMin, vMax);
				visibleLight.m_sortKey		= (U32)(fDistance * (4294967295.0f / 5000.0f));
				visibleLight.m_near			= fNear;
				visibleLight.m_far			= fFar;
				visibleLight.m_pLight		= &m_PointLights[lightNo];

				visibleLights.PushBack(visibleLight);
			}
		}
	}

	m_RadixSorter.Sort(visibleLights);
}

void C_PointLights::GatherVisibleParameters(const VisiblePointLights& visibleLights, const Maths::C_Rect& rectangle, U32& lightCount, Display::C_Float4* pPositionFalloffs, Display::C_Float4* pColours, F32& fNear, F32& fFar) const
{
	
}

void C_PointLights::AddLight(const MathsStorage::Float3& vPosition, const F32 fRadius, const MathsStorage::Float3& vColour)
{
	m_PointLights.PushBack();

	m_PointLights.Back().m_vPosition = vPosition;
	m_PointLights.Back().m_fRadius = fRadius;
	m_PointLights.Back().m_vColour = vColour;
	m_PointLights.Back().m_fRcpRadiusSq = 1.0f / (fRadius * fRadius);
}
