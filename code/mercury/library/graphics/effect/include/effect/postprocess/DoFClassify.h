#ifndef EFFECT_POSTPROCESS_DEPTHOFFIELD_CLASSIFY_H
#define EFFECT_POSTPROCESS_DEPTHOFFIELD_CLASSIFY_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Manager.h>
#include <Render/Texture/Downsample.h>
#include <Render/Texture/GaussianBlur.h>
#include <Effect/PostProcess/DoFBase.h>

namespace PostProcess
{

class C_DoFClassify : public I_PostProcess, public C_DoFBase
{
public:

	C_DoFClassify(C_Manager& manager);
	~C_DoFClassify();

	bool Create();
	bool CreateVolatile();
	void Destroy();
	void DestroyVolatile();
	void Update() {}

	void Display() const;

private:

	void RenderBlurAll() const;

	void RenderTilesFull() const;
	void RenderTilesIntermediate() const;
	void RenderTilesNone() const;

	void RenderDepthWeights() const;
	void RenderFocalDistances() const;

private:

	Render::Texture::C_Downsample	m_DownsampleUtil;
	Render::Texture::C_GaussianBlur	m_GaussianBlurUtil;

	Display::C_RenderTexture2D	m_FocusDistanceRenderTexture;
	Display::C_RenderTexture2D	m_DownsampledColourRenderTexture;
	Display::C_RenderTexture2D	m_DownsampledDepthMinRenderTexture;
	Display::C_RenderTexture2D	m_DownsampledDepthMaxRenderTexture;
	Display::C_RenderTexture2D	m_DownsampledBlurXRenderTexture;
	Display::C_RenderTexture2D	m_DownsampledBlurredRenderTexture;
	Display::C_RenderTexture2D	m_DepthWeightsRenderTexture;

	Display::C_VertexBuffer		m_VertexBuffer;
	U32							m_VertexCount;

	Display::C_PixelShader		m_BlurAllShader;

	Display::C_PixelShader		m_DepthMinDownsampleShader;
	Display::C_PixelShader		m_DepthMaxDownsampleShader;
	Display::C_PixelShader		m_DepthWeightsShader;
	Display::C_PixelShader		m_FocalDistanceShader;

	Display::C_PixelShader		m_TilesFullShader;
	Display::C_PixelShader		m_TilesIntermediateShader;
	Display::C_PixelShader		m_TilesNoneShader;
	Display::C_VertexShader		m_TilesVertexShader;

	Display::C_PixelShaderParameterArray	m_BlurDistancesParameter;
	Display::C_PixelShaderParameterArray	m_BlurWeightsParameter;
	Display::C_PixelShaderParameterArray	m_BlurTapsParameter;
	
	Display::C_PixelShaderParameterArray	m_TilesDistancesParameter;
	Display::C_PixelShaderParameterArray	m_TilesWeightsParameter;
	Display::C_PixelShaderParameterArray	m_TilesTapsParameter;

	Display::C_PixelShaderParameter			m_DepthMinSettingsParameter;
	Display::C_PixelShaderParameter			m_DepthMaxSettingsParameter;
	Display::C_PixelShaderParameter			m_DepthWeightsParameter;
	Display::C_PixelShaderParameter			m_FocalSettingsParameter;
	Display::C_PixelShaderParameter			m_FocalScaleParameter;

	F32	m_vTapDistances[25];
	F32	m_vTapWeights[25];
	F32	m_vTaps[50];

};

}; // namespace PostProcess

#endif // EFFECT_POSTPROCESS_DEPTHOFFIELD_CLASSIFY_H
