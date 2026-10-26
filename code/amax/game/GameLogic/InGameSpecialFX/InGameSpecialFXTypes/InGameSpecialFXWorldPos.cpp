#include "stdafx.h"

#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXWorldPos.h>

namespace Game
{
	void C_InGameSpecialFXWorldPos::Update()
	{
		PERF_EVENT("InGameSFX.WorldPos.Update");

		if(mBumperSpecialFX)
			mBumperSpecialFX->UpdateWorldMatrix(mMatrix);

		if(mChaseSpecialFX)
			mChaseSpecialFX->UpdateWorldMatrix(mMatrix);

		if(mGlobalSpecialFX)
			mGlobalSpecialFX->UpdateWorldMatrix(mMatrix);

		C_InGameSpecialFXBase::Update();
	}
}