#include "stdafx.h"

#include "Effect/Rain/Splash.h"
#include "Effect/Shaders/Constants.h"

#include <Display/Buffer/VertexDeclarations.h>
#include <Display/Bundler/Bundler.h>
#include <Render/View/ViewManager.h>
#include <System/Debug/I_DebugRender.h>

EXTERN_SHADER(vsSplash);
EXTERN_SHADER(psRainDrop);

using namespace Effect::Rain;

#define EFFECT_RAIN_USE_HVEC

namespace Effect
{
namespace Rain
{
	Util::C_Random g_Random;

	Debug::Bool g_debugSplashEnable("Graphics.Render.Effects.Rain.Splash.Enable", true);
	Debug::Bool g_debugSplashOverride("Graphics.Render.Effects.Rain.Splash.Override", false);
	Debug::Bool g_debugSplashDebugGraphics("Graphics.Render.Effects.Rain.Splash.DebugGraphics", false);
	Debug::Bool g_debugSplashReduceSpawnByDistance("Graphics.Render.Effects.Rain.Splash.ReduceSpawnByDistance", true);
	Debug::Bool g_debugSplashCull("Graphics.Render.Effects.Rain.Splash.Cull", true);
	Debug::Bool g_debugSplashFreezeOffscreen("Graphics.Render.Effects.Rain.Splash.FreezeOffscreen", true);

	Debug::Float g_debugSplashEmitRate("Graphics.Render.Effects.Rain.Splash.EmitRate", 200.0f, Debug::Range(0.f, 1000.0f), Debug::Step(1.f));
	Debug::Float g_debugSplashSizeStart("Graphics.Render.Effects.Rain.Splash.SizeStart", 0.1f, Debug::Range(0.f, 1.0f), Debug::Step(0.01f));
	Debug::Float g_debugSplashSizeEnd("Graphics.Render.Effects.Rain.Splash.SizeEnd", 0.15f, Debug::Range(0.f, 1.0f), Debug::Step(0.01f));
	Debug::Float g_debugSplashSizeVariation("Graphics.Render.Effects.Rain.Splash.SizeVariation", 0.025f, Debug::Range(0.f, 1.0f), Debug::Step(0.005f));
	Debug::Float g_debugSplashHeightOffset("Graphics.Render.Effects.Rain.Splash.HeightOffset", 0.f, Debug::Range(0.f, 1.0f), Debug::Step(0.1f));
	Debug::Float g_debugSplashLife("Graphics.Render.Effects.Rain.Splash.Life", 0.5f, Debug::Range(0.f, 1.0f), Debug::Step(0.01f));
	Debug::Float g_debugSplashLifeVariation("Graphics.Render.Effects.Rain.Splash.LifeVariation", 0.01f, Debug::Range(0.f, 1.0f), Debug::Step(0.005f));
	Debug::Float g_debugSplashColourScale("Graphics.Render.Effects.Rain.Splash.ColourScale", 0.9f, Debug::Range(0.f, 1.0f), Debug::Step(0.1f));
	Debug::Float g_debugSplashColourScaleVariation("Graphics.Render.Effects.Rain.Splash.ColourScaleVariation", 0.1f, Debug::Range(0.f, 1.0f), Debug::Step(0.1f));
	Debug::Float g_debugSplashBlockSize("Graphics.Render.Effects.Rain.Splash.BlockSize", 10.0f, Debug::Range(0.f, 20.0f), Debug::Step(1.0f));
	
	Debug::Float g_debugSplashSpawnScaleSpeedBegin("Graphics.Render.Effects.Rain.Splash.SpawnScaleSpeedBegin", 15.0f, Debug::Range(0.f, 100.f), Debug::Step(0.5f));
	Debug::Float g_debugSplashSpawnScaleSpeedEnd("Graphics.Render.Effects.Rain.Splash.SpawnScaleSpeedEnd", 75.0f, Debug::Range(0.f, 100.f), Debug::Step(0.5f));
	
	Debug::Int	g_debugSplashVisible("Graphics.Render.Effects.Rain.Splash.Visible", 3, Debug::Range(0, 100), Debug::Step(1));

}
}

C_SplashRendererBase::C_SplashRendererBase() : Display::I_Renderer("RainSplashes")
{
	m_pTexture = NULL;
	m_fCycle = 0.0f;
	m_fTime = 0.0f;
	SetAssetPath("Textures/");
	m_vLighting = MathsStorage::Float4(1.0f, 1.0f, 1.0f, 1.0f);

	SetDefaults();
}

C_SplashRendererBase::~C_SplashRendererBase()
{
	DestroyRenderer();

	for(BlockList::Iterator pBlock = m_Blocks.Begin(); pBlock != m_Blocks.End(); )
	{
		SplashBlock* pDelete = &(*pBlock);
		pBlock = m_Blocks.Erase(pBlock);
		m_BlockPool.Free(pDelete);
	}
	
}

void C_SplashRendererBase::SetDefaults()
{
	SplashParameters parameters;
	parameters.fEmitRate = g_debugSplashEmitRate;
	parameters.fSizeStart = g_debugSplashSizeStart;
	parameters.fSizeEnd = g_debugSplashSizeEnd;
	parameters.fSizeVariation = g_debugSplashSizeVariation;
	parameters.fHeightOffset = g_debugSplashHeightOffset;
	parameters.fLife = g_debugSplashLife;
	parameters.fLifeVariation = g_debugSplashLifeVariation;
	parameters.fColourScale = g_debugSplashColourScale;
	parameters.fColourScaleVariation = g_debugSplashColourScaleVariation;

	parameters.fSpawnScaleSpeedBegin = g_debugSplashSpawnScaleSpeedBegin;
	parameters.fSpawnScaleSpeedEnd = g_debugSplashSpawnScaleSpeedEnd;


	SetParameters(parameters);

	
	m_nBlocks = g_debugSplashVisible;

	m_Settings.fBlockSize = g_debugSplashBlockSize;
	m_Settings.fDistanceFade = m_nBlocks * m_Settings.fBlockSize;
	m_Settings.fDistanceFadeSq = m_Settings.fDistanceFade * m_Settings.fDistanceFade;
	m_Settings.fDistanceFadeRcp = 1.0f / m_Settings.fDistanceFade;
	
	m_GridPositions.Clear();
	for(S32 x = -m_nBlocks; x <= m_nBlocks; ++x)
	{
		for(S32 z = -m_nBlocks; z <= m_nBlocks; ++z)
		{
			if(abs(x) != m_nBlocks || abs(z) != m_nBlocks)
				m_GridPositions.PushBack(MathsStorage::Short2(x, z));
		}
	}
}
bool C_SplashRendererBase::OnCreate()
{
	RESOURCE_MEMORY_ID("RainSplash");



	return true;
}

void C_SplashRendererBase::OnDestroy()
{
	if(m_pTexture && m_pTexture->IsCreated() && !(m_pTexture->GetFlags() & Display::E_Usage_Bundled))
	{
		Display::Device()->Destroy(m_pTexture);
		delete m_pTexture;
	}
	m_pTexture = NULL;
	Display::Device()->Destroy(&m_VertexShader);
	Display::Device()->Destroy(&m_PixelShader);
}

bool C_SplashRendererBase::OnInitialise()
{
	RESOURCE_MEMORY_ID("RainSplash");

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

	// reset camera positions
	m_CameraPositions.Resize(4);
	for(S32 i = 0; i < m_CameraPositions.Size(); ++i)
	{
		m_CameraPositions[i] = MathsStorage::Short2(-32768, -32768);
	}

	return true;
}

void C_SplashRendererBase::OnShutdown()
{
	if(m_pTexture && m_pTexture->IsCreated() && !(m_pTexture->GetFlags() & Display::E_Usage_Bundled))
	{
		Display::Device()->Destroy(m_pTexture);
		delete m_pTexture;
	}
	m_pTexture = NULL;
}

void C_SplashRendererBase::Update(const F32 fTime, const bool bPaused)
{
	if(g_debugSplashOverride)
	{
		SetDefaults();
	}

	if(!m_bActive)
		return;

	m_fFrameTime = fTime;
	m_fFrameTimeRcp = 1.0f / fTime;
	m_fTime += m_fFrameTime;

	if(!bPaused && g_debugSplashEnable)
	{
		static Util::C_String128 pixName;
		PERF_EVENT(pixName.Printf("RainSplashes::Update (%d)", m_nParticles));
		m_nParticles = 0;

		// update current particles
		for(BlockList::Iterator pBlock = m_Blocks.Begin(); pBlock != m_Blocks.End(); ++pBlock)
		{
			// only bother updating if block was displayed recently
			if(pBlock->m_bVisible || !g_debugSplashFreezeOffscreen)
			{
				m_nParticles += pBlock->m_Element.Size();
				
				for(S32 a = 0; a < pBlock->m_Element.Size(); )
				{

					SplashParticle* pParticle = &pBlock->m_Element[a];
				#ifdef USE_INT_LIFE
					pParticle->iLife -= pParticle->iLifeReduce;
					if(pParticle->iLife < 0)
				#else
					pParticle->fLife -= fTime * pParticle->vForceSizeLife.y;
					if(pParticle->fLife < 0.0f)
				#endif
					{
						*pParticle = pBlock->m_Element.Back();
						pBlock->m_Element.PopBack();
					}
					else
					{
					#ifdef USE_INT_LIFE
						pParticle->fLife -= fTime * pParticle->vForceSizeLife.y;
					#endif
						++a;
					}
				}
			}
		}
	}
}

S32 C_SplashRendererBase::GetSpawnAmount()
{
	S32 nTotalEmit = 0;

	F32 fSpawnTime = 1.0f / m_Parameters.fEmitRate;
	m_fCycle += m_fFrameTime;
	if(m_fCycle > fSpawnTime)
	{
		F32 fNumEmit = m_fCycle / fSpawnTime;
		nTotalEmit = (S32)fNumEmit;

		m_fCycle -= fSpawnTime * nTotalEmit;
	}

	return nTotalEmit;
}

void C_SplashRendererBase::PreBuild()
{
	if(!g_debugSplashEnable || !m_bActive)
		return;

	m_VisBits.Clear();
	S32 nViews = Render::C_ViewManager::Instance().GetViewCount();
	m_VisBits.Resize(nViews);

	// make sure theres space in the visbits
	for(S32 i = 0; i < m_VisBits.Size(); ++i)
	{
		m_VisBits[i].Resize(m_Blocks.Size());
		m_VisBits[i].ClearBits();
	}

	Maths::HF32 fDistanceFadeRcp(m_Settings.fDistanceFadeRcp);

	// go through all views and work out a spawn reduction amount based on camera speed
	Ut::UtVector<Ut::ArrayVector<F32, 4> > fSpeedSpawnScale;
	for(S32 i = 0; i < Render::C_ViewManager::Instance().GetViewCount(); ++i)
	{
		const Display::C_View* pView = &Render::C_ViewManager::Instance().GetView(i)->m_View;

		if(pView->GetUserIndex() != -1)
		{

			const Display::C_ViewSettings3D& viewSettings3D = pView->GetViewSettings3D();
			const Maths::HVec3 vCameraPosition = viewSettings3D.GetCameraPosition();
			const Maths::HVec3 vCameraPositionPrev = viewSettings3D.GetCameraPositionPrev();
			Maths::HVec3 vCameraMovement = vCameraPositionPrev - vCameraPosition;
			F32 fCameraSpeed = Maths::Length(vCameraMovement) * m_fFrameTimeRcp;
			fCameraSpeed = (fCameraSpeed - m_Parameters.fSpawnScaleSpeedBegin) / m_Parameters.fSpawnScaleSpeedEnd;
			fSpeedSpawnScale[i] = 1.0f - Maths::Clamped(fCameraSpeed, 0.0f, 1.0f);
		}
		else
			fSpeedSpawnScale[i] = 0.0f; 
	}

	// cull each block against all viewports
	S32 iVisBit = 0;
	for(BlockList::Iterator pBlockIterator = m_Blocks.Begin(); pBlockIterator != m_Blocks.End(); ++pBlockIterator, ++iVisBit)
	{
		SplashBlock* pBlock = &(*pBlockIterator);

		Maths::HVec4 vCentreOne(Maths::Load<Maths::HVec4>(&pBlock->m_vCentre));
		Maths::HVec4 vSizeCull(Maths::Load<Maths::HVec4>(&pBlock->m_vSize));

		bool bVisible = false;
		Maths::HF32 fMinDistance = g_debugSplashReduceSpawnByDistance ? Maths::Constants<Maths::HF32>::One() : Maths::Constants<Maths::HF32>::Zero();
		pBlock->m_fSpeedSpawnScale = 0.0f;

		for(S32 i = 0; i < Render::C_ViewManager::Instance().GetViewCount(); ++i)
		{
			const Display::C_View* pView = &Render::C_ViewManager::Instance().GetView(i)->m_View;
			bool bVisibleView = false;

			if(pView->GetUserIndex() != -1)
			{
				// if this block is within reasonable 2D distance then contribute its speed spawn scale
				Maths::HVec2 vCentre2D(vCentreOne.X(), vCentreOne.Z());
				Maths::HVec2 vBlock2D(pView->GetViewSettings3D().GetCameraPosition().X(), pView->GetViewSettings3D().GetCameraPosition().Z());
				if(Maths::Length2(vCentre2D - vBlock2D) < m_Settings.fDistanceFadeSq)
				{
					// nearby so use this view's speed scale
					pBlock->m_fSpeedSpawnScale = Maths::Max(fSpeedSpawnScale[i], pBlock->m_fSpeedSpawnScale);
				}

				if(pView->GetViewFrustum().CullAxisBox(vCentreOne, vSizeCull))
				{
					bVisible = true;
					m_VisBits[i].SetBit(iVisBit);
				}

				// if visible in either view calculate distance
				if(bVisible)
				{
					Maths::HVec3 vCentre(vCentreOne);
					Maths::HVec3 vCamera(pView->GetViewSettings3D().GetCameraPosition());
					Maths::HF32 fDistance = Maths::Length(vCentre - vCamera) * fDistanceFadeRcp;
					fMinDistance = Maths::Min(fMinDistance, fDistance);
				}
			}

		}

		pBlock->m_fDistanceSpawnScale = Maths::Constants<Maths::HF32>::One() - fMinDistance;
		pBlock->m_bVisible = bVisible;
	}
}

void C_SplashRendererBase::Display() const
{
	if(!g_debugSplashEnable || !m_bActive || !Initialised())
		return;

	PERF_EVENT("RainSplashes::Display");

#ifndef PLATFORM_PS3
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	Display::C_Float4 viewPosition = viewSettings3D.GetViewPosition();

	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	Display::Device()->SetFillMode(Display::E_FillMode_Solid);
	Display::Device()->SetBlendMode(Display::E_BlendMode_Additive);

	Display::Device()->SetTexture(0, m_pTexture);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);
	Display::Device()->SetVertexDeclaration(Display::Vertex_XYZ_HDR_UV::m_pDeclaration);
	Display::Device()->SetCullMode(Display::E_CullMode_None);

	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_VIEW_POSITION, (F32*)&viewPosition, 1);
	Display::Device()->SetVertexShaderConstantF(CV_EFFECT_RAIN_COLOURSCALE, (F32*)&m_Settings.fDistanceFade, 1);
	Display::Device()->SetVertexShaderConstantF(CV_EFFECT_RAIN_LIGHTING, (F32*)&m_vLighting, 1);

	Display::Device()->SetVertexShader(&m_VertexShader);
	Display::Device()->SetPixelShader(&m_PixelShader);

	S32 vertexCount = 0;
	S32 iViewIndex = Display::Device()->GetView()->m_Index;
	
	assert(m_VisBits[iViewIndex].Size() == m_Blocks.Size());

	Ut::Vector<const SplashBlock*> blocks;

	S32 iVisBit = 0;
	for(BlockList::ConstIterator pBlock = m_Blocks.Begin(); pBlock != m_Blocks.End(); ++pBlock, ++iVisBit)
	{
		if(!g_debugSplashCull || m_VisBits[iViewIndex].GetBitValue(iVisBit))
		{
			S32 vertexCountBlock = pBlock->m_Element.Size() * m_nNumPrimitives * m_nNumVertices;

			if(vertexCount + vertexCountBlock > 10000)
			{
				DisplayBlocks(blocks, vertexCount);
				blocks.Clear();
				vertexCount = 0;
			} // vertexCount + vertexCountBlock > 10000
			
			vertexCount += vertexCountBlock;
			blocks.PushBack(&(*pBlock));
		}
	}

	DisplayBlocks(blocks, vertexCount);


	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	
	if(g_debugSplashDebugGraphics)
	{
		Maths::HMat43 mTransform;
		iVisBit = 0;
		for(BlockList::ConstIterator pBlock = m_Blocks.Begin(); pBlock != m_Blocks.End(); ++pBlock, ++iVisBit)
		{
			if(m_VisBits[iViewIndex].GetBitValue(iVisBit))
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(Maths::HVec3((F32*)&pBlock->m_vMin), Maths::HVec3((F32*)&pBlock->m_vMax), mTransform, Maths::RGBA(255, 255, 255, 255));
			else
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(Maths::HVec3((F32*)&pBlock->m_vMin), Maths::HVec3((F32*)&pBlock->m_vMax), mTransform, Maths::RGBA(255, 0, 0, 255));
		}
	}
#endif
}

void C_SplashRendererBase::DisplayBlocks(const Ut::Vector<const SplashBlock*> &blocks, const S32 vertexCount) const
{
	if(vertexCount == 0)
		return;

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	const Display::C_Matrix& viewMatrix = viewSettings3D.GetViewMatrix();

	#ifdef EFFECT_RAIN_USE_HVEC
	Maths::HVec3 vRight(viewMatrix._11, viewMatrix._21, viewMatrix._31);
	Maths::HVec3 vUp(viewMatrix._12, viewMatrix._22, viewMatrix._32);
	Maths::HVec3 vForward(viewMatrix._13, viewMatrix._23, viewMatrix._33);
	Maths::HVec3 vTopRight = vRight + vUp;
	Maths::HVec3 vTopLeft = vUp - vRight;
	Maths::HVec3 vBottomRight = -vTopLeft;
	Maths::HVec3 vBottomLeft = -vTopRight;
	Maths::HF32 vOne = 1.0f;
	Maths::HF32 vSizeStart(m_Parameters.fSizeStart);
	Maths::HF32 vSizeDiff(m_Parameters.fSizeEnd - m_Parameters.fSizeStart);
	#else
	XMVECTOR vRight = XMVectorSet(viewMatrix._11, viewMatrix._21, viewMatrix._31, 0.0f);
	XMVECTOR vUp = XMVectorSet(viewMatrix._12, viewMatrix._22, viewMatrix._32, 0.0f);
	XMVECTOR vForward = XMVectorSet(viewMatrix._13, viewMatrix._23, viewMatrix._33, 0.0f);
	XMVECTOR vTopRight = XMVectorAdd(vRight, vUp);
	XMVECTOR vTopLeft = XMVectorSubtract(vUp, vRight);
	XMVECTOR vBottomRight = XMVectorNegate(vTopLeft);
	XMVECTOR vBottomLeft = XMVectorNegate(vTopRight);
	XMVECTOR vOne = XMVectorSplatOne();
	XMVECTOR vSizeStart = XMVectorReplicate(m_Parameters.fSizeStart);
	XMVECTOR vSizeDiff = XMVectorReplicate(m_Parameters.fSizeEnd - m_Parameters.fSizeStart);
	#endif

	Display::Vertex_XYZ_HDR_UV* pVertices = (Display::Vertex_XYZ_HDR_UV*)Display::Device()->BeginVertices(m_ePrimitiveType, vertexCount, sizeof(Display::Vertex_XYZ_HDR_UV), Display::Vertex_XYZ_HDR_UV::m_pDeclaration, true); // This can fail

	if(pVertices)
	{
		for(S32 i = 0; i < blocks.Size(); ++i)
		{
			const SplashBlock* pBlock = blocks[i];

			for(S32 a = 0; a < pBlock->m_Element.Size(); a++)
			{
				const SplashParticle* pParticle0 = &pBlock->m_Element[a];
				
		
#ifdef EFFECT_RAIN_USE_HVEC
				Maths::HVec4 vPositionLife(Maths::Load<Maths::HVec4>((MathsStorage::Float4A16*)&pParticle0->vPosition));	
				Maths::HVec3 vPosition(vPositionLife);
				
				Maths::HF32 vLife = vPositionLife.W();
				Maths::HF32 vOneMinusLife = vOne - vLife;

				Maths::HVec4 vForceSizeLife(Maths::Load<Maths::HVec4>((MathsStorage::Float4A16*)&pParticle0->vForceSizeLife));
				Maths::HF32 vSize = vForceSizeLife.X() * (vSizeStart + vOneMinusLife * vSizeDiff);

				Maths::HVec4 vColour(Maths::Load<Maths::HVec4>(&pParticle0->vColour));
				vColour *= vLife;
				
				Maths::HVec3 vPosition0 = vBottomLeft * vSize + vPosition;
				Maths::HVec3 vPosition1 = vBottomRight * vSize + vPosition;
				Maths::HVec3 vPosition2 = vTopRight * vSize + vPosition;
				Maths::HVec3 vPosition3 = vTopLeft * vSize + vPosition;
							
				// 0
				Maths::Store<Maths::HVec3>(vPosition0, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = 0;
				pVertices->v = 1;
				++pVertices;
				// 1
				Maths::Store<Maths::HVec3>(vPosition1, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = 1;
				pVertices->v = 1;
				++pVertices;
				// 2
				Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = 1;
				pVertices->v = 0;
				++pVertices;
				// 2
				Maths::Store<Maths::HVec3>(vPosition2, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = 1;
				pVertices->v = 0;
				++pVertices;
				// 3
				Maths::Store<Maths::HVec3>(vPosition3, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = 0;
				pVertices->v = 0;
				++pVertices;
				// 0
				Maths::Store<Maths::HVec3>(vPosition0, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::HVec4>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = 0;
				pVertices->v = 1;
				++pVertices;
#endif
			
#ifdef EFFECT_RAIN_USE_XMVECTOR
				XMVECTOR vPosition0 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle0->vPosition);	
				XMVECTOR vLife0 = XMVectorSplatW(vPosition0);
				XMVECTOR vOneMinusLife0 = XMVectorSubtract(vOne, vLife0);
				XMVECTOR vForceSizeLife0 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle0->vForceSizeLife);
				XMVECTOR vSize0 = XMVectorMultiply(XMVectorSplatX(vForceSizeLife0), XMVectorMultiplyAdd(vOneMinusLife0, vSizeDiff, vSizeStart));
				XMVECTOR vColour0 = XMLoadFloat4A16((XMFLOAT4A16*)&pParticle0->vColour);
				vColour0 = XMVectorMultiply(vLife0, vColour0);
				
				XMVECTOR vPosition00 = XMVectorMultiplyAdd(vBottomLeft, vSize0, vPosition0);
				XMVECTOR vPosition01 = XMVectorMultiplyAdd(vBottomRight, vSize0, vPosition0);
				XMVECTOR vPosition02 = XMVectorMultiplyAdd(vTopRight, vSize0, vPosition0);
				XMVECTOR vPosition03 = XMVectorMultiplyAdd(vTopLeft, vSize0, vPosition0);
				
				// particle 0
				// 0
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition00);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour0);
				pVertices->u = 0;
				pVertices->v = 1;
				++pVertices;
				// 1
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition01);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour0);
				pVertices->u = 1;
				pVertices->v = 1;
				++pVertices;
				// 2
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition02);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour0);
				pVertices->u = 1;
				pVertices->v = 0;
				++pVertices;
				// 3
				XMStoreFloat3((XMFLOAT3*)&pVertices->x, vPosition03);
				XMStoreFloat4NC((XMFLOAT4*)&pVertices->r, vColour0);
				pVertices->u = 0;
				pVertices->v = 0;
				++pVertices;	
#endif
			}
		}

		Display::Device()->EndVertices(pVertices);
	}
}


