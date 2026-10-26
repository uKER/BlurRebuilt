#include "stdafx.h"
#include "Effect/PostProcess/DoFClassify.h"

#include "Display/Buffer/VertexDeclarations.h"

#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/Utils/Globals.h"

EXTERN_SHADER(psDoFClassifyBlurAll);
EXTERN_SHADER(psDoFClassifyMinDownsample);
EXTERN_SHADER(psDoFClassifyMaxDownsample);
EXTERN_SHADER(psDoFClassifyWeights);
EXTERN_SHADER(psDoFClassifyDistance);
EXTERN_SHADER(psDoFClassifyTilesFull);
EXTERN_SHADER(psDoFClassifyTilesIntermediate);
EXTERN_SHADER(psDoFClassifyTilesNone);
EXTERN_SHADER(vsDoFClassifyTiles);

namespace PostProcess
{

Debug::Float	g_debugClassifyFocalDistance("Graphics.Effect.PostProcess.DepthOfFieldClassify.FocalDistance", 5.0f, Debug::Range(0.0f, 100.0f));
Debug::Float	g_debugClassifyFocalScale("Graphics.Effect.PostProcess.DepthOfFieldClassify.FocalScale", 2.0f, Debug::Range(0.0f, 10.0f));
Debug::Bool		g_debugClassify("Graphics.Effect.PostProcess.DepthOfFieldClassify.Classify", true);

C_DoFClassify::C_DoFClassify(C_Manager& manager) : I_PostProcess(manager)
{
	F32 vWeights[5];
	for (S32 i = 0; i < 5; ++i)
	{
		vWeights[i] = GaussianDistribution(i - 2.0f, 0.0f, 2.0f);
	}
	
	for (S32 y = 0; y < 5; ++y)
	{
		for (S32 x = 0; x < 5; ++x)
		{
			const S32 iIndex = (y * 5) + x;
				
			m_vTaps[iIndex * 2] = (x - 2) * 2.0f / 1280.0f;
			m_vTaps[iIndex * 2 + 1] = (y - 2) * 2.0f / 720.0f;

			const F32 fX = ((0.5f + x) / 5.0f) * 2.0f - 1.0f;
			const F32 fY = ((0.5f + y) / 5.0f) * 2.0f - 1.0f;
			const F32 fLen = Maths::Clamp01(sqrtf(fX * fX + fY * fY) * (1.0f / sqrtf(1.8f)));

			m_vTapDistances[iIndex] = fLen;
		}
	}

	F32 fTotalWeight = 0.0f;
	for (S32 j = 0; j < 5; ++j)
	{
		for (S32 i = 0; i < 5; ++i)
		{
			m_vTapWeights[j * 5 + i] = vWeights[i] * vWeights[j];
			fTotalWeight += m_vTapWeights[j * 5 + i];
		}
	}

	for (S32 i = 0; i < 25; ++i)
		m_vTapWeights[i] = m_vTapWeights[i] / fTotalWeight;
}

C_DoFClassify::~C_DoFClassify()
{
}

bool C_DoFClassify::Create()
{
	const U32 iWidth = 1280;
	const U32 iHeight = 720;

	const U32 iTilesWidth = iWidth / 8;
	const U32 iTilesHeight = iHeight / 8;

	m_VertexCount = iTilesWidth * iTilesHeight;

	if (!Display::Device()->CreateVertexBuffer(m_VertexCount * sizeof(Display::Vertex_XY), 0, &m_VertexBuffer))
		return false;

	Display::Vertex_XY* pVertex = (Display::Vertex_XY*)m_VertexBuffer.Lock();
	if (pVertex)
	{
		for (U32 y = 0; y < iTilesHeight; ++y)
		{
			for (U32 x = 0; x < iTilesWidth; ++x)
			{
				pVertex->x = (x + 0.5f) * (1.0f / (F32) iTilesWidth) * 2.0f - 1.0f;
				pVertex->y = (y + 0.5f) * (1.0f / (F32) iTilesHeight) * 2.0f - 1.0f;
				++pVertex;
			}
		}

		m_VertexBuffer.Unlock();
	}

	if (!Display::CreatePixelShaderFromFunc("Shaders/psDoFClassifyBlurAll", 0, &m_BlurAllShader, REF_SHADER(psDoFClassifyBlurAll)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDoFClassifyMinDownsample", 0, &m_DepthMinDownsampleShader, REF_SHADER(psDoFClassifyMinDownsample)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDoFClassifyMaxDownsample", 0, &m_DepthMaxDownsampleShader, REF_SHADER(psDoFClassifyMaxDownsample)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDoFClassifyWeights", 0, &m_DepthWeightsShader, REF_SHADER(psDoFClassifyWeights)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDoFClassifyDistance", 0, &m_FocalDistanceShader, REF_SHADER(psDoFClassifyDistance)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDoFClassifyTilesFull", 0, &m_TilesFullShader, REF_SHADER(psDoFClassifyTilesFull)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDoFClassifyTilesIntermediate", 0, &m_TilesIntermediateShader, REF_SHADER(psDoFClassifyTilesIntermediate)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDoFClassifyTilesNone", 0, &m_TilesNoneShader, REF_SHADER(psDoFClassifyTilesNone)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsDoFClassifyTiles", 0, &m_TilesVertexShader, REF_SHADER(vsDoFClassifyTiles)))
		return false;

	m_BlurDistancesParameter	= m_BlurAllShader.GetParameterArray(0, "g_vTapDistances", 6);
	m_BlurWeightsParameter		= m_BlurAllShader.GetParameterArray(6, "g_vTapWeights", 6);
	m_BlurTapsParameter			= m_BlurAllShader.GetParameterArray(12, "g_vTaps", 12);

	m_DepthMinSettingsParameter	= m_DepthMinDownsampleShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_DepthMaxSettingsParameter	= m_DepthMaxDownsampleShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_DepthWeightsParameter		= m_DepthWeightsShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_FocalSettingsParameter	= m_FocalDistanceShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_FocalScaleParameter		= m_FocalDistanceShader.GetParameter(CP_RENDER_SETTINGS_1, "g_vScale");

	m_TilesDistancesParameter	= m_TilesIntermediateShader.GetParameterArray(0, "g_vTapDistances", 6);
	m_TilesWeightsParameter		= m_TilesIntermediateShader.GetParameterArray(6, "g_vTapWeights", 6);
	m_TilesTapsParameter		= m_TilesIntermediateShader.GetParameterArray(12, "g_vTaps", 12);

	return true;
}

bool C_DoFClassify::CreateVolatile()
{
	const U32 iWidth = 1280;
	const U32 iHeight = 720;

	if (!Display::Device()->CreateRenderTexture2D(iWidth, iHeight, 1, 0, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_FocusDistanceRenderTexture))
		return false;
	if (!Display::Device()->CreateRenderTexture2D(iWidth / 8, iHeight / 8, 1, 0, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_DownsampledDepthMinRenderTexture))
		return false;
	if (!Display::Device()->CreateRenderTexture2D(iWidth / 8, iHeight / 8, 1, 0, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_DownsampledDepthMaxRenderTexture))
		return false;
	if (!Display::Device()->CreateRenderTexture2D(iWidth / 8, iHeight / 8, 1, 0, Display::E_Format_R32F, Display::E_Multisample_None, 0, &m_FocusDistanceRenderTexture))
		return false;
	if (!Display::Device()->CreateRenderTexture2D(iWidth / 2, iHeight / 2, 1, 0, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_DownsampledColourRenderTexture))
		return false;
	if (!Display::Device()->CreateRenderTexture2D(iWidth / 2, iHeight / 2, 1, 0, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_DownsampledBlurXRenderTexture))
		return false;
	if (!Display::Device()->CreateRenderTexture2D(iWidth / 2, iHeight / 2, 1, 0, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_DownsampledBlurredRenderTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_FocusDistanceRenderTexture, "_FocusDistanceRT");
	Display2::Device()->SetRenderTexture2DName(&m_DownsampledDepthMinRenderTexture, "_DownsampledDepthMinRT");
	Display2::Device()->SetRenderTexture2DName(&m_DownsampledDepthMaxRenderTexture, "_DownsampledDepthMaxRT");
	Display2::Device()->SetRenderTexture2DName(&m_FocusDistanceRenderTexture, "_FocusDistanceRT");
	Display2::Device()->SetRenderTexture2DName(&m_DownsampledColourRenderTexture, "_DownsampledColourRT");
	Display2::Device()->SetRenderTexture2DName(&m_DownsampledBlurXRenderTexture, "_DownsampledBlurXRT");
	Display2::Device()->SetRenderTexture2DName(&m_DownsampledBlurredRenderTexture, "_DownsampledBlurredRT");
	#endif

	return true;
}

void C_DoFClassify::Destroy()
{
	Display::Device()->Destroy(&m_VertexBuffer);

	Display::Device()->Destroy(&m_BlurAllShader);
	Display::Device()->Destroy(&m_DepthMinDownsampleShader);
	Display::Device()->Destroy(&m_DepthMaxDownsampleShader);
	Display::Device()->Destroy(&m_DepthWeightsShader);
	Display::Device()->Destroy(&m_FocalDistanceShader);
	Display::Device()->Destroy(&m_TilesFullShader);
	Display::Device()->Destroy(&m_TilesIntermediateShader);
	Display::Device()->Destroy(&m_TilesNoneShader);
	Display::Device()->Destroy(&m_TilesVertexShader);
}

void C_DoFClassify::DestroyVolatile()
{
	Display::Device()->Destroy(&m_FocusDistanceRenderTexture);
	Display::Device()->Destroy(&m_DownsampledColourRenderTexture);
	Display::Device()->Destroy(&m_DownsampledBlurXRenderTexture);
	Display::Device()->Destroy(&m_DownsampledBlurredRenderTexture);
	Display::Device()->Destroy(&m_DownsampledDepthMinRenderTexture);
	Display::Device()->Destroy(&m_DownsampledDepthMaxRenderTexture);
	Display::Device()->Destroy(&m_DepthWeightsRenderTexture);
}

void C_DoFClassify::Display() const
{
	if(!m_Manager.Enabled())
		return;

	PERF_EVENT("DepthOfFieldClassify");

	SetViewport();

	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);

	RenderFocalDistances();

	m_DownsampleUtil.Downsample(&m_DownsampledColourRenderTexture, Render::GetGlobalColourBuffer());
	m_GaussianBlurUtil.GaussBlurX(&m_DownsampledBlurXRenderTexture, &m_DownsampledColourRenderTexture, Render::Texture::E_GaussianBlurShader_5);
	m_GaussianBlurUtil.GaussBlurY(&m_DownsampledBlurredRenderTexture, &m_DownsampledBlurXRenderTexture, Render::Texture::E_GaussianBlurShader_5);

	if (g_debugClassify)
	{
		RenderDepthWeights();
		m_Manager.SetViewport();
		RenderTilesNone();
		RenderTilesFull();
		RenderTilesIntermediate();
	}
	else
	{
		m_Manager.SetViewport();
		RenderBlurAll();
	}
}

void C_DoFClassify::RenderBlurAll() const
{
	Display::Device()->SetPixelShaderParameterArrayF(m_BlurTapsParameter, m_vTaps);
	Display::Device()->SetPixelShaderParameterArrayF(m_BlurWeightsParameter, m_vTapWeights);
	Display::Device()->SetPixelShaderParameterArrayF(m_BlurDistancesParameter, m_vTapDistances);
	Display::Device()->SetPixelShader(&m_BlurAllShader);
	Display::Device()->SetTexture(0, Render::GetGlobalColourBuffer());
	Display::Device()->SetTexture(1, &m_DownsampledColourRenderTexture.GetTexture());
	Display::Device()->SetTexture(2, &m_FocusDistanceRenderTexture.GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Linear);

	Display::Quad::DrawFullScreen();
}

void C_DoFClassify::RenderTilesFull() const
{
	Display::Device()->SetVertexDeclaration(Display::Vertex_XY::m_pDeclaration);
	Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XY));

	Display::C_Float4 vsSettings(1.0f / 1280.0f, 1.0f / 720.0f, 8.0f, 1.0f);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS, (F32*)&vsSettings, 1);
	Display::Device()->SetVertexShader(&m_TilesVertexShader);
	Display::Device()->SetPixelShader(&m_TilesFullShader);


	Display::Device()->SetTexture(0, Render::GetGlobalColourBuffer());
	Display::Device()->SetTexture(1, &m_DownsampledBlurredRenderTexture.GetTexture());
	Display::Device()->SetTexture(2, &m_FocusDistanceRenderTexture.GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Linear);

	Display::Device()->SetDepthEnable(Display::E_Bool_False);

	Display::Device()->DrawPrimitive(Display::E_PrimitiveType_PointList, 0, m_VertexCount); 

	Display::Device()->SetDepthEnable(Display::E_Bool_Default); 

}

void C_DoFClassify::RenderTilesIntermediate() const
{
	Display::Device()->SetVertexDeclaration(Display::Vertex_XY::m_pDeclaration);
	Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XY));
	
	Display::C_Float4 vsSettings(1.0f / 1280.0f, 1.0f / 720.0f, 8.0f, 0.0f);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS, (F32*)&vsSettings, 1);	Display::Device()->SetVertexShader(&m_TilesVertexShader);
	Display::Device()->SetVertexShader(&m_TilesVertexShader);

	Display::Device()->SetPixelShaderParameterArrayF(m_TilesTapsParameter, m_vTaps);
	Display::Device()->SetPixelShaderParameterArrayF(m_TilesWeightsParameter, m_vTapWeights);
	Display::Device()->SetPixelShaderParameterArrayF(m_TilesDistancesParameter, m_vTapDistances);
	Display::Device()->SetPixelShader(&m_TilesIntermediateShader);


	Display::Device()->SetTexture(0, Render::GetGlobalColourBuffer());
	Display::Device()->SetTexture(1, &m_DownsampledColourRenderTexture.GetTexture());
	Display::Device()->SetTexture(2, &m_FocusDistanceRenderTexture.GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Linear);

	Display::Device()->SetDepthEnable(Display::E_Bool_False);

	Display::Device()->DrawPrimitive(Display::E_PrimitiveType_PointList, 0, m_VertexCount); 

	Display::Device()->SetDepthEnable(Display::E_Bool_Default);

}

void C_DoFClassify::RenderTilesNone() const
{
	Display::Device()->SetVertexDeclaration(Display::Vertex_XY::m_pDeclaration);
	Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XY));
	
	Display::C_Float4 vsSettings(1.0f / 1280.0f, 1.0f / 720.0f, 8.0f, -1.0f);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS, (F32*)&vsSettings, 1);	Display::Device()->SetVertexShader(&m_TilesVertexShader);
	Display::Device()->SetVertexShader(&m_TilesVertexShader);
	Display::Device()->SetPixelShader(&m_TilesNoneShader);


	Display::Device()->SetTexture(0, Render::GetGlobalColourBuffer());
	Display::Device()->SetTexture(1, &m_DownsampledColourRenderTexture.GetTexture());
	Display::Device()->SetTexture(2, &m_FocusDistanceRenderTexture.GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Linear);

	Display::Device()->SetDepthEnable(Display::E_Bool_False);

	Display::Device()->DrawPrimitive(Display::E_PrimitiveType_PointList, 0, m_VertexCount); 

	Display::Device()->SetDepthEnable(Display::E_Bool_Default);

}

void C_DoFClassify::RenderFocalDistances() const
{
	Display::Device()->SetColourDepthStencilSurface(&m_FocusDistanceRenderTexture.GetSurface(), 0);
	
	const MathsStorage::Float4 focalSettings = MathsStorage::Float4(g_debugClassifyFocalDistance, g_debugClassifyFocalScale, 0.0f, 0.0f);
	const MathsStorage::Float4 focalScale = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	Display::Device()->SetPixelShaderParameterF(m_FocalSettingsParameter, (F32*)&focalSettings);
	Display::Device()->SetPixelShaderParameterF(m_FocalScaleParameter, (F32*)&focalScale);
	Display::Device()->SetPixelShader(&m_FocalDistanceShader);
	Display::Device()->SetTexture(0, Render::GetGlobalDepthStencilBuffer());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	
	Display::Quad::DrawFullScreen();

	m_FocusDistanceRenderTexture.Resolve();
}

void C_DoFClassify::RenderDepthWeights() const
{
	const MathsStorage::Float4 vDepthSettings = MathsStorage::Float4(1.0f / 1280.0f, 1.0f / 720.0f, 0.0f, 0.0f);

	Display::Device()->SetColourDepthStencilSurface(&m_DownsampledDepthMinRenderTexture.GetSurface(), 0);
	
	Display::Device()->SetPixelShaderParameterF(m_DepthMinSettingsParameter, (F32*)&vDepthSettings);
	Display::Device()->SetPixelShader(&m_DepthMinDownsampleShader);
	Display::Device()->SetTexture(0, &m_FocusDistanceRenderTexture.GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	
	Display::Quad::DrawFullScreen();

	m_DownsampledDepthMinRenderTexture.Resolve();
	
	Display::Device()->SetColourDepthStencilSurface(&m_DownsampledDepthMaxRenderTexture.GetSurface(), 0);
	
	Display::Device()->SetPixelShaderParameterF(m_DepthMaxSettingsParameter, (F32*)&vDepthSettings);
	Display::Device()->SetPixelShader(&m_DepthMaxDownsampleShader);
	Display::Device()->SetTexture(0, &m_FocusDistanceRenderTexture.GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	
	Display::Quad::DrawFullScreen();

	m_DownsampledDepthMaxRenderTexture.Resolve();

	Display::Device()->SetColourDepthStencilSurface(&m_DepthWeightsRenderTexture.GetSurface(), 0);
	
	const Display::C_Float4 psSettings(8.0f / 1280.0f, 8.0f / 720.0f, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_DepthWeightsParameter, (F32*)&psSettings);
	Display::Device()->SetPixelShader(&m_DepthWeightsShader);
	Display::Device()->SetTexture(0, &m_DownsampledDepthMinRenderTexture.GetTexture());
	Display::Device()->SetTexture(1, &m_DownsampledDepthMaxRenderTexture.GetTexture());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	
	Display::Quad::DrawFullScreen();

	m_DepthWeightsRenderTexture.Resolve();
}

}; // namespace PostProcess
