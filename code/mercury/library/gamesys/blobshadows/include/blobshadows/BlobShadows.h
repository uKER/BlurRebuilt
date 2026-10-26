#ifndef RENDER_DEFERRED_BLOB_SHADOWS_H
#define RENDER_DEFERRED_BLOB_SHADOWS_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Btl/Containers/Vector.h>
#include <Render/texture/GaussianBlur.h>
#include <world_render/MaterialParameter.h>
#include <TextureResource/TextureResource.h>
#include <SceneGraph/RenderNode.h>
#include <SceneGraph/GatherNode.h>

namespace SceneGraph
{
	class C_Context;
	class C_View;
}
namespace Display
{
	class C_DeviceContext;
	class C_Context;
}

namespace WorldRender
{

class C_Scene;
class C_ModelInstance;
}

namespace BlobShadows
{
struct C_BlobShadowCaster
{
	Maths::HVec3	m_Position;
	F32				m_Intensity;
	F32				m_Fade;

	C_BlobShadowCaster()
	{
		m_Intensity  = 0.f;
		m_Fade = 1.f;
	}
};

enum E_BlobShadowType
{
	E_BlobShadowType_Dynamic,
	E_BlobShadowType_Texture
};

class C_BaseBlobShadowInstance
{
protected:
	//Render settings
	Maths::C_Plane		m_ShadowPlane;
	Maths::HMat44		m_ShadowPlaneSpace;
	Maths::HMat44		m_ShadowPlaneSpaceInv;

	//State
	bool	m_Enable;

	//Priority settings
	F32				m_Priority;
	F32				m_CameraDistant;

	//Position settings
	Maths::HVec3	m_ShadowPosition;
	Maths::HVec3	m_ShadowNormal;
	Maths::HMat43	m_CasterWorldMatrix;
	F32				m_GroundDistant;
	F32				m_Intensity;

	//Visibility Flags
	U32				m_VisibilityFlags;

	//Functions
	void CalculateShadowMatrices(F32 shadowSizeX, F32 shadowSizeY, F32 shadowHeight);

	friend class C_BlobShadows;
public:
	C_BaseBlobShadowInstance();
	virtual ~C_BaseBlobShadowInstance(){};

	virtual E_BlobShadowType GetType() = 0;

	void Enable(bool enable);

	void SetVisibilityFlags(U32 visibilityFlags) { m_VisibilityFlags = visibilityFlags;};
};

class C_DynamicBlobShadowInstance : public C_BaseBlobShadowInstance
{
public:
	//Add lighting settings
	void ClearShadowCasters();
	void AddShadowCaster(const C_BlobShadowCaster& shadowCaster);

	//Update position
	void UpdatePosition(const Maths::HMat43& casterWorldMatrix, const Maths::HVec3& shadowPosition, const Maths::HVec3& shadowNormal, F32 priority, F32 cameraDistant, F32 intensity = 1.f);

	E_BlobShadowType GetType() {return E_BlobShadowType_Dynamic;}

	friend class C_BlobShadows;
private:
	C_DynamicBlobShadowInstance();
	~C_DynamicBlobShadowInstance();

	void CalculateShadowCasters(F32 quality, S32 maxShadows, F32 maxShadowDistant, F32 deltaFade);


	//Shadow mesh
	const Display::C_VertexBuffer*	m_pShadowVertexBuffer;
	U32								m_ShadowVertexCount;
	const Display::C_IndexBuffer*	m_pShadowIndexBuffer;
	U32								m_ShadowIndexCount;

	//Shadow model instance
	WorldRender::C_ModelInstance*	m_pShadowModelInstance;
	WorldRender::MaterialParameter	m_WorldMatrixParameter;
	WorldRender::MaterialParameter	m_ShadowPlaneSpaceMatrixParameter;
	WorldRender::MaterialParameter	m_ShadowPlaneParameter;
	WorldRender::MaterialParameter	m_LightPositionParameter;
	WorldRender::MaterialParameter	m_ShadowAlphaParameter;

	// [Blur2PC] render interpolation: caster pose and ground point at the previous and latest update
	Maths::HMat43					m_Blur2PCPrevCaster, m_Blur2PCCurCaster;
	Maths::HVec3					m_Blur2PCPrevShadowPos, m_Blur2PCCurShadowPos;
	U32								m_Blur2PCTick;
	bool							m_Blur2PCHasPrev;
	void							Blur2PCApplyRenderPose();

	//Light Array Settings
	Btl::Vector<C_BlobShadowCaster>	m_ShadowCastersVector[2];
	U32								m_DoubleBufferShadowCastersVector;
};

class C_TextureBlobShadowInstance : public C_BaseBlobShadowInstance
{
public:

	E_BlobShadowType GetType() {return E_BlobShadowType_Texture;}

	//Update position
	void UpdatePosition(const Maths::HMat43& casterWorldMatrix, const Maths::HVec3& shadowPosition, F32 intensity);

	friend class C_BlobShadows;
private:
	C_TextureBlobShadowInstance();
	~C_TextureBlobShadowInstance();

	//BlobTexture
	const Display::C_Texture2D* m_pBlobTexture;
	F32							m_FadeDistant;
	Maths::HVec3				m_ShadowSize;
};

// [Blur2PC] Retail PC is built per channel as well (Blur.exe 1.2): the blob texture render picks a
// R/G/B/A write mask from the sub-texture index (0x00733730), the pre-render blurs the textures that
// need it (0x00734e80), and the screen pass selects psScreenBlobShadowR/G/B/A (0x00731560). The
// snapshot's single-texture PC path never set a write mask, so the blob alpha the screen shader reads
// stayed 0 whenever the device mask was the default RGB, and cars had no shadow.
#if defined(PLATFORM_PS3) || defined(PLATFORM_PC)
#define BLOBSHADOW_PER_CHANNEL 1
#endif

#if defined(BLOBSHADOW_PER_CHANNEL)
#define BLOBSHADOWS_NUM_TEXTURES_256	1
#define BLOBSHADOWS_NUM_TEXTURES_128	1
#define BLOBSHADOWS_NUM_TEXTURES_64		1
#define BLOBSHADOWS_NUM_TEXTURES_32		1
#else
#define BLOBSHADOWS_NUM_TEXTURES_256	4
#define BLOBSHADOWS_NUM_TEXTURES_128	4
#define BLOBSHADOWS_NUM_TEXTURES_64		4
#define BLOBSHADOWS_NUM_TEXTURES_32		4
#endif

class C_BlobShadows : public Display::I_Renderer, public SceneGraph::I_RenderNode, public SceneGraph::I_GatherNode, public Resource::C_Listener
{
public:

	struct C_BlobShadowsInitSettings
	{
		U8		m_Num256Textures;
		U8		m_Num128Textures;
		U8		m_Num64Textures;
		U8		m_Num32Textures;
		F32		m_256TexturesDistant;
		F32		m_128TexturesDistant;
		F32		m_64TexturesDistant;
		F32		m_32TexturesDistant;
		U32		m_MinBlurResolution;
		F32		m_MaxShadowDistant;

		C_BlobShadowsInitSettings() :	m_Num256Textures(BLOBSHADOWS_NUM_TEXTURES_256),
										m_Num128Textures(BLOBSHADOWS_NUM_TEXTURES_128),
										m_Num64Textures(BLOBSHADOWS_NUM_TEXTURES_64),
										m_Num32Textures(BLOBSHADOWS_NUM_TEXTURES_32),
										m_256TexturesDistant(10.f),
										m_128TexturesDistant(20.f),
										m_64TexturesDistant(40.f),
										m_32TexturesDistant(60.f),
										m_MinBlurResolution(128),
										m_MaxShadowDistant(100.f)
		{};
	};

	C_BlobShadows();
	virtual ~C_BlobShadows();

	bool	OnCreate();
	bool	OnCreateVolatile();
	void	OnDestroy();
	void	OnDestroyVolatile();

	//Allocate resources
	void	Create(C_BlobShadowsInitSettings initSettings, SceneGraph::C_Context* pContext);
	void	Destroy();

	//Scenegraph
	void Render(Display::C_Context* pContext, U32 visibilityFlags = 0xFFFFFFFF) const;

	void OnRender(SceneGraph::C_Context& context) const;
	void OnGather(const SceneGraph::C_View& view) const;
	void Accept(SceneGraph::I_Visitor& visitor) const;
	void SetRenderPass(const U32 renderPass);
	void SetVisibilityFlags(const U32 visibilityFlags) {m_VisibilityFlags = visibilityFlags;}

	//Blob shadows instances
	C_DynamicBlobShadowInstance*	CreateInstance(const Display::C_VertexBuffer* pVertexBuffer, const U32 vertexBufferCount, const Display::C_IndexBuffer* pIndexBuffer, const U32 indexBufferCount);
	C_DynamicBlobShadowInstance*	CreateInstance(WorldRender::C_ModelInstance* pModelInstance);
	C_TextureBlobShadowInstance*	CreateInstance(const Display::C_Texture2D* pBlobTexture, F32 fadeDistant, const Maths::HVec3& shadowSize);
	void							DestroyInstance(C_BaseBlobShadowInstance* pInstance);

	//PreRender, render all the shadows textures
	void	PreRender();
	//Render the multiplied shadows to final screen
	void	ScreenRender() const;


	void	SetDepthTexture(const Display::C_Texture2D* pDepthTexture)							{ m_pDepthTexture = pDepthTexture; }

	//Settings access
	void	SetShadowSize(F32 shadowSize);
	void	SetShadowHeight(F32 shadowHeight);
	void	SetShadowFade(F32 shadowFace);
	void	SetShadowMult(const Maths::HVec3& multVector);

	void	SetDropShadowIntensity(F32 dropShadowIntensity);
	void	SetDropShadowHeight(F32 dropShadowHeight);
	void	SetIndividualShadowMaxIntensity(F32 individualShadowMaxIntensity);
	void	SetMultiShadowIntensity(F32 multiShadowIntensity);

	void	SetMaxShadows(S32 maxShadows);

	//Quality
	void	SetQuality(F32 quality) {m_Quality = quality;};

	//Draw collision mesh shadow
	void	RenderCollisionMeshShadow(const Maths::HMat43& casterWorldMatrix, const Maths::HVec3& shadowPosition, F32 intensity, const Maths::HVec3* pCollisionMeshVertex, U32 numCollisionMeshVertex);

private:
	// inherited from Resource::C_Listener
	public:
		virtual void ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData);

	//Params
	C_BlobShadowsInitSettings			m_ShadowSettings;
	SceneGraph::C_Context*				m_pContext;
	
	//Resources

	struct BlobTexture
	{
		Display::C_RenderTexture2D	m_Texture;
	#if defined(BLOBSHADOW_PER_CHANNEL)
		U32							m_Used;

	#else
		bool						m_Used;
	#endif
		U32							m_ResourceID;
	};

	volatile bool						m_resourcesPending;
	
	Btl::Vector<BlobTexture>			m_BlobRenderTexture;

	Display::C_Surface					m_DepthStencilSurface[4];

	Render::Texture::C_GaussianBlur		m_GaussianBlur;
	Display::C_RenderTexture2D			m_BlurBlobRenderTexture[4];
	S32									m_BlurCount;
	U8									m_NeedsBlurring[4];
	
	const Display::C_Texture2D*			m_pDepthTexture;

	//Shaders and parameteres
	Display::C_VertexShader				m_ProjectBlobVertexShader;
	Display::C_PixelShader				m_ProjectBlobPixelShader;

	Display::C_VertexShader				m_ScreenBlobVertexShader;
	Display::C_PixelShader				m_ScreenBlobPixelShader;
#if PLATFORM_PC
	// [Blur2PC] Retail PC: one screen shader per blob texture channel, R, G, B, A (Blur.exe 1.2 0x00730160)
	Display::C_PixelShader				m_ScreenBlobChannelPixelShader[4];
#endif

	Display::C_VertexShader				m_CollisionMeshBlobVertexShader;
	Display::C_PixelShader				m_CollisionMeshBlobPixelShader;

	Display::C_VertexShaderParameter			m_ProjectShadowPlaneSpaceParameter;
	Display::C_VertexShaderParameter			m_ProjectShadowPlaneParameter;
	Display::C_VertexShaderParameter			m_ProjectLightPositionParameter;
	Display::C_PixelShaderParameter				m_ProjectShadowAlphaParameter;

	Display::C_PixelShaderParameterArray		m_ScreenShadowPlaneSpaceParameter;
	Display::C_PixelShaderParameter				m_ScreenOriginPositionParameter;
	Display::C_PixelShaderParameter				m_ScreenDepthParameter;
	Display::C_PixelShaderParameter				m_PixelOffsetParameter;
	Display::C_PixelShaderParameter				m_ShadowMultParameter;

	Display::C_VertexShaderParameter			m_CollisionMeshShadowSpaceParameter;
	Display::C_PixelShaderParameter				m_CollisionMeshShadowMultParameter;

	Display::C_VertexBuffer						m_VertexBuffer;


	//Internal structures
	struct C_ProcessShadow
	{
		C_BaseBlobShadowInstance*	m_pInstance;
		U32							m_RenderTextureIndex;

		C_ProcessShadow() : m_pInstance(NULL), m_RenderTextureIndex(0){};
	};

	Btl::Vector<C_ProcessShadow>	m_ProcessShadows;

	//Shadows instances
	Btl::List<C_BaseBlobShadowInstance*> m_BlobShadowInstanceList; 


	//Settings
	F32					m_ShadowSize;
	F32					m_ShadowHeight;
	F32					m_ShadowFade;
	S32					m_MaxShadows;
	
	F32					m_DropShadowIntensity;
	F32					m_DropShadowHeight;
	F32					m_IndividualShadowMaxIntensity;
	F32					m_MultiShadowIntensity;
	F32					m_Quality;
	Maths::HVec3		m_ShadowMult;

	U32					m_RenderPass;
	U32					m_VisibilityFlags;

	//Imposter Shadow
	TextureResource::C_TextureHandle		m_ImposterShadowTextureHandle;

	//Internal render functions
	bool CreateRenderTarget(U32 index, U32 resolution, const Display::C_BackBuffer& backBuffer);
	U32 GetResourceID(U32 resolution);
	void CalculateProcessShadows();
	void RenderBlobTexture(const C_ProcessShadow& processShadow);
	void RenderScreenShadow(const C_ProcessShadow& processShadow, Display::C_Context* pContext) const;

	static int PrioritySortFunc(const void *A, const void *B);
};

};

#endif // RENDER_DEFERRED_DROP_SHADOWS_H
