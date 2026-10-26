#include "stdafx.h"

#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXBase.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>	// [Blur2PC] RLPickupSpecialFX
#include <GameApp.h>	// [Blur2PC] C_GameApp

namespace Game
{
	C_InGameSpecialFXBase::C_InGameSpecialFXBase(E_InGameSpecialFXBase type, const RLPickupSpecialFX& sfxBase, U32 id, int vehicle_index, float time, int viewport, 
		int firerViewport, Maths::HVec3 scale) 
		: mType(type), mUniqueId(id), mLifetime(time), mCurrentMode(E_InGameSpecialFX_Mode_Intro), mVehicleIndex(vehicle_index), mViewport(viewport), 
		mFirerViewport(firerViewport), mScale(scale), mForceChaseCam(false)
	{
		mChaseSpecialFX = mBumperSpecialFX = mGlobalSpecialFX = NULL;
		mInBumperCam = false;

		mIntroStart = mCurrentTime = mTotalElapsedTime = sfxBase.mDurationIntro.mStart;
		mIntroEnd	= sfxBase.mDurationIntro.mEnd;
		mLoopStart	= sfxBase.mDurationLoop.mStart;
		mLoopEnd	= sfxBase.mDurationLoop.mEnd;
		mOutroStart = sfxBase.mDurationOutro.mStart;
		mOutroEnd	= sfxBase.mDurationOutro.mEnd;

		if(sfxBase.mGlobalSpecialFX.Object())
		{
			mGlobalSpecialFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(
				sfxBase.mGlobalSpecialFX, 
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetDefaultContextProvider(), 
				C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());
		}

		if(firerViewport != -1 && firerViewport == viewport && sfxBase.mLocalSpecialFX.Object())
		{
			mChaseSpecialFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(
				sfxBase.mLocalSpecialFX, 
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetDefaultContextProvider(), 
				C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());
		}
		else if(sfxBase.mExternalSpecialFX.Object())
		{
			mChaseSpecialFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(
				sfxBase.mExternalSpecialFX, 
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetDefaultContextProvider(), 
				C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());
		}

		if(firerViewport != -1 && firerViewport == viewport && sfxBase.mInternalSpecialFX.Object())
		{
			mBumperSpecialFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(
				sfxBase.mInternalSpecialFX, 
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetDefaultContextProvider(), 
				C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());
		}
		else
		{
			mBumperSpecialFX = mChaseSpecialFX;
		}

		if(mLoopStart == mLoopEnd && time > sfxBase.mDurationIntro.mEnd)
			mSpeed = sfxBase.mDurationIntro.mEnd / time;
		else
			mSpeed = 1.0f;
	}

	// -----------------------------------------------------------------------

	C_InGameSpecialFXBase::~C_InGameSpecialFXBase()
	{
		if(mChaseSpecialFX) 
		{ 
			if(mChaseSpecialFX != mBumperSpecialFX)
			{
				mChaseSpecialFX->Stop(); 
				SAFE_DELETE_PTR(mChaseSpecialFX); 
			}
			else
				mChaseSpecialFX = NULL;
		}

		if(mBumperSpecialFX) 
		{ 
			mBumperSpecialFX->Stop(); 
			SAFE_DELETE_PTR(mBumperSpecialFX); 
		}

		if(mGlobalSpecialFX)
		{
			mGlobalSpecialFX->Stop();
			SAFE_DELETE_PTR(mGlobalSpecialFX);
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXBase::Update()
	{
		if(mCurrentTime > 0.f)
		{
			bool inBumperCamNow = C_InGameSpecialFXManager::Instance().IsCamInternal(mVehicleIndex, mViewport) && mViewport == mFirerViewport;

			if((mInBumperCam && !inBumperCamNow) || mForceChaseCam)
			{
				if(mBumperSpecialFX)
					mBumperSpecialFX->Stop();

				if(mChaseSpecialFX)
					mChaseSpecialFX->Start();

				mInBumperCam = false;
			}
			else if(!mInBumperCam && inBumperCamNow)
			{
				if(mChaseSpecialFX)
					mChaseSpecialFX->Stop();

				if(mBumperSpecialFX)
					mBumperSpecialFX->Start();

				mInBumperCam = true;
			}

			if(mGlobalSpecialFX)
			{
				mGlobalSpecialFX->Update(mCurrentTime * mSpeed);
			}

			if(mInBumperCam && mBumperSpecialFX && !mForceChaseCam)
			{
				mBumperSpecialFX->Update(mCurrentTime * mSpeed);
			}
			else if(mChaseSpecialFX)
			{
				mChaseSpecialFX->Update(mCurrentTime * mSpeed);
			}
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXBase::ScaleMatrix(Maths::HMat43& matrix) const
	{
		Maths::HVec3 pos = matrix.AW();
		Maths::HVec3 origin(0,0,0);
		matrix.AW(origin);

		Maths::C_MatrixScale scale(mScale);
		Maths::HMat43 scaleMat(scale);

		Maths::HMat43 invMatrix = matrix.InverseRotTrans();
		Maths::HMat43 localMatrix = invMatrix * matrix;
		localMatrix = localMatrix * scaleMat;
		matrix = localMatrix * matrix;

		matrix.AW(pos);
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXBase::StartLooping()
	{
		if(mCurrentMode != E_InGameSpecialFX_Mode_Looping)
		{
			mCurrentTime = mLoopStart;
			mCurrentMode = E_InGameSpecialFX_Mode_Looping;	
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXBase::StartOutro()
	{
		if(mCurrentMode != E_InGameSpecialFX_Mode_Outro)
		{
			mCurrentTime = mOutroStart;
			mCurrentMode = E_InGameSpecialFX_Mode_Outro;	
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXBase::ForceChaseCam()
	{
		mForceChaseCam = true;
	}

	void C_InGameSpecialFXBase::CeaseForceChaseCam()
	{
		mForceChaseCam = false;
	}
}