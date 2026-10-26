#ifndef UILAYER_H
#define UILAYER_H

#ifdef _MSC_VER
#pragma once
#include <framework/view/View.h>	// [Blur2PC]
#endif

#include <Framework/Layer/Layer.h>

#include "ui/UiManager.h"

namespace PostProcess
{
class C_Manager;
class C_Bloom;
struct BloomParameters;
};

namespace UI
{

class C_UiRenderBuffer;
class C_UiPostProcessBuffer;
class C_DepthOfField;
class C_UiXtBloomParameters;

class C_UiLayer : public Framework::C_Layer, public Messaging::C_MessageMultiHandler
{
public:
	C_UiLayer( );
	virtual ~C_UiLayer();

								// For layer messages
	HRESULT						ProcessMessage(const Framework::E_LayerMessage message);

								// For game messages
	bool						MessageHandler(const Messaging::C_MessageBase* pMessage);
							
	HRESULT 					Create();
	HRESULT 					Destroy();
	HRESULT 					GlobalUpdate();
	HRESULT						OnResize();
	HRESULT						RenderOnce();
	HRESULT						RenderFullscreen();

	C_UiManager*				GetUiManager()	{ return m_pUiManager; }
	bool						IsFrontend() const { return (m_IsFrontend || m_LoadingFrontend == true); }

	void						EnablePostProcessing(bool postProcessing)	{ m_PostProcessing = postProcessing; }

	void						AddDeferredGameMessage( Messaging::TMsgType type );
	void						ProcessDeferredGameMessages();

private:
	// Setup
	void						CreateRenderBuffer();
	void						DestroyRenderBuffer();

	void						CreateMiniMapRenderTarget( const U32 localPlayerIndex, const U32 size );
	void						DestroyMiniMapRenderTarget( const U32 localPlayerIndex );

	void						CreateFePostProcessBuffer();
	void						CreateIgPostProcessBuffer();

	void						DestroyIgPostProcessBuffer();
	void						DestroyFePostProcessBuffer();
							
	void						CreateBloom();
	void						DestroyBloom();
	void						SetBloomParameters( const C_UiXtBloomParameters& xtParams );

	// Frontend
	void						FeRenderComposite();
	void						FeRenderDepthOfField();
	void						FeRenderPost();
	// Ingame
	void						IgRenderComposite();
	void						IgRenderPost();
	void						IgRenderMiniMap( const U32 localPlayerIndex );

	void						SetupViews();
	void						RefreshXtConfig();

	C_UiRenderBuffer*			CreateRenderBufferImpl( U32 sizeX = 0, U32 sizeY = 0 );
	void						DestroyRenderBufferImpl( C_UiRenderBuffer* rb );

private:
	static const U32			m_IngamePostProcessWidth	= 1280 / 4;
	static const U32			m_IngamePostProcessHeight	= 720 / 4;
	static const U32			m_MaxLocalPlayers = 4;

	C_UiManager*				m_pUiManager;
	PostProcess::C_Manager*		m_pPostProcessMgr;
	bool						m_IsFrontend;
	bool						m_LoadingFrontend;
	bool						m_StencilClear;
	bool						m_PostProcessing;
	Framework::C_View			m_View;

	C_UiRenderBuffer*			m_pFeCompositeRB;

	C_UiPostProcessBuffer*		m_pFeCompositeRT;
	C_UiPostProcessBuffer*		m_pIgCompositeRT;

	PostProcess::C_Bloom*			m_pBloom;
	const Display::C_Texture*		m_pBloomTexture;
	PostProcess::BloomParameters*	m_pBloomParameters;
	int								m_SchedulerGroup;
	Display::C_PixelShader			m_fastBloomCompositeShader;					//!< New shader to composite bloom and UI in frontend in single pass,
																				//   batch is already heavily ROP bound, addition downsampled Bloom
																				//	 tap should effectively be free (w.r.t to frame latency).
	Display::C_PixelShaderParameter	m_textureParam;								//!< Parameter.

	MT::Lock						m_CriticalSecion;
	Btl::Vector< Messaging::TMsgType >													m_DeferredMessages;
	Btl::Vector< C_UiRenderBuffer*, Btl::VectorParameters_Array< m_MaxLocalPlayers > >	m_pMiniMapRT;
};

} // namespace UI
#endif
