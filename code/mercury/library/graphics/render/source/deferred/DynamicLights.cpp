#include "stdafx.h"
#include "Render/Deferred/DynamicLights.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Core/Maths/HVec/New/IncludeMinimal.h"
#include "Core/Maths/HVec/New/IncludeMinimal.inl"
#include "System/Debug/I_DebugRender.h"
#include "Display/Buffer/VertexDeclarations.h"

Debug::Bool g_UseAddSmoothBlendMode("Graphics.Render.Deferred.UseAddSmoothBlendMode", true);

EXTERN_SHADER(vsDynamicLights);
EXTERN_SHADER(psDynamicLights1);
EXTERN_SHADER(psDynamicLights2);
EXTERN_SHADER(psDynamicLights3);
EXTERN_SHADER(psDynamicLights4);
EXTERN_SHADER(psDynamicLights5);
EXTERN_SHADER(psDynamicLights6);
EXTERN_SHADER(psDynamicLights7);
EXTERN_SHADER(psDynamicLights8);
EXTERN_SHADER(psDynamicLights1HDR);
EXTERN_SHADER(psDynamicLights2HDR);
EXTERN_SHADER(psDynamicLights3HDR);
EXTERN_SHADER(psDynamicLights4HDR);
EXTERN_SHADER(psDynamicLights5HDR);
EXTERN_SHADER(psDynamicLights6HDR);
EXTERN_SHADER(psDynamicLights7HDR);
EXTERN_SHADER(psDynamicLights8HDR);
EXTERN_SHADER(psDynamicLights1HDRU32);
EXTERN_SHADER(psDynamicLights2HDRU32);
EXTERN_SHADER(psDynamicLights3HDRU32);
EXTERN_SHADER(psDynamicLights4HDRU32);
EXTERN_SHADER(psDynamicLights5HDRU32);
EXTERN_SHADER(psDynamicLights6HDRU32);
EXTERN_SHADER(psDynamicLights7HDRU32);
EXTERN_SHADER(psDynamicLights8HDRU32);
EXTERN_SHADER(vsDynamicVolumeSpotlight);
EXTERN_SHADER(psDynamicVolumeSpotlight);
EXTERN_SHADER(psDynamicVolumeSpotlightHDR);
EXTERN_SHADER(psDynamicVolumeSpotlightHDRU32);

Debug::Bool gDebugDynamicLights("Graphics.Render.Deferred.ShowDynamicLights", false);
Debug::Bool gDebugDynamicLightsTiles("Graphics.Render.Deferred.ShowDynamicLightTiles", false);

namespace Render
{
namespace Deferred
{

const F32 c_MinDynamicLightSize = 0.015f;

C_DynamicLights::C_DynamicLights() : Display::I_Renderer("DeferredDynamicLights")
{
	m_pDepthTexture = 0;
	m_pNormalTexture = 0;

	m_HorizontalTiles = 8;
	m_VerticalTiles = 4;

	m_FirstFreeDynamicLightSlot = 0;
}

C_DynamicLights::~C_DynamicLights()
{
	DestroyRenderer();
}

bool C_DynamicLights::OnCreate()
{
	RESOURCE_MEMORY_ID("DynamicLights");

	if (!Display::CreateVertexShaderFromFunc("Shader/vsDynamicLights", 0, &m_VertexShader, REF_SHADER(vsDynamicLights)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("Shader/vsDynamicVolumeSpotlight", 0, &m_VolumeSpotlightVertexShader, REF_SHADER(vsDynamicVolumeSpotlight)))
		return false;

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		if (eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights1HDRU32", 0, &m_PixelShader[E_DynamicLightShader_1], REF_SHADER(psDynamicLights1HDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights2HDRU32", 0, &m_PixelShader[E_DynamicLightShader_2], REF_SHADER(psDynamicLights2HDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights3HDRU32", 0, &m_PixelShader[E_DynamicLightShader_3], REF_SHADER(psDynamicLights3HDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights4HDRU32", 0, &m_PixelShader[E_DynamicLightShader_4], REF_SHADER(psDynamicLights4HDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights5HDRU32", 0, &m_PixelShader[E_DynamicLightShader_5], REF_SHADER(psDynamicLights5HDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights6HDRU32", 0, &m_PixelShader[E_DynamicLightShader_6], REF_SHADER(psDynamicLights6HDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights7HDRU32", 0, &m_PixelShader[E_DynamicLightShader_7], REF_SHADER(psDynamicLights7HDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights8HDRU32", 0, &m_PixelShader[E_DynamicLightShader_8], REF_SHADER(psDynamicLights8HDRU32)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicVolumeSpotlightHDRU32", 0, &m_VolumeSpotlightPixelShader, REF_SHADER(psDynamicVolumeSpotlightHDRU32)))
				return false;
		}
		else if (eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights1HDR", 0, &m_PixelShader[E_DynamicLightShader_1], REF_SHADER(psDynamicLights1HDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights2HDR", 0, &m_PixelShader[E_DynamicLightShader_2], REF_SHADER(psDynamicLights2HDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights3HDR", 0, &m_PixelShader[E_DynamicLightShader_3], REF_SHADER(psDynamicLights3HDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights4HDR", 0, &m_PixelShader[E_DynamicLightShader_4], REF_SHADER(psDynamicLights4HDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights5HDR", 0, &m_PixelShader[E_DynamicLightShader_5], REF_SHADER(psDynamicLights5HDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights6HDR", 0, &m_PixelShader[E_DynamicLightShader_6], REF_SHADER(psDynamicLights6HDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights7HDR", 0, &m_PixelShader[E_DynamicLightShader_7], REF_SHADER(psDynamicLights7HDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights8HDR", 0, &m_PixelShader[E_DynamicLightShader_8], REF_SHADER(psDynamicLights8HDR)))
				return false;
			if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicVolumeSpotlightHDR", 0, &m_VolumeSpotlightPixelShader, REF_SHADER(psDynamicVolumeSpotlightHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights1", 0, &m_PixelShader[E_DynamicLightShader_1], REF_SHADER(psDynamicLights1)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights2", 0, &m_PixelShader[E_DynamicLightShader_2], REF_SHADER(psDynamicLights2)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights3", 0, &m_PixelShader[E_DynamicLightShader_3], REF_SHADER(psDynamicLights3)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights4", 0, &m_PixelShader[E_DynamicLightShader_4], REF_SHADER(psDynamicLights4)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights5", 0, &m_PixelShader[E_DynamicLightShader_5], REF_SHADER(psDynamicLights5)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights6", 0, &m_PixelShader[E_DynamicLightShader_6], REF_SHADER(psDynamicLights6)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights7", 0, &m_PixelShader[E_DynamicLightShader_7], REF_SHADER(psDynamicLights7)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicLights8", 0, &m_PixelShader[E_DynamicLightShader_8], REF_SHADER(psDynamicLights8)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDynamicVolumeSpotlight", 0, &m_VolumeSpotlightPixelShader, REF_SHADER(psDynamicVolumeSpotlight)))
			return false;
	}

	for (U32 shaderNo = 0; shaderNo < E_DynamicLightShader_Count; ++shaderNo)
	{
		m_PositionParameter[shaderNo]	= m_PixelShader[shaderNo].GetParameterArray(CP_RENDER_SETTINGS, "g_vPosition", 8);
		m_ColourParameter[shaderNo]		= m_PixelShader[shaderNo].GetParameterArray(CP_RENDER_SETTINGS_8, "g_vColour", 8);
		m_MatrixParameter[shaderNo]		= m_PixelShader[shaderNo].GetParameterArray(CP_RENDER_SETTINGS_16, "g_mToWorld", 4);
		m_DepthParameter[shaderNo]		= m_PixelShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_20, "g_vDepth");
		m_LuminanceParameter[shaderNo]	= m_PixelShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE,	"g_vLuminance");
	}

	m_VolumeSpotlightMatrixParameter	= m_VolumeSpotlightPixelShader.GetParameterArray(CP_RENDER_SETTINGS, "g_mLight", 4);
	m_VolumeSpotlightToWorldParameter	= m_VolumeSpotlightPixelShader.GetParameterArray(CP_RENDER_SETTINGS_4, "g_mToWorld", 4);
	m_VolumeSpotlightDirectionParameter	= m_VolumeSpotlightPixelShader.GetParameter(CP_RENDER_SETTINGS_8, "g_vDirection");
	m_VolumeSpotlightColourParameter	= m_VolumeSpotlightPixelShader.GetParameter(CP_RENDER_SETTINGS_9, "g_vColour");
	m_VolumeSpotlightDepthParameter		= m_VolumeSpotlightPixelShader.GetParameter(CP_RENDER_SETTINGS_10, "g_vDepth");
	m_VolumeSpotlightLuminanceParameter	= m_VolumeSpotlightPixelShader.GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");

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

bool C_DynamicLights::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("DynamicLights");

	const Display::C_RenderTarget& renderTarget = Display::Device()->GetRenderTarget();
	
	const U32 iWidth = renderTarget.GetColourSurface()->GetWidth();
	const U32 iHeight = renderTarget.GetColourSurface()->GetHeight();
	const Display::E_Format eFormat = Display::GetFormatFromSurfaceFormat(renderTarget.GetColourSurface()->GetFormat());

	if (!Display::Device()->CreateRenderTexture2D(iWidth, iHeight, 1, 0, eFormat, Display::E_Multisample_None, 0, &m_RenderTexture))
		return false;
	
	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_RenderTexture, "_DynamicLightsRT");
	#endif

	return true;
}

void C_DynamicLights::OnDestroy()
{
	for (U32 shaderNo = 0; shaderNo < E_DynamicLightShader_Count; ++shaderNo)
	{
		Display::Device()->Destroy(&m_PixelShader[shaderNo]);
	}
	Display::Device()->Destroy(&m_VertexShader);

	Display::Device()->Destroy(&m_VolumeSpotlightVertexShader);
	Display::Device()->Destroy(&m_VolumeSpotlightPixelShader);

	Display::Device()->Destroy(&m_VertexBuffer);
}

void C_DynamicLights::OnDestroyVolatile()
{
	Display::Device()->Destroy(&m_RenderTexture);
}

void C_DynamicLights::Render() const
{
	PERF_EVENT("RenderDynamicLights");

	VisibleDynamicLights visibleLights;
	GatherVisibleLights(visibleLights);
	
	Display::Device()->SetBlendMode(Display::E_BlendMode_Additive);

	if(g_UseAddSmoothBlendMode)
	{
		Display::Device()->SetBlendMode(Display::E_BlendMode_AdditiveSmooth);
	}

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

	Display::C_Float4 vLuminance = Render::C_RenderState::Instance().GetGainShaderParameters();
	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		vLuminance = Render::C_RenderState::Instance().HDR()->GetShaderParameters();

		Display::Device()->SetTexture(15, Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture());
		Display::Device()->SetTextureFilter(15, Display::E_TextureFilter_Point);
		Display::Device()->SetTextureAddress(15, Display::E_TextureAddress_Clamp);
	}

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
	const Maths::HMat44 mDeviceToWorld = Display::Device()->GetViewSettings3D().GetWorldToDevice().Inverse();

	Display::C_Matrix mMatrixParameter;
	Maths::Store(mTexture * mDeviceToWorld, &mMatrixParameter);

	MathsStorage::Float4 vDepthParameter = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();

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
			
			U32 firstLight = 0;
			U32 renderCount = 0;
			bool endProcess = false;

			do 
			{
				endProcess = GatherVisibleParameters(visibleLights, rectangle, firstLight, lightCount, vLightPositions, vLightColours);

				if (lightCount > 0)
				{
					const U32 shader = lightCount - 1;

					Display::Device()->SetPixelShaderParameterArrayF(m_PositionParameter[shader], (F32*)&vLightPositions);
					Display::Device()->SetPixelShaderParameterArrayF(m_ColourParameter[shader], (F32*)&vLightColours);
					Display::Device()->SetPixelShaderParameterArrayF(m_MatrixParameter[shader], (F32*)&mMatrixParameter);
					Display::Device()->SetPixelShaderParameterF(m_DepthParameter[shader], (F32*)&vDepthParameter);
					Display::Device()->SetPixelShaderParameterF(m_LuminanceParameter[shader], (F32*)&vLuminance);
					Display::Device()->SetPixelShaderParameters(&m_PixelShader[shader]);

					Display::Device()->SetPixelShader(&m_PixelShader[shader]);

					Display::Device()->DrawPrimitive(Display::E_PrimitiveType_TriangleStrip, (y * m_HorizontalTiles + x) * 4, 4);

					renderCount++;
				}
			} while (!endProcess);

			if (gDebugDynamicLightsTiles)
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
}

void C_DynamicLights::RenderSpotlightVolume(const MathsStorage::MatrixRow44& mWorld, const MathsStorage::Float4& vDirection, const MathsStorage::Float4& vColour, const Display::C_Texture3D* pTexture) const
{
	PERF_EVENT("RenderSpotlightVolume");

	Display::Device()->SetBlendMode(Display::E_BlendMode_Additive);

	if(g_UseAddSmoothBlendMode)
	{
		Display::Device()->SetBlendMode(Display::E_BlendMode_AdditiveSmooth);
	}

	Display::Device()->SetCullMode(Display::E_CullMode_Ccw);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

	Display::Device()->SetTexture(0, m_pDepthTexture);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_False);
	
	Display::Device()->SetTexture(1, m_pNormalTexture);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_False);
	
	Display::Device()->SetTexture(2, pTexture);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Border);
	Display::Device()->SetTextureBorderColour(2, 0x00000000);
	Display::Device()->SetTextureGamma(2, Display::E_Bool_False);
	
	Display::Device()->SetVertexShader(&m_VolumeSpotlightVertexShader);
	Display::Device()->SetPixelShader(&m_VolumeSpotlightPixelShader);

	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&mWorld, 4);

	Maths::Mat44 mInvWorld = Maths::Inverse(Maths::Load<Maths::Mat44>(&mWorld));
	MathsStorage::MatrixRow44 mLight;
	Maths::Store(mInvWorld, &mLight);

	Display::Device()->SetPixelShaderParameterArrayF(m_VolumeSpotlightMatrixParameter, (F32*)&mLight);

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
	const Maths::HMat44 mDeviceToWorld = Display::Device()->GetViewSettings3D().GetWorldToDevice().Inverse();

	Display::C_Matrix mToWorld;
	Maths::Store(mTexture * mDeviceToWorld, &mToWorld);

	Display::Device()->SetPixelShaderParameterArrayF(m_VolumeSpotlightToWorldParameter, (F32*)&mToWorld);

	Display::Device()->SetPixelShaderParameterF(m_VolumeSpotlightDirectionParameter, (F32*)&vDirection);
	Display::Device()->SetPixelShaderParameterF(m_VolumeSpotlightColourParameter, (F32*)&vColour);

	MathsStorage::Float4 vDepth = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	Display::Device()->SetPixelShaderParameterF(m_VolumeSpotlightDepthParameter, (F32*)&vDepth);

	MathsStorage::Float4 vLuminance = Render::C_RenderState::Instance().GetGainShaderParameters();
	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		vLuminance = Render::C_RenderState::Instance().HDR()->GetShaderParameters();

		Display::Device()->SetTexture(15, Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture());
		Display::Device()->SetTextureFilter(15, Display::E_TextureFilter_Point);
		Display::Device()->SetTextureAddress(15, Display::E_TextureAddress_Clamp);
	}
	Display::Device()->SetPixelShaderParameterF(m_VolumeSpotlightLuminanceParameter, (F32*)&vLuminance);

	Display::Device()->SetPixelShaderParameters(&m_VolumeSpotlightPixelShader);

	Display::Vertex_XYZ vertex[8] = 
	{
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f },
	};

	Display::Vertex_XYZ* pVertex = (Display::Vertex_XYZ*)Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleList, 36, sizeof(Display::Vertex_XYZ), Display::Vertex_XYZ::m_pDeclaration);
	if (pVertex)
	{
		// top
		*pVertex++ = vertex[2]; *pVertex++ = vertex[3]; *pVertex++ = vertex[7];
		*pVertex++ = vertex[2]; *pVertex++ = vertex[7]; *pVertex++ = vertex[6];
		// bottom
		*pVertex++ = vertex[0]; *pVertex++ = vertex[5]; *pVertex++ = vertex[1];
		*pVertex++ = vertex[0]; *pVertex++ = vertex[4]; *pVertex++ = vertex[5];
		// left
		*pVertex++ = vertex[1]; *pVertex++ = vertex[3]; *pVertex++ = vertex[2];
		*pVertex++ = vertex[1]; *pVertex++ = vertex[2]; *pVertex++ = vertex[0];
		// right
		*pVertex++ = vertex[4]; *pVertex++ = vertex[6]; *pVertex++ = vertex[5];
		*pVertex++ = vertex[5]; *pVertex++ = vertex[6]; *pVertex++ = vertex[7];
		// front
		*pVertex++ = vertex[2]; *pVertex++ = vertex[6]; *pVertex++ = vertex[0];
		*pVertex++ = vertex[0]; *pVertex++ = vertex[6]; *pVertex++ = vertex[4];
		// back
		*pVertex++ = vertex[1]; *pVertex++ = vertex[7]; *pVertex++ = vertex[3];
		*pVertex++ = vertex[1]; *pVertex++ = vertex[5]; *pVertex++ = vertex[7];

		Display::Device()->EndVertices();
	}

	for (U32 samplerNo = 0; samplerNo <= 2; ++samplerNo)
	{
		Display::Device()->SetTexture(samplerNo, 0);
		Display::Device()->SetTextureAddress(samplerNo, Display::E_TextureAddress_Default);
		Display::Device()->SetTextureFilter(samplerNo, Display::E_TextureFilter_Default);
		Display::Device()->SetTextureGamma(samplerNo, Display::E_Bool_Default);
	}

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthEnable(Display::E_Bool_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_Default);
}

void C_DynamicLights::GatherVisibleLights(VisibleDynamicLights& visibleLights) const
{
	const Maths::VF32 vZero			= Maths::Const<Maths::VF32>::Zero();
	const Maths::VF32 vOne			= Maths::Const<Maths::VF32>::One();
	const Maths::VF32 vMinSize		= Maths::MakeVF32(1.0f / 480.0f);

	const Maths::Mat44 mView = Maths::Load<Maths::HMat44>(&Display::Device()->GetViewSettings3D().GetViewMatrix());
	const Maths::Mat44 mProj = Maths::Load<Maths::HMat44>(&Display::Device()->GetViewSettings3D().GetProjMatrix());
	for (S32 lightNo = 0; lightNo < m_DynamicLights.Size(); ++lightNo)
	{
		if (m_DynamicLights[lightNo].m_SlotFree)
		{
			//Optimize free shadow look for
			if (m_FirstFreeDynamicLightSlot == 0xFFFFFFFF)
			{
				m_FirstFreeDynamicLightSlot = lightNo;
			}
			continue;
		}

		const Maths::Vec4F32	vPositionRadius		= Maths::Load<Maths::Vec4F32>((MathsStorage::Float4*) &m_DynamicLights[lightNo].m_vPosition);
		const Maths::Vec4F32	vColour				= Maths::Load<Maths::Vec4F32>((MathsStorage::Float3*) &m_DynamicLights[lightNo].m_vColour);

		const Maths::Vec4F32	vWorldPosition		= Maths::MakeVec4F32(Maths::X(vPositionRadius), Maths::Y(vPositionRadius), Maths::Z(vPositionRadius), vOne);
		const Maths::VF32		fRadius				= Maths::W(vPositionRadius);
		const Maths::Vec4F32	vViewExtents0		= Maths::MakeVec4F32(-fRadius, -fRadius, -fRadius, vZero);
		const Maths::Vec4F32	vViewExtents1		= Maths::MakeVec4F32(-fRadius, -fRadius, +fRadius, vZero);
		const Maths::Vec4F32	vViewExtents2		= Maths::MakeVec4F32(+fRadius, +fRadius, -fRadius, vZero);
		const Maths::Vec4F32	vViewExtents3		= Maths::MakeVec4F32(+fRadius, +fRadius, +fRadius, vZero);

		const Maths::Vec4F32	vViewPosition		= vWorldPosition * mView;
		const Maths::Vec4F32	vViewPosition0		= vViewPosition + vViewExtents0;
		const Maths::Vec4F32	vViewPosition1		= vViewPosition + vViewExtents1;
		const Maths::Vec4F32	vViewPosition2		= vViewPosition + vViewExtents2;
		const Maths::Vec4F32	vViewPosition3		= vViewPosition + vViewExtents3;

		const Maths::Vec4F32	vProjPosition0		= vViewPosition0 * mProj;
		const Maths::Vec4F32	vProjPosition1		= vViewPosition1 * mProj;
		const Maths::Vec4F32	vProjPosition2		= vViewPosition2 * mProj;
		const Maths::Vec4F32	vProjPosition3		= vViewPosition3 * mProj;

		const Maths::VF32		fMinSize = Maths::MakeVF32(c_MinDynamicLightSize);

		if ((vZero < Maths::W(vProjPosition3)) && (fMinSize < (fRadius / Maths::W(vProjPosition3))))
		{
			const Maths::Vec4F32	vScreenPosition0	= vProjPosition0 / Maths::W(vProjPosition0);
			const Maths::Vec4F32	vScreenPosition1	= vProjPosition1 / Maths::W(vProjPosition1);
			const Maths::Vec4F32	vScreenPosition2	= vProjPosition2 / Maths::W(vProjPosition2);
			const Maths::Vec4F32	vScreenPosition3	= vProjPosition3 / Maths::W(vProjPosition3);

			const Maths::Vec4F32	vScreenMin			= Maths::Min(vScreenPosition0, Maths::Min(vScreenPosition1, Maths::Min(vScreenPosition2, vScreenPosition3)));
			const Maths::Vec4F32	vScreenMax			= Maths::Max(vScreenPosition0, Maths::Max(vScreenPosition1, Maths::Max(vScreenPosition2, vScreenPosition3)));
			const Maths::Vec4F32	vScreenSize			= vScreenMax - vScreenMin;

			if (vMinSize < Maths::X(vScreenSize) && vMinSize < Maths::Y(vScreenSize))
			{
				const Maths::HVec2 vMin(Maths::X(vScreenMin), Maths::Y(vScreenMin));
				const Maths::HVec2 vMax(Maths::X(vScreenMax), Maths::Y(vScreenMax));

				C_VisibleDynamicLight visibleLight;
				visibleLight.m_rectangle	= Maths::C_Rect(vMin, vMax);
				visibleLight.m_pLight		= &m_DynamicLights[lightNo];
			

				if (gDebugDynamicLights)
				{
					Maths::HVec3 vPosition(m_DynamicLights[lightNo].m_vPosition.x, m_DynamicLights[lightNo].m_vPosition.y, m_DynamicLights[lightNo].m_vPosition.z);

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(vPosition, m_DynamicLights[lightNo].m_fRadius, Maths::RGBA(Maths::Min(1.f,F32(m_DynamicLights[lightNo].m_vColour.x)), Maths::Min(1.f, F32(m_DynamicLights[lightNo].m_vColour.y)), Maths::Min(1.f,F32(m_DynamicLights[lightNo].m_vColour.z)),F32(1.f)));
				}

				visibleLights.PushBack(visibleLight);
			}
		}
	}
}

bool C_DynamicLights::GatherVisibleParameters(const VisibleDynamicLights& visibleLights, const Maths::C_Rect& rectangle, U32& firstLight, U32& lightCount, Display::C_Float4* pPositionFalloffs, Display::C_Float4* pColours) const
{
	lightCount = 0;
	for (S32 i = firstLight; i < visibleLights.Size(); ++i)
	{
		if (Maths::Intersect(rectangle, visibleLights[i].m_rectangle))
		{
			const C_DynamicLight* const pLight = visibleLights[i].m_pLight;

			pPositionFalloffs[lightCount]	= MathsStorage::Float4(pLight->m_vPosition.x, pLight->m_vPosition.y, pLight->m_vPosition.z, 1.0f / (pLight->m_fRadius * pLight->m_fRadius));
			pColours[lightCount]			= MathsStorage::Float4(pLight->m_vColour.x, pLight->m_vColour.y, pLight->m_vColour.z, 1.0f);

			++lightCount;
		}
		if (c_MaximumLights <= lightCount)
		{
			firstLight = i + 1;
			return false;
		}
	}
	return true;
}

U32 C_DynamicLights::AddLight()
{
	//Look for a free slot
	U32 lightSlot = 0xFFFFFFFF;
	for (S32 i = m_FirstFreeDynamicLightSlot; i < m_DynamicLights.Size(); i++)
	{
		if (m_DynamicLights[i].m_SlotFree)
		{
			lightSlot = i;
			m_FirstFreeDynamicLightSlot = i + 1;
			break;
		}
	}
	if (lightSlot == 0xFFFFFFFF)
	{
		//Allocate a slot
		m_DynamicLights.PushBack();
		lightSlot = m_DynamicLights.Size() - 1;
		m_FirstFreeDynamicLightSlot = m_DynamicLights.Size();
	}

	m_DynamicLights[lightSlot].m_fRadius = 0.f;
	m_DynamicLights[lightSlot].m_vColour = MathsStorage::Float3(0.f, 0.f, 0.f);
	m_DynamicLights[lightSlot].m_SlotFree = false;
	return lightSlot;
}

void C_DynamicLights::RemoveLight( const U32 lightSlot )
{
	assert(lightSlot < (U32)m_DynamicLights.Size());
	assert(!m_DynamicLights[lightSlot].m_SlotFree);

	m_DynamicLights[lightSlot].m_SlotFree =  true;
	if (m_FirstFreeDynamicLightSlot > lightSlot)
		m_FirstFreeDynamicLightSlot = lightSlot;
}

void C_DynamicLights::UpdateLight(const U32 lightSlot, const MathsStorage::Float3& vPosition, const F32 fRadius, const MathsStorage::Float3& vColour)
{
	assert(lightSlot < (U32)m_DynamicLights.Size());
	assert(!m_DynamicLights[lightSlot].m_SlotFree);

	m_DynamicLights[lightSlot].m_vPosition = vPosition;
	m_DynamicLights[lightSlot].m_fRadius = fRadius;
	m_DynamicLights[lightSlot].m_vColour = vColour;
}

}; // namespace Deferred
}; // namespace Render
