#ifndef POST_PROCESS_LAYER_H
#define POST_PROCESS_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Framework/Layer/Layer.h>
#include <misc_shared/Messaging/MessageHandler.h>

namespace PostProcess
{
	class C_BloomFeedbackGhost;
	class C_Manager;
	class C_MotionBlurReduced;
	class C_DoFFgBg;
}

class C_HighDynamicRange;

namespace Game
{

//! C_GameWorldPostProcess

class C_GameWorldPostProcess
{
public:

	C_GameWorldPostProcess();
	~C_GameWorldPostProcess();

	bool Create();
	void Destroy();
	void Update();

	bool CreateMemoryBuffer();
	void DestroyMemoryBuffer();

	void RenderSplitScreenWithAA();

	void BeginRenderSplitscreenWithoutAA();
	void EndRenderSplitscreenWithoutAA();

	void SetupPostProcess();
	void Resize();

	PostProcess::C_Manager*				GetManager()		{ return m_pPostProcessMgr; }
	PostProcess::C_BloomFeedbackGhost*	GetBloom()			{ return m_pPostProcessBloom; }
	C_HighDynamicRange*					GetHDR()			{ return m_pPostProcessHdr; }
	PostProcess::C_DoFFgBg*				GetDepthOfField()	{ return m_pPostProcessDoF; }
	PostProcess::C_MotionBlurReduced*	GetMotionBlur()		{ return m_pPostProcessMotionBlur; }

	void SetMotionBlur();
	void SetDepthOfField();

	void SetDofInfo(const F32 fFocusPlane, const F32 fAperture);

	void ClearExternalTextures();

	void SetQuality(const F32 fQuality);

protected:
	PostProcess::C_Manager*					m_pPostProcessMgr;
	PostProcess::C_MotionBlurReduced*		m_pPostProcessMotionBlur;
	PostProcess::C_DoFFgBg*					m_pPostProcessDoF;
	PostProcess::C_BloomFeedbackGhost*		m_pPostProcessBloom;
	C_HighDynamicRange*						m_pPostProcessHdr;
	bool									m_CanRender;

	F32										m_fFocusPlane;
	F32										m_fAperture;

	bool									m_bMotionBlur;
	bool									m_bDepthOfField;

	U32										m_iTextureBufferSize;
	U32										m_iDownSampleTextureBufferSize;
	U8*										m_pTextureBuffer;
	U8*										m_pDownSampleTextureBuffer;
};

} // namespace Game

#endif // GAME_LAYER_H
