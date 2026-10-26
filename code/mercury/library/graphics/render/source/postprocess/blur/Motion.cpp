#include "StdAfx.h"
#include "Render/PostProcess/Blur/Motion.h"

#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"

#include "Render/Utils/Globals.h"

EXTERN_SHADER(psMotionBlur);

using namespace Display;

namespace PostProcess
{

Debug::Bool g_MotionBlur("Graphics.Render.PostProcess.MotionBlur.Enable", true);
Debug::Bool g_MotionBlurBlurDynamicObjects("Graphics.Render.PostProcess.MotionBlur.BlurDynamicObjects", false);

bool C_Motion::Create()
{
	RESOURCE_MEMORY_ID("PostProcess");

	Display::CreatePixelShaderFromFunc("MotionBlur", 0, &m_Shader, REF_SHADER(psMotionBlur));
	SetShader(&m_Shader);

	// default implementation has 5 samples
	SetNumSamples(5);
	
	
	return true;
}

void C_Motion::Destroy()
{
	if(m_Shader.IsCreated())
		Display::Device()->Destroy(&m_Shader);
}

void C_Motion::Display() const
{
	if(m_Manager.Enabled() && g_MotionBlur)
	{
		PERF_EVENT("MotionBlur");

		SetViewport();
		SetConstants();
		
		Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
		Display::Device()->SetTexture(1, Render::GetGlobalDepthStencilBuffer());
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp); 
		DisplayFinal();
	}
}

void C_Motion::SetConstants() const
{
	const Display::C_View* pView = Display::Device()->GetView();
	const Display::C_ViewSettings3D& viewSettings3D = pView->GetViewSettings3D();
	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
	
	// first we compute our motion blur matrix
	{
		// first matrix transforms from uv * depth space to projected space
		Maths::HMat44 mTexCoord0;
		{
			Display::C_Matrix mTemp;
			Display::MatrixSetIdentity(&mTemp);
			
			mTemp._11 = 2.0f;
			mTemp._31 = -1.0f;
			mTemp._22 = -2.0f;
			mTemp._32 = 1.0f;
			
			mTexCoord0 = Maths::Load<Maths::HMat44>(&mTemp);
		}

		// second matrix takes into account field of view and aspect ratio to get position in view space
		Maths::HMat44 mFovAspect;
		{
			Display::C_Matrix mTemp;
			Display::MatrixSetIdentity(&mTemp);

			Maths::HMat44 mProjection(Maths::Load<Maths::HMat44>(&viewSettings3D.GetProjMatrix()));

			mTemp._11 = 1.f / viewSettings3D.GetProjMatrix()._11;
			mTemp._22 = 1.f / viewSettings3D.GetProjMatrix()._22;

			// if the viewport has been flipped we need to negate these
			if(m_Manager.GetView().bFlip)
			{
				mTemp._11 *= -1.0f;
				mTemp._22 *= -1.0f;
			}

			mFovAspect = Maths::Load<Maths::HMat44>(&mTemp);
		}

		// third matrix is inverse view times previous matrix
		Maths::HMat44 mMove;
		{
			Maths::HMat44 mViewInverse(Maths::Load<Maths::HMat44>(&viewSettings3D.GetViewMatrix()));
			Maths::HMat44 mViewPrev(Maths::Load<Maths::HMat44>(&viewSettings3D.GetViewMatrixPrev()));
			mMove = mViewInverse.Inverse() * mViewPrev;

			// make sure pixel always gets projected into screen not outside it
			Maths::HVec4 vZero;
			vZero.W(Maths::Constants<Maths::HF32>::One());

			vZero = vZero * mMove;
			m_vMotionAmount= Maths::HVec3(vZero);
			if(vZero.Z() < Maths::Constants<Maths::HF32>::Zero())
			{
				mMove = mMove.Inverse();
			}
		}

		// forth matrix is just the projection matrix
		Maths::HMat44 mProjection(Maths::Load<Maths::HMat44>(&viewSettings3D.GetProjMatrix()));

		// fifth matrix is projected space to texture * depth space transform (sorta - depth in W)
		Maths::HMat44 mTexCoord1;
		{
			Display::C_Matrix mTemp;
			Display::MatrixSetIdentity(&mTemp);
			mTemp._11 = 0.5f;
			mTemp._41 = 0.5f;
			mTemp._22 = -0.5f;
			mTemp._42 = 0.5f;

			mTexCoord1 = Maths::Load<Maths::HMat44>(&mTemp);
		}

		// combine them all together to get final matrix
		Maths::HMat44 mFinal;
		mFinal = mFinal * mTexCoord0;
		mFinal = mFinal * mFovAspect;
		mFinal = mFinal * mMove;
		mFinal = mFinal * mProjection;
		mFinal = mFinal * mTexCoord1;

		Display::C_Matrix mFinalStore;
		Maths::Store(mFinal, &mFinalStore);

		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_PARAMETERS_0, (F32*)&mFinalStore, 4);
	}

	// now compute scaling factors to get linear depth
	{
		MathsStorage::Float4 fvec;
		fvec.x = viewSettings3D.GetFarClip() * viewSettings3D.GetNearClip() / (viewSettings3D.GetNearClip() - viewSettings3D.GetFarClip());
		fvec.y = viewSettings3D.GetFarClip() / (viewSettings3D.GetFarClip() - viewSettings3D.GetNearClip());
		fvec.z = viewSettings3D.GetFarClip();

		if(Display::Device()->GetDepthFlip())
			fvec.y = 1.0f - fvec.y;

		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_PARAMETERS_4, (F32*)&fvec, 1);
	}

	// step factor
	{
		F32 fNumSteps = 10.0f;
		F32 fStep = m_fShutterAngle * m_fSamplesMinusOneRcp;

		MathsStorage::Float4 fvec(fStep, fStep, -fStep, -fStep);
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_PARAMETERS_5, (F32*)&fvec, 1);

	}

	// pixel factor
	{
		F32 fWidth = (F32)pView->GetViewport().GetW();
		F32 fHeight = (F32)pView->GetViewport().GetH();
		
		if(!g_MotionBlurBlurDynamicObjects)
		{
			fWidth = 0.0f;
			fHeight = 0.0f;
		}
		else
		{
			fWidth = 1.0f / fWidth;
			fHeight = 1.0f / fHeight;
		}

		MathsStorage::Float4 fvec(fWidth, fHeight, fWidth, fHeight);
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_PARAMETERS_6, (F32*)&fvec, 1);
	}

}

}; // namespace PostProcess
