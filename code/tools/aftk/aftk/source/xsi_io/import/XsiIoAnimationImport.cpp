#include "stdafx.h"

#include "../XsiIoUtility.h"

#include "XsiIoSceneImport.h"
#include "XsiIoSceneNodeImport.h"
#include "XsiIoSceneMeshImport.h"
#include "XsiIoImportOptions.h"
#include "XsiIoAnimationImport.h"
#include "XsiIoAnimationCurveImport.h"

//#define XSI_IO_EXPORT_ANIM_VIA_CLIP_MIXER

using namespace XsiIo;

Aftk::C_Status AnimationImport::Import(
	C_SceneImport* pSceneImport,
	CSLModel* pXsiNode, 
	Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;

	if (!pXsiNode->Transform())
	{
		return Aftk::C_Status::E_Status_Success;
	}

	U32 nCurves = pXsiNode->Transform()->GetFCurveCount();
	//std::cout << "C_Node " << m_pXsiNode->Name().GetText() << " has " << nCurves << " curves" << std::endl;
	if (nCurves == 0)
		return Aftk::C_Status::E_Status_Success;

	//Create proxy objects to contain and route animation data

	//Firstly - create a new source clip to store our animation data
#ifdef XSI_IO_EXPORT_ANIM_VIA_CLIP_MIXER
	Aftk::C_ClipSourceNode* pAftkClipSourceNode = 
		new Aftk::C_ClipSourceNode(pAftkSceneNode->GetScenePtr());
	
	std::string clipName = "xsiClip_";
	clipName += pAftkSceneNode->GetName();

	pAftkClipSourceNode->SetName(clipName);

	//Create an instance of our source clip
	Aftk::C_ClipNode* pAftkClipNode = 
		new Aftk::C_ClipNode(pAftkSceneNode->GetScenePtr());
#endif

	CSLFCurve** ppXsiCurves = pXsiNode->Transform()->FCurves();

	float	oversampling    = 1.f;

	CSLSceneInfo* pSceneInfo	= pXsiNode->Scene()->SceneInfo();
	float	frameStart			= pSceneInfo->GetStart();
	float	frameEnd			= pSceneInfo->GetEnd();
	float	frameDuration		= frameEnd - frameStart;
	float	frameRate			= pSceneInfo->GetFrameRate();

	pAftkSceneNode->GetScenePtr()->SetStartTime(frameStart);
	pAftkSceneNode->GetScenePtr()->SetEndTime(frameEnd);
	pAftkSceneNode->GetScenePtr()->SetFrameRate(frameRate);

#ifdef XSI_IO_EXPORT_ANIM_VIA_CLIP_MIXER
	//Mapping between clip curves and scene node
	Aftk::C_ClipCurveMapping mapping;
	mapping.SetSceneNodePtr(pAftkSceneNode);
	pAftkClipNode->SetName("xsiClipInstance");

	bool mixerCurves[Aftk::E_AnimAttribute_Count];
	memset(mixerCurves, 0, sizeof(bool)*Aftk::E_AnimAttribute_Count);

	//Add the FCurves to our source clip
	for (U32 iCurveIndex = 0; iCurveIndex < nCurves; iCurveIndex++)
	{
		//Create a new anim curve in our scene.
		Aftk::C_AnimationCurveNode* pAftkAnimCurve = 
			new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());

		// Find rotation, scaling and translation for current frame from curves.
		CSLFCurve* pXsiAnimCurve = ppXsiCurves[iCurveIndex];

		if (
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_SCALING_X) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_SCALING_Y) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_SCALING_Z) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_ROTATION_X) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_ROTATION_Y) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_ROTATION_Z) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_TRANSLATION_X) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_TRANSLATION_Y) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_TRANSLATION_Z))
			continue;

		status = C_AnimationCurveImport::Import(
			pSceneImport,
			pXsiAnimCurve,
			pAftkAnimCurve);
		if (!status) return status;

		//Flag the the curve's attribute as animated.
		mixerCurves[pAftkAnimCurve->GetUsage()] = true;

		//Associate our anim curve with our clip source.
		U32 curveIndex = pAftkClipSourceNode->SetCurvePtr(pAftkAnimCurve->GetUsage(), pAftkAnimCurve);
		//Add a mapping between this node, this clip and the curves of the clip
		mapping.AddCurveIndex(curveIndex);
	}

	//Establish the instance link - THIS MUST BE DONE AFTER ADDING THE CURVES
	pAftkClipNode->SetSourceNodePtr(pAftkClipSourceNode);
	pAftkClipNode->SetStartX(pAftkClipSourceNode->GetStartX());
	pAftkClipNode->SetEndX(pAftkClipSourceNode->GetEndX());
    
	//Create a mixer to link the model with its animation
	Aftk::C_ClipMixerNode* pAftkClipMixerNode = 
		new Aftk::C_ClipMixerNode(pAftkSceneNode->GetScenePtr());
	pAftkClipMixerNode->SetName("xsiClipMixer");

	//Create a new track for the clip to inhabit
	pAftkClipMixerNode->AddTrack();
	Aftk::C_Track* pAftkTrack = &pAftkClipMixerNode->GetTrack(0);

    //Add the clip to the mixer track
	pAftkTrack->AddClipNodePtr(pAftkClipNode);

	//Attach the curve mapping info to our clip.
	pAftkClipNode->AddClipCurveMapping(mapping);

	for (U32 i = 0; i < Aftk::E_AnimAttribute_Count; i++)
	{
		//If this attribute is animated...
		if (mixerCurves[i])
		{
			Aftk::C_NullPlug* pPlug = 
				pAftkSceneNode->GetTransformPlugPtr((Aftk::E_AnimAttribute)i);
			AFTK_ASSERT(pPlug);
			pPlug->SetSourceConnectionPtr(pAftkClipMixerNode);
		}
	}
	
	AFTK_ASSERT(pAftkClipMixerNode->IsUsed());
//Export animation curves as direct connections.
#else
	//Add the FCurves to our source clip
	for (U32 iCurveIndex = 0; iCurveIndex < nCurves; iCurveIndex++)
	{
		//Create a new anim curve in our scene.
		Aftk::C_AnimationCurveNode* pAftkAnimCurve = 
			new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());

		// Find rotation, scaling and translation for current frame from curves.
		CSLFCurve* pXsiAnimCurve = ppXsiCurves[iCurveIndex];

		if (
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_SCALING_X) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_SCALING_Y) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_SCALING_Z) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_ROTATION_X) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_ROTATION_Y) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_ROTATION_Z) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_TRANSLATION_X) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_TRANSLATION_Y) &&
			(pXsiAnimCurve->GetFCurveType() != CSLTemplate::SI_TRANSLATION_Z))
			continue;

		status = C_AnimationCurveImport::Import(
			pSceneImport,
			pXsiAnimCurve,
			pAftkAnimCurve);
		if (!status) return status;

		//Convert animation curves into radians.
		if (
			(pXsiAnimCurve->GetFCurveType() == CSLTemplate::SI_ROTATION_X) ||
			(pXsiAnimCurve->GetFCurveType() == CSLTemplate::SI_ROTATION_Y) ||
			(pXsiAnimCurve->GetFCurveType() == CSLTemplate::SI_ROTATION_Z))
		{
			for (U32 i = 0; i < pAftkAnimCurve->GetNumKeys(); i++)
			{
				Aftk::C_LinearKey key;
				pAftkAnimCurve->GetKey(key, i);
				key.GetValue() = Maths::DegToRad(key.GetValue());
				pAftkAnimCurve->SetKey(i, key);
			}
		}

		Aftk::I_Plug* pPlugToConnect;
		std::string curveName = pAftkSceneNode->GetName();
	switch(pXsiAnimCurve->GetFCurveType())
	{
		case CSLTemplate::SI_TRANSLATION_X:	
		{
			curveName += "_TranslateX";
			pPlugToConnect = pAftkSceneNode->GetTranslatePlug().GetChildPtr(0);
			break;
		}
		case CSLTemplate::SI_TRANSLATION_Y:
		{
			curveName += "_TranslateY";
			pPlugToConnect = pAftkSceneNode->GetTranslatePlug().GetChildPtr(1);
			break;
		}
		case CSLTemplate::SI_TRANSLATION_Z:	
		{
			curveName += "_TranslateZ";
			pPlugToConnect = pAftkSceneNode->GetTranslatePlug().GetChildPtr(2);
			break;
		}

		case CSLTemplate::SI_ROTATION_X:
		{
			curveName += "_RotateX";
			pPlugToConnect = pAftkSceneNode->GetRotatePlug().GetChildPtr(0);
			break;
		}
		case CSLTemplate::SI_ROTATION_Y:
		{
			curveName += "_RotateY";
			pPlugToConnect = pAftkSceneNode->GetRotatePlug().GetChildPtr(1);
			break;
		}
		case CSLTemplate::SI_ROTATION_Z:
		{
			curveName += "_RotateZ";
			pPlugToConnect = pAftkSceneNode->GetRotatePlug().GetChildPtr(2);
			break;
		}

		case CSLTemplate::SI_SCALING_X:
		{
			curveName += "_ScaleX";
			pPlugToConnect = pAftkSceneNode->GetScalePlug().GetChildPtr(0);
			break;
		}
		case CSLTemplate::SI_SCALING_Y:
		{
			curveName += "_ScaleY";
			pPlugToConnect = pAftkSceneNode->GetScalePlug().GetChildPtr(1);
			break;
		}
		case CSLTemplate::SI_SCALING_Z:
		{
			curveName += "_ScaleZ";
			pPlugToConnect = pAftkSceneNode->GetScalePlug().GetChildPtr(2);
			break;
		}

		default:
			pPlugToConnect = 0;
			break;
		};

		if (!pPlugToConnect)
			AFTK_DO_BEGIN << "Failed to find plug to connect to." AFTK_DO_ERROR_RETURN_END_1;

		//Connect our animation curve to our scene node.
		pPlugToConnect->SetSourceConnectionPtr(pAftkAnimCurve);
		pAftkAnimCurve->SetName(curveName);
	}
#endif
	return Aftk::C_Status::E_Status_Success;
}

