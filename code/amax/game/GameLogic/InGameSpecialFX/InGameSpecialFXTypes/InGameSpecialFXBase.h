#ifndef INGAME_SPECIALFX_BASE_H
#define INGAME_SPECIALFX_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SpecialFX\SpecialFX.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXEnums.h>

struct RLPickupSpecialFX;

namespace Game
{
	class C_InGameSpecialFXBase
	{
	public:
		C_InGameSpecialFXBase(E_InGameSpecialFXBase type, const RLPickupSpecialFX& sfxBase, U32 id, int vehicle_index, float time, int viewport, int firerViewport, Maths::HVec3 scale);
		virtual ~C_InGameSpecialFXBase();

		E_InGameSpecialFXBase GetType() { return mType; }

		virtual void			Update();

		void StartLooping();
		void StartOutro();
		void ScaleMatrix(Maths::HMat43& matrix) const;

		void ForceChaseCam();
		void CeaseForceChaseCam();

		SpecialFX::I_SpecialFXInstance* mGlobalSpecialFX;
		SpecialFX::I_SpecialFXInstance* mChaseSpecialFX;
		SpecialFX::I_SpecialFXInstance* mBumperSpecialFX;

		E_InGameSpecialFXBase			mType;

		float							mCurrentTime;
		float							mTotalElapsedTime;

		float							mIntroStart;
		float							mIntroEnd;
		float							mLoopStart;
		float							mLoopEnd;
		float							mOutroStart;
		float							mOutroEnd;

		E_InGameSpecialFX_Mode			mCurrentMode;
		float							mSpeed;
		float							mLifetime;

		U32								mUniqueId;
		int								mViewport;
		int								mFirerViewport;
		int								mVehicleIndex;

		bool							mInBumperCam;
		bool							mForceChaseCam;
		Maths::HVec3					mScale;
	};
};

#endif //INGAME_SPECIALFX_BASE_H