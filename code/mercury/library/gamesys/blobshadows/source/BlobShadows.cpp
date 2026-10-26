#include "stdafx.h"
#include "blobshadows/BlobShadows.h"
#include <world_render/ModelInstance.h>	// [Blur2PC] Blur2PCBlendMatrix
#include <renderingdata/renderingdata.h>
#include <renderingdata/context.h>
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Core/Maths/HVec/New/IncludeMinimal.h"
#include "Core/Maths/HVec/New/IncludeMinimal.inl"
#include "Display/Buffer/VertexDeclarations.h"
#include "System/debug/I_DebugRender.h"
#include "Render/Utils/Globals.h"
#include "world_render/ModelInstance.h"
#include "world_render/RenderModel.h"
#include <scenegraph/Context.h>
#include <Display/Context/Context.h>
#include <SceneGraph/View.h>
#include <SceneGraph/Visitor.h>
#include <SceneGraph/Context.h>
#define FX_ALLOW_INCLUDE
#include <FX/FxContext.h>


EXTERN_SHADER(vsProjectBlobShadows);
EXTERN_SHADER(psProjectBlobShadows);

EXTERN_SHADER(vsScreenBlobShadows);
EXTERN_SHADER(psScreenBlobShadows);
EXTERN_SHADER(psScreenBlobShadowsHDR);
#if PLATFORM_PC
EXTERN_SHADER(psScreenBlobShadowsR);	// [Blur2PC] retail PC channel shaders
EXTERN_SHADER(psScreenBlobShadowsG);
EXTERN_SHADER(psScreenBlobShadowsB);
EXTERN_SHADER(psScreenBlobShadowsA);
#endif

EXTERN_SHADER(vsCollisionMeshBlobShadows);
EXTERN_SHADER(psCollisionMeshBlobShadows);
EXTERN_SHADER(psCollisionMeshBlobShadowsHDR);

Debug::Bool gDebugBlobShadowsEnable("Graphics.Render.Deferred.BlobShadows.Enable", true);
Debug::Bool gDebugShadowMatrices("Graphics.Render.Deferred.BlobShadows.ShowShadowMatrices", false);
Debug::Int gBlobShadowsMaxExtraBlurCount("Graphics.Render.Deferred.BlobShadows.MaxExtraBlurCount", 3);
Debug::Bool gDebugShadowCasters("Graphics.Render.Deferred.BlobShadows.ShowShadowCasters", false);
Debug::Float gBlobShadowAllShadowsRange("Graphics.Render.Deferred.BlobShadows.AllShadowCastersRange", 0.1f, Debug::Range(0.f, 1.f));
Debug::Float gShadowCasterReductionFactor("Graphics.Render.Deferred.BlobShadows.ShadowCasterReductionFactor", 5.0f, Debug::Range(1.f, 10.f));
Debug::String gImposterShadowFileName("Graphics.Render.Deferred.BlobShadows.ImposterShadowFileName", "graphics/textures/shadows/impostershadow.texture");
Debug::Float gImposterShadowSizeX("Graphics.Render.Deferred.BlobShadows.ImposterShadowSizeX", 1.1f);
Debug::Float gImposterShadowSizeY("Graphics.Render.Deferred.BlobShadows.ImposterShadowSizeY", 2.5f);
Debug::Float gShadowScreenYOffset("Graphics.Render.Deferred.BlobShadows.ScreenYOffset", 1.0f, Debug::Range(0.f, 2.f));
Debug::Float gShadowScreenXOffset("Graphics.Render.Deferred.BlobShadows.ScreenXOffset", 0.5f, Debug::Range(0.f, 1.f));
Debug::Float gShadowFadeSpeed("Graphics.Render.Deferred.BlobShadows.FadeSpeed", 0.1f, Debug::Range(0.f, 1.f));

Debug::Bool gDebugShowCollisionMesh("Graphics.Render.Deferred.BlobShadows.ShowCollisionMesh", false);

namespace BlobShadows
{

Debug::Logger& g_Log = Debug::GetLogger( "gamesys.blobshadows");

C_BlobShadows::C_BlobShadows() : Display::I_Renderer("DeferredBlobShadows")
{
	m_resourcesPending = true;
	m_pDepthTexture = 0;

	m_ShadowSize = 6.f;
	m_ShadowHeight = 2.f;
	m_ShadowFade = 1.0f / 0.6f;
	m_MaxShadows = 100;
	m_Quality = 1.0f;

	m_DropShadowIntensity = 0.5f;
	m_DropShadowHeight = 8.f;
	m_MultiShadowIntensity = 5.f;
	m_IndividualShadowMaxIntensity = 0.02f;
	m_ShadowMult = Maths::HVec3(1.f,1.f,1.f);

	m_VisibilityFlags = 0xFFFFFFFF;
}

C_BlobShadows::~C_BlobShadows()
{
	DestroyRenderer();
}

bool C_BlobShadows::OnCreate()
{
	RESOURCE_MEMORY_ID("BlobShadows");

	if (!Display::CreateVertexShaderFromFunc("Shader/vsProjectBlobShadows", 0, &m_ProjectBlobVertexShader, REF_SHADER(vsProjectBlobShadows)))
		return false;

	if (!Display::CreatePixelShaderFromFunc("Shader/psProjectBlobShadows", 0, &m_ProjectBlobPixelShader, REF_SHADER(psProjectBlobShadows)))
		return false;

	if (!Display::CreateVertexShaderFromFunc("Shader/vsScreenBlobShadows", 0, &m_ScreenBlobVertexShader, REF_SHADER(vsScreenBlobShadows)))
		return false;

	if (!Display::CreateVertexShaderFromFunc("Shader/vsCollisionMeshBlobShadows", 0, &m_CollisionMeshBlobVertexShader, REF_SHADER(vsCollisionMeshBlobShadows)))
		return false;

#if PLATFORM_PC
	// [Blur2PC] Retail PC creates the four channel shaders and no HDR variants (Blur.exe 1.2 0x00730160).
	if (!Display::CreatePixelShaderFromFunc("Shader/psScreenBlobShadowR", 0, &m_ScreenBlobChannelPixelShader[0], REF_SHADER(psScreenBlobShadowsR)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psScreenBlobShadowG", 0, &m_ScreenBlobChannelPixelShader[1], REF_SHADER(psScreenBlobShadowsG)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psScreenBlobShadowB", 0, &m_ScreenBlobChannelPixelShader[2], REF_SHADER(psScreenBlobShadowsB)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psScreenBlobShadowA", 0, &m_ScreenBlobChannelPixelShader[3], REF_SHADER(psScreenBlobShadowsA)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shader/psCollisionBlobShadows", 0, &m_CollisionMeshBlobPixelShader, REF_SHADER(psCollisionMeshBlobShadows)))
		return false;
#else
	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		if (!Display::CreatePixelShaderFromFunc("Shader/psScreenBlobShadowsHDR", 0, &m_ScreenBlobPixelShader, REF_SHADER(psScreenBlobShadowsHDR)))
			return false;

		if (!Display::CreatePixelShaderFromFunc("Shader/psCollisionBlobShadowsHDR", 0, &m_CollisionMeshBlobPixelShader, REF_SHADER(psCollisionMeshBlobShadowsHDR)))
			return false;
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("Shader/psScreenBlobShadows", 0, &m_ScreenBlobPixelShader, REF_SHADER(psScreenBlobShadows)))
			return false;

		if (!Display::CreatePixelShaderFromFunc("Shader/psCollisionMeshBlobShadows", 0, &m_CollisionMeshBlobPixelShader, REF_SHADER(psCollisionMeshBlobShadows)))
			return false;
	}
#endif

	//Get Parameters for project blob
	m_ProjectShadowPlaneSpaceParameter = m_ProjectBlobVertexShader.GetParameter(CV_RENDER_SETTINGS, "g_ShadowPlaneSpaceMatrix");
	m_ProjectShadowPlaneParameter		= m_ProjectBlobVertexShader.GetParameter(CV_RENDER_SETTINGS_5, "g_ShadowPlane");
	m_ProjectLightPositionParameter	= m_ProjectBlobVertexShader.GetParameter(CV_RENDER_SETTINGS_6, "g_LightPosition");
	m_ProjectShadowAlphaParameter		= m_ProjectBlobPixelShader.GetParameter(CP_RENDER_SETTINGS, "g_ShadowAlpha");


	//Get Parameter for screen blob
#if PLATFORM_PC
	// [Blur2PC] the four channel shaders share one register layout
	const Display::C_PixelShader& screenBlobPixelShader = m_ScreenBlobChannelPixelShader[0];
#else
	const Display::C_PixelShader& screenBlobPixelShader = m_ScreenBlobPixelShader;
#endif
	m_ScreenShadowPlaneSpaceParameter = screenBlobPixelShader.GetParameterArray(CP_RENDER_SETTINGS, "g_ShadowPlaneSpaceMatrix", 4);
	m_ScreenDepthParameter		= screenBlobPixelShader.GetParameter(CP_RENDER_SETTINGS_4, "g_vDepth");
	m_PixelOffsetParameter = screenBlobPixelShader.GetParameter(CP_RENDER_SETTINGS_5, "g_PixelOffset");
	m_ShadowMultParameter = screenBlobPixelShader.GetParameter(CP_RENDER_SETTINGS_6, "g_ShadowMult");

	//Get Parameter for collision mesh blob
	m_CollisionMeshShadowSpaceParameter = m_CollisionMeshBlobVertexShader.GetParameter(CV_RENDER_SETTINGS, "g_ShadowPlaneSpaceMatrix");
	m_CollisionMeshShadowMultParameter = m_CollisionMeshBlobPixelShader.GetParameter(CP_RENDER_SETTINGS, "g_ShadowMult");


	//Create Imposter Shadow
	const Util::C_String fileName(gImposterShadowFileName);
	if(fileName.ISearch(".texture") != -1 && Sys::GetDefaultFileSystem()->FileExists(fileName))
	{
		m_ImposterShadowTextureHandle.Load(Util::C_String(fileName), this);
	}
	else
	{
		LOGWARN(g_Log, "Imposter shadow texture %s not found", fileName.Chars());	
	}

	if (!Display::Device()->CreateVertexBuffer(sizeof(Display::Vertex_XYZ) * 36, 0, &m_VertexBuffer))
		return false;

	Display::Vertex_XYZ vertex[8] = 
	{
		{ -1, -1, -1 },
		{ -1, -1, 1 },
		{ -1, 1, -1 },
		{ -1, 1, 1 },
		{ 1, -1, -1 },
		{ 1, -1, 1 },
		{ 1, 1, -1 },
		{ 1, 1, 1 },
	};

	Display::Vertex_XYZ* pVertex = (Display::Vertex_XYZ*)m_VertexBuffer.Lock();
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

		m_VertexBuffer.Unlock();
	}

	return true;
}

bool C_BlobShadows::CreateRenderTarget(U32 index, U32 resolution, const Display::C_BackBuffer& backBuffer )
{
	U32 auxTextureIndex = GetResourceID(resolution);

	//Add blob render texture
	BlobTexture& blobTexture = m_BlobRenderTexture[index];
	blobTexture.m_ResourceID = auxTextureIndex;

	//Create Render Buffer
	if (!Display::Device()->CreateRenderTexture2D(resolution, resolution, 1, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &blobTexture.m_Texture))
		return false;

	//Create remap textures
#ifdef BLOBSHADOW_PER_CHANNEL
#endif

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&blobTexture.m_Texture, "_BlobShadowRT");
	#endif

	Display::C_SurfaceParams surfaceParams;


	//Create blur texture
	if (!m_BlurBlobRenderTexture[auxTextureIndex].IsCreated())
	{
		if (!Display::Device()->CreateRenderTexture2D(resolution, resolution, 1, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_BlurBlobRenderTexture[auxTextureIndex]))
			return false;

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetRenderTexture2DName(&m_BlurBlobRenderTexture[auxTextureIndex], "_BlurBlobShadowRT");
		#endif
	}

	//Create depth buffer if needed
	if (!m_DepthStencilSurface[auxTextureIndex].IsCreated())
	{
		if (!Display::Device()->CreateDepthStencilSurface(resolution, resolution, Display::E_Usage_DepthStencil, backBuffer.GetDepthStencilFormat(), Display::E_Multisample_None, &surfaceParams, &m_DepthStencilSurface[auxTextureIndex]))
			return false;

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetSurfaceName(&m_DepthStencilSurface[auxTextureIndex], "_BlobShadowsDS");
		#endif
	}

	return true;
}

U32 C_BlobShadows::GetResourceID( U32 resolution )
{
	switch (resolution)
	{
	case 256:
		return 0;
	case 128:
		return 1;
	case 64:
		return 2;
	case 32:
		return 3;
	}
	return -1;
}

bool C_BlobShadows::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("BlobShadows");

	MEMORY_TRACKER_PUSH("graphics.renderers.blobshadows");

	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

	m_BlobRenderTexture.Resize(m_ShadowSettings.m_Num256Textures + m_ShadowSettings.m_Num128Textures + m_ShadowSettings.m_Num64Textures + m_ShadowSettings.m_Num32Textures);

	U32 index = 0;

	//Create resources using init settings
	for (U32 i = 0; i < m_ShadowSettings.m_Num256Textures; i++)
	{
		CreateRenderTarget(index, 256, backBuffer);
		index++;
	}
	for (U32 i = 0; i < m_ShadowSettings.m_Num128Textures; i++)
	{
		CreateRenderTarget(index, 128, backBuffer);
		index++;
	}
	for (U32 i = 0; i < m_ShadowSettings.m_Num64Textures; i++)
	{
		CreateRenderTarget(index, 64, backBuffer);
		index++;
	}
	for (U32 i = 0; i < m_ShadowSettings.m_Num32Textures; i++)
	{
		CreateRenderTarget(index, 32, backBuffer);
		index++;
	}

	MEMORY_TRACKER_POP("graphics.renderers.blobshadows");

	return true;
}

void C_BlobShadows::OnDestroy()
{
	Display::Device()->Destroy(&m_ProjectBlobVertexShader);
	Display::Device()->Destroy(&m_ProjectBlobPixelShader);
	Display::Device()->Destroy(&m_ScreenBlobVertexShader);
	Display::Device()->Destroy(&m_ScreenBlobPixelShader);
#if PLATFORM_PC
	for (U32 i = 0; i < 4; i++)
		Display::Device()->Destroy(&m_ScreenBlobChannelPixelShader[i]);	// [Blur2PC]
#endif
	Display::Device()->Destroy(&m_CollisionMeshBlobVertexShader);
	Display::Device()->Destroy(&m_CollisionMeshBlobPixelShader);
	Display::Device()->Destroy(&m_VertexBuffer);
}

void C_BlobShadows::OnDestroyVolatile()
{
	for (S32 i = 0; i < m_BlobRenderTexture.Size(); i++)
		Display::Device()->Destroy(&m_BlobRenderTexture[i].m_Texture);
	for (S32 i = 0; i < 4; i++)
		if (m_DepthStencilSurface[i].IsCreated())
			Display::Device()->Destroy(&m_DepthStencilSurface[i]);
	for (S32 i = 0; i < 4; i++)
		if (m_BlurBlobRenderTexture[i].IsCreated())
			Display::Device()->Destroy(&m_BlurBlobRenderTexture[i]);
}

int C_BlobShadows::PrioritySortFunc(const void *A, const void *B)
{
	C_ProcessShadow* pA = (C_ProcessShadow*)A;
	C_ProcessShadow* pB = (C_ProcessShadow*)B;
	
	if (pA->m_pInstance->m_Priority > pB->m_pInstance->m_Priority)
		return 1;
	else
		return -1;
}

void C_BlobShadows::CalculateProcessShadows()
{
	m_ProcessShadows.Clear();

	if (m_BlobShadowInstanceList.Size() == 0)
		return;

	//Sort shadows instances by priority
	U32 index = 0;
	Btl::List<C_BaseBlobShadowInstance*>::Iterator it = m_BlobShadowInstanceList.Begin();
	while (it != m_BlobShadowInstanceList.End())
	{
		C_BaseBlobShadowInstance* pInstance = *it;

		if (pInstance->m_Enable && pInstance->m_Intensity > 0.01f)
		{

			if (pInstance->m_CameraDistant < m_ShadowSettings.m_MaxShadowDistant)
			{

				if ((pInstance->m_CameraDistant + m_ShadowSize) > 0.f)
				{
					//In front of the camera
					C_ProcessShadow processShadow;
					processShadow.m_pInstance = pInstance;

					m_ProcessShadows.PushBack(processShadow);
				}
			}
		}
		++it;
	}
	if (m_ProcessShadows.Size() > 0)
	{

		//free blobshadows texture
		for (S32 i = 0; i < m_BlobRenderTexture.Size(); i++)
		#if defined(BLOBSHADOW_PER_CHANNEL)
			m_BlobRenderTexture[i].m_Used = 0;
		#else
			m_BlobRenderTexture[i].m_Used = false;
		#endif

		U32 currentBlobRenderTextureIndex = 0;

		//Calculate vector to select quality base to distant
		F32 distantFactor = m_Quality;

		F32 distantRange[4];
		distantRange[0] = m_ShadowSettings.m_256TexturesDistant * distantFactor;
		distantRange[1] = m_ShadowSettings.m_128TexturesDistant * distantFactor;
		distantRange[2] = m_ShadowSettings.m_64TexturesDistant * distantFactor;
		distantRange[3] = m_ShadowSettings.m_32TexturesDistant * distantFactor;

		//qsort by distant
		qsort(&m_ProcessShadows[0], m_ProcessShadows.Size(), sizeof(C_ProcessShadow), PrioritySortFunc);

		for (S32 i = 0; i < m_ProcessShadows.Size(); i++)
		{
			C_ProcessShadow& processShadow = m_ProcessShadows[i];

			switch (processShadow.m_pInstance->GetType())
			{
				case E_BlobShadowType_Dynamic:
				{
					C_DynamicBlobShadowInstance* pInstance = (C_DynamicBlobShadowInstance*)processShadow.m_pInstance;

					//Find the best blobtexture to this shadow
					bool found = false;
					S32 renderTextureIndex = 0;
					S32 subTextureIndex = 0;
				#if defined(BLOBSHADOW_PER_CHANNEL)
					for (S32 j = currentBlobRenderTextureIndex; j < m_BlobRenderTexture.Size(); j++)
					{
						BlobTexture& blobTexture = m_BlobRenderTexture[j];

						//Check if the distant is OK
						if (pInstance->m_CameraDistant > distantRange[blobTexture.m_ResourceID])
							continue;
						
						U32 mask = 1;
						for(S32 k = 0; k < 4; k++, mask<<=1)
						{
							//Check if it is free
							if (blobTexture.m_Used & mask)
								continue;

							//Founded
							found = true;
							blobTexture.m_Used |= mask; // mark as used
							currentBlobRenderTextureIndex = j + ((0x0f == blobTexture.m_Used) ? 1 : 0); //Next
							processShadow.m_RenderTextureIndex = (j*4) + k;
							renderTextureIndex = j;
							subTextureIndex = k;
							j = m_BlobRenderTexture.Size();
							break;
						}
					}
				#else
					for (S32 j = currentBlobRenderTextureIndex; j < m_BlobRenderTexture.Size(); j++)
					{
					
						BlobTexture& blobTexture = m_BlobRenderTexture[j];
						//Check if it is free
						if (blobTexture.m_Used)
							continue;
						//Check if the distant is OK
						if (pInstance->m_CameraDistant > distantRange[blobTexture.m_ResourceID])
							continue;

						//Founded
						found = true;
						currentBlobRenderTextureIndex = j + 1; //Next
						processShadow.m_RenderTextureIndex = j;
						renderTextureIndex = j;
						subTextureIndex = -1;
						break;
					}
				#endif
					if (!found)
					{
						//No more blobtextures
						//Use imposter texture
						processShadow.m_RenderTextureIndex = 0xFFFFFFFF;
						processShadow.m_pInstance->CalculateShadowMatrices(gImposterShadowSizeX, gImposterShadowSizeY, 0.5f);
						continue;
					}
					if (gDebugShadowMatrices)
					{
						Util::C_String msg = Util::C_String::Format("%i(%i)", m_BlobRenderTexture[renderTextureIndex].m_Texture.GetTexture().GetWidth(), subTextureIndex);
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(Maths::HVec3(m_ProcessShadows[i].m_pInstance->m_ShadowPlaneSpace.AW()), 1, msg);
					}

					//Adjust size depends quality
					F32 sizeX, sizeY, height;
					F32 sizeFactor = m_Quality * Maths::Clamp(processShadow.m_pInstance->m_CameraDistant / m_ShadowSettings.m_MaxShadowDistant, 0.f, 1.f);
					//Only 0.05 steps
					sizeFactor *= 20.f;
					sizeFactor = floorf(sizeFactor);
					sizeFactor /= 20.f;
					sizeX = Maths::Lerp(m_ShadowSize, gImposterShadowSizeX, sizeFactor);
					sizeY = Maths::Lerp(m_ShadowSize, gImposterShadowSizeY, sizeFactor);
					height = Maths::Lerp(m_ShadowHeight, 0.5f, sizeFactor);
					processShadow.m_pInstance->CalculateShadowMatrices(sizeX, sizeY, height);
				}
				break;
			case E_BlobShadowType_Texture:
				{
					C_TextureBlobShadowInstance* pInstance = (C_TextureBlobShadowInstance*)processShadow.m_pInstance;
					processShadow.m_pInstance->CalculateShadowMatrices(pInstance->m_ShadowSize.X(), pInstance->m_ShadowSize.Y(), pInstance->m_ShadowSize.Z());
				}
				break;
			}

			if (gDebugShadowMatrices)
			{
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(Maths::HMat43(m_ProcessShadows[i].m_pInstance->m_ShadowPlaneSpace), 1.f);
			}
		}
	}
}

void C_BlobShadows::RenderBlobTexture( const C_ProcessShadow& processShadow )
{
	size_t renderTextureIndex = processShadow.m_RenderTextureIndex;
#if defined(BLOBSHADOW_PER_CHANNEL)
	size_t subTextureIndex = renderTextureIndex & 3;
	renderTextureIndex /= 4;
	Display::E_ColourWriteEnable colourWriteEnable = Display::E_ColourWriteEnable_None;
	switch (subTextureIndex)
	{
		case 0: colourWriteEnable = Display::E_ColourWriteEnable_Red; break;
		case 1: colourWriteEnable = Display::E_ColourWriteEnable_Green; break;
		case 2: colourWriteEnable = Display::E_ColourWriteEnable_Blue; break;
		case 3: colourWriteEnable = Display::E_ColourWriteEnable_Alpha; break;
	}
#endif
	Display::C_RenderTexture2D& blobRenderTexture = m_BlobRenderTexture[renderTextureIndex].m_Texture;
	Display::C_Surface& depthStencilSurface = m_DepthStencilSurface[m_BlobRenderTexture[renderTextureIndex].m_ResourceID];
	Display::C_RenderTexture2D& blurBlobRenderTexture = m_BlurBlobRenderTexture[m_BlobRenderTexture[renderTextureIndex].m_ResourceID];

	C_DynamicBlobShadowInstance* pInstance = (C_DynamicBlobShadowInstance*)processShadow.m_pInstance;

	pInstance->CalculateShadowCasters(m_Quality, m_MaxShadows, m_ShadowSettings.m_MaxShadowDistant, gShadowFadeSpeed);

	Btl::Vector<C_BlobShadowCaster>& currentShadowCasters = pInstance->m_ShadowCastersVector[pInstance->m_DoubleBufferShadowCastersVector];
	S32 maxShadows = currentShadowCasters.Size() + 1;


	//Render Blob texture (two version)
	if (pInstance->m_pShadowModelInstance)
	{
		//Bind blob texture and clear
	#if defined(BLOBSHADOW_PER_CHANNEL)
		// Clear colour if first texture
		Display::E_Clear clearMask = (Display::E_Clear)((0 == subTextureIndex) ? (Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil) : (Display::E_Clear_Depth | Display::E_Clear_Stencil));
			
		//optim todo: since rendering is untextured, use 4x MSAA for speed
		// - needs tiled buffer with compressed colour
		// - works with colour mask?
	#else
		Display::E_Clear clearMask = (Display::E_Clear)(Display::E_Clear_Colour | Display::E_Clear_Depth);
	#endif
		
		Display::Device()->SetColourDepthStencilSurface(&blobRenderTexture.GetSurface(), &depthStencilSurface);	
		Display::Device()->Clear(clearMask, Maths::RGBA(0, 0, 0, 0));

		Display::C_Viewport viewport;
		viewport.m_W = blobRenderTexture.GetTexture().GetWidth(); 
		viewport.m_H = blobRenderTexture.GetTexture().GetHeight();
		Display::Device()->SetViewport(viewport);

		m_pContext->CloneState(NULL);
		m_pContext->Begin(NULL);
		m_pContext->SetRenderPassIndex(0);
		m_pContext->SetView(NULL);
		
		RenderingData::Target::RenderFlags renderFlags;
		renderFlags.setPass = 0;
		RenderingData::Target::Context* pRdContext = RenderingData::Target::GetContext(m_pContext);
		pRdContext->SetRenderFlags(renderFlags);

		FX::RenderFlags fxRenderFlags;
		fxRenderFlags.setBlendState = 0;
		FX::Context* pFXContext = FX::GetContext(m_pContext);
		pFXContext->SetRenderFlags(fxRenderFlags);

		//setup states
	#if defined(BLOBSHADOW_PER_CHANNEL)
		Display::Device()->SetBlendMode(Display::E_BlendMode_AlphaPreMultiply);
	#else
		Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);
	#endif
		Display::Device()->SetCullMode(Display::E_CullMode_Cw);
		Display::Device()->SetDepthEnable(Display::E_Bool_True);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Greater);

	#if defined(BLOBSHADOW_PER_CHANNEL)
		// Set channel mask for sub texture
		Display::Device()->SetColourWriteEnable(colourWriteEnable);
	#endif

		Maths::HMat44 hWorldMatrix = Maths::HMat44(pInstance->m_CasterWorldMatrix);
		Display::C_Matrix worldMatrix;
		Maths::Store(hWorldMatrix, &worldMatrix);
		pInstance->m_pShadowModelInstance->SetParameter(pInstance->m_WorldMatrixParameter, (F32*)&worldMatrix);

		Display::C_Matrix shadowPlaneSpaceInv;
		Maths::Store(pInstance->m_ShadowPlaneSpaceInv, &shadowPlaneSpaceInv);
		pInstance->m_pShadowModelInstance->SetParameter(pInstance->m_ShadowPlaneSpaceMatrixParameter, (F32*)&shadowPlaneSpaceInv);
		Display::C_Float4 shadowPlane; 
		Maths::Store(Maths::HVec4(pInstance->m_ShadowPlane.Normal().X(), pInstance->m_ShadowPlane.Normal().Y(), pInstance->m_ShadowPlane.Normal().Z(), pInstance->m_ShadowPlane.DistanceToOrigin()), &shadowPlane);
		pInstance->m_pShadowModelInstance->SetParameter(pInstance->m_ShadowPlaneParameter, (F32*)&shadowPlane);


		for (S32 s = 0; s < maxShadows; s++) 
		{
			Maths::HVec3 shadowLights;
			F32 intensity;
			if (s == 0)
			{
				shadowLights = pInstance->m_CasterWorldMatrix.AW() + Maths::HVec3(0.f, m_DropShadowHeight, 0.f); 
				intensity = m_DropShadowIntensity / (1.f + (Maths::HVec3(pInstance->m_CasterWorldMatrix.AW()) - pInstance->m_ShadowPosition).Length2());
			}
			else
			{
				shadowLights = currentShadowCasters[s-1].m_Position; 
				if (currentShadowCasters[s-1].m_Intensity > 0.f)
					intensity =  currentShadowCasters[s-1].m_Fade * m_MultiShadowIntensity * Maths::Min(m_IndividualShadowMaxIntensity, currentShadowCasters[s-1].m_Intensity / (1.f + (Maths::HVec3(pInstance->m_CasterWorldMatrix.AW()) - shadowLights).Length2()));
				else
					intensity =  currentShadowCasters[s-1].m_Fade * -currentShadowCasters[s-1].m_Intensity;
				
				//Check position height
				Maths::HVec4 lightShadowSpacePosition = (shadowLights || Maths::HVecW(1.f)) * pInstance->m_ShadowPlaneSpaceInv;
				intensity *= Maths::Clamp(F32(lightShadowSpacePosition.Z() / sqrtf(lightShadowSpacePosition.X() * lightShadowSpacePosition.X() + lightShadowSpacePosition.Y() * lightShadowSpacePosition.Y())) - 0.25f, 0.f, 1.f);

				if (intensity < 0.01f)
					continue;
	
				if (gDebugShadowCasters)
				{
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(pInstance->m_ShadowPosition, shadowLights, Maths::RGBA(0.f,1.f,0.f,1.f) );
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(shadowLights, 1.f, Maths::RGBA(intensity,intensity,intensity,1.f));
					Util::C_String msg = Util::C_String::Format("%f",intensity);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(shadowLights, msg);
				}
			}

			Display::C_Float4 lightPosition(shadowLights.X(), shadowLights.Y(), shadowLights.Z(), F32(s + 1) / F32(maxShadows + 1)); //W -> Z position for the shadow
			pInstance->m_pShadowModelInstance->SetParameter(pInstance->m_LightPositionParameter, (F32*)&lightPosition);

			Display::C_Float4 shadowAlphaArray(intensity, m_ShadowFade, 0.f, 0.f);
			pInstance->m_pShadowModelInstance->SetParameter(pInstance->m_ShadowAlphaParameter, (F32*)&shadowAlphaArray);

			//Render
			m_pContext->Render(pInstance->m_pShadowModelInstance, 0, 0);
		}
	
		renderFlags.setPass = 1;
		pRdContext->SetRenderFlags(renderFlags);

		fxRenderFlags.setBlendState = 1;
		pFXContext->SetRenderFlags(fxRenderFlags);

		m_pContext->End();

		//Unbind

		Display::Device()->SetView(NULL, false);
		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		Display::Device()->SetCullMode(Display::E_CullMode_Default);
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Default);
		Display::Device()->SetDepthEnable(Display::E_Bool_Default);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_Default);

	#if defined(BLOBSHADOW_PER_CHANNEL)
		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
	#endif
		
	}
	else
	{
	#if defined(BLOBSHADOW_PER_CHANNEL)
		// Clear colour if first texture
		Display::E_Clear clearMask = (Display::E_Clear)((0 == subTextureIndex) ? (Display::E_Clear)(Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil) : (Display::E_Clear_Depth | Display::E_Clear_Stencil));
	#else
		Display::E_Clear clearMask = (Display::E_Clear)(Display::E_Clear_Colour | Display::E_Clear_Depth);
	#endif
		//Bind blob texture and clear
		Display::Device()->SetColourDepthStencilSurface(&blobRenderTexture.GetSurface(), &depthStencilSurface);	
		Display::Device()->Clear(clearMask, Maths::RGBA(0, 0, 0, 0));

		Display::C_Viewport viewport;
		viewport.m_W = blobRenderTexture.GetTexture().GetWidth(); 
		viewport.m_H = blobRenderTexture.GetTexture().GetHeight();
		Display::Device()->SetViewport(viewport);

		//setup shaders

		Maths::HMat44 hWorldMatrix = Maths::HMat44(pInstance->m_CasterWorldMatrix);
		Display::C_Matrix worldMatrix;
		Maths::Store(hWorldMatrix, &worldMatrix);
		Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);

		Display::C_Matrix shadowPlaneSpaceInv;
		Maths::Store(pInstance->m_ShadowPlaneSpaceInv, &shadowPlaneSpaceInv);
		Display::Device()->SetVertexShaderParameterF(m_ProjectShadowPlaneSpaceParameter, (F32*)&shadowPlaneSpaceInv, 4); 
		Display::C_Float4 shadowPlane;
		Maths::Store(Maths::HVec4(pInstance->m_ShadowPlane.Normal().X(), pInstance->m_ShadowPlane.Normal().Y(), pInstance->m_ShadowPlane.Normal().Z(), pInstance->m_ShadowPlane.DistanceToOrigin()), &shadowPlane);
		Display::Device()->SetVertexShaderParameterF(m_ProjectShadowPlaneParameter, (F32*)&shadowPlane, 1);

		//setup states
	#if defined(BLOBSHADOW_PER_CHANNEL)
		Display::Device()->SetBlendMode(Display::E_BlendMode_AlphaPreMultiply);
	#else
		Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);
	#endif
		Display::Device()->SetCullMode(Display::E_CullMode_Cw);
		Display::Device()->SetDepthEnable(Display::E_Bool_True);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Greater);

	#if defined(BLOBSHADOW_PER_CHANNEL)
		// Set channel mask for sub texture
		Display::Device()->SetColourWriteEnable(colourWriteEnable);
	#endif


		for (S32 s = 0; s < maxShadows; s++)
		{
			Maths::HVec3 shadowLights;
			F32 intensity;
			if (s == 0)
			{
				shadowLights = pInstance->m_CasterWorldMatrix.AW() + Maths::HVec3(0.f, m_DropShadowHeight, 0.f); 
				intensity = m_DropShadowIntensity / (1.f + (Maths::HVec3(pInstance->m_CasterWorldMatrix.AW()) - pInstance->m_ShadowPosition).Length2());
			}
			else
			{
				shadowLights = currentShadowCasters[s-1].m_Position; 
				intensity =  currentShadowCasters[s-1].m_Fade * m_MultiShadowIntensity * Maths::Min(m_IndividualShadowMaxIntensity, currentShadowCasters[s-1].m_Intensity / (1.f + (Maths::HVec3(pInstance->m_CasterWorldMatrix.AW()) - shadowLights).Length2()));

				if (gDebugShadowCasters)
				{
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(pInstance->m_ShadowPosition, shadowLights, Maths::RGBA(0.f,1.f,0.f,1.f) );
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(shadowLights, 1.f, Maths::RGBA(intensity,intensity,intensity,1.f));
					Util::C_String msg = Util::C_String::Format("%f",intensity);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(shadowLights, msg);
				}
			}

			Display::C_Float4 lightPosition;
			Maths::Store(Maths::HVec4(shadowLights.X(), shadowLights.Y(), shadowLights.Z(), F32(s + 1) / F32(maxShadows + 1)), &lightPosition);
			Display::Device()->SetVertexShaderParameterF(m_ProjectLightPositionParameter, (F32*)&lightPosition, 1); 

			Display::C_Float4 shadowAlphaArray(intensity, m_ShadowFade, 0.f, 0.f);
			Display::Device()->SetPixelShaderParameterF(m_ProjectShadowAlphaParameter, (F32*)&shadowAlphaArray); 

			Display::Device()->SetVertexShader(&m_ProjectBlobVertexShader);
			Display::Device()->SetPixelShader(&m_ProjectBlobPixelShader);
			Display::Device()->SetPixelShaderParameters(&m_ProjectBlobPixelShader);
			//Draw
			Display::Device()->SetVertexBuffer(0, pInstance->m_pShadowVertexBuffer, 0, sizeof(Display::Vertex_XYZ));
			Display::Device()->SetIndexBuffer(pInstance->m_pShadowIndexBuffer);

			Display::Device()->SetVertexDeclaration(Display::Vertex_XYZ::m_pDeclaration);

			Display::Device()->DrawIndexedPrimitive(Display::E_PrimitiveType_TriangleList, 0, 0, pInstance->m_ShadowVertexCount, 0, pInstance->m_ShadowIndexCount);
		}

		//Unbind

		Display::Device()->SetView(NULL, false);

		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		Display::Device()->SetCullMode(Display::E_CullMode_Default);
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Default);
		Display::Device()->SetDepthEnable(Display::E_Bool_Default);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_Default);
	#if defined(BLOBSHADOW_PER_CHANNEL)
		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
	#endif
	}

	//look for realocate that
	#if !defined(BLOBSHADOW_PER_CHANNEL)
	Display::Device()->InvalidateState();
	#endif

	//Blur blob texture if needed
	if (blobRenderTexture.GetTexture().GetWidth() >= m_ShadowSettings.m_MinBlurResolution || m_BlurCount < gBlobShadowsMaxExtraBlurCount)
	{
		m_BlurCount ++;
#if defined(BLOBSHADOW_PER_CHANNEL)
		m_NeedsBlurring[renderTextureIndex] = 1;
#else
		//Change with better blur
		m_GaussianBlur.GaussBlurX(&blurBlobRenderTexture, &blobRenderTexture, Render::Texture::E_GaussianBlurShader_5);
		m_GaussianBlur.GaussBlurY(&blobRenderTexture, &blurBlobRenderTexture, Render::Texture::E_GaussianBlurShader_5);
#endif
	}
	
}

void C_BlobShadows::PreRender()
{
	PERF_EVENT("PreRenderBlobShadows");
	
	if (!gDebugBlobShadowsEnable)
		return;

	// [Blur2PC] interpolated poses for this render
	for (Btl::List<C_BaseBlobShadowInstance*>::Iterator it = m_BlobShadowInstanceList.Begin(); it != m_BlobShadowInstanceList.End(); ++it)
		if ((*it)->GetType() == E_BlobShadowType_Dynamic)
			static_cast<C_DynamicBlobShadowInstance*>(*it)->Blur2PCApplyRenderPose();

	//Distribute shadows
	CalculateProcessShadows();

	m_BlurCount = 0;
	
	
#if defined(BLOBSHADOW_PER_CHANNEL)
	for (S32 i = 0; i < m_BlobRenderTexture.Size(); i++)
		m_NeedsBlurring[i] = 0;
#endif

	//Draw Blob textures
	for (S32 i = 0; i < m_ProcessShadows.Size(); i++)
	{
		if (m_ProcessShadows[i].m_pInstance->GetType() == E_BlobShadowType_Dynamic)
			if (m_ProcessShadows[i].m_RenderTextureIndex != 0xFFFFFFFF)
				RenderBlobTexture(m_ProcessShadows[i]);
	}
	
#if defined(BLOBSHADOW_PER_CHANNEL)

	Display::Device()->InvalidateState();
	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
	// blur the textures
	for (S32 i = 0; i < m_BlobRenderTexture.Size(); i++)
	{
		if (!m_NeedsBlurring[i])
			continue;
			
		Display::C_RenderTexture2D& blobRenderTexture = m_BlobRenderTexture[i].m_Texture;
		Display::C_RenderTexture2D& blurBlobRenderTexture = m_BlurBlobRenderTexture[m_BlobRenderTexture[i].m_ResourceID];
			
		//Change with better blur
		m_GaussianBlur.GaussBlurX(&blurBlobRenderTexture, &blobRenderTexture, Render::Texture::E_GaussianBlurShader_5);
		m_GaussianBlur.GaussBlurY(&blobRenderTexture, &blurBlobRenderTexture, Render::Texture::E_GaussianBlurShader_5);
	}
#endif
	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);
}

void C_BlobShadows::RenderScreenShadow( const C_ProcessShadow& processShadow, Display::C_Context* pContext ) const
{
	PERF_EVENT("BlobShadows::RenderScreenShadow");
	Display::C_Float4 shadowMult(1.f, 1.f, 1.f, 1.f);
	C_BaseBlobShadowInstance* pInstance = processShadow.m_pInstance;
	
	size_t renderTextureIndex = 0;
	
#if defined(BLOBSHADOW_PER_CHANNEL)
	U32 subTextureIndex = 4;
#endif
	
	const Display::C_Texture2D* pBlobTexture = NULL;
	switch (processShadow.m_pInstance->GetType())
	{
	case E_BlobShadowType_Dynamic:
		{
			if (processShadow.m_RenderTextureIndex != 0xFFFFFFFF)
			{
				renderTextureIndex = processShadow.m_RenderTextureIndex;
			#if defined(BLOBSHADOW_PER_CHANNEL)
				subTextureIndex = renderTextureIndex & 3;
				renderTextureIndex /= 4;

					pBlobTexture = &m_BlobRenderTexture[renderTextureIndex].m_Texture.GetTexture();
			#else
				pBlobTexture = &m_BlobRenderTexture[renderTextureIndex].m_Texture.GetTexture();
			#endif
				shadowMult = Display::C_Float4(pInstance->m_Intensity * m_ShadowMult.X(), pInstance->m_Intensity * m_ShadowMult.Y(), pInstance->m_Intensity * m_ShadowMult.Z(), 1.f); 
			}
			else
			{
				//Imposter shadow texture
				TextureResource::C_TextureResource* pTextureResource = m_ImposterShadowTextureHandle;
				if (pTextureResource && pTextureResource->IsDataValid())
				{
					pBlobTexture = (const Display::C_Texture2D*) pTextureResource->GetDisplayTexture();

					//Fade with distant to floor in 1 meter
					F32 shadowIntensity = pInstance->m_Intensity * (1.f / (1.f + pInstance->m_GroundDistant)) * m_DropShadowIntensity;
					shadowMult = Display::C_Float4( shadowIntensity * m_ShadowMult.X(), shadowIntensity * m_ShadowMult.Y(), shadowIntensity * m_ShadowMult.Z(), 1.f); 
				}
				else
					return; //No fake texture
			}
		}
		break;
	case E_BlobShadowType_Texture:
		{
			C_TextureBlobShadowInstance* pTextureInstance = (C_TextureBlobShadowInstance*)processShadow.m_pInstance;
			pBlobTexture = pTextureInstance->m_pBlobTexture;

			//Calculate intensity
			F32 shadowIntensity =  pTextureInstance->m_Intensity * (1.f - Maths::Clamp(pInstance->m_GroundDistant /pTextureInstance->m_FadeDistant , 0.f, 1.f));
			shadowMult = Display::C_Float4( shadowIntensity * m_ShadowMult.X(), shadowIntensity * m_ShadowMult.Y(), shadowIntensity * m_ShadowMult.Z(), 1.f); 
		}
	}
	

	Display::C_Matrix shadowPlaneSpace;
	Maths::Store(pInstance->m_ShadowPlaneSpace, &shadowPlaneSpace);
	pContext->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&shadowPlaneSpace, 4); 
	pContext->SetVertexShaderConstantF(CV_DISPLAY_VIEWPROJ_MATRIX, (F32*)&pContext->GetView()->GetViewSettings3D().GetViewProjMatrix(), 4);

	pContext->SetVertexShader(&m_ScreenBlobVertexShader);

	pContext->SetTexture(0, pContext->GetDepthStencilTexture()->GetGPUTexture());
	pContext->SetTexture(1, pBlobTexture->GetGPUTexture());
#if PLATFORM_PC
	// [Blur2PC] Retail's Render (Blur.exe 1.2 0x00732d20) binds the context's stencil texture at s2 (linear,
	// clamp): the display window's car mask (C_PCD3D9Device::RenderWindowMask, T089)
	{
		const Display::C_Texture2D* pStencilTexture = pContext->GetStencilTexture();
		pContext->SetTexture(2, pStencilTexture ? pStencilTexture->GetGPUTexture() : 0);
	}
#endif

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

	Display::C_SamplerState blobSampleState;
	blobSampleState.SetDefaultStates();
	blobSampleState.SetTextureAddress(Display2::E_TextureAddress_Clamp);
	blobSampleState.SetTextureFilter(Display2::E_TextureFilter_Linear);
	//blobSampleState.SetTextureGamma(Display2::E_Bool_False);
	pContext->SetSamplerState(1, &blobSampleState);
#if PLATFORM_PC
	pContext->SetSamplerState(2, &blobSampleState);	// [Blur2PC] retail 0x00732d20: s2 as s1
#endif

#if PLATFORM_PS3
#if defined(BLOBSHADOW_PER_CHANNEL)
#define REMAP_OUT_BITS(a, r, g, b) ((CELL_GCM_TEXTURE_REMAP_##b<<14)|(CELL_GCM_TEXTURE_REMAP_##g<<12)|(CELL_GCM_TEXTURE_REMAP_##r<<10)|(CELL_GCM_TEXTURE_REMAP_##a<<8))
#define REMAP_SRC_BITS(a, r, g, b) ((CELL_GCM_TEXTURE_REMAP_FROM_##b<<6)|(CELL_GCM_TEXTURE_REMAP_FROM_##g<<4)|(CELL_GCM_TEXTURE_REMAP_FROM_##r<<2)|(CELL_GCM_TEXTURE_REMAP_FROM_##a<<0))
#if 0
// debug: show colours (remember to change shader)
#define REMAP_R() REMAP_OUT_BITS(REMAP,ONE,ZERO,ZERO) | REMAP_SRC_BITS(R,R,R,R)
#define REMAP_G() REMAP_OUT_BITS(REMAP,ZERO,ONE,ZERO) | REMAP_SRC_BITS(G,G,G,G)
#define REMAP_B() REMAP_OUT_BITS(REMAP,ZERO,ZERO,ONE) | REMAP_SRC_BITS(B,B,B,B)
#define REMAP_A() REMAP_OUT_BITS(REMAP,ONE,ZERO,ONE) | REMAP_SRC_BITS(A,A,A,A)
#else
#define REMAP_R() REMAP_OUT_BITS(REMAP,REMAP,REMAP,REMAP) | REMAP_SRC_BITS(R,R,R,R)
#define REMAP_G() REMAP_OUT_BITS(REMAP,REMAP,REMAP,REMAP) | REMAP_SRC_BITS(G,G,G,G)
#define REMAP_B() REMAP_OUT_BITS(REMAP,REMAP,REMAP,REMAP) | REMAP_SRC_BITS(B,B,B,B)
#define REMAP_A() REMAP_OUT_BITS(REMAP,REMAP,REMAP,REMAP) | REMAP_SRC_BITS(A,A,A,A)
#endif

	if (subTextureIndex < 4)
	{
		// set texture remap to pick correct channel
		U32 remap = 0;
		switch (subTextureIndex)
		{
			case 0: remap = REMAP_R(); break;
			case 1: remap = REMAP_G(); break;
			case 2: remap = REMAP_B(); break;
			case 3: remap = REMAP_A(); break;
		}
		pContext->m_Context.SetTextureRemap(1, remap);
	}
#undef REMAP_R
#undef REMAP_G
#undef REMAP_B
#undef REMAP_A
#undef REMAP_OUT_BITS
#undef REMAP_SRC_BITS
#endif

	pContext->m_Context.SetTextureControl(0, Display2::E_Bool_True, 0, 1<<8, CELL_GCM_TEXTURE_MAX_ANISO_1);
#endif

	//Stencil
#if PLATFORM_PS3

#endif

	
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

	Maths::HMat44 mShadowTexture;
	{
		Display::C_Matrix mTemp;
		Display::MatrixSetIdentity(&mTemp);
		mTemp._11 = 0.5f;
		mTemp._22 = -0.5f;
		mTemp._41 = 0.5f;
		mTemp._42 = 0.5f;
		mShadowTexture = Maths::Load<Maths::HMat44>(&mTemp);
	}
	const Maths::HMat44 mDeviceToWorld = pContext->GetView()->GetViewSettings3D().GetWorldToDevice().Inverse();

	

	//Calculate Shadow Plane Matrix = deviceworld * shadow * texture
	Display::C_Matrix mMatrixParameter;
	Maths::Store(mTexture * mDeviceToWorld * pInstance->m_ShadowPlaneSpaceInv * mShadowTexture, &mMatrixParameter);
	pContext->SetPixelShaderParameterArrayF(m_ScreenShadowPlaneSpaceParameter, (F32*)&mMatrixParameter); 
	
	MathsStorage::Float4 vDepthParameter = Render::C_RenderState::Instance().GetDepthScaleShaderParameter(pContext->GetView()->GetViewSettings3D().GetNearClip(), pContext->GetView()->GetViewSettings3D().GetFarClip());
	pContext->SetPixelShaderParameterF(m_ScreenDepthParameter, (F32*)&vDepthParameter);

	Display::C_Float4 pixelOffset(gShadowScreenXOffset/m_pDepthTexture->GetWidth(), gShadowScreenYOffset/m_pDepthTexture->GetHeight(), 0.f, 0.f);
	pContext->SetPixelShaderParameterF(m_PixelOffsetParameter, (F32*)&pixelOffset); 

	pContext->SetPixelShaderParameterF(m_ShadowMultParameter, (F32*)&shadowMult); 

#if PLATFORM_PC
	// [Blur2PC] Retail PC reads the blob from its channel: R, G, B, and A also for the imposter and
	// texture blobs (sub-texture index 4) (Blur.exe 1.2 0x00731560).
	const Display::C_PixelShader* pScreenBlobPixelShader = &m_ScreenBlobChannelPixelShader[(subTextureIndex < 3) ? subTextureIndex : 3];
	pContext->SetPixelShader(pScreenBlobPixelShader);
	pContext->SetPixelShaderParameters(pScreenBlobPixelShader);
#else
#ifndef PLATFORM_PS3
	pContext->SetPixelShader(&m_ScreenBlobPixelShader);
#endif
	pContext->SetPixelShaderParameters(&m_ScreenBlobPixelShader);
#endif

	pContext->SetVertexDeclaration(Display::Vertex_XYZ::m_pDeclaration);
	pContext->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XYZ));

	pContext->DrawPrimitive(Display2::E_PrimitiveType_TriangleList, 0, 36);


	Display::C_SamplerState defaultSample;
	defaultSample.SetDefaultStates();

	pContext->SetTexture(0, 0);
	pContext->SetSamplerState(0, &defaultSample);
	pContext->SetTexture(1, 0);
	pContext->SetSamplerState(1, &defaultSample);
	pContext->SetTexture(2, 0);
	pContext->SetSamplerState(2, &defaultSample);

}

void C_BlobShadows::Render(Display::C_Context* pContext, U32 visibilityFlags) const
{
	if (!gDebugBlobShadowsEnable)
		return;

	if (m_ProcessShadows.Size() == 0)
		return;

	if (m_resourcesPending)
		return;

	//Render the blob texture to the screen
	PERF_EVENT("RenderBlobShadows");

	pContext->SetBlendEnable(Display::E_Bool_True);
	pContext->SetBlendState(Display::E_Blend_One, Display::E_Blend_InvSrcAlpha, Display::E_BlendOp_Add, Display::E_Blend_Zero, Display::E_Blend_InvSrcAlpha, Display::E_BlendOp_Add);
	pContext->SetAlphaTestState(Display::E_Bool_True, 0); 
	pContext->SetBlendHighPrecisionEnable(Display::E_Bool_True);
	Display::C_StencilState stencilState;

	stencilState.m_StencilEnable = Display::E_Bool_False;
	pContext->SetStencilState(stencilState);

#if PLATFORM_PC // I have no idea why I have to do this, CW should work fine on all platforms
	// [Blur2PC] retail PC (Blur.exe 1.2 0x00732d20) culls CW as well: the box's far faces draw, so the shadow
	// shows with the camera inside it (showroom, countdown shots); CCW left only the near faces (T115)
	pContext->SetCullMode(Display::E_CullMode_Cw);
#else
	pContext->SetCullMode(Display::E_CullMode_Ccw);
#endif
	pContext->SetDepthEnable(Display::E_Bool_False);
	pContext->SetDepthWriteEnable(Display::E_Bool_False);

#ifdef PLATFORM_PS3
	pContext->SetPixelShader(&m_ScreenBlobPixelShader); //Set the pixel shader for PS3, so we only need SetPixelShaderParameters
#endif

	for (S32 i = 0; i < m_ProcessShadows.Size(); i++)
	{
		if (visibilityFlags & m_ProcessShadows[i].m_pInstance->m_VisibilityFlags)
			RenderScreenShadow(m_ProcessShadows[i], pContext);
	}

#if PLATFORM_PC
	// [Blur2PC] retail 0x00732d20 unbinds the three stages after the shadows; the window mask texture is a
	// render target again next frame
	pContext->SetTexture(0, 0);
	pContext->SetTexture(1, 0);
	pContext->SetTexture(2, 0);
#endif

	pContext->SetBlendMode(Display::E_BlendMode_Solid);
	pContext->SetCullMode(Display::E_CullMode_Default);
	pContext->SetDepthEnable(Display::E_Bool_True);
	pContext->SetDepthWriteEnable(Display::E_Bool_True);
}


C_DynamicBlobShadowInstance* C_BlobShadows::CreateInstance(const Display::C_VertexBuffer* pVertexBuffer, const U32 vertexBufferCount, const Display::C_IndexBuffer* pIndexBuffer, const U32 indexBufferCount)
{
	assert(pVertexBuffer);
	assert(vertexBufferCount);
	assert(pIndexBuffer);
	assert(indexBufferCount);

	C_DynamicBlobShadowInstance* pBlobShadowInstance = new C_DynamicBlobShadowInstance();

	pBlobShadowInstance->m_pShadowVertexBuffer = pVertexBuffer;
	pBlobShadowInstance->m_ShadowVertexCount = vertexBufferCount;
	pBlobShadowInstance->m_pShadowIndexBuffer = pIndexBuffer;
	pBlobShadowInstance->m_ShadowIndexCount = indexBufferCount;

	//Add to the list
	m_BlobShadowInstanceList.PushBack(pBlobShadowInstance);

	return pBlobShadowInstance;
}

C_DynamicBlobShadowInstance* C_BlobShadows::CreateInstance( WorldRender::C_ModelInstance* pModelInstance )
{
	assert(pModelInstance);

	C_DynamicBlobShadowInstance* pBlobShadowInstance = new C_DynamicBlobShadowInstance();
	
	pBlobShadowInstance->m_pShadowModelInstance = pModelInstance;

	//Get Material Parameters handles

	pBlobShadowInstance->m_WorldMatrixParameter = pModelInstance->GetRenderModel()->GetParameter("g_WorldMatrix");
	pBlobShadowInstance->m_ShadowPlaneSpaceMatrixParameter = pModelInstance->GetRenderModel()->GetParameter("g_ShadowPlaneSpaceMatrix");
	pBlobShadowInstance->m_ShadowPlaneParameter = pModelInstance->GetRenderModel()->GetParameter("g_ShadowPlane");
	pBlobShadowInstance->m_LightPositionParameter = pModelInstance->GetRenderModel()->GetParameter("g_LightPosition");
	pBlobShadowInstance->m_ShadowAlphaParameter = pModelInstance->GetRenderModel()->GetParameter("g_ShadowAlpha");

	//Add to the list
	m_BlobShadowInstanceList.PushBack(pBlobShadowInstance);

	return pBlobShadowInstance;
}

C_TextureBlobShadowInstance* C_BlobShadows::CreateInstance( const Display::C_Texture2D* pBlobTexture, F32 fadeDistant, const Maths::HVec3& shadowSize)
{
	assert(pBlobTexture);

	C_TextureBlobShadowInstance* pBlobShadowInstance = new C_TextureBlobShadowInstance();
	
	pBlobShadowInstance->m_FadeDistant = fadeDistant;
	pBlobShadowInstance->m_pBlobTexture = pBlobTexture;
	pBlobShadowInstance->m_ShadowSize = shadowSize;
	//Add to the list
	m_BlobShadowInstanceList.PushBack(pBlobShadowInstance);

	return pBlobShadowInstance;
}

void C_BlobShadows::DestroyInstance( C_BaseBlobShadowInstance* pInstance )
{
	for (S32 i = 0; i < m_ProcessShadows.Size(); i++)
	{
		if (m_ProcessShadows[i].m_pInstance == pInstance)
			m_ProcessShadows.EraseByIndex(i);
	}

	Btl::List<C_BaseBlobShadowInstance*>::Iterator it = m_BlobShadowInstanceList.Begin();
	while (it != m_BlobShadowInstanceList.End())
	{
		if (*it == pInstance)
		{
			m_BlobShadowInstanceList.Erase(it);

			SAFE_DELETE_PTR(pInstance);
			return;
		}
		++it;
	}
	BIZ_ASSERT_FAIL("C_BlobShadows::DestroyInstance - pInstance not found!");
}

void C_BlobShadows::Create( C_BlobShadowsInitSettings initSettings, SceneGraph::C_Context* pContext)
{
	m_ShadowSettings = initSettings;
	m_pContext = pContext;
}

void C_BlobShadows::Destroy()
{

}

void C_BlobShadows::OnRender(SceneGraph::C_Context& context) const
{
	Display::C_Context* pDisplay = Display::GetContext(&context);
	context.SetContextFlags(pDisplay->GetFlags());

	Render(pDisplay, context.GetView()->GetTypeFlag());
}

void C_BlobShadows::OnGather(const SceneGraph::C_View& view) const
{
	if (view.GetTypeFlag() & m_VisibilityFlags)
		view.AddVisibleNode(this, m_RenderPass);
}

void C_BlobShadows::Accept(SceneGraph::I_Visitor& visitor) const
{
	visitor.Visit(*this);
}

void C_BlobShadows::SetRenderPass(const U32 renderPass)
{
	m_RenderPass = renderPass;
}

void C_BlobShadows::ScreenRender() const
{
	//Display::C_DeviceContext *pContext = new Display::C_DeviceContext;
	Display::C_Context *pDContext = Display::GetContext(m_pContext);
	pDContext->SetColourDepthStencilTexture(NULL, m_pDepthTexture);
	pDContext->SetView(*Display::Device()->GetView());
	pDContext->BeginCommandBuffer(NULL);
	Render(Display::GetContext(pDContext));
	pDContext->EndCommandBuffer();
	//delete pContext;
}


void C_BlobShadows::SetShadowSize( F32 shadowSize )
{
	m_ShadowSize = shadowSize;
}

void C_BlobShadows::SetShadowHeight( F32 shadowHeight )
{
	m_ShadowHeight = shadowHeight;
}

void C_BlobShadows::SetShadowFade( F32 shadowFace )
{
	// shader uses reciprocal - calculate it here instead.
	m_ShadowFade = (fabsf(shadowFace) < 0.00002f) ? 65504.0f : (1.0f/shadowFace);
}

void C_BlobShadows::SetMaxShadows( S32 maxShadows )
{
	m_MaxShadows = maxShadows;
}

void C_BlobShadows::SetDropShadowIntensity( F32 dropShadowIntensity )
{
	m_DropShadowIntensity = dropShadowIntensity;
}

void C_BlobShadows::SetDropShadowHeight( F32 dropShadowHeight )
{
	m_DropShadowHeight = dropShadowHeight;
}

void C_BlobShadows::SetIndividualShadowMaxIntensity( F32 individualShadowMaxIntensity )
{
	m_IndividualShadowMaxIntensity = individualShadowMaxIntensity;
}

void C_BlobShadows::SetMultiShadowIntensity( F32 multiShadowIntensity )
{
	m_MultiShadowIntensity = multiShadowIntensity;
}

void C_BlobShadows::SetShadowMult( const Maths::HVec3& multVector )
{
	m_ShadowMult = multVector;
}
//////////////////////////////////////////////////////////////////////////
// BaseBlobShadowInstance
//////////////////////////////////////////////////////////////////////////
C_BaseBlobShadowInstance::C_BaseBlobShadowInstance()
{
	m_CasterWorldMatrix.SetIdentity();
	m_ShadowNormal = Maths::HVec3(0.f, 1.f, 0.f);
	m_Intensity = 1.f;
	m_Enable = true;
	m_VisibilityFlags = 0xFFFFFFFF;
}

void C_BaseBlobShadowInstance::CalculateShadowMatrices(F32 shadowSizeX, F32 shadowSizeY,  F32 shadowHeight)
{
	Maths::HVec3 planeNormal = m_ShadowNormal;

	//Calculate shadow plane
	m_ShadowPlane = Maths::C_Plane(Maths::C_Normal(planeNormal), Maths::C_Vertex(m_ShadowPosition));

	//Calculate shadow plane space using cameraZ
	Maths::HVec3 tox,toy,toz;
	Maths::HMat33 look;
	toz = planeNormal;
	toz.Normalise();
	tox = Maths::HVec3(m_CasterWorldMatrix.AZ())^toz;
	tox.Normalise();
	toy = toz^tox;
	toy.Normalise();
	look.AX(tox * shadowSizeX);
	look.AY(toy * shadowSizeY);
	look.AZ(toz * shadowHeight);
	m_ShadowPlaneSpace.SetIdentity();
	m_ShadowPlaneSpace = Maths::HMat44(look);
	m_ShadowPlaneSpace.AW(m_ShadowPosition || Maths::HVecW(1.f));

	m_ShadowPlaneSpaceInv = m_ShadowPlaneSpace.Inverse();
}

void C_BaseBlobShadowInstance::Enable( bool enable )
{
	m_Enable = enable;
}

//////////////////////////////////////////////////////////////////////////
// DynamicBlobShadowInstance
//////////////////////////////////////////////////////////////////////////

C_DynamicBlobShadowInstance::C_DynamicBlobShadowInstance()
{
	m_pShadowVertexBuffer = NULL;
	m_ShadowVertexCount = 0;
	m_pShadowIndexBuffer =  NULL;
	m_ShadowIndexCount = 0;
	m_pShadowModelInstance = NULL;

	m_DoubleBufferShadowCastersVector = 0;

	// [Blur2PC] render interpolation state
	m_Blur2PCPrevCaster.SetIdentity();
	m_Blur2PCCurCaster.SetIdentity();
	m_Blur2PCPrevShadowPos = Maths::HVec3(0.f, 0.f, 0.f);
	m_Blur2PCCurShadowPos = Maths::HVec3(0.f, 0.f, 0.f);
	m_Blur2PCTick = 0;
	m_Blur2PCHasPrev = false;
}

C_DynamicBlobShadowInstance::~C_DynamicBlobShadowInstance()
{
	m_pShadowModelInstance = NULL;
}

void C_DynamicBlobShadowInstance::ClearShadowCasters()
{
	m_DoubleBufferShadowCastersVector = (m_DoubleBufferShadowCastersVector + 1) % 2;
	m_ShadowCastersVector[m_DoubleBufferShadowCastersVector].Clear();
}

void C_DynamicBlobShadowInstance::AddShadowCaster( const C_BlobShadowCaster& shadowCaster )
{
	m_ShadowCastersVector[m_DoubleBufferShadowCastersVector].PushBack(shadowCaster);
}

void C_DynamicBlobShadowInstance::UpdatePosition( const Maths::HMat43& casterWorldMatrix, const Maths::HVec3& shadowPosition, const Maths::HVec3& shadowNormal, F32 priority, F32 cameraDistant, F32 intensity)
{
	// [Blur2PC] keep the previous update's pose for render interpolation (see Blur2PCApplyRenderPose)
	{
		const U32 tick = WorldRender::Blur2PCInterpCurrentTick();
		if (m_Blur2PCTick != tick)
		{
			const bool consecutive = (m_Blur2PCTick + 1 == tick);
			m_Blur2PCPrevCaster = consecutive ? m_Blur2PCCurCaster : casterWorldMatrix;
			m_Blur2PCPrevShadowPos = consecutive ? m_Blur2PCCurShadowPos : shadowPosition;
			m_Blur2PCHasPrev = consecutive && (F32)(casterWorldMatrix.AW() - m_Blur2PCPrevCaster.AW()).Length() < 20.0f;
			m_Blur2PCTick = tick;
		}
		m_Blur2PCCurCaster = casterWorldMatrix;
		m_Blur2PCCurShadowPos = shadowPosition;
	}
	m_CasterWorldMatrix = casterWorldMatrix;
	m_ShadowPosition = shadowPosition;
	m_ShadowNormal = shadowNormal;
	m_Priority = priority;
	m_CameraDistant = cameraDistant;
	m_GroundDistant = casterWorldMatrix.AW().Y() - shadowPosition.Y();
	m_Intensity = intensity;
}

// [Blur2PC] before a render, place the shadow at the pose the car is drawn at (between the last two updates)
void C_DynamicBlobShadowInstance::Blur2PCApplyRenderPose()
{
	const F32 alpha = WorldRender::Blur2PCInterpRenderAlpha();
	if (m_Blur2PCHasPrev && m_Blur2PCTick == WorldRender::Blur2PCInterpCurrentTick() && alpha < 1.f)
	{
		m_CasterWorldMatrix = WorldRender::Blur2PCBlendMatrix(m_Blur2PCPrevCaster, m_Blur2PCCurCaster, alpha);
		m_ShadowPosition = m_Blur2PCPrevShadowPos + (m_Blur2PCCurShadowPos - m_Blur2PCPrevShadowPos) * alpha;
	}
	else
	{
		m_CasterWorldMatrix = m_Blur2PCCurCaster;
		m_ShadowPosition = m_Blur2PCCurShadowPos;
	}
	m_GroundDistant = m_CasterWorldMatrix.AW().Y() - m_ShadowPosition.Y();
}

void C_DynamicBlobShadowInstance::CalculateShadowCasters(F32 quality, S32 maxShadowsSetting, F32 maxShadowDistant, F32 deltaFade)
{
	//Check current and latest shadow casters vector and calculate fades
	Btl::Vector<C_BlobShadowCaster>& latestShadowCasters = m_ShadowCastersVector[(m_DoubleBufferShadowCastersVector + 1) % 2];
	Btl::Vector<C_BlobShadowCaster>& currentShadowCasters = m_ShadowCastersVector[m_DoubleBufferShadowCastersVector];
	Btl::BitVector latestShadowCastersProcessed;
	latestShadowCastersProcessed.Resize(latestShadowCasters.Size());
	latestShadowCastersProcessed.ClearBits();

	//Calculate the max shadows from the distant and quality
	F32 maxShadowsFactor = F32(Maths::Min(currentShadowCasters.Size(), maxShadowsSetting));
	F32 distantFactor = Maths::Clamp(m_CameraDistant / maxShadowDistant, 0.f, 1.f);
	distantFactor = Maths::Clamp(((distantFactor - gBlobShadowAllShadowsRange) * gShadowCasterReductionFactor) / (1.f - gBlobShadowAllShadowsRange), 0.f, 1.f);
	maxShadowsFactor *= Maths::Clamp((1.f - distantFactor / Maths::Clamp(quality, 0.01f, 1.f)), 0.f, 1.f);
	S32 maxShadows = U32(maxShadowsFactor);

	//Clamp vector
	currentShadowCasters.Resize(maxShadows);

	for (S32 i = 0; i < currentShadowCasters.Size(); i++)
	{
		C_BlobShadowCaster& shadowCaster = currentShadowCasters[i];
		bool found = false;
		for (S32 j = 0; (j < latestShadowCasters.Size()) && (!found); j++)
		{
			if ((latestShadowCasters[j].m_Position - shadowCaster.m_Position).Length2() < 0.1f)
			{
				shadowCaster.m_Fade = Maths::Min(latestShadowCasters[j].m_Fade + deltaFade, 1.f);

				latestShadowCastersProcessed.SetBit(j);
				found = true;
			}
		}
		if (!found)
			shadowCaster.m_Fade = 0.f;
	}
	//Add all the rest of latest with fade out
	U32 fadeShadowCount = 0;
	for (S32 j = 0; j < latestShadowCasters.Size() && (fadeShadowCount < 5); j++)
	{
		if (!latestShadowCastersProcessed.GetBitValue(j))
		{
			if ((latestShadowCasters[j].m_Fade - deltaFade) > 0.f)
			{
				//Add
				currentShadowCasters.PushBack(latestShadowCasters[j]);
				currentShadowCasters.Back().m_Fade = latestShadowCasters[j].m_Fade - deltaFade;
				fadeShadowCount++;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
// TextureBlobShadowInstance
//////////////////////////////////////////////////////////////////////////
C_TextureBlobShadowInstance::C_TextureBlobShadowInstance()
{
	m_FadeDistant = 1.f;
}

C_TextureBlobShadowInstance::~C_TextureBlobShadowInstance()
{
}

void C_TextureBlobShadowInstance::UpdatePosition(const Maths::HMat43& casterWorldMatrix, const Maths::HVec3& shadowPosition, F32 intensity)
{
	m_CasterWorldMatrix = casterWorldMatrix;
	m_ShadowPosition = shadowPosition;
	m_Priority = -1.f; //not used
	m_CameraDistant = 0.f; //not used
	m_GroundDistant = casterWorldMatrix.AW().Y() - shadowPosition.Y();
	m_Intensity = intensity;
}


void C_BlobShadows::RenderCollisionMeshShadow( const Maths::HMat43& casterWorldMatrix, const Maths::HVec3& shadowPosition, F32 intensity, const Maths::HVec3* pCollisionMeshVertex, U32 numCollisionMeshVertex )
{
	if (m_resourcesPending)
		return;

	const Display::C_Texture2D* pBlobTexture = NULL;
	TextureResource::C_TextureResource* pTextureResource = m_ImposterShadowTextureHandle;
	if (pTextureResource && pTextureResource->IsDataValid())
	{
		pBlobTexture = (const Display::C_Texture2D*) pTextureResource->GetDisplayTexture();
	}
	else
		return;

	F32 shadowFade = 1.f;
	//Calculate shadow matrix
	//Calculate shadow plane space using cameraZ
	Maths::HVec3 tox,toy,toz;
	Maths::HMat33 look;
	toz = Maths::HVec3(0.f, 1.f, 0.f);
	toz.Normalise();
	tox = Maths::HVec3(casterWorldMatrix.AZ())^toz;
	tox.Normalise();
	toy = toz^tox;
	toy.Normalise();
	look.AX(tox * gImposterShadowSizeX);
	look.AY(toy * gImposterShadowSizeY);
	look.AZ(toz * shadowFade);
	Maths::HMat44 shadowPlaneSpace;
	shadowPlaneSpace.SetIdentity();
	shadowPlaneSpace = Maths::HMat44(look);
	shadowPlaneSpace.AW(shadowPosition || Maths::HVecW(1.f));

	Maths::HMat44 shadowPlaneSpaceInv = shadowPlaneSpace.Inverse();

	//Set States to render
	Display::Device()->SetBlendMode(Display::E_BlendMode_Multiply);
	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	Display::Device()->SetDepthFunc(Display::E_CmpFunc_Default);

	//Set Texture
	Display::Device()->SetTexture(0, pBlobTexture);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

	//Set Pixel and Vertex shader
	Display::C_Matrix shadowPlaneSpaceInvParam;
	Maths::Store(shadowPlaneSpaceInv, &shadowPlaneSpaceInvParam);
	Display::Device()->SetVertexShaderParameterF(m_CollisionMeshShadowSpaceParameter, (F32*)&shadowPlaneSpaceInvParam, 4); 

	Display::Device()->SetVertexShader(&m_CollisionMeshBlobVertexShader);


	Display::C_Float4 shadowMult(m_ShadowMult.X() * intensity * m_DropShadowIntensity, m_ShadowMult.Y() * intensity * m_DropShadowIntensity, m_ShadowMult.Z() * intensity * m_DropShadowIntensity, 1.f);
	Display::Device()->SetPixelShaderParameterF(m_CollisionMeshShadowMultParameter, (F32*)&shadowMult); 

	Display::Device()->SetPixelShader(&m_CollisionMeshBlobPixelShader);
	Display::Device()->SetPixelShaderParameters(&m_CollisionMeshBlobPixelShader);
	
	//Render
	Display::Vertex_XYZ* pVertex = (Display::Vertex_XYZ*)Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleList, numCollisionMeshVertex, sizeof(Display::Vertex_XYZ), Display::Vertex_XYZ::m_pDeclaration);
	if (pVertex)
	{
		for (U32 i = 0; i < numCollisionMeshVertex; i++)
		{
			//Maths::Store(pCollisionMeshVertex[i], (MathsStorage::Float3*)&pVertex->x);
			pVertex->x = pCollisionMeshVertex[i].X();
			pVertex->y = pCollisionMeshVertex[i].Y();
			pVertex->z = pCollisionMeshVertex[i].Z();
			pVertex++;
		}

		Display::Device()->EndVertices();
	}

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthEnable(Display::E_Bool_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_Default);
	Display::Device()->SetDepthFunc(Display::E_CmpFunc_Default);
	Display::Device()->SetTexture(0, NULL);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_Default);

	if (gDebugShowCollisionMesh)
	{
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw(pInstance->m_ShadowPosition, shadowLights, Maths::RGBA(0.f,1.f,0.f,1.f) );
	}
}

void C_BlobShadows::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
{
	switch(result)
	{
		case Resource::E_Notify_DataReady:
		{				
			m_resourcesPending = false;
		}
		break;
		case Resource::E_Notify_DataError:
		case Resource::E_Notify_NoData:
		break;
	}
}

};
