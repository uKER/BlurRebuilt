
#include "stdafx.h"

#include "Render/Debug/Charts/ChartRenderer.h"
#include "Render/Debug/Charts/ChartInterface.h"

#include "Render/Shaders/Constants.h"
#include <system/DebugLog.h>

EXTERN_SHADER(vsDebug2D);
EXTERN_SHADER(psDebug);

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/Debug/PS3/vsDebug2D.shader"
#include "Render/Shaders/Debug/PS3/psDebug.shader"
#endif
#endif

using namespace Render::Chart;

namespace Render
{
namespace Chart
{
	::Debug::Logger &g_Log = ::Debug::GetLogger("mercury.render.chart");
};
;}

C_ChartRenderer::C_ChartRenderer() : Display::I_Renderer("Chart")
{
}

C_ChartRenderer::~C_ChartRenderer()
{
	DestroyRenderer();
}

bool C_ChartRenderer::OnCreate()
{
	RESOURCE_MEMORY_ID("ChartRenderer");

	// use basic 2D shaders from DebugRenderer
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsDebug2D", 0, &m_VertexShader, REF_SHADER(vsDebug2D)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDebug", 0, &m_PixelShader, REF_SHADER(psDebug)))
		return false;

	return true;
}

void C_ChartRenderer::OnDestroy()
{
	Display::Device()->Destroy(&m_VertexShader);
	Display::Device()->Destroy(&m_PixelShader);
}

void C_ChartRenderer::Prepare(const I_Chart* pChart) const
{
	const Display::C_Viewport& viewport = Display::Device()->GetViewport();

	MathsStorage::Float4 vClipVector;

	if(pChart->m_eSpace == E_ChartSpace_Relative)
		vClipVector = MathsStorage::Float4(2.0f, -2.f, -1.f, 1.f);
	else
		vClipVector = MathsStorage::Float4(2.0f / viewport.GetW(), -2.0f / viewport.GetH(), -1.f, 1.f);

	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_CLIP_VECTOR, (F32*)&vClipVector, 1);
	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetVertexShader(&m_VertexShader);
	Display::Device()->SetPixelShader(&m_PixelShader);
}

void C_ChartRenderer::RenderDeferred()
{
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

	for(S32 ii = 0; ii < m_DeferredCharts.Size(); ++ii)
	{
		m_DeferredCharts[ii]->Render();
	}
	ResetDeferred();

	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
}




