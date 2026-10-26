#include "stdafx.h"

#include "../XsiIoUtility.h"

#include "XsiIoSceneImport.h"
#include "XsiIoSceneNodeImport.h"
#include "XsiIoSceneMeshImport.h"
#include "XsiIoImportOptions.h"
#include "XsiIoAnimationCurveImport.h"

using namespace XsiIo;

Aftk::C_Status C_AnimationCurveImport::Import(
	C_SceneImport* pSceneImport,
	CSLFCurve* pXsiAnimCurve,
	Aftk::C_AnimationCurveNode* pAftkAnimCurve)
{
	Aftk::C_Status status;

	//pAftkAnimCurve->SetName(pXsiAnimCurve->GetName());

	F32	oversampling    = 1.f;

	CSLSceneInfo*	pSceneInfo	= pSceneImport->GetXsiScenePtr()->SceneInfo();

	F32	frameStart		= pSceneInfo->GetStart();
	F32	frameEnd		= pSceneInfo->GetEnd();
	F32	frameDuration	= frameEnd - frameStart;
	F32	frameRate		= pSceneInfo->GetFrameRate();
	F32	frameStep		= 1.f;
	S32	frameCount		= 1 + (S32)(frameDuration / frameStep);

	if (pSceneInfo->GetTimingType() == CSLSceneInfo::SI_SECONDS)
	{
		frameStep /= frameRate;
	}

	F32 durationInSeconds = ((F32)frameCount * frameStep);

	if (pSceneInfo->GetTimingType() == CSLSceneInfo::SI_FRAMES)
	{
		durationInSeconds /= frameRate;
	}

	frameStep = 1.0f / oversampling;
	frameCount = static_cast<S32>(frameCount * oversampling);

	for(int frameNo = 0; frameNo < frameCount; frameNo++)
	{
		F32 frameX		= frameStart + (F32)frameNo * frameStep;
		F32 frameFraction	= (F32)frameNo / (F32)(frameCount - 1);
		F32 actualX = frameFraction * durationInSeconds;

		pXsiAnimCurve->Evaluate(frameX);
		F32 value = pXsiAnimCurve->GetLastEvaluation();

		U32 numKeys = pAftkAnimCurve->GetNumKeys();
		if (numKeys > 0)
		{
			//Ensure keys are ordererd
			Aftk::C_LinearKey key;
			pAftkAnimCurve->GetKey(key, numKeys-1);
			if (frameX < key.GetX())
			{
				//frameX is 0;
				break;
			}
		}
		
		Aftk::C_LinearKey newAftkKey;
		newAftkKey.GetX() = frameX;
		newAftkKey.GetValue() = value;
		pAftkAnimCurve->AddKey(newAftkKey);
	}

	return Aftk::C_Status::E_Status_Success;
}

