#ifndef POST_PROCESS_LAYER_H
#define POST_PROCESS_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Framework/Layer/Layer.h>

namespace PostProcess
{
	class C_Manager;
	class C_EdgeAntiAliasing;
	class C_HighDynamicRange;
	class I_Bloom;
	class C_MotionBlurDoFReduced;
	class C_MotionBlurReduced;
	class C_DoFClassify;
	class C_DoFFgBg;
	class C_DoFReduced;
	class C_DoFSimple;
}

namespace Game
{

//! C_PostProcessLayer

class C_PostProcessLayer : public Framework::C_Layer
{
public:

	C_PostProcessLayer();
	virtual ~C_PostProcessLayer();

	HRESULT ProcessMessage(const Framework::E_LayerMessage message);

	HRESULT Create();
	HRESULT Destroy();
	HRESULT Resize();
	HRESULT Update();

	HRESULT RenderSplitscreenWithAA();

	HRESULT BeginRenderSplitscreenWithoutAA();
	HRESULT EndRenderSplitscreenWithoutAA();

	void SetupPostProcess();
	
	S32 GetLayerPriority() const { return Framework::E_LayerPriority_First; }

	PostProcess::C_Manager* GetManager() { return m_pPostProcessMgr; }
	PostProcess::C_DoFFgBg* GetDepthOfField() { return m_pPostProcessDoF; }

	void SetDofInfo(F32 fFocusPlane, F32 fAperture);

private:

	PostProcess::C_Manager*					m_pPostProcessMgr;
	PostProcess::C_EdgeAntiAliasing*		m_pPostProcessEdgeAA;
	PostProcess::C_HighDynamicRange*		m_pPostProcessHdr;
	PostProcess::I_Bloom*					m_pPostProcessBloom;
	PostProcess::C_DoFFgBg*					m_pPostProcessDoF;
	PostProcess::C_MotionBlurReduced*		m_pPostProcessMotionBlur;
	PostProcess::C_MotionBlurDoFReduced*	m_pPostProcessMotionBlurDoF;
	
	U8*	m_pTextureBuffer;
	U32	m_iTextureBufferSize;

	F32	m_fFocusPlane;
	F32 m_fAperture;
};

} // namespace Game

#endif // GAME_LAYER_H
