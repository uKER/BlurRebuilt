#include "stdafx.h"

#include "../FbxIoUtility.h"

#include "FbxIoSceneImport.h"
#include "FbxIoSceneNodeImport.h"
#include "FbxIoImportOptions.h"
#include "FbxIoAnimationCurveImport.h"

using namespace FbxIo;

Aftk::C_Status C_AnimationCurveImport::Import(
	C_SceneImport* pSceneImport,
	KFCurve* pFbxAnimCurve,
	Aftk::C_AnimationCurveNode* pAftkAnimCurve)
{
	Aftk::C_Status status;

	//Get time info of the animation
	KFbxTakeInfo* takeInfo  = pSceneImport->GetTakeInfo();
	KTimeSpan localTs		= takeInfo->mLocalTimeSpan;
	KTime start				= localTs.GetStart();
	KTime stop				= localTs.GetStop();
	KTime duration			= localTs.GetDuration();
	KTime::ETimeMode mode	= duration.GetGlobalTimeMode();
		
	//We need to evaluate the source curve on every frame and add
	//keys to every frame of the target curve in order to keep
	//the accuracy.
	KTime period;
	period.SetTime(0, 0, 0, 1, 0, mode); //1 frame period

	for(KTime i = start; i <= stop; i += period) 
	{
		double value = pFbxAnimCurve->Evaluate(i);
		Aftk::C_LinearKey newAftkKey;
		newAftkKey.GetX() = (F32)i.GetFrame(true);
		newAftkKey.GetValue() = (F32)value;
		pAftkAnimCurve->AddKey(newAftkKey);
	}

	/*
	//Old evaluation method, only done on keyframes
	int keyCount = pFbxAnimCurve->KeyGetCount();

	for(int i = 0; i < keyCount; i++) {
		KFCurveKey key = pFbxAnimCurve->KeyGet(i);
		KTime time = key.GetTime();
		double value = key.GetValue();

		Aftk::C_LinearKey newAftkKey;
		newAftkKey.GetX() = (F32)time.GetFrame(true);
		newAftkKey.GetValue() = (F32)value;
		pAftkAnimCurve->AddKey(newAftkKey);
	}*/

	return Aftk::C_Status::E_Status_Success;
}

