#include "SplashCameraPch.h"
#include "SplashCamera/SplashCamera.h"
#include "display/shaders/Constants.h"
#include <SceneGraph/View.h>

#define SCENEGRAPH_ALLOW_INCLUDE // only for SplashCamera.cpp
#include <SceneGraph/SceneGraphImpl.h>

#include <TextureResource/TextureResource.h>
#include <resource_utils/ResourceBagUtils.h>

#include "misc_shared/Curve/HermiteCurve.h"

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

#include <scenegraph/Context.h>
#include <Display/Context/Context.h>
#include <SceneGraph/View.h>
#include <SceneGraph/Visitor.h>
#include <SceneGraph/Context.h>

EXTERN_SHADER(vsSplashCamera);
EXTERN_SHADER(vsSplashCameraRefract);
EXTERN_SHADER(psSplashCamera);
EXTERN_SHADER(psSplashCameraRefract);
EXTERN_SHADER(psSplashCameraLight);

//Debug vars
Debug::Bool	 gEnabled("GameSys.SplashCamera.Enable", true);
Debug::Float gScreenDiagonalFactor("GameSys.SplashCamera.ScreenDiagonalFactor", 1.f, Debug::Range(0.f, 3.f));
Debug::Float gCullCameraAngle("GameSys.SplashCamera.CullCameraAngle", 0.3f, Debug::Range(-1.f, 1.f));
Debug::Float gCullCameraDistanceStart("GameSys.SplashCamera.CullCameraDistanceStart", 10.0f, Debug::Range(0.f, 40.f));
Debug::Float gCullCameraDistanceEnd("GameSys.SplashCamera.CullCameraDistanceEnd", 20.0f, Debug::Range(0.f, 40.f));
Debug::Float gFovOffset("GameSys.SplashCamera.FovOffset", 70.0f, Debug::Range(0.f, 180.f));
Debug::Float gFovFactor("GameSys.SplashCamera.FovFactor", 100.0f, Debug::Range(0.f, 150.f));
Debug::Float gSpreadAngleFactor("GameSys.SplashCamera.SpreadAngleFactor", 200.0f, Debug::Range(0.f, 150.f));
Debug::Float gSlideFactor("GameSys.SplashCamera.SlideFactor", 0.1f, Debug::Range(0.f, 1.f));
Debug::Bool	 gReloadTextures("GameSys.SplashCamera.ReloadTextures", false);


void SplashTypeEventHandler(XtEventType event, XtRecord* record, SplashCamera::C_SplashType* object, XtCallbackContext*)
{
	if(event == kXtEtConstruct)
	{
		SplashCamera::C_HermiteKeyFrame keyFrame;

		//Fade curve
		object->m_FadeCurve.Clear();
		for(int j = 0; j < object->m_FadeGraph.count; j++)
		{
			keyFrame = object->m_FadeGraph[j];
			object->m_FadeCurve.AddKeyFrame(keyFrame.mTime, keyFrame.mValue, keyFrame.mInTangent,	keyFrame.mOutTangent);
		}
		//Distribution Fade curve
		object->m_DistributionFadeCurve.Clear();
		for(int j = 0; j < object->m_DistributionFadeGraph.count; j++)
		{
			keyFrame = object->m_DistributionFadeGraph[j];
			object->m_DistributionFadeCurve.AddKeyFrame(keyFrame.mTime, keyFrame.mValue, keyFrame.mInTangent,	keyFrame.mOutTangent);
		}
		//Speed curve
		object->m_SpeedCurve.Clear();
		for(int j = 0; j < object->m_SpeedGraph.count; j++)
		{
			keyFrame = object->m_SpeedGraph[j];
			object->m_SpeedCurve.AddKeyFrame(keyFrame.mTime, keyFrame.mValue * object->m_SpeedFactor, keyFrame.mInTangent,	keyFrame.mOutTangent);
		}
		//Distribution Speed curve
		object->m_DistributionSpeedCurve.Clear();
		for(int j = 0; j < object->m_DistributionSpeedGraph.count; j++)
		{
			keyFrame = object->m_DistributionSpeedGraph[j];
			object->m_DistributionSpeedCurve.AddKeyFrame(keyFrame.mTime, keyFrame.mValue * object->m_SpeedFactor, keyFrame.mInTangent,	keyFrame.mOutTangent);
		}

	}
}

//Sorcery Record
XT_TYPE(SplashCamera::C_HermiteKeyFrame,	"SCHermiteKeyFrame")
	XT_FIELD(mTime,							"Time")
	XT_FIELD(mValue,						"Value")
	XT_FIELD(mInTangent,					"InTangent")
	XT_FIELD(mOutTangent,					"OutTangent")
XT_END_TYPE

XT_TYPE(SplashCamera::C_HermiteCurve,		"SCHermiteCurve")
	XT_FIELD(mKeyFrames,					"KeyFrames")
XT_END_TYPE

XT_TYPE(SplashCamera::C_SplashType,	"SplashType")
	XT_CALLBACK(SplashTypeEventHandler)
	XT_FIELD(m_LifeTime,					"LifeTime")
	XT_FIELD(m_DistributionLifeTime,		"DistributionLifeTime")
	XT_FIELD(m_InitScreenCentrePosition,	"InitScreenCentrePosition")
	XT_FIELD(m_InitScreenVerticalPosition,	"InitScreenVerticalPosition")
	XT_FIELD(m_InitOverCarFactor,			"InitOverCarFactor")
	XT_FIELD(m_Size,						"Size")
	XT_FIELD(m_DistributionSize,			"DistributionSize")
	XT_FIELD(m_Rotate,						"Rotate")
	XT_FIELD(m_DistributionRotate,			"DistributionRotate")
	XT_FIELD(m_FadeGraph,					"FadeGraph")
	XT_FIELD(m_DistributionFadeGraph,		"DistributionFadeGraph")
	XT_FIELD(m_SpeedGraph,					"SpeedGraph")
	XT_FIELD(m_DistributionSpeedGraph,		"DistributionSpeedGraph")
	XT_FIELD(m_SpeedFactor,					"SpeedFactor")
	XT_FIELD(m_SplashTextureName,			"SplashTextureName")
	XT_FIELD(m_DistorsionTextureName,		"DistorsionTextureName")
	XT_FIELD(m_RefractionIntensity,			"RefractionIntensity")
	XT_FIELD(m_DistorsionIntensity,			"DistorsionIntensity")
	XT_FIELD(m_NormalTextureName,			"NormalTextureName")
	XT_FIELD(m_AmbientIntensity,			"AmbientIntensity")
	XT_FIELD(m_LightIntensity,				"LightIntensity")
XT_END_TYPE

XT_LIBRARY(CAMERASPLASH)
	XT_REGISTER_TYPE(SplashCamera::C_HermiteKeyFrame)
	XT_REGISTER_TYPE(SplashCamera::C_HermiteCurve)
	XT_REGISTER_TYPE(SplashCamera::C_SplashType)
XT_END_LIBRARY

namespace SplashCamera
{
	Debug::Logger &g_SplashCameraLog( Debug::GetLogger("gamesys.SplashCamera") );

	typedef Display::Vertex_XY_C_UV Vertex;

	C_SplashCamera::C_SplashCamera() : Display::I_Renderer("SplashCamera")
	{
		m_bCreated = false;
		m_pRefractionTexture = NULL;
	}

	C_SplashCamera::~C_SplashCamera()
	{
		DestroyRenderer();
	}

	bool C_SplashCamera::OnCreate()
	{
		if (m_bCreated)
			return true;

		RESOURCE_MEMORY_ID("SplashCamera");

		m_SamplerState.SetDefaultStates();
		m_SamplerState.SetTextureFilter(Display2::E_TextureFilter_Linear);
		m_SamplerState.SetTextureAddress(Display2::E_TextureAddress_Clamp);
#if PLATFORM_PS3
		m_SamplerState.SetTextureGamma(0);
#endif

		m_MirrorSamplerState.SetDefaultStates();
		m_MirrorSamplerState.SetTextureFilter(Display2::E_TextureFilter_Linear);
		m_MirrorSamplerState.SetTextureAddress(Display2::E_TextureAddress_Mirror);
#if PLATFORM_PS3
		m_MirrorSamplerState.SetTextureGamma(0);
#endif

		if (!Display::CreateVertexShaderFromFunc("Shader/vsSplashCamera", 0, &m_SplashCameraVertexShader, REF_SHADER(vsSplashCamera)))
			return false;

		if (!Display::CreatePixelShaderFromFunc("Shader/psSplashCamera", 0, &m_SplashCameraPixelShader, REF_SHADER(psSplashCamera)))
			return false;

		if (!Display::CreateVertexShaderFromFunc("Shader/vsSplashCameraRefract", 0, &m_SplashCameraRefractVertexShader, REF_SHADER(vsSplashCameraRefract)))
			return false;

		if (!Display::CreatePixelShaderFromFunc("Shader/psSplashCameraRefract", 0, &m_SplashCameraRefractPixelShader, REF_SHADER(psSplashCameraRefract)))
			return false;
		
		if (!Display::CreatePixelShaderFromFunc("Shader/psSplashCameraLight", 0, &m_SplashCameraLightPixelShader, REF_SHADER(psSplashCameraLight)))
			return false;

		//Parameter
		m_SplashCameraRefractParameter = m_SplashCameraRefractPixelShader.GetParameter(0, "g_RefractionParams");

		m_SplashCameraLightParameter = m_SplashCameraLightPixelShader.GetParameter(0, "g_LightParams");
		m_SplashCameraLightCameraMatrixXParameter = m_SplashCameraLightPixelShader.GetParameter(1, "cameraMatrixX");
		m_SplashCameraLightCameraMatrixYParameter = m_SplashCameraLightPixelShader.GetParameter(2, "cameraMatrixY");
		m_SplashCameraLightCameraMatrixZParameter = m_SplashCameraLightPixelShader.GetParameter(3, "cameraMatrixZ");

		m_bCreated = true;

		return true;
	}

	void C_SplashCamera::OnDestroy()
	{
		Display::Device()->Destroy(&m_SplashCameraVertexShader);
		Display::Device()->Destroy(&m_SplashCameraPixelShader);
		Display::Device()->Destroy(&m_SplashCameraRefractVertexShader);
		Display::Device()->Destroy(&m_SplashCameraRefractPixelShader);
		Display::Device()->Destroy(&m_SplashCameraLightPixelShader);

		//Delete all splashes
		for (S32 i = 0; i < m_ViewSettings.Size(); i++)
		{
			for (S32 j = 0; j < m_ViewSettings[i].m_FastAccessSplash.Size(); j++)
				m_ViewSettings[i].m_FastAccessSplash[j].m_SplashList.Clear();

			m_ViewSettings[i].m_FastAccessSplash.Clear();
		}
		
		m_bCreated = false;
	}

	// scenegraph I_GatherNode implementation
	void C_SplashCamera::OnGather(const SceneGraph::C_View& view) const
	{
		if (!gEnabled) return;

		if (view.GetTypeFlag() & m_VisibilityFlags)
		{
			//Something to draw
			const C_ViewSettings& viewSettings = m_ViewSettings[view.GetType()];
			for (S32 j = 0; j < viewSettings.m_FastAccessSplash.Size(); j++)
			{
				if (viewSettings.m_FastAccessSplash[j].m_SplashList.Size() > 0)
				{
					view.AddVisibleNode(this, m_RenderPass);
					return;
				}
			}
		}
	}
	
	void C_SplashCamera::Accept(SceneGraph::I_Visitor& visitor) const
	{
		visitor.Visit(*this);
	}

	void C_SplashCamera::OnRender(SceneGraph::C_Context& context) const
	{
		Render(context);
	}
	
	void C_SplashCamera::Render(SceneGraph::C_Context& context) const
	{
		PERF_EVENT("SplashCamera.Render");

		Display::C_Context* pDisplay = Display::GetContext(&context);
		context.SetContextFlags(pDisplay->GetFlags());

		const SceneGraph::C_View& view = *context.GetView();

		//Setup states
		pDisplay->SetBlendEnable(Display::E_Bool_True);
		pDisplay->SetBlendState(Display::E_Blend_One, Display::E_Blend_InvSrcAlpha, Display::E_BlendOp_Add, Display::E_Blend_Zero, Display::E_Blend_InvSrcAlpha, Display::E_BlendOp_Add);
		pDisplay->SetAlphaTestState(Display::E_Bool_True, 0); 
		pDisplay->SetBlendHighPrecisionEnable(Display::E_Bool_True);
		pDisplay->SetDepthEnable(Display::E_Bool_False);
		pDisplay->SetDepthWriteEnable(Display::E_Bool_False);
		pDisplay->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
		pDisplay->SetCullMode(Display::E_CullMode_None);
		//Set Params
		
		//Set vertex and pixel shaders
		pDisplay->SetVertexDeclaration(Vertex::m_pDeclaration);
#ifdef PLATFORM_PS3
		pDisplay->SetVtxAttrUsageFlags(UINT_MAX);
#endif

		S32 viewIndex = view.GetType();
		const C_ViewSettings& viewSettings = m_ViewSettings[view.GetType()];

		//Calculate FOV factor
		F32 fovFactor;
		fovFactor = Maths::Clamp(1.f + (Maths::RadToDeg(view.GetViewSettings3D().GetFieldOfView()) - gFovOffset) / gFovFactor, 0.4f, 1.2f);

		for (S32 j = 0; j < viewSettings.m_FastAccessSplash.Size(); j++)
		{
			const SplashList& splashList = viewSettings.m_FastAccessSplash[j].m_SplashList;
			const C_SplashType* pSplashType = viewSettings.m_FastAccessSplash[j].m_pSplashType;

			S32 splashCount = splashList.Size();

			if (splashCount == 0)
				continue;
			

			S32 vertexCount = 4 * splashCount;

			//Set Texture for splash group
			const TextureResource::C_TextureResource* pTextureResource = pSplashType->m_SplashTextureHandle;
			const Display::C_Texture2D* pSplashTexture = NULL;
			if (pTextureResource && pTextureResource->IsDataValid())
			{
				pSplashTexture = (const Display::C_Texture2D*) pTextureResource->GetDisplayTexture();
			}
			const TextureResource::C_TextureResource* pDistorsionTextureResource = pSplashType->m_DistorsionTextureHandle;
			const Display::C_Texture2D* pDistorsionTexture = NULL;
			
			//Check if refraction with slipscreen and full refraction
			if (!m_pRefractionTexture && pDistorsionTextureResource && pDistorsionTextureResource->IsDataValid() && (pSplashType->m_RefractionIntensity > 0.01f))
			{
				if (pSplashType->m_RefractionIntensity > 0.98f)
					continue;
			}

			pDisplay->SetTexture(0, pSplashTexture ? pSplashTexture->GetGPUTexture() : NULL);
			pDisplay->SetSamplerState(0, &m_SamplerState);

			if (m_pRefractionTexture && pDistorsionTextureResource && pDistorsionTextureResource->IsDataValid() && (pSplashType->m_RefractionIntensity > 0.01f))
			{
				pDistorsionTexture = (const Display::C_Texture2D*) pDistorsionTextureResource->GetDisplayTexture();

				pDisplay->SetTexture(1, m_pRefractionTexture ? m_pRefractionTexture->GetGPUTexture() : NULL);
				pDisplay->SetSamplerState(1, &m_MirrorSamplerState);

				pDisplay->SetTexture(2, pDistorsionTexture ? pDistorsionTexture->GetGPUTexture() : NULL);
				pDisplay->SetSamplerState(2, &m_SamplerState);

				Display::C_SamplerUsageFlags flags;
				flags.SetEnabled(0, 0);
				flags.SetEnabled(0, 1);
				flags.SetEnabled(0, 2);
				pDisplay->SetSamplerUsageFlags(&flags);


				MathsStorage::Float4 refractionParams(pSplashType->m_RefractionIntensity, pSplashType->m_DistorsionIntensity, 0.f, 0.f);
				pDisplay->SetPixelShaderParameterF(m_SplashCameraRefractParameter, (F32*)&refractionParams);

				//Set refract shaders
				pDisplay->SetVertexShader(&m_SplashCameraRefractVertexShader);
				pDisplay->SetPixelShader(&m_SplashCameraRefractPixelShader);
				pDisplay->SetPixelShaderParameters(&m_SplashCameraRefractPixelShader);
			}
			else
			{
				const TextureResource::C_TextureResource* pNormalTextureResource = pSplashType->m_NormalTextureHandle;
				const Display::C_Texture2D* pNormalTexture = NULL;
				if (pNormalTextureResource && pNormalTextureResource->IsDataValid() && (pSplashType->m_LightIntensity > 0.f) && viewSettings.m_pLightCubeTexture)
				{
					pNormalTexture = (const Display::C_Texture2D*) pNormalTextureResource->GetDisplayTexture();

					pDisplay->SetTexture(1, pNormalTexture ? pNormalTexture->GetGPUTexture() : NULL);
					pDisplay->SetSamplerState(1, &m_SamplerState);

					pDisplay->SetTexture(2, viewSettings.m_pLightCubeTexture->GetGPUTexture() );
					pDisplay->SetSamplerState(2, &m_SamplerState);

					Display::C_SamplerUsageFlags flags;
					flags.SetEnabled(0, 0);
					flags.SetEnabled(0, 1);
					flags.SetEnabled(0, 2);
					pDisplay->SetSamplerUsageFlags(&flags);

					//set light shaders
					MathsStorage::Float4	lightParams(pSplashType->m_AmbientIntensity, pSplashType->m_LightIntensity, 0.f, 0.f);
					pDisplay->SetPixelShaderParameterF(m_SplashCameraLightParameter, (F32*)&lightParams);

					MathsStorage::Float4	cameraMatrixXParams;
					Maths::Store(viewSettings.m_CameratoWorldMatrix.AX() || Maths::HVecW(1.f), &cameraMatrixXParams);
					pDisplay->SetPixelShaderParameterF(m_SplashCameraLightCameraMatrixXParameter, (F32*)&cameraMatrixXParams);
					MathsStorage::Float4	cameraMatrixYParams;
					Maths::Store(viewSettings.m_CameratoWorldMatrix.AY() || Maths::HVecW(1.f), &cameraMatrixYParams);
					pDisplay->SetPixelShaderParameterF(m_SplashCameraLightCameraMatrixYParameter, (F32*)&cameraMatrixYParams);
					MathsStorage::Float4	cameraMatrixZParams;
					Maths::Store(viewSettings.m_CameratoWorldMatrix.AZ() || Maths::HVecW(1.f), &cameraMatrixZParams);
					pDisplay->SetPixelShaderParameterF(m_SplashCameraLightCameraMatrixZParameter, (F32*)&cameraMatrixZParams);

					pDisplay->SetVertexShader(&m_SplashCameraRefractVertexShader); //Same shaders that refract
					pDisplay->SetPixelShader(&m_SplashCameraLightPixelShader);
					pDisplay->SetPixelShaderParameters(&m_SplashCameraLightPixelShader);
				}
				else
				{
					Display::C_SamplerUsageFlags flags;
					flags.SetEnabled(0, 0);
					pDisplay->SetSamplerUsageFlags(&flags);

					//set not refract shaders
					pDisplay->SetVertexShader(&m_SplashCameraVertexShader);
					pDisplay->SetPixelShader(&m_SplashCameraPixelShader);
				}
			}


		#ifndef PLATFORM_PC
			Vertex* pVertices = (Vertex*)pDisplay->BeginVertices(Display::E_PrimitiveType_QuadList, vertexCount, sizeof(Vertex), Vertex::m_pDeclaration);

			if(pVertices)
			{
				float alphaFactor = 1.f;
 				if (!m_pRefractionTexture && pDistorsionTextureResource && pDistorsionTextureResource->IsDataValid() && (pSplashType->m_RefractionIntensity > 0.01f))
 				{
 					alphaFactor = 1.f - pSplashType->m_RefractionIntensity;
 				}
				SplashList::ConstIterator it = splashList.Begin();
				for (S32 i = 0; i < splashCount; i++)
				{
					const C_Splash& splash = *it;
					
					Maths::HVec2 splashAxisY(splash.m_Direction.X() * Maths::Cos(splash.m_Rotate) + splash.m_Direction.Y() * Maths::Sin(splash.m_Rotate),
											-splash.m_Direction.X() * Maths::Sin(splash.m_Rotate) + splash.m_Direction.Y() * Maths::Cos(splash.m_Rotate));
					Maths::HVec2 splashAxisX(splashAxisY.Y(), -splashAxisY.X());

					splashAxisX *= splash.m_Size;
					splashAxisY *= splash.m_Size;

					Maths::HVec2 position = Maths::HVec2((splash.m_Position.X() / fovFactor), (splash.m_Position.Y() / fovFactor));

					//Fill vertex
					Maths::Store(position - splashAxisX - splashAxisY, (MathsStorage::Float2*)&pVertices[0].x);
					pVertices[0].c = GPU_U8_4_TO_U32(0,0,0,U8(Maths::Clamp(splash.m_Fade * alphaFactor * 255.f, 0.f, 255.f)));
					pVertices[0].u = 0.f;
					pVertices[0].v = F32(splash.m_TextureArrayIndex) / F32(pSplashType->m_TextureArraySize);

					Maths::Store(position - splashAxisX + splashAxisY, (MathsStorage::Float2*)&pVertices[1].x);
					pVertices[1].c = GPU_U8_4_TO_U32(0,0,0,U8(Maths::Clamp(splash.m_Fade * alphaFactor * 255.f, 0.f, 255.f)));
					pVertices[1].u = 1.f;
					pVertices[1].v = F32(splash.m_TextureArrayIndex) / F32(pSplashType->m_TextureArraySize);

					Maths::Store(position + splashAxisX + splashAxisY, (MathsStorage::Float2*)&pVertices[2].x);
					pVertices[2].c = GPU_U8_4_TO_U32(0,0,0,U8(Maths::Clamp(splash.m_Fade * alphaFactor * 255.f, 0.f, 255.f)));
					pVertices[2].u = 1.f;
					pVertices[2].v = F32(splash.m_TextureArrayIndex + 1) / F32(pSplashType->m_TextureArraySize);

					Maths::Store(position + splashAxisX - splashAxisY, (MathsStorage::Float2*)&pVertices[3].x);
					pVertices[3].c = GPU_U8_4_TO_U32(0,0,0,U8(Maths::Clamp(splash.m_Fade * alphaFactor * 255.f, 0.f, 255.f)));
					pVertices[3].u = 0.f;
					pVertices[3].v = F32(splash.m_TextureArrayIndex + 1) / F32(pSplashType->m_TextureArraySize);	
			
					pVertices += 4;
					++it;
				}
				pDisplay->EndVertices();
			}
		#else
			// [Blur2PC] T107: retail PC (Blur.exe 1.2, C_SplashCamera::Render 0x00b2a5b0) draws the splashes through
			// the device's dynamic vertices as a triangle list, six vertices per splash: the quad's corners 0, 1, 2
			// and 3, 0, 2. Retail writes the repeated corners 0 and 2 with the fade alone, without alphaFactor;
			// kept as retail has it.
			Vertex* pVertices = (Vertex*)Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleList, 6 * splashCount, sizeof(Vertex), Vertex::m_pDeclaration);

			if(pVertices)
			{
				float alphaFactor = 1.f;
				if (!m_pRefractionTexture && pDistorsionTextureResource && pDistorsionTextureResource->IsDataValid() && (pSplashType->m_RefractionIntensity > 0.01f))
				{
					alphaFactor = 1.f - pSplashType->m_RefractionIntensity;
				}
				SplashList::ConstIterator it = splashList.Begin();
				for (S32 i = 0; i < splashCount; i++)
				{
					const C_Splash& splash = *it;

					Maths::HVec2 splashAxisY(splash.m_Direction.X() * Maths::Cos(splash.m_Rotate) + splash.m_Direction.Y() * Maths::Sin(splash.m_Rotate),
											-splash.m_Direction.X() * Maths::Sin(splash.m_Rotate) + splash.m_Direction.Y() * Maths::Cos(splash.m_Rotate));
					Maths::HVec2 splashAxisX(splashAxisY.Y(), -splashAxisY.X());

					splashAxisX *= splash.m_Size;
					splashAxisY *= splash.m_Size;

					Maths::HVec2 position = Maths::HVec2((splash.m_Position.X() / fovFactor), (splash.m_Position.Y() / fovFactor));

					const U32 colour = GPU_U8_4_TO_U32(0,0,0,U8(Maths::Clamp(splash.m_Fade * alphaFactor * 255.f, 0.f, 255.f)));
					const U32 colourRepeated = GPU_U8_4_TO_U32(0,0,0,U8(Maths::Clamp(splash.m_Fade * 255.f, 0.f, 255.f)));
					const F32 v0 = F32(splash.m_TextureArrayIndex) / F32(pSplashType->m_TextureArraySize);
					const F32 v1 = F32(splash.m_TextureArrayIndex + 1) / F32(pSplashType->m_TextureArraySize);

					Maths::Store(position - splashAxisX - splashAxisY, (MathsStorage::Float2*)&pVertices[0].x);
					pVertices[0].c = colour;
					pVertices[0].u = 0.f;
					pVertices[0].v = v0;

					Maths::Store(position - splashAxisX + splashAxisY, (MathsStorage::Float2*)&pVertices[1].x);
					pVertices[1].c = colour;
					pVertices[1].u = 1.f;
					pVertices[1].v = v0;

					Maths::Store(position + splashAxisX + splashAxisY, (MathsStorage::Float2*)&pVertices[2].x);
					pVertices[2].c = colour;
					pVertices[2].u = 1.f;
					pVertices[2].v = v1;

					Maths::Store(position + splashAxisX - splashAxisY, (MathsStorage::Float2*)&pVertices[3].x);
					pVertices[3].c = colour;
					pVertices[3].u = 0.f;
					pVertices[3].v = v1;

					Maths::Store(position - splashAxisX - splashAxisY, (MathsStorage::Float2*)&pVertices[4].x);
					pVertices[4].c = colourRepeated;
					pVertices[4].u = 0.f;
					pVertices[4].v = v0;

					Maths::Store(position + splashAxisX + splashAxisY, (MathsStorage::Float2*)&pVertices[5].x);
					pVertices[5].c = colourRepeated;
					pVertices[5].u = 1.f;
					pVertices[5].v = v1;

					pVertices += 6;
					++it;
				}
				Display::Device()->EndVertices();
			}
		#endif
		}

		pDisplay->SetBlendMode(Display::E_BlendMode_Unknown);
		pDisplay->SetTexture(0, NULL);
		pDisplay->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);
	}

	void C_SplashCamera::AddSplash( U32 viewIndex, C_SplashType* pSplashType, U32 count, const Maths::HVec3& position, const Maths::HVec3& velocity, F32 spread, const ResourceUtils::I_ResourceBag* pResourceBag, bool distanceFadeDisabled )
	{
		PERF_EVENT("SplashCamera.AddSplash");

		BIZ_ASSERT(S32(viewIndex) < m_ViewSettings.Size());
		BIZ_ASSERT(pSplashType);

		C_ViewSettings& viewSettings = m_ViewSettings[viewIndex];

		F32 initCentrePosition = Maths::Clamp(pSplashType->m_InitScreenCentrePosition, 0.01f, 0.99f);
		Maths::HVec3 splash3Ddirection = position - viewSettings.m_CameratoWorldMatrix.AW();
		F32 distance = splash3Ddirection.Length();
		splash3Ddirection.Normalise();
		Maths::HVec3 velocityVector = velocity; 
		velocityVector.Normalise();
		F32 factor = (splash3Ddirection | -velocityVector);
		if (factor < gCullCameraAngle)
			return;

		spread += (1.f - ((factor - gCullCameraAngle) / (1.f - gCullCameraAngle))) * gSpreadAngleFactor;

		F32 fadeDistantFactor = 1.f;
		if (!distanceFadeDisabled)
		{
			//Calculate fade factor distant related
			fadeDistantFactor = Maths::Clamp((gCullCameraDistanceEnd - distance) / (gCullCameraDistanceEnd - gCullCameraDistanceStart), 0.f, 1.f);
			if (fadeDistantFactor < 0.01f)
				return;
		}
		fadeDistantFactor *= ((factor - gCullCameraAngle) / (1.f - gCullCameraAngle));

		//Get Texture data
		TextureResource::C_TextureResource* pTextureResource = pSplashType->m_SplashTextureHandle;
		if (!pTextureResource || !pTextureResource->IsDataValid() || gReloadTextures)
		{
			pSplashType->m_SplashTextureHandle = pResourceBag->GetResource<TextureResource::C_TextureResource>( MakeResourceBagTag(pSplashType->m_SplashTextureName.Text()));
			pTextureResource = pSplashType->m_SplashTextureHandle;
		}

		TextureResource::C_TextureResource* pDistorsionTextureResource = pSplashType->m_DistorsionTextureHandle;
		if (!pDistorsionTextureResource || !pDistorsionTextureResource->IsDataValid() || gReloadTextures)
		{
			pSplashType->m_DistorsionTextureHandle = pResourceBag->GetResource<TextureResource::C_TextureResource>( MakeResourceBagTag(pSplashType->m_DistorsionTextureName.Text()));
		}

		TextureResource::C_TextureResource* pNormalTextureResource = pSplashType->m_NormalTextureHandle;
		if (!pNormalTextureResource || !pNormalTextureResource->IsDataValid() || gReloadTextures)
		{
			pSplashType->m_NormalTextureHandle = pResourceBag->GetResource<TextureResource::C_TextureResource>( MakeResourceBagTag(pSplashType->m_NormalTextureName.Text()));
		}
		
		if (pTextureResource && pTextureResource->GetDisplayTexture())
		{
			U32 width = pTextureResource->GetDisplayTexture()->GetWidth();
			U32 height = pTextureResource->GetDisplayTexture()->GetHeight();
			//Calculate ratio
			U32 ratio = height / width;
			if (ratio == 0 || (height%width) != 0)
			{
				LOGWARN(g_SplashCameraLog, "Splash texture %s needs to be an array of textures, Size(x,nx)", pSplashType->m_SplashTextureName.Text());
				ratio = 1;
			}
			pSplashType->m_TextureArraySize = ratio;
		}
		else
		{
			LOGWARN(g_SplashCameraLog, "Error loading splash texture %s", pSplashType->m_SplashTextureName.Text());
			return;
		}

		SplashList* pSplashList = NULL;
		//Look for fast access
		for (S32 j = 0; j < viewSettings.m_FastAccessSplash.Size(); j++)
		{
			if (viewSettings.m_FastAccessSplash[j].m_pSplashType == pSplashType)
			{
				pSplashList = &viewSettings.m_FastAccessSplash[j].m_SplashList;
			}
		}
		if (pSplashList == NULL)
		{
			//Create new
			viewSettings.m_FastAccessSplash.PushBack();
			viewSettings.m_FastAccessSplash.Back().m_pSplashType = pSplashType;
			pSplashList = &viewSettings.m_FastAccessSplash.Back().m_SplashList;
		}
		SplashList& splashList = *pSplashList;

		for (U32 i = 0; i < count; i++)
		{
			//Calculate init position
			F32 initDistance = powf(m_RandomGenerator.FRand(0.f, 1.f - initCentrePosition), 1.f - initCentrePosition) + initCentrePosition;

			splash3Ddirection = velocity +  Maths::HVec3(m_RandomGenerator.FRand(-spread, spread), m_RandomGenerator.FRand(-spread, spread), m_RandomGenerator.FRand(-spread, spread));
			splash3Ddirection.Normalise();
			Maths::HVec3 splashCameraDirection = splash3Ddirection * Maths::HMat33(viewSettings.m_WorldtoCameraMatrix);
			Maths::HVec2 splash2DCameraDirection = Maths::HVec2(splashCameraDirection.X(), -splashCameraDirection.Y());
			splash2DCameraDirection.Normalise();

			//Create splash
			splashList.PushBack();
			C_Splash* pSplash = &splashList.Back();

			splash2DCameraDirection.Y((splash2DCameraDirection.Y() - pSplashType->m_InitScreenVerticalPosition) / (1.f + pSplashType->m_InitScreenVerticalPosition));
			splash2DCameraDirection.X(splash2DCameraDirection.X() * (1.f + Maths::Max(initCentrePosition + pSplashType->m_InitOverCarFactor, 0.f) * Maths::Clamp(F32(-splash2DCameraDirection.Y()), 0.f, 1.f)));
			splash2DCameraDirection.Normalise();

			pSplash->m_Position = splash2DCameraDirection * initDistance * gScreenDiagonalFactor;

			pSplash->m_Direction = splash2DCameraDirection;
			pSplash->m_Rotate = pSplashType->m_Rotate + m_RandomGenerator.FRand(-1.f, 1.f) * pSplashType->m_DistributionRotate;
			pSplash->m_Size = Maths::Max(0.f, pSplashType->m_Size + m_RandomGenerator.FRand(-1.f, 1.f) * pSplashType->m_DistributionSize);
			pSplash->m_LifeTime = Maths::Max(0.f, pSplashType->m_LifeTime + m_RandomGenerator.FRand(-1.f, 1.f) * pSplashType->m_DistributionLifeTime);
			pSplash->m_DistributionFadeDeviation = m_RandomGenerator.FRand(-1.f, 1.f);
			pSplash->m_DistributionSpeedDeviation = m_RandomGenerator.FRand(-1.f, 1.f);
			pSplash->m_Time = 0.f;
			pSplash->m_DistanceFade = fadeDistantFactor;

			//Texture
			pSplash->m_TextureArrayIndex = m_RandomGenerator.Rand(pSplashType->m_TextureArraySize);
		}
	}	

	void C_SplashCamera::Update(const U32 viewIndex,  const F32 timeStep, const Maths::HMat43& cameraMatrix, const Maths::HVec3& cameraSpeed )
	{
		PERF_EVENT("SplashCamera.Update");

		BIZ_ASSERT(S32(viewIndex) < m_ViewSettings.Size());
		
		C_ViewSettings& viewSettings = m_ViewSettings[viewIndex];
		
		viewSettings.m_CameratoWorldMatrix = cameraMatrix;
		viewSettings.m_WorldtoCameraMatrix = cameraMatrix.InverseRotTrans();
		viewSettings.m_CameraSpeed = cameraSpeed;
		viewSettings.m_CameraSpeedFactor = Maths::Clamp(F32(cameraSpeed | cameraMatrix.AZ()), 0.f, 1.f);
		Maths::HVec3 slideVector = cameraSpeed * Maths::HMat33(viewSettings.m_WorldtoCameraMatrix);
		slideVector = slideVector * gSlideFactor;
		viewSettings.m_CameraSlideFactor = Maths::HVec2(-slideVector.X(), -slideVector.Y());

		for (S32 j = 0; j < viewSettings.m_FastAccessSplash.Size(); j++)
		{
			SplashList& splashList = viewSettings.m_FastAccessSplash[j].m_SplashList;
			C_SplashType* pSplashType = viewSettings.m_FastAccessSplash[j].m_pSplashType;
			SplashList::Iterator it = splashList.Begin();
			while (it != splashList.End())
			{
				C_Splash& splash = *it;

				if (!UpdateSplash(viewSettings, timeStep, &splash, pSplashType))
				{
					//It is dead, remove
					it = splashList.Erase(it);	
				}
				else
					++it;
			}
		}
	}

	bool C_SplashCamera::UpdateSplash( const C_ViewSettings& viewSettings, const F32& timeStep, C_Splash* pSplash, C_SplashType* pSplashType )
	{
		pSplash->m_Time += timeStep;

		if (pSplash->m_Time < pSplash->m_LifeTime)
		{
			F32 factor = pSplash->m_Time/pSplash->m_LifeTime;

			//Update data
			F32 speed = Maths::Max(0.f, pSplashType->m_SpeedCurve.Evaluate(factor) + pSplash->m_DistributionSpeedDeviation * pSplashType->m_DistributionSpeedCurve.Evaluate(factor));
			pSplash->m_Position += (pSplash->m_Direction * speed * timeStep * viewSettings.m_CameraSpeedFactor + viewSettings.m_CameraSlideFactor * speed * timeStep);
			pSplash->m_Fade = Maths::Clamp((pSplashType->m_FadeCurve.Evaluate(factor) + pSplash->m_DistributionFadeDeviation * pSplashType->m_DistributionFadeCurve.Evaluate(factor)) * pSplash->m_DistanceFade, 0.f, 1.f);

			return true;
		}
		else
		{
			//Remove
			return false;
		}
	}

	void C_SplashCamera::Clear()
	{
		//Delete all splashes
		for (S32 i = 0; i < m_ViewSettings.Size(); i++)
		{
			for (S32 j = 0; j < m_ViewSettings[i].m_FastAccessSplash.Size(); j++)
				m_ViewSettings[i].m_FastAccessSplash[j].m_SplashList.Clear();
		}
	}

	void C_SplashCamera::SetLightCubeTexture( U32 viewIndex, const Display::C_TextureCube* pLightCubeTexture )
	{
		BIZ_ASSERT(S32(viewIndex) < m_ViewSettings.Size());
		m_ViewSettings[viewIndex].m_pLightCubeTexture = pLightCubeTexture;
	}

	void C_SplashType::AddToResourceBag( ResourceUtils::I_ResourceBag& pResourceBag ) const
	{
		if (strlen(m_SplashTextureName.Text()) > 0)
			pResourceBag.AddResource( MakeResourceBagTag(m_SplashTextureName.Text()), m_SplashTextureName.Text(), TextureResource::C_TextureResource::StaticGetResourceTypeId());
		if (strlen(m_DistorsionTextureName.Text()) > 0)
			pResourceBag.AddResource( MakeResourceBagTag(m_DistorsionTextureName.Text()), m_DistorsionTextureName.Text(), TextureResource::C_TextureResource::StaticGetResourceTypeId());
		if (strlen(m_NormalTextureName.Text()) > 0)
			pResourceBag.AddResource( MakeResourceBagTag(m_NormalTextureName.Text()), m_NormalTextureName.Text(), TextureResource::C_TextureResource::StaticGetResourceTypeId());
	}
}
