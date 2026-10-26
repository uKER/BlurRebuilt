#include "stdafx.h"
#include "Graphics/NormalBuffer.h"

#include "Render/State/RenderState.h"
#include "Render/Utils/Globals.h"

C_NormalBuffer::C_NormalBuffer() : Display::I_Renderer("NormalBuffer")
{
}

C_NormalBuffer::~C_NormalBuffer()
{
	DestroyRenderer();
}

bool C_NormalBuffer::OnCreate()
{
	return true;
}

bool C_NormalBuffer::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("NormalBuffer");

	// TODO: make this work with splitscreen
	const U32 iWidth = Display::Device()->GetBackBuffer().GetWidth();
	const U32 iHeight = Display::Device()->GetBackBuffer().GetHeight();

	if (!Display::Device()->CreateRenderTexture2D(iWidth, iHeight, 1, 0, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_RenderTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_RenderTexture, "_NormalBufferRT");
	#endif

	return true;
}

void C_NormalBuffer::OnDestroy()
{
}

void C_NormalBuffer::OnDestroyVolatile()
{
	Display::Device()->Destroy(&m_RenderTexture);
}

void C_NormalBuffer::Render()
{
	PERF_EVENT("NormalBuffer");

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
	const Maths::HMat44 mDeviceToWorld = Display::Device()->GetViewSettings3D().GetWorldToDevice().Inverse();

	MathsStorage::Float4 vDepthScale = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	Display::C_Matrix mToWorld;
	Maths::Store(mTexture * mDeviceToWorld, &mToWorld);

	m_DepthTextureUtil.WorldNormal(&m_RenderTexture, Render::GetGlobalDepthStencilBuffer(), vDepthScale, mToWorld);
}
