#ifndef RENDER_POSTPROCESSMANAGER_H
#define RENDER_POSTPROCESSMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Utility/Singleton.h>
#include <Render/PostProcess/Interface.h>

#if PLATFORM_PS3
#define USE_POSTPROCESS_RENDERBUFFERS 1
#endif

namespace PostProcess
{

enum E_Shader
{
	E_Shader_WeightedFilter4 = 0,
	E_Shader_WeightedFilter5,
	E_Shader_WeightedFilter8,
	E_Shader_WeightedFilter9,
	E_Shader_WeightedFilter13,
	E_Shader_WeightedFilter15,
	E_Shader_WeightedFilter16,
	E_Shader_WeightedFilter4Gamma,
	E_Shader_WeightedFilter5Gamma,
	E_Shader_WeightedFilter8Gamma,
	E_Shader_WeightedFilter9Gamma,
	E_Shader_WeightedFilter13Gamma,
	E_Shader_WeightedFilter15Gamma,
	E_Shader_WeightedFilter16Gamma,
	E_Shader_Count
};

enum E_RenderTargetFormat
{
	// fixed point formats
	E_RenderTargetFormat_RGBA_U8,
	E_RenderTargetFormat_RG_U16,
	E_RenderTargetFormat_RGBA_U16,
	E_RenderTargetFormat_RGBA_U32,
	E_RenderTargetFormat_RG_U32,
	E_RenderTargetFormat_Depth_U24,
	E_RenderTargetFormat_Depth_U16,

	// floating point formats
	E_RenderTargetFormat_RGBA_F16,
	E_RenderTargetFormat_RGBA_F10,
	E_RenderTargetFormat_RG_F16,
	E_RenderTargetFormat_Depth_F24,

	E_RenderTargetFormat_Unknown
};

INLINE Display::E_Format DisplayTextureFormat(const E_RenderTargetFormat format)
{
	switch(format)
	{
	case E_RenderTargetFormat_RGBA_U8:
		return Display::E_Format_A8R8G8B8;
	
	case E_RenderTargetFormat_RG_F16:
		return Display::E_Format_G16R16F;
	
	case E_RenderTargetFormat_RGBA_F16:
		return Display::E_Format_A16B16G16R16F;

	case E_RenderTargetFormat_RGBA_F10:
		return Display::E_Format_A16B16G16R16F;

	case E_RenderTargetFormat_Depth_U24:
		return Display::E_Format_D24S8;

	case E_RenderTargetFormat_Depth_F24:
		#if PLATFORM_PC
		return Display::E_Format_D24S8;
		#else
		return Display::E_Format_D24FS8;
		#endif

	default:
		assert(0 && "Unsupported format");
		return Display::E_Format_Unknown;
	}
}

struct C_View
{
	friend class C_Manager;
private:
	U32												iWidth;
	U32												iHeight;
	S32												iIndex;
	U32												nDownsamples;
	E_RenderTargetFormat							eFormat;
	Display::E_Multisample							eMultisample;
public:
	bool											bFlip;
public:
	U32						GetWidth() const				{ return iWidth; }
	U32						GetHeight() const				{ return iHeight; }
	E_RenderTargetFormat	GetRenderTargetFormat() const	{ return eFormat; }
};

struct C_RenderTarget
{
	U32												iWidth;
	U32												iHeight;
	S32												iIndex;
	Display::C_Surface*							m_pDepthStencilSurface;
#if USE_POSTPROCESS_RENDERBUFFERS
	Ut::Vector<Display2::C_RenderBuffer*>		m_RenderTextures;
#else
	Ut::Vector<Display::C_RenderTexture2D*>		m_RenderTextures;
#endif
};

class I_PostProcess;
typedef Ut::UtSet<I_PostProcess*>					PostProcesses;

class C_Manager : public Display::I_Renderer
{
public:

	C_Manager();
	~C_Manager();

	bool								OnCreate();
	bool								OnCreateVolatile();
	void								OnDestroy();
	void								OnDestroyVolatile();

	// public interface
	bool								Enabled() const;
	void								Begin(S32 iIndex, const Display::C_Viewport& viewport) const;
	void								SetView(S32 iIndex) const;
	void								Suspend() const;
	void								End() const;
	
	bool								AddView(const U32 iWidth, const U32 iHeight, const S32 iIndex, const PostProcess::E_RenderTargetFormat eFormat, const Display::E_Multisample eMultisample, const U32 nDownsamples = 0, const bool bFlip = false);
	void								ClearViews();

	// primary texture buffer is for a viewport sized texture, it can overlap the current texture for that viewport
	void								SetPrimaryTextureBuffer(U8* pPrimaryTextureBuffer, U32 iPrimaryTextureBufferSize);

	// secondary texture buffer is for downsamples etc, so can probably overlap depth buffer
	void								SetSecondaryTextureBuffer(U8* pSecondaryTextureBuffer, U32 iSecondaryTextureBufferSize);

	void								SetFrameTime(F32 fFrameTime);
	F32									GetFrameTime() const;

	void								SetViewport(const Display::C_Viewport& viewport) const;
	void								SetViewport() const;
	const Display::C_Viewport&			GetViewport() const;
	const Display::C_Viewport&			GetViewportOrigin() const;

	void								SetRenderTexture(const Display::C_RenderTexture2D& renderTexture) const;
	const Display::C_RenderTexture2D*	GetRenderTexture() const;
	
	void								SetMainTexture(const Display::C_Texture2D* pMainTexture) const;
	const Display::C_Texture2D*			GetMainTexture() const;
	const Display::C_Texture2D*			GetChainTexture() const;

	const Display::C_PixelShader*		GetShader(U32 iIndex) const;
	C_View&								GetView(U32 iIndex);
	
	const C_RenderTarget&				GetRenderTarget(U32 iIndex) const;
	const C_RenderTarget&				GetRenderTargetMapped(U32 iIndex) const;
	const C_View&						GetView(U32 iIndex) const;
	const C_View&						GetView() const;

	U32									GetRenderTargetCount() const;
	U32									GetViewCount() const;
	S32									GetViewIndex() const;
	S32									GetViewIndex(S32 displayViewIndex) const;
	S32									GetRenderTargetIndex() const;

	void								ReserveView(const U32 width, const U32 height) { m_ReserveWidth = width; m_ReserveHeight = height; }

	U32									GetMaximumSurfaceSize(const S32 iWidthScale = 1, const S32 iHeightScale = 1, const Display::E_Format eFormat = Display::E_Format_Unknown) const;
	U32									GetMaximumTextureSize(const S32 iWidthScale = 1, const S32 iHeightScale = 1, const Display::E_Format eFormat = Display::E_Format_Unknown) const;

	bool								CreateUserRenderTextures(const U32 iWidthScale, const U32 iHeightScale, const U32 flags, const Display::E_Format eFormat, const Display::C_SurfaceParams* pSurfaceParams, RenderTextures& renderTextures, U8* pAddress, U32 bufferSize) const;
	void								DestroyUserRenderTextures(RenderTextures& renderTextures) const;

	void								SetActive(bool bActive);
	void								SetName(const Util::C_String32& name);

	friend class	I_PostProcess;
protected:
	void										AddPostProcess(I_PostProcess* pPostProcess);
	void										RemovePostProcess(I_PostProcess* pPostProcess);
	bool										CreateRenderTextures();
	
	const Display::C_RenderTexture2D*			GetRenderTexture(const U32 iViewIndex, const Display::C_Viewport& viewport) const;
	void										ResolveRenderTexture() const;
	void										SetChainTexture() const;
	void										RestoreViewport() const;
	void										RestoreColourSurface() const;
	void										SetFinalQuadSettings() const;
	
	mutable const Display::C_Texture2D*			m_pMainTexture;
	mutable const Display::C_Texture2D*			m_pMainTextureDefault;

	mutable const Display::C_Texture2D*			m_pChainTexture;

	mutable const Display::C_RenderTexture2D*	m_pRenderTexture;
	mutable const Display::C_Viewport*			m_pViewportRestore;
	mutable Display::C_Viewport					m_Viewport;
	mutable Display::C_Viewport					m_ViewportOrigin;
	mutable S32									m_iView;				// current view
	mutable S32									m_iRenderTarget;		// current RenderTargett
	mutable bool								m_bActive;			// this is true during rendering which will have post effects applied later
	mutable bool								m_bEnabled;			// this is whether post effects are enabled or disabled

	Display::C_PixelShader						m_Shaders[E_Shader_Count];

	U32											m_ReserveWidth;
	U32											m_ReserveHeight;
	
	U8*											m_pPrimaryTextureBuffer;
	U32											m_iPrimaryTextureBufferSize;

	U8*											m_pSecondaryTextureBuffer;
	U32											m_iSecondaryTextureBufferSize;
	
	PostProcesses								m_PostProcesses;
	Ut::Vector<PostProcess::C_RenderTarget>		m_RenderTargets;		// a number of different sized/format textures
	Ut::Vector<S32>								m_RenderTargetMapping;	// mapping between view index and m_RenderTargets, -1 indicates no mapping
	Ut::Vector<PostProcess::C_View>				m_Views;			// one for each viewport which has post effects
	Ut::Vector<S32>								m_ViewMapping;		// mapping between view index and unique post effects view index
	F32											m_fFrameTime;
	Util::C_String32							m_Name;
};

INLINE const Display::C_Viewport& C_Manager::GetViewport() const
{
	return m_Viewport;
}

INLINE const Display::C_Viewport& C_Manager::GetViewportOrigin() const
{
	return m_ViewportOrigin;
}

INLINE bool C_Manager::Enabled() const
{
	return m_bEnabled;
}

INLINE void C_Manager::SetActive(bool bActive)
{
	m_bActive = bActive;
}

INLINE void C_Manager::SetPrimaryTextureBuffer(U8* pPrimaryTextureBuffer, U32 iPrimaryTextureBufferSize)
{
	m_pPrimaryTextureBuffer = pPrimaryTextureBuffer;
	m_iPrimaryTextureBufferSize = iPrimaryTextureBufferSize;
}

INLINE void C_Manager::SetSecondaryTextureBuffer(U8* pSecondaryTextureBuffer, U32 iSecondaryTextureBufferSize)
{
	m_pSecondaryTextureBuffer = pSecondaryTextureBuffer;
	m_iSecondaryTextureBufferSize = iSecondaryTextureBufferSize;
}

INLINE void C_Manager::ClearViews()
{
	m_Views.Clear();
}

INLINE U32 C_Manager::GetViewCount() const
{
	return m_Views.Size();
}

INLINE U32 C_Manager::GetRenderTargetCount() const
{
	return m_RenderTargets.Size();
}

INLINE S32 C_Manager::GetViewIndex() const
{
	return m_iView;
}

INLINE S32 C_Manager::GetViewIndex(S32 displayViewIndex) const
{
	if(displayViewIndex < m_ViewMapping.Size())
		return m_ViewMapping[displayViewIndex];
	else
		return -1;
}

INLINE C_View& C_Manager::GetView(U32 iIndex)
{
	return m_Views[iIndex];
}

INLINE const C_View& C_Manager::GetView(U32 iIndex) const
{
	return m_Views[iIndex];
}

INLINE const C_View& C_Manager::GetView() const
{
	return m_Views[m_iView];
}

INLINE const C_RenderTarget& C_Manager::GetRenderTarget(U32 iIndex) const
{
	return m_RenderTargets[iIndex];
}

INLINE const C_RenderTarget& C_Manager::GetRenderTargetMapped(U32 iIndex) const
{
	return m_RenderTargets[m_RenderTargetMapping[iIndex]];
}

INLINE S32 C_Manager::GetRenderTargetIndex() const
{
	return m_iRenderTarget;
}

INLINE void C_Manager::SetFrameTime(F32 fFrameTime)
{
	m_fFrameTime = fFrameTime;
}

INLINE F32 C_Manager::GetFrameTime() const
{
	return m_fFrameTime;
}

INLINE const Display::C_RenderTexture2D* C_Manager::GetRenderTexture() const
{
	return m_pRenderTexture;
}

INLINE const Display::C_Texture2D* C_Manager::GetChainTexture() const
{
	return m_pChainTexture;
}

INLINE void C_Manager::SetMainTexture(const Display::C_Texture2D* pMainTexture) const
{
	m_pMainTexture = pMainTexture;
}

INLINE const Display::C_Texture2D* C_Manager::GetMainTexture() const
{
	// return main texture if we have it, otherwise the default
	return m_pMainTexture ? m_pMainTexture : m_pMainTextureDefault;
}

INLINE const Display::C_PixelShader* C_Manager::GetShader(U32 iIndex) const
{
	if (iIndex < E_Shader_Count)
		return &m_Shaders[iIndex];
	else
		return NULL;
}

} // namespace PostProcess

#endif // RENDER_POSTPROCESSMANAGER_H
