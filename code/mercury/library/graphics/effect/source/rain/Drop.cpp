#include "stdafx.h"

#include "Effect/Rain/Drop.h"
#include "Effect/Shaders/Constants.h"

// display library
#include <Display/Buffer/VertexDeclarations.h>
#include <Display/Bundler/Bundler.h>

// render library
#include <Render/State/RenderState.h>
#include <Render/Utils/DebugLog.h>
#include <Render/View/ViewManager.h>

// others
#include <system/Debug/I_DebugRender.h>
#include <misc_shared/Noise/perlin.h>

EXTERN_SHADER(vsDrop);
EXTERN_SHADER(vsDropOcclusion);
EXTERN_SHADER(vsDropSimple);
EXTERN_SHADER(vsDropSimpleOcclusion);
EXTERN_SHADER(psRainDrop);
EXTERN_SHADER(psRainDropOcclusion);
EXTERN_SHADER(psRainDropOcclusion2);

using namespace Effect::Rain;

namespace Effect
{
namespace Rain
{
	Render::Chart::C_LineChart*		m_pLineChart;

	const F32						g_fParticleBox = 30.0f;

	Debug::Bool g_DropEnable("Graphics.Render.Effects.Rain.Drop.Enable", true);
	Debug::Bool g_DropDebugShowOcclusionZones("Graphics.Render.Effects.Rain.Occlusion.ShowOcclusionZones", false);
	Debug::Bool g_DropDebugShowOcclusionZonesDepthEnable("Graphics.Render.Effects.Rain.Occlusion.DepthEnable", true);
	
	Debug::Bool g_DropWireframe("Graphics.Render.Effects.Rain.Drop.Wireframe", false);
	Debug::Bool g_DropShowNoise("Graphics.Render.Effects.Rain.Drop.ShowNoise", false);
	Debug::Bool g_DropOverride("Graphics.Render.Effects.Rain.Drop.OverrideSettings", false);
	Debug::Float g_DropSpeed("Graphics.Render.Effects.Rain.Drop.Speed", 6.0f, Debug::Range(0.f, 10.f), Debug::Step(0.05f));
	Debug::Float g_DropLen("Graphics.Render.Effects.Rain.Drop.Length", 0.2f, Debug::Range(0.f, 0.5f), Debug::Step(0.01f));
	Debug::Float g_DropWidth("Graphics.Render.Effects.Rain.Drop.Width", 0.01f, Debug::Range(0.f, 0.5f), Debug::Step(0.01f));
	Debug::Float g_DropSizeVariation("Graphics.Render.Effects.Rain.Drop.SizeVariation", 0.0f, Debug::Range(0.f, 1.f), Debug::Step(0.1f));
	Debug::Float g_DropWind("Graphics.Render.Effects.Rain.Drop.Wind", 4.0f, Debug::Range(0.f, 20.f), Debug::Step(0.1f));
	Debug::Float g_DropColourScale("Graphics.Render.Effects.Rain.Drop.ColourScale", 0.1f, Debug::Range(0.f, 5.f), Debug::Step(0.1f));
	Debug::Float g_DropColourScaleVariation("Graphics.Render.Effects.Rain.Drop.ColourVariation", 0.0f, Debug::Range(0.f, 1.f), Debug::Step(0.1f));
	Debug::Float g_DropDensity("Graphics.Render.Effects.Rain.Drop.Density", 5.0f, Debug::Range(0.0f, (F32)g_nPasses), Debug::Step(0.1f));
	Debug::Float g_DropLengthAlpha("Graphics.Render.Effects.Rain.Drop.LengthAlpha", 0.01f, Debug::Range(0.f, 1.f), Debug::Step(0.001f));
	Debug::Float g_DropForward("Graphics.Render.Effects.Rain.Drop.Forward", 0.0f, Debug::Range(0.f, 100.f), Debug::Step(1.0f));

	Debug::Float g_DropAlphaFadeSpeedBegin("Graphics.Render.Effects.Rain.Drop.SpeedFadeBegin", 15.0f, Debug::Range(0.f, 100.f), Debug::Step(0.5f));
	Debug::Float g_DropAlphaFadeSpeedEnd("Graphics.Render.Effects.Rain.Drop.SpeedFadeEnd", 75.0f, Debug::Range(0.f, 100.f), Debug::Step(0.5f));
	
}
}

// some rain numbers:
// Drizzle				1.6 mph		0.8 m/s
// Small raindrop		9 mph		4 m/s
// Typical raindrop		14 mph		6 m/s
// Large raindrop		20 mph		9 m/s


#include "Render\Debug\Charts\ChartRenderer.h"
#include "Render\Debug\Charts\StackedBarChart.h"
#include "Render\Debug\Charts\StackedBarChartHistory.h"
//#include "Render\Debug\Charts\ClusteredBarChart.h"
#include "Render\Debug\Charts\LineChart.h"

C_DropRenderer::C_DropRenderer() : Display::I_Renderer("RainDrops")
{
	m_pTexture = NULL;
	// set up chart
	m_pLineChart = new Render::Chart::C_LineChart;
	//m_pLineChart->SetAnnotated(true, 0.1f, 0.05f);
	if(rand()%2 == 0)
		m_pLineChart->SetOrientation(Render::Chart::E_ChartOrientation_Vertical);
	m_pLineChart->SetSize(1.0f, 0.5f);
	m_pLineChart->SetPosition(0.0f, 0.5f);
	m_pLineChart->SetAxisRange(-1.0f, 10.0f);
	//m_pLineChart->SetSpacing(0.01f);
	m_pLineChart->SetHistorySize(2048);
	//m_pLineChart->SetSynchronise(true);

	// add lines
	m_pLineChart->AddSeries("X", Maths::C_RGBA_U8(255, 0, 0, 255));
	m_pLineChart->AddSeries("Y", Maths::C_RGBA_U8(0, 255, 0, 255));
	m_pLineChart->AddSeries("Z", Maths::C_RGBA_U8(0, 0, 255, 255));
	
	m_iNoiseX = 0;
	m_iNoiseY = FP32(2.0f);
	m_iNoiseZ = FP32(4.0f);

	m_fParticleCountScale= 1.0f;

	for(int a = 0; a < g_nPasses; a++)
	{
		m_fSpeedScale[a] = 0.75f + m_RainRandom.FRand(0.5f);
		m_vOffset[a] = Maths::HVec3(0.f, 0.f, 0.f);
		m_vWind[a] = Maths::HVec3(0.f, 0.f, 0.f);
		m_vRandom[a] = Maths::HVec3(m_RainRandom.FRand(g_fParticleBox), m_RainRandom.FRand(g_fParticleBox), m_RainRandom.FRand(g_fParticleBox));
	}

	m_vLighting = MathsStorage::Float4(1.0f, 1.0f, 1.0f, 1.0f);
	SetAssetPath("Textures/");

	SetDefaults();
}

C_DropRenderer::~C_DropRenderer()
{
	DestroyRenderer();

	delete m_pLineChart;
}

void C_DropRenderer::SetDefaults()
{
	DropParameters parameters;
	parameters.texture = "";
	parameters.fDensity = g_DropDensity;
	parameters.fSpeed = g_DropSpeed;
	parameters.fLength = g_DropLen;
	parameters.fWidth = g_DropWidth;
	parameters.fColourScale = g_DropColourScale;
	parameters.fColourScaleVariation = g_DropColourScaleVariation;
	parameters.fSizeVariation = g_DropSizeVariation;
	parameters.iAtlasSize = 1;
	parameters.fWind = g_DropWind;
	parameters.fAlphaFadeSpeedBegin = g_DropAlphaFadeSpeedBegin;
	parameters.fAlphaFadeSpeedEnd = g_DropAlphaFadeSpeedEnd;

	SetParameters(parameters);

	m_nSplits = 1;
}
	

bool C_DropRenderer::OnCreate()
{
	RESOURCE_MEMORY_ID("RainDrop");

	if (!Display::CreateVertexShaderFromFunc("vsDrop", 0, &m_VertexShader, REF_SHADER(vsDrop)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("vsDropOcclusion", 0, &m_VertexShaderOcclusion, REF_SHADER(vsDropOcclusion)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("vsDropSimple", 0, &m_VertexShaderSimple, REF_SHADER(vsDropSimple)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("vsDropSimpleOcclusion", 0, &m_VertexShaderSimpleOcclusion, REF_SHADER(vsDropSimpleOcclusion)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("psRainDrop", 0, &m_PixelShader, REF_SHADER(psRainDrop)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("psRainDropOcclusion", 0, &m_PixelShaderOcclusion, REF_SHADER(psRainDropOcclusion)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("psRainDropOcclusion2", 0, &m_PixelShaderOcclusion2, REF_SHADER(psRainDropOcclusion2)))
		return false;

	if (!Display::Device()->CreateTexture2D(1, 1, 1, Display::E_Usage_Managed, Display::E_Format_A8R8G8B8, &m_WhiteTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetTextureName(&m_WhiteTexture, "_DropWhiteTexture");
	#endif

	// fill a 1x1 texture with white for when occlusion boxes don't have heightmaps
	Display::C_LockedRect rect = m_WhiteTexture.Lock(0);
	if(rect.pBits)
	{
		U32 *pData = reinterpret_cast<U32*>(rect.pBits);
		*pData = 0xffffffff;
		m_WhiteTexture.Unlock(0);
	}

	return true;
}

void C_DropRenderer::OnDestroy()
{
	Display::Device()->Destroy(&m_VertexBuffer);
	Display::Device()->Destroy(&m_IndexBuffer);
	if (m_pTexture && !(m_pTexture->GetFlags() & Display::E_Usage_Bundled))
	{
		Display::Device()->Destroy(m_pTexture);
		delete m_pTexture;
	}
	m_pTexture = NULL;

	Display::Device()->Destroy(&m_WhiteTexture);
	Display::Device()->Destroy(&m_VertexShader);
	Display::Device()->Destroy(&m_VertexShaderOcclusion);
	Display::Device()->Destroy(&m_VertexShaderSimple);
	Display::Device()->Destroy(&m_PixelShader);
	Display::Device()->Destroy(&m_VertexShaderSimpleOcclusion);
	Display::Device()->Destroy(&m_PixelShaderOcclusion);
	Display::Device()->Destroy(&m_PixelShaderOcclusion2);
}

bool C_DropRenderer::OnInitialise()
{
	RESOURCE_MEMORY_ID("RainDrop");

	assert(!m_VertexBuffer.IsCreated());
	assert(!m_IndexBuffer.IsCreated());

	U32 iAtlasSize = m_Parameters.iAtlasSize;
	if(iAtlasSize == 0)
		iAtlasSize = 1;

	// atlas size 1 and no size variation means simple vertex format
	if(iAtlasSize == 1 && m_Parameters.fSizeVariation == 0.0f)
		m_bSimple = true;
	else
		m_bSimple = false;

//	const OverrideSettings &overrides= GetCurrentOverrideSettings();

	if(m_bSimple)
		m_nParticles = static_cast<U32>(10000*m_fParticleCountScale);
	else
		m_nParticles = static_cast<U32>(5000*m_fParticleCountScale);

	Maths::HVec3 vDefaultOffset(g_fParticleBox, g_fParticleBox, g_fParticleBox);

	// we split the total particle box into many boxes to allow improved vertex performance if tiling 
	m_nSplitBoxes = (m_nSplits * m_nSplits * m_nSplits);
	m_fBoxSize = g_fParticleBox / (F32)m_nSplits;
	m_nSplitParticles = m_nParticles / (U32)m_nSplitBoxes;
	m_nParticles = m_nSplitParticles * m_nSplitBoxes;

	m_iVertexCount = m_nParticles * 4;

		m_ePrimitiveType = Display::E_PrimitiveType_TriangleList;
		m_iIndexCount = m_nParticles * 6;

	
	if(m_bSimple)
	{
		// use 16 byte vertex format for simple version (no per particle scaling and no texture atlas)
		if (!Display::Device()->CreateVertexBuffer(m_iVertexCount * sizeof(Display::Vertex_XYZ_SUV), 0, &m_VertexBuffer))
			return false;
	
	// we can save ~20k on xenon by not using index buffers... awesome!
		if (!Display::Device()->CreateIndexBuffer(m_iIndexCount * sizeof(U16), 0, Display::E_Format_Index16, &m_IndexBuffer))
			return false;
		
		Display::Vertex_XYZ_SUV* pVertices = (Display::Vertex_XYZ_SUV*)m_VertexBuffer.Lock();
	
		U16* pIndex = (U16*)m_IndexBuffer.Lock();

		U32 iParticle = 0;
		for(S32 x = 0; x < m_nSplits; ++x)
		{
			for(S32 y = 0; y < m_nSplits; ++y)
			{
				for(S32 z = 0; z < m_nSplits; ++z)
				{
					Maths::HVec3 vSplitOffset(x * m_fBoxSize, y * m_fBoxSize, z * m_fBoxSize);

					for(U32 a = 0; a < m_nSplitParticles; ++a, ++iParticle)
					{
						Maths::HVec3 vPos = Maths::HVec3(m_RainRandom.FRand(m_fBoxSize), m_RainRandom.FRand(m_fBoxSize), m_RainRandom.FRand(m_fBoxSize));
						vPos += vDefaultOffset;
						vPos += vSplitOffset;

						// bottom left
						pVertices->x = vPos.X(); pVertices->y = vPos.Y(); pVertices->z = vPos.Z();
						pVertices->u = 0;
						pVertices->v = 1;
						++pVertices;

						//top left
						pVertices->x = vPos.X(); pVertices->y = vPos.Y(); pVertices->z = vPos.Z();
						pVertices->u = 0;
						pVertices->v = 0;
						++pVertices;

						//top right
						pVertices->x = vPos.X(); pVertices->y = vPos.Y(); pVertices->z = vPos.Z();
						pVertices->u = 1;
						pVertices->v = 0;
						++pVertices;

						// bottom right
						pVertices->x = vPos.X(); pVertices->y = vPos.Y(); pVertices->z = vPos.Z();
						pVertices->u = 1;
						pVertices->v = 1;
						++pVertices;

						*pIndex = iParticle * 4; pIndex++;
						*pIndex = iParticle * 4 + 1; pIndex++;
						*pIndex = iParticle * 4 + 2; pIndex++;
						*pIndex = iParticle * 4 + 2; pIndex++;
						*pIndex = iParticle * 4 + 3; pIndex++;
						*pIndex = iParticle * 4; pIndex++;
					}
				} // z
			} // y
		} // x
		m_IndexBuffer.Unlock();
		m_VertexBuffer.Unlock();
	}
	else
	{
		// use 28 byte vertex format which allows float uvs and size/colour variations
		if (!Display::Device()->CreateVertexBuffer(m_iVertexCount * sizeof(Display::Vertex_XYZ_UV2), 0, &m_VertexBuffer))
			return false;
		if (!Display::Device()->CreateIndexBuffer(m_iIndexCount * sizeof(U16), 0, Display::E_Format_Index16, &m_IndexBuffer))
			return false;
		
		Display::Vertex_XYZ_UV2* pVertices = (Display::Vertex_XYZ_UV2*)m_VertexBuffer.Lock();

		U16* pIndex = (U16*)m_IndexBuffer.Lock();

		F32 fCoordOffset = 1.0f / (F32)iAtlasSize;

		U32 iParticle = 0;
		for(S32 x = 0; x < m_nSplits; ++x)
		{
			for(S32 y = 0; y < m_nSplits; ++y)
			{
				for(S32 z = 0; z < m_nSplits; ++z)
				{
					Maths::HVec3 vSplitOffset(x * m_fBoxSize, y * m_fBoxSize, z * m_fBoxSize);

					for(U32 a = 0; a < m_nSplitParticles; ++a, ++iParticle)
					{
						Maths::HVec3 vPos = Maths::HVec3(m_RainRandom.FRand(m_fBoxSize), m_RainRandom.FRand(m_fBoxSize), m_RainRandom.FRand(m_fBoxSize));
						vPos += vDefaultOffset;
						vPos += vSplitOffset;

						F32 fSizeScale = 1.0f + m_RainRandom.FRand(-m_Parameters.fSizeVariation, m_Parameters.fSizeVariation);
						F32 fColourScale = 1.0f + m_RainRandom.FRand(-m_Parameters.fColourScaleVariation, m_Parameters.fColourScaleVariation);
						S32 iTextureX = m_RainRandom.Rand(iAtlasSize);
						S32 iTextureY = m_RainRandom.Rand(iAtlasSize);

						// bottom left
						pVertices->x = vPos.X(); pVertices->y = vPos.Y(); pVertices->z = vPos.Z();
						pVertices->u0 = iTextureX * fCoordOffset;
						pVertices->v0 = iTextureY * fCoordOffset + fCoordOffset;
						pVertices->u1 = fSizeScale;
						pVertices->v1 = fColourScale;
						++pVertices;

						//top left
						pVertices->x = vPos.X(); pVertices->y = vPos.Y(); pVertices->z = vPos.Z();
						pVertices->u0 = iTextureX * fCoordOffset;
						pVertices->v0 = iTextureY * fCoordOffset;
						pVertices->u1 = fSizeScale;
						pVertices->v1 = fColourScale;
						++pVertices;

						//top right
						pVertices->x = vPos.X(); pVertices->y = vPos.Y(); pVertices->z = vPos.Z();
						pVertices->u0 = iTextureX * fCoordOffset + fCoordOffset;
						pVertices->v0 = iTextureY * fCoordOffset;
						pVertices->u1 = fSizeScale;
						pVertices->v1 = fColourScale;
						++pVertices;

						// bottom right
						pVertices->x = vPos.X(); pVertices->y = vPos.Y(); pVertices->z = vPos.Z();
						pVertices->u0 = iTextureX * fCoordOffset + fCoordOffset;
						pVertices->v0 = iTextureY * fCoordOffset + fCoordOffset;
						pVertices->u1 = fSizeScale;
						pVertices->v1 = fColourScale;
						++pVertices;

						*pIndex = iParticle * 4; pIndex++;
						*pIndex = iParticle * 4 + 1; pIndex++;
						*pIndex = iParticle * 4 + 2; pIndex++;
						*pIndex = iParticle * 4 + 2; pIndex++;
						*pIndex = iParticle * 4 + 3; pIndex++;
						*pIndex = iParticle * 4; pIndex++;
					}
				} // z
			} // y
		} // x
		m_IndexBuffer.Unlock();

		m_VertexBuffer.Unlock();
	}
	
	Util::C_String texture = m_Parameters.texture;
	if(texture == "")
		texture = "raindrop";

	Display::ResourceBundler::I_ResourceBundle* pBundle = GetResourceBundle();
	if (pBundle)
	{
		m_pTexture = pBundle->GetTexture2D(texture);
	}
	if (!m_pTexture)
	{
		Display::C_Texture2D* pNewTexture = new Display::C_Texture2D;
		if (!Display::CreateTexture2DFromFile(GetAssetPath() + texture + ".dds", pNewTexture))
		{
			if (!Display::CreateTexture2DFromFile(GetAssetPath() + texture + ".tga", pNewTexture))
			{
				delete pNewTexture;
				return false;
			}
		}
		m_pTexture = pNewTexture;
	}

	return true;
}

void C_DropRenderer::OnShutdown()
{
	if(m_pTexture && m_pTexture->IsCreated() && !(m_pTexture->GetFlags() & Display::E_Usage_Bundled))
	{
		Display::Device()->Destroy(m_pTexture);
		delete m_pTexture;
	}
	m_pTexture = NULL;

	if(m_VertexBuffer.IsCreated())
		Display::Device()->Destroy(&m_VertexBuffer);
	if(m_IndexBuffer.IsCreated())
		Display::Device()->Destroy(&m_IndexBuffer);
}

void C_DropRenderer::AddOcclusionBox(const Maths::C_AxisBox& axisBox, const Maths::HMat43& transform, const Display::C_Texture2D* pTexture, const bool bFlipV)
{
	C_OcclusionBox heightMap;
	heightMap.SetBox(axisBox, transform);
	heightMap.SetFlipV(bFlipV);
	heightMap.SetTexture(pTexture);
	
	m_OcclusionBoxes.PushBack(heightMap);
	m_OcclusionBoxes.Back().CalculateMatrix();
}

void C_DropRenderer::ClearOcclusionBoxes()
{
	m_OcclusionBoxes.Clear();
}

void C_DropRenderer::Update(const F32 fTime, const bool bPaused)
{
	m_fTime = fTime;

	if(g_DropOverride)
	{
		SetDefaults();
	}

	// calculate how near the occlusion boxes are to the game cameras
	for(S32 i = 0; i < m_OcclusionBoxes.Size(); ++i)
	{
		C_OcclusionBox& box = m_OcclusionBoxes[i];
		Maths::HF32 fMin(10000.0f);

		for(S32 j = 0; j < Render::C_ViewManager::Instance().GetViewCount(); ++j)
		{
			const Display::C_View* pView = &Render::C_ViewManager::Instance().GetView(j)->m_View;
			if(pView->GetUserIndex() != -1)
			{
				Maths::HVec3 vCamera = pView->GetViewSettings3D().GetCameraPosition();
				Maths::HF32 fDistance = box.GetBoundingSphere().Distance(vCamera);
				fMin = Maths::Min(fMin, fDistance);
			}
		}

		box.m_fMinDistance = fMin;
	}
	
	if(!bPaused && Active())
	{
		C_PerlinNoiseDefault noise;

		m_iNoiseX += m_fTime * 0.25f;
		m_iNoiseY += m_fTime * 0.25f;
		m_iNoiseZ += m_fTime * 0.25f;

		
		m_vNoise.X(noise.SoftNoise1D(m_iNoiseX));
		m_vNoise.Y(noise.SoftNoise1D(m_iNoiseY));
		m_vNoise.Z(noise.SoftNoise1D(m_iNoiseZ));

		m_pLineChart->SetValue(0, m_vNoise.X());
		m_pLineChart->SetValue(1, m_vNoise.Y());
		m_pLineChart->SetValue(2, m_vNoise.Z());

		m_pLineChart->Update();


		U32 nWrap = g_nPasses / 5;
		F32 fWind = m_Parameters.fWind / (F32)nWrap;

		for(int a = 0; a < (int)m_nPasses; a++)
		{
			// calculate and add the per-pass gravity and wind
			F32 gravity = m_Parameters.fSpeed * m_fTime * m_fSpeedScale[a];
			m_vWind[a] = m_vNoise * fWind * (F32)((a+1)%(int)nWrap);

			// add the movement from wind
			m_vOffset[a] += m_vWind[a] * m_fTime;

			// add the movement due to gravity
			m_vOffset[a].Y(m_vOffset[a].Y() + gravity);

			if(m_vOffset[a].X() > g_fParticleBox)
				m_vOffset[a].X(0.0f);
			else if(m_vOffset[a].X() < 0.0f)
				m_vOffset[a].X(g_fParticleBox);

			if(m_vOffset[a].Y() > g_fParticleBox)
				m_vOffset[a].Y(0.0f);
			else if(m_vOffset[a].Y() < 0.0f)
				m_vOffset[a].Y(g_fParticleBox);

			if(m_vOffset[a].Z() > g_fParticleBox)
				m_vOffset[a].Z(0.0f);
			else if(m_vOffset[a].Z() < 0.0f)
				m_vOffset[a].Z(g_fParticleBox);
		}

		
	}
};

void C_DropRenderer::Display() const
{
	if(!g_DropEnable || !Active() || !Initialised())
		return;

	PERF_EVENT("Rain::Drop::Display");
	
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	const Maths::HVec3 vViewPosition = viewSettings3D.GetCameraPosition();
	Maths::C_Sphere viewSphere(vViewPosition, g_fParticleBox);

	// only set constants and display if alpha is greater than zero
	if(SetConstants())
	{
		if(g_DropWireframe)
			Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);
		else
			Display::Device()->SetFillMode(Display::E_FillMode_Solid);

		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
		Display::Device()->SetBlendMode(Display::E_BlendMode_Additive);

		Display::Device()->SetTexture(0, m_pTexture);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);
		

		Display::Device()->SetIndexBuffer(&m_IndexBuffer);
		
		// see which of the heightmaps we need to use for this rendering
		Ut::Vector<U32>	occlusionBoxes;
		for(S32 i = 0; i < m_OcclusionBoxes.Size(); ++i)
		{
			S32 iVisible = Display::Device()->GetView()->GetViewFrustum().CullSphere(m_OcclusionBoxes[i].GetBoundingSphere());
			bool bIntersect = m_OcclusionBoxes[i].GetBoundingSphere().Intersect(viewSphere);
			
			if(iVisible && bIntersect)
				occlusionBoxes.PushBack(i);
		}

		// if we need heightmaps set them here, else use simple shader
		if(occlusionBoxes.Size() > 0)
		{
			if(m_bSimple)
			{
				Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XYZ_SUV));
				Display::Device()->SetVertexDeclaration(Display::Vertex_XYZ_SUV::m_pDeclaration);
				Display::Device()->SetVertexShader(&m_VertexShaderSimpleOcclusion);
			}
			else
			{
				Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XYZ_UV2));
				Display::Device()->SetVertexDeclaration(Display::Vertex_XYZ_UV2::m_pDeclaration);
				Display::Device()->SetVertexShader(&m_VertexShaderOcclusion);
			}

			if(occlusionBoxes.Size() == 1)
				Display::Device()->SetPixelShader(&m_PixelShaderOcclusion);
			else
				Display::Device()->SetPixelShader(&m_PixelShaderOcclusion2);

			for(S32 i = 0; i < occlusionBoxes.Size(); ++i)
			{
				const Display::C_Texture2D* pTexture = m_OcclusionBoxes[occlusionBoxes[i]].GetTexture();
				Display::Device()->SetTexture(1 + i, pTexture ? pTexture : &m_WhiteTexture);
				Display::Device()->SetTextureFilter(1 + i, Display::E_TextureFilter_Linear);
				Display::Device()->SetTextureAddress(1 + i, Display::E_TextureAddress_Clamp);
				Display::Device()->SetPixelShaderConstantF(CP_EFFECT_RAIN_OCCLUSION_MATRIX0 + i * 4, (F32*)m_OcclusionBoxes[occlusionBoxes[i]].GetMatrix(), 4);
			}
		}
		else
		{
			if(m_bSimple)
			{
				Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XYZ_SUV));
				Display::Device()->SetVertexDeclaration(Display::Vertex_XYZ_SUV::m_pDeclaration);
				Display::Device()->SetVertexShader(&m_VertexShaderSimple);
			}
			else
			{
				Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XYZ_UV2));
				Display::Device()->SetVertexDeclaration(Display::Vertex_XYZ_UV2::m_pDeclaration);
				Display::Device()->SetVertexShader(&m_VertexShader);
			}
			Display::Device()->SetPixelShader(&m_PixelShader);
		}
		
		F32 fLengthScale = (m_Parameters.fLength / m_Parameters.fSpeed) / m_fTime;

		F32 verticalZoom = viewSettings3D.GetProjMatrix()._22 / 1.778341f;  // This is a normalizing value so that ingame it is 1.0
		F32 fWidth = m_Parameters.fWidth * verticalZoom;
		
		MathsStorage::Float4 vSizeScale(fWidth, fLengthScale, 0.0f, g_DropLengthAlpha);
		
		Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_VIEW_POSITION, (F32*)&viewSettings3D.m_ViewPosition, 1);
		Display::Device()->SetVertexShaderConstantF(CV_EFFECT_RAIN_SIZESCALE, (F32*)&vSizeScale, 1);
		Display::Device()->SetVertexShaderConstantF(CV_EFFECT_RAIN_LIGHTING, (F32*)&m_vLighting, 1);

		F32 fBoxOffset = g_fParticleBox * 0.5f;
		F32 fForwardOffset = g_DropForward;
		
		const Display::C_Matrix& viewMatrix = viewSettings3D.GetViewMatrix();
		F32 fVec[8] = {
			// Forward vector
			viewMatrix._13 * (fBoxOffset + fForwardOffset),
			viewMatrix._23 * (fBoxOffset + fForwardOffset),
			viewMatrix._33 * (fBoxOffset + fForwardOffset),
			viewMatrix._43,
			// Forward offset
			viewMatrix._13 * fForwardOffset,
			viewMatrix._23 * fForwardOffset,
			viewMatrix._33 * fForwardOffset,
			viewMatrix._43 };
		
		Display::Device()->SetVertexShaderConstantF(CV_EFFECT_RAIN_FORWARD, (F32*)&fVec, 2);

		// fadein values
		Render::C_RenderState::Instance().SetFadeInShaderParameters();
		
		F32 fRainDensity = m_Parameters.fDensity;
		for(U32 iPass = 0; iPass < m_nPasses; ++iPass)
		{
			// set pass specific constants needed to render rain
			SetPassConstants(iPass, fRainDensity);

			for(S32 iSplit = 0; iSplit < m_nSplitBoxes; ++iSplit)
			{
				S32 iVertexCount = m_nSplitParticles * 4;
				S32 iIndexCount = m_nSplitParticles * 6;
				S32 iStartIndex = iSplit * iIndexCount;
				Display::Device()->DrawIndexedPrimitive(m_ePrimitiveType, 0, 0, iVertexCount, iStartIndex, iIndexCount);
			}

			fRainDensity -= 1.0f;
		}

		if(g_DropWireframe)
			Display::Device()->SetFillMode(Display::E_FillMode_Solid);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	}

	if(g_DropShowNoise)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(Maths::HVec3(), -m_vNoise, 0.5f, 0.1f, Maths::RGBA(255, 255, 255, 255));
		if(Render::Chart::ChartRendererExists())
			m_pLineChart->RenderDeferred();
	}

	// show debug graphics
	if(g_DropDebugShowOcclusionZones)
	{
		if(!g_DropDebugShowOcclusionZonesDepthEnable)
			DbUtil::DefaultDebugRenderSys()->Deferred()->SetDepthEnable(false);

		for(S32 i = 0; i < m_OcclusionBoxes.Size(); ++i)
		{
			Maths::HVec3 vMin(m_OcclusionBoxes[i].GetAxisBox().Min());
			Maths::HVec3 vMax(m_OcclusionBoxes[i].GetAxisBox().Max());

			S32 iVisible = Display::Device()->GetView()->GetViewFrustum().CullSphere(m_OcclusionBoxes[i].GetBoundingSphere());
			bool bIntersect = m_OcclusionBoxes[i].GetBoundingSphere().Intersect(viewSphere);

			Maths::C_RGBA_U8 colour = bIntersect ? Maths::C_RGBA_U8(255, 255, 255, 255) : Maths::C_RGBA_U8(255, 0, 0, 255);
			
			Maths::HVec3 vCameraPosition = Display::Device()->GetView()->GetViewSettings3D().GetCameraPosition();
			Maths::HVec3 vUVs = m_OcclusionBoxes[i].GetUVs(vCameraPosition);

			if(vUVs.X() > 0.0f && vUVs.X() < 1.0f && vUVs.Z() > 0.0f && vUVs.Z() < 1.0f)
				colour = Maths::C_RGBA_U8(255, 255, 0, 255);

			if(iVisible)
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(vMin, vMax, Maths::HMat43(m_OcclusionBoxes[i].GetTransform()), colour);
		}

		if(!g_DropDebugShowOcclusionZonesDepthEnable)
			DbUtil::DefaultDebugRenderSys()->Deferred()->SetDepthEnable(true);
	}
	
}

bool C_DropRenderer::SetConstants() const
{
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	const Maths::HVec3 vCameraPosition = viewSettings3D.GetCameraPosition();
	const Maths::HVec3 vCameraPositionPrev = viewSettings3D.GetCameraPositionPrev();
	Maths::HVec3 vCameraMovement = vCameraPositionPrev - vCameraPosition;
	F32 fCameraSpeed = Maths::Length(vCameraMovement) * (1.0f / m_fTime);
	

	fCameraSpeed = (fCameraSpeed - m_Parameters.fAlphaFadeSpeedBegin) / m_Parameters.fAlphaFadeSpeedEnd;
	m_fAlphaFade = 1.0f - Maths::Clamped(fCameraSpeed, 0.0f, 1.0f); 
	
	if(m_fAlphaFade > 0.0f)
	{
		// current matrix
		{
			Display::Device()->SetVertexShaderConstantF(CV_RENDER_VIEWPROJORIGIN_MATRIX, (F32*)&viewSettings3D.GetViewProjMatrix(), 4);
		}

		
		// previous matrix
		{
			Display::Device()->SetVertexShaderConstantF(CV_RENDER_VIEWPROJORIGINPREV_MATRIX, (F32*)&viewSettings3D.GetViewProjMatrixPrev(), 4);
		}
		return true;
	}
	else
		return false;
}

void C_DropRenderer::SetPassConstants(U32 i, F32 fAlpha) const
{	
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	Maths::HVec3 vCameraPosition = viewSettings3D.GetCameraPosition();

	const Display::C_Matrix& viewMatrix = viewSettings3D.GetViewMatrix();

	F32 fBoxOffset = g_fParticleBox * 0.5f;
	Maths::HVec3 vForward(viewMatrix._13 * fBoxOffset, viewMatrix._23 * fBoxOffset, viewMatrix._33 * fBoxOffset);

	// combine all offsets together
	Maths::HVec3 vOffset = vCameraPosition + m_vRandom[i] + m_vOffset[i] + vForward;

	MathsStorage::Float4A16 vPositionOffset;
	vPositionOffset.x = fmodf(vOffset.X(), g_fParticleBox);
	vPositionOffset.y = fmodf(vOffset.Y(), g_fParticleBox);
	vPositionOffset.z = fmodf(vOffset.Z(), g_fParticleBox);
	vPositionOffset.w = 0.0f;

	MathsStorage::Float4A16 vRainVelocity;
	vRainVelocity.x = m_vWind[i].X() * m_fTime;
	vRainVelocity.y = m_vWind[i].Y() * m_fTime + m_fSpeedScale[i] * m_Parameters.fSpeed * m_fTime;
	vRainVelocity.z = m_vWind[i].Z() * m_fTime;
	vRainVelocity.w = 0.0f;

	if(fAlpha > 1.0f)
		fAlpha = 1.0f;

	fAlpha = fAlpha * m_Parameters.fColourScale * m_fAlphaFade;

	MathsStorage::Float4 vColourScale(fAlpha, fAlpha, fAlpha, fAlpha);
	Display::Device()->SetVertexShaderConstantF(CV_EFFECT_RAIN_COLOURSCALE, (F32*)&vColourScale, 1);
	
	Display::Device()->SetVertexShaderConstantF(CV_EFFECT_RAIN_POSITIONOFFSET, (F32*)&vPositionOffset, 1);
	Display::Device()->SetVertexShaderConstantF(CV_EFFECT_RAIN_RAINVELOCITY, (F32*)&vRainVelocity, 1);
}

void C_OcclusionBox::CalculateMatrix()
{
	Maths::HVec3 vDiff = m_AxisBox.Diagonal();
	Maths::HVec3 vTranslate = -(m_AxisBox.Min() / vDiff);
	
	Maths::HVec3 vDiffRcp = Maths::Reciprocal(vDiff);

	if(m_bFlipV)
	{
		vDiffRcp.Z(-vDiffRcp.Z());
		vTranslate.Z(1.0f - vTranslate.Z());
	}

	Maths::C_MatrixRotateRPH noRotation;
	// take into account that V coord needs to flipped by flipping the Z
	Maths::C_MatrixScale scale(vDiffRcp.X(), vDiffRcp.Y(), vDiffRcp.Z());
	Maths::C_MatrixTranslate translate(vTranslate.X(), vTranslate.Y(), vTranslate.Z() );
	Maths::C_MatrixSRT srt(scale, noRotation, translate);
	
	// multiply by the inverse transform then apply bounds
	Maths::HMat43 boundScale(srt);
	Maths::HMat44 transform(m_mTransform);
	Maths::HMat44 levelMatrix = transform.Inverse() * Maths::HMat44(boundScale);
	
	Maths::Store(levelMatrix, &m_mMatrix);
}

void C_OcclusionBox::SetBox(const Maths::C_AxisBox& axisBox, const Maths::HMat43& transform)
{
	m_mTransform = transform;
	m_AxisBox = axisBox;

	// get bounding sphere taking into account transformation
	Maths::HVec4 vSize(axisBox.Diagonal());
	vSize *= 0.5f;
	vSize.W(0.0f);
	vSize = vSize * Maths::HMat44(transform);
	Maths::HF32 fLength = Maths::Length(Maths::HVec3(vSize));
	
	m_BoundingSphere = m_AxisBox.BoundingSphere() * m_mTransform;
	m_BoundingSphere = Maths::C_Sphere(m_BoundingSphere.Centre(), fLength);
}

Maths::HVec3 C_OcclusionBox::GetUVs(const Maths::HVec3& position) const
{
	Maths::HMat44 matrix(Maths::Load<Maths::HMat44>(&m_mMatrix));

	Maths::HVec3 transformedPosition = position * matrix;
	return transformedPosition;
}



