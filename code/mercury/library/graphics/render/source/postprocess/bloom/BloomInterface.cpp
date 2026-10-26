#include "StdAfx.h"
#include "Render/PostProcess/Bloom/BloomInterface.h"

#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"

using namespace Display;

namespace PostProcess
{

Debug::Int g_HDRBloomDownsamples("Graphics.Render.PostProcess.HDR.Bloom.Downsamples", 4, Debug::Range(0, 4));

Debug::Int g_HDRBloomOverride("Graphics.Render.PostProcess.HDR.Bloom.Override", false);
Debug::Float g_HDRBloomDownsampleScale0("Graphics.Render.PostProcess.HDR.Bloom.DownsampleScale0", 1.5f, Debug::Range(0.f, 8.0f), Debug::Step(0.1f));
Debug::Float g_HDRBloomDownsampleScale1("Graphics.Render.PostProcess.HDR.Bloom.DownsampleScale1", 1.0f, Debug::Range(0.f, 8.0f), Debug::Step(0.1f));
Debug::Float g_HDRBloomDownsampleScale2("Graphics.Render.PostProcess.HDR.Bloom.DownsampleScale2", 1.0f, Debug::Range(0.f, 8.0f), Debug::Step(0.1f));
Debug::Float g_HDRBloomDownsampleScale3("Graphics.Render.PostProcess.HDR.Bloom.DownsampleScale3", 1.0f, Debug::Range(0.f, 8.0f), Debug::Step(0.1f));

Debug::Float g_HDRBloomScale0("Graphics.Render.PostProcess.HDR.Bloom.Scale0", 1.0f, Debug::Range(0.f, 4.0f), Debug::Step(0.1f));
Debug::Float g_HDRBloomScale1("Graphics.Render.PostProcess.HDR.Bloom.Scale1", 1.0f, Debug::Range(0.f, 4.0f), Debug::Step(0.1f));
Debug::Float g_HDRBloomScale2("Graphics.Render.PostProcess.HDR.Bloom.Scale2", 1.0f, Debug::Range(0.f, 4.0f), Debug::Step(0.1f));
Debug::Float g_HDRBloomScale3("Graphics.Render.PostProcess.HDR.Bloom.Scale3", 1.0f, Debug::Range(0.f, 4.0f), Debug::Step(0.1f));

Debug::Float g_HDRBloomDeviation("Graphics.Render.PostProcess.HDR.Parameters.BloomDeviation", 2.0f, Debug::Range(0.f, 8.0f), Debug::Step(0.1f));

Debug::Bool g_HDRBloomFreeze("Graphics.Render.PostProcess.HDR.Bloom.Freeze", false);

extern Debug::Float g_HDRGamma;

BloomParameters::BloomParameters()
{
	nDownsamples = g_HDRBloomDownsamples <= 4 ? g_HDRBloomDownsamples : 4;
	fBloomDeviation = g_HDRBloomDeviation;

	for(U32 i = 0; i < nDownsamples; ++i)
	{
		bDownsample[i] = true;
		bBloom[i] = true;
		fDownsampleScale[i] = 1.0f;
		fBloomScale[i] = 1.0f;
		fCombinedScale[i] = 1.0f;
	}
}

BloomParameters::~BloomParameters()
{
}

void BloomParameters::Update()
{
	fCombinedScale[0] = fBloomScale[0] / fDownsampleScale[0];
	fCombinedScale[1] = fBloomScale[1] / (fDownsampleScale[0] * fDownsampleScale[1]);
	fCombinedScale[2] = fBloomScale[2] / (fDownsampleScale[0] * fDownsampleScale[1] * fDownsampleScale[2]);
	fCombinedScale[3] = fBloomScale[3] / (fDownsampleScale[0] * fDownsampleScale[1] * fDownsampleScale[2] * fDownsampleScale[3]);
}

I_Bloom::I_Bloom(C_Manager& manager) : I_PostProcess(manager), m_BloomDownsample(manager), m_BloomFilter(manager)
{
	m_TextureFilter = Display::E_TextureFilter_Point;

	m_bBloomGamma = false;
}

I_Bloom::~I_Bloom()
{
}

bool I_Bloom::Create()
{
	RESOURCE_MEMORY_ID("HDR");
	return true;
}

bool I_Bloom::CreateVolatile()
{
	return true;
}

void I_Bloom::Destroy()
{
	if(m_BloomCombine.IsCreated())
		Display::Device()->Destroy(&m_BloomCombine);
	if(m_BloomCombineGamma.IsCreated())
		Display::Device()->Destroy(&m_BloomCombineGamma);
}

void I_Bloom::DestroyVolatile()
{
}

void I_Bloom::UpdateParameters(BloomParameters& parameters) const
{
	if(!g_HDRBloomOverride)
		return;

	parameters.fBloomDeviation = g_HDRBloomDeviation;

	parameters.fDownsampleScale[0] = g_HDRBloomDownsampleScale0;
	parameters.fDownsampleScale[1] = g_HDRBloomDownsampleScale1;
	parameters.fDownsampleScale[2] = g_HDRBloomDownsampleScale2;
	parameters.fDownsampleScale[3] = g_HDRBloomDownsampleScale3;

	parameters.fCombinedScale[0] = g_HDRBloomScale0 / (parameters.fDownsampleScale[0]);
	parameters.fCombinedScale[1] = g_HDRBloomScale1 / (parameters.fDownsampleScale[0] * parameters.fDownsampleScale[1]);
	parameters.fCombinedScale[2] = g_HDRBloomScale2 / (parameters.fDownsampleScale[0] * parameters.fDownsampleScale[1] * parameters.fDownsampleScale[2]);
	parameters.fCombinedScale[3] = g_HDRBloomScale3 / (parameters.fDownsampleScale[0] * parameters.fDownsampleScale[1] * parameters.fDownsampleScale[2] * parameters.fDownsampleScale[3]);
	
	m_BloomFilter.SetDeviation(parameters.fBloomDeviation);
}

void I_Bloom::CalculateBloomTextures(const BloomParameters& parameters, RenderTexturesConst& bloomTextures) const
{
	for(U32 i = 0; i < parameters.nDownsamples && i < 4; ++i)
	{
		if(parameters.bDownsample[i])
		{
			m_BloomDownsample.SetMultiplier(parameters.fDownsampleScale[i]);
			m_BloomDownsample.Display();
		}

		if(parameters.bBloom[i])
		{
			m_BloomFilter.Display();
			const Display::C_RenderTexture2D* pBloomTexture = m_Manager.GetRenderTexture();
			bloomTextures.PushBack(pBloomTexture);
		}
	}

	Resolve();
}

}; // namespace PostProcess
