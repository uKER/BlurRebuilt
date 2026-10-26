#include "stdafx.h"
#include "Deferred/PrePass/TiledVolumeSpotlights.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Btl/Containers/Radix.h"
#include "Core/Maths/HVec/New/IncludeMinimal.h"
#include "Core/Maths/HVec/New/IncludeMinimal.inl"
#include "Display/Buffer/VertexDeclarations.h"

EXTERN_SHADER(vsTiledVolumeSpotlightsPrepass);
EXTERN_SHADER(psTiledVolumeSpotlightsPrepass1);
EXTERN_SHADER(psTiledVolumeSpotlightsPrepass2);
EXTERN_SHADER(psTiledVolumeSpotlightsPrepass3);
EXTERN_SHADER(psTiledVolumeSpotlightsPrepass4);
EXTERN_SHADER(psTiledVolumeSpotlightsPrepass5);
EXTERN_SHADER(psTiledVolumeSpotlightsPrepass6);
EXTERN_SHADER(psTiledVolumeSpotlightsPrepass7);
EXTERN_SHADER(psTiledVolumeSpotlightsPrepass8);
EXTERN_SHADER(psTiledVolumeSpotlightsPrepass9);
EXTERN_SHADER(psTiledVolumeSpotlightsPrepass10);
EXTERN_SHADER(psTiledVolumeSpotlightsPrepass11);
EXTERN_SHADER(psTiledVolumeSpotlightsPrepass12);

namespace Deferred
{

C_TiledVolumeSpotlights::C_TiledVolumeSpotlights() : Display::I_Renderer("DeferredTiledVolumeSpotlights")
{
	m_pDepthTexture = 0;
	m_pNormalTexture = 0;

	m_HorizontalTiles = 8;
	m_VerticalTiles = 4;
}

C_TiledVolumeSpotlights::~C_TiledVolumeSpotlights()
{
	DestroyRenderer();
}

bool C_TiledVolumeSpotlights::OnCreate()
{
	RESOURCE_MEMORY_ID("TiledVolumeSpotlights");

	if (!Display::CreateVertexShaderFromFunc("Shader/vsTiledVolumeSpotlightsPrepass", 0, &m_VertexShader, REF_SHADER(vsTiledVolumeSpotlightsPrepass)))
		return false;

	if (!Display::CreatePixelShaderFromFunc("Shader/psTiledVolumeSpotlightsPrepass1", 0, &m_PixelShader[E_TiledVolumeSpotlightsShader_1], REF_SHADER(psTiledVolumeSpotlightsPrepass1)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psTiledVolumeSpotlightsPrepass2", 0, &m_PixelShader[E_TiledVolumeSpotlightsShader_2], REF_SHADER(psTiledVolumeSpotlightsPrepass2)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psTiledVolumeSpotlightsPrepass3", 0, &m_PixelShader[E_TiledVolumeSpotlightsShader_3], REF_SHADER(psTiledVolumeSpotlightsPrepass3)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psTiledVolumeSpotlightsPrepass4", 0, &m_PixelShader[E_TiledVolumeSpotlightsShader_4], REF_SHADER(psTiledVolumeSpotlightsPrepass4)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psTiledVolumeSpotlightsPrepass5", 0, &m_PixelShader[E_TiledVolumeSpotlightsShader_5], REF_SHADER(psTiledVolumeSpotlightsPrepass5)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psTiledVolumeSpotlightsPrepass6", 0, &m_PixelShader[E_TiledVolumeSpotlightsShader_6], REF_SHADER(psTiledVolumeSpotlightsPrepass6)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psTiledVolumeSpotlightsPrepass7", 0, &m_PixelShader[E_TiledVolumeSpotlightsShader_7], REF_SHADER(psTiledVolumeSpotlightsPrepass7)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psTiledVolumeSpotlightsPrepass8", 0, &m_PixelShader[E_TiledVolumeSpotlightsShader_8], REF_SHADER(psTiledVolumeSpotlightsPrepass8)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psTiledVolumeSpotlightsPrepass9", 0, &m_PixelShader[E_TiledVolumeSpotlightsShader_9], REF_SHADER(psTiledVolumeSpotlightsPrepass9)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psTiledVolumeSpotlightsPrepass10", 0, &m_PixelShader[E_TiledVolumeSpotlightsShader_10], REF_SHADER(psTiledVolumeSpotlightsPrepass10)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psTiledVolumeSpotlightsPrepass11", 0, &m_PixelShader[E_TiledVolumeSpotlightsShader_11], REF_SHADER(psTiledVolumeSpotlightsPrepass11)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psTiledVolumeSpotlightsPrepass12", 0, &m_PixelShader[E_TiledVolumeSpotlightsShader_12], REF_SHADER(psTiledVolumeSpotlightsPrepass12)))
		return false;

	for (U32 shaderNo = 0; shaderNo < E_TiledVolumeSpotlightsShader_Count; ++shaderNo)
	{
		m_PositionParameter[shaderNo]	= m_PixelShader[shaderNo].GetParameterArray(0, "g_vPosition", 12);
		m_ColourParameter[shaderNo]		= m_PixelShader[shaderNo].GetParameterArray(12, "g_vColour", 12);
		m_MatrixParameter[shaderNo]		= m_PixelShader[shaderNo].GetParameterArray(24, "g_mLight", 48);
		m_DepthParameter[shaderNo]		= m_PixelShader[shaderNo].GetParameter(72, "g_vDepth");
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

void C_TiledVolumeSpotlights::OnDestroy()
{
	for (U32 shaderNo = 0; shaderNo < E_TiledVolumeSpotlightsShader_Count; ++shaderNo)
		Display::Device()->Destroy(&m_PixelShader[shaderNo]);

	Display::Device()->Destroy(&m_VertexShader);
	Display::Device()->Destroy(&m_VertexBuffer);
}

void C_TiledVolumeSpotlights::Render() const
{
	PERF_EVENT("RenderTiledVolumeSpotlights");

	VisibleTiledVolumeSpotlights visibleLights;
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

	Display::Device()->SetTexture(2, m_pVolumeTexture);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureGamma(2, Display::E_Bool_False);

	Display::Device()->SetVertexDeclaration(Display::Vertex_XY::m_pDeclaration);
	Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XY));

	const Maths::HMat44 mDeviceToWorld = Display::Device()->GetViewSettings3D().GetWorldToDevice().Inverse();

	MathsStorage::Float4 vDepthParameter = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	vDepthParameter.y = vDepthParameter.y / Display::Device()->GetViewSettings3D().GetFarClip();

	const Maths::HVec4	vCameraPosition	= Maths::HVec4(Display::Device()->GetViewSettings3D().GetCameraPosition());
	Maths::HVec4 vFar0;
	Maths::HVec4 vFar1;
	Maths::HVec4 vFar2;
	Maths::HVec4 vFar3;
	if (Display::Device()->GetDepthFlip())
	{
		vFar0 = Maths::HVec4(-1.0f, -1.0f, 0.0f, 1.0f) * mDeviceToWorld;
		vFar1 = Maths::HVec4(-1.0f, +1.0f, 0.0f, 1.0f) * mDeviceToWorld;
		vFar2 = Maths::HVec4(+1.0f, +1.0f, 0.0f, 1.0f) * mDeviceToWorld;
		vFar3 = Maths::HVec4(+1.0f, -1.0f, 0.0f, 1.0f) * mDeviceToWorld;
	}
	else
	{
		vFar0 = Maths::HVec4(-1.0f, -1.0f, 1.0f, 1.0f) * mDeviceToWorld;
		vFar1 = Maths::HVec4(-1.0f, +1.0f, 1.0f, 1.0f) * mDeviceToWorld;
		vFar2 = Maths::HVec4(+1.0f, +1.0f, 1.0f, 1.0f) * mDeviceToWorld;
		vFar3 = Maths::HVec4(+1.0f, -1.0f, 1.0f, 1.0f) * mDeviceToWorld;
	}

	vFar0 = vFar0 * vFar0.W().Rcp() - vCameraPosition;
	vFar1 = vFar1 * vFar1.W().Rcp() - vCameraPosition;
	vFar2 = vFar2 * vFar2.W().Rcp() - vCameraPosition;
	vFar3 = vFar3 * vFar3.W().Rcp() - vCameraPosition;
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
	const F32 fOffsetX = -1.0f / (F32) renderTarget.GetColourSurface()->GetWidth();
	const F32 fOffsetY = +1.0f / (F32) renderTarget.GetColourSurface()->GetHeight();
#else
	const F32 fOffsetX = 0.0f;
	const F32 fOffsetY = 0.0f;
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
			MathsStorage::MatrixRow44 mLightMatrices[c_MaximumLights];
			
			GatherVisibleParameters(visibleLights, rectangle, lightCount, vLightPositions, vLightColours, mLightMatrices);

			if (lightCount > 0)
			{
				const U32 shader = lightCount - 1;

				Display::Device()->SetPixelShaderParameterArrayF(m_PositionParameter[shader], (F32*)&vLightPositions);
				Display::Device()->SetPixelShaderParameterArrayF(m_ColourParameter[shader], (F32*)&vLightColours);
				Display::Device()->SetPixelShaderParameterArrayF(m_MatrixParameter[shader], (F32*)&mLightMatrices);
				Display::Device()->SetPixelShaderParameterF(m_DepthParameter[shader], (F32*)&vDepthParameter);
				Display::Device()->SetPixelShaderParameters(&m_PixelShader[shader]);

				Display::Device()->SetPixelShader(&m_PixelShader[shader]);

				Display::Device()->DrawPrimitive(Display::E_PrimitiveType_TriangleStrip, (y * m_HorizontalTiles + x) * 4, 4);
			}
		}
	}

	Display::Device()->SetTexture(0, 0);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_Default);
	
	Display::Device()->SetTexture(1, 0);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_Default);

	Display::Device()->SetTexture(2, 0);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(2, Display::E_Bool_Default);

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthEnable(Display::E_Bool_Default);

	m_TiledVolumeSpotlights.Clear();
}

void C_TiledVolumeSpotlights::GatherVisibleLights(VisibleTiledVolumeSpotlights& visibleLights) const
{
	const Maths::VF32 vZero			= Maths::Const<Maths::VF32>::Zero();
	const Maths::VF32 vOne			= Maths::Const<Maths::VF32>::One();
	const Maths::VF32 vMinSize		= Maths::MakeVF32(1.0f / 480.0f);
	const Maths::VF32 vMaxDistance	= Maths::MakeVF32(500.0f);

	const Maths::Mat44 mViewProj = Maths::Load<Maths::HMat44>(&Display::Device()->GetViewSettings3D().GetViewProjMatrix());
	for (S32 lightNo = 0; lightNo < m_TiledVolumeSpotlights.Size(); ++lightNo)
	{
		const Maths::Mat44		mWorld			= Maths::Load<Maths::Mat44>((MathsStorage::MatrixRow44*)&m_TiledVolumeSpotlights[lightNo].m_mMatrix);
		const Maths::Mat44		mWorldViewProj	= mWorld * mViewProj;

		Maths::Vec4F32	v000	= Maths::MakeVec4F32(0.0f, 0.0f, 0.0f, 1.0f) * mWorldViewProj;
		Maths::Vec4F32	v001	= Maths::MakeVec4F32(0.0f, 0.0f, 1.0f, 1.0f) * mWorldViewProj;
		Maths::Vec4F32	v010	= Maths::MakeVec4F32(0.0f, 1.0f, 0.0f, 1.0f) * mWorldViewProj;
		Maths::Vec4F32	v011	= Maths::MakeVec4F32(0.0f, 1.0f, 1.0f, 1.0f) * mWorldViewProj;
		Maths::Vec4F32	v100	= Maths::MakeVec4F32(1.0f, 0.0f, 0.0f, 1.0f) * mWorldViewProj;
		Maths::Vec4F32	v101	= Maths::MakeVec4F32(1.0f, 0.0f, 1.0f, 1.0f) * mWorldViewProj;
		Maths::Vec4F32	v110	= Maths::MakeVec4F32(1.0f, 1.0f, 0.0f, 1.0f) * mWorldViewProj;
		Maths::Vec4F32	v111	= Maths::MakeVec4F32(1.0f, 1.0f, 1.0f, 1.0f) * mWorldViewProj;

		Maths::VF32		vW000	= Maths::W(v000);
		Maths::VF32		vW001	= Maths::W(v001);
		Maths::VF32		vW010	= Maths::W(v010);
		Maths::VF32		vW011	= Maths::W(v011);
		Maths::VF32		vW100	= Maths::W(v100);
		Maths::VF32		vW101	= Maths::W(v101);
		Maths::VF32		vW110	= Maths::W(v110);
		Maths::VF32		vW111	= Maths::W(v111);

		Maths::VF32 vMaxW = vW000;
		vMaxW = Maths::Max(vMaxW, vW001);
		vMaxW = Maths::Max(vMaxW, vW010);
		vMaxW = Maths::Max(vMaxW, vW011);
		vMaxW = Maths::Max(vMaxW, vW100);
		vMaxW = Maths::Max(vMaxW, vW101);
		vMaxW = Maths::Max(vMaxW, vW110);
		vMaxW = Maths::Max(vMaxW, vW111);

		if (vZero < vMaxW && vMaxW < vMaxDistance)
		{
			Maths::Vec4F32	vS000	= Maths::Sign(v000);
			Maths::Vec4F32	vS001	= Maths::Sign(v001);
			Maths::Vec4F32	vS010	= Maths::Sign(v010);
			Maths::Vec4F32	vS011	= Maths::Sign(v011);
			Maths::Vec4F32	vS100	= Maths::Sign(v100);
			Maths::Vec4F32	vS101	= Maths::Sign(v101);
			Maths::Vec4F32	vS110	= Maths::Sign(v110);
			Maths::Vec4F32	vS111	= Maths::Sign(v111);

			Maths::VF32		vZero	= Maths::Const<Maths::VF32>::Zero();
			v000 = Maths::Select(vW000 < vZero, vS000, v000 * Maths::Rcp(vW000));
			v001 = Maths::Select(vW001 < vZero, vS001, v001 * Maths::Rcp(vW001));
			v010 = Maths::Select(vW010 < vZero, vS010, v010 * Maths::Rcp(vW010));
			v011 = Maths::Select(vW011 < vZero, vS011, v011 * Maths::Rcp(vW011));
			v100 = Maths::Select(vW100 < vZero, vS100, v100 * Maths::Rcp(vW100));
			v101 = Maths::Select(vW101 < vZero, vS101, v101 * Maths::Rcp(vW101));
			v110 = Maths::Select(vW110 < vZero, vS110, v110 * Maths::Rcp(vW110));
			v111 = Maths::Select(vW111 < vZero, vS111, v111 * Maths::Rcp(vW111));

			Maths::Vec4F32	vMin	= Maths::Min(v000, Maths::Min(v001, Maths::Min(v010, Maths::Min(v011, Maths::Min(v100, Maths::Min(v101, Maths::Min(v110, v111)))))));
			Maths::Vec4F32	vMax	= Maths::Max(v000, Maths::Max(v001, Maths::Max(v010, Maths::Max(v011, Maths::Max(v100, Maths::Max(v101, Maths::Max(v110, v111)))))));

			const Maths::HVec2 vRectMin(Maths::X(vMin), Maths::Y(vMin));
			const Maths::HVec2 vRectMax(Maths::X(vMax), Maths::Y(vMax));

			F32 fDistance;
			Maths::Store(vMaxW, &fDistance);

			C_VisibleTiledVolumeSpotlight visibleLight;
			visibleLight.m_rectangle	= Maths::C_Rect(vRectMin, vRectMax);
			visibleLight.m_sortKey		= (U32)(fDistance * (4294967295.0f / 5000.0f));
			visibleLight.m_pLight		= &m_TiledVolumeSpotlights[lightNo];
		
			visibleLights.PushBack(visibleLight);
		}
	}
	
	Btl::RadixSorter<VisibleTiledVolumeSpotlights> radixSorter;
	radixSorter.Sort(visibleLights);
}

void C_TiledVolumeSpotlights::GatherVisibleParameters(const VisibleTiledVolumeSpotlights& visibleLights, const Maths::C_Rect& rectangle, U32& lightCount, Display::C_Float4* pPositions, Display::C_Float4* pColours, MathsStorage::MatrixRow44* pMatrices) const
{
	lightCount = 0;
	for (S32 i = 0; i < visibleLights.Size(); ++i)
	{
		if (Maths::Intersect(rectangle, visibleLights[i].m_rectangle))
		{
			const C_TiledVolumeSpotlight* const pLight = visibleLights[i].m_pLight;

			pPositions[lightCount]	= Display::C_Float4(pLight->m_vPosition.x, pLight->m_vPosition.y, pLight->m_vPosition.z, 1.0f);
			pColours[lightCount]	= Display::C_Float4(pLight->m_vColour.x, pLight->m_vColour.y, pLight->m_vColour.z, 1.0f);

			const Maths::Mat44 mWorld = Maths::Load<Maths::Mat44>(&pLight->m_mMatrix);
			Maths::Store(Maths::Inverse(mWorld), &pMatrices[lightCount]);

			++lightCount;
		}
		if (c_MaximumLights <= lightCount)
			break;
	}
}

void C_TiledVolumeSpotlights::AddLight(const MathsStorage::Float3& vPosition, const MathsStorage::Float3& vColour, const MathsStorage::MatrixRow44& mMatrix)
{
	m_TiledVolumeSpotlights.PushBack();

	m_TiledVolumeSpotlights.Back().m_vPosition = vPosition;
	m_TiledVolumeSpotlights.Back().m_vColour = vColour;
	m_TiledVolumeSpotlights.Back().m_mMatrix = mMatrix;
}

}; // namespace Deferred
