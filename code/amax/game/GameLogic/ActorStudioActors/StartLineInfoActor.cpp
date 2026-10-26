#include "stdafx.h"
#include <shared_actors/Utility/VisibilityDetection.h>	// [Blur2PC]


//#include <actor_system/ActorSys.h>
//#include <Game/ActorDef/ActorTypes.h>
//#include <Game/ActorDef/GameActor.h>
//#include <shared_actors/XtActorBase.h>
//
//#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
//#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
//#include <shared_actors/Components/Messages/ResourceMessages.h>
//
//#include <Dynamic/Particle/EventInstance.h>
//#include <Dynamic/Particle/EventInstanceHandle.h>
//#include <Dynamic/Common.h>
//#include <Dynamic/Renderer.h>
//#include <Dynamic/Particle/Manager.h>
//
//#include <GameLogic/ActorStudioActors/ActorUtils.h>
//
#include <GameLogic/ActorStudioActors/EffectGeneratorActor.h>
#include <GameLogic/ActorStudioActors/StartLineInfoActor.h>

#include <ui\Game\UILocaliser.h>
//
//#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
//
//#include <Game/GameUtil/ResourceBag.h>
//
//#include <btl/containers/algorithm.h>
//
//#include <actor_studio_msg_game/ActorStudioMessaging.h>
//
//#include <GameApp.h>
//
//#include <misc_shared\Messaging\GameMessaging.h>
//#include <shared_actors\Components\Messages\MovementMessages.h>
//
#include <GameLogic\InGameStartLineInfoManager.h>

//=========================================================================
//		Sorcery definitions
//=========================================================================


XT_TYPE(Actors::StartInfoType,								"StartLineInfoEnum")
	XT_OPTION(Actors::StartInfoType_Start,					"Start")
	XT_OPTION(Actors::StartInfoType_Checkpoint,				"Checkpoint")
	XT_OPTION(Actors::StartInfoType_Final,					"Final")
	XT_OPTION(Actors::StartInfoType_Finish,					"Finish")
	XT_OPTION(Actors::StartInfoType_Checkpoint_StartFinish,	"Checkpoint_StartFinish")
XT_END_TYPE


//	Actor creation definition (Sorcery)

XT_TYPE(Actors::StartLineInfoCreationData,	"StartLineInfo")
	XT_BASE(Actors::EffectGeneratorCreationData)
	XT_FIELD(mType,						"Type")
XT_END_TYPE



//=========================================================================
//					EffectGenerator Actor Core Routines
//=========================================================================

namespace Actors
{
//#pragma optimize("", off)


//=========================================================================
//						STANDARD FUNCTIONS
//=========================================================================

StartLineInfoActor::StartLineInfoActor()
{ 

}

StartLineInfoActor::~StartLineInfoActor()
{

}

bool StartLineInfoActor::InitialiseImpl()
{
	EffectGeneratorActor::InitialiseImpl();

	return(true);
}

//=========================================================================

bool StartLineInfoActor::CreateImpl()
{
	pCreationData = GetCreationData();

	mType			= pCreationData->mType;
	m_pVisDetect	= Actors::C_VisibilityDetector::CreateVisDetect(GetHandle());

	StartLineInfoSpecialFXList* pSFXList = C_GameData::Instance().mStartLineInfoSpecialFXList.Object();
	StartLineInfoSpecialFX* pSFX = NULL;

	if(pSFXList)
	{
		switch(mType)
		{
		case StartInfoType_Start:
			{
				pSFX = pSFXList->mStartSpecialFX.Object();
				break;
			}

		case StartInfoType_Finish:
			{
				pSFX = pSFXList->mFinishSpecialFX.Object();
				break;
			}

		case StartInfoType_Checkpoint:
			{
				pSFX = pSFXList->mCheckpointSpecialFX.Object();
				break;
			}

		case StartInfoType_Final:
			{
				pSFX = pSFXList->mFinalSpecialFX.Object();
				break;
			}

		case StartInfoType_Checkpoint_StartFinish:
			{
				pSFX = pSFXList->mCheckpointSpecialFX.Object();
				break;
			}
		}
	}

	if(pSFX)
	{
		UI::E_UILanguage language = UI::C_UILocaliser::Instance().Language();
		switch(language)
		{
		case UI::E_UILanguage_French:
			{
				if(pSFX->mFrench.Object())
					m_pSFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pSFX->mFrench, GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
				break;
			}

		case UI::E_UILanguage_German:
			{
				if(pSFX->mGerman.Object())
					m_pSFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pSFX->mGerman, GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
				break;
			}

		case UI::E_UILanguage_Italian:
			{
				if(pSFX->mItalian.Object())
					m_pSFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pSFX->mItalian, GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
				break;
			}

		case UI::E_UILanguage_Spanish:
			{
				if(pSFX->mSpanish.Object())
					m_pSFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pSFX->mSpanish, GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
				break;
			}
		}

		// Default to English
		if(!m_pSFX && pSFX->mEnglish)
		{
			m_pSFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pSFX->mEnglish, GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
		}
	}
	
	EffectGeneratorActor::CreateImpl();

	Game::C_InGameStartLineInfoManager::Instance().AddStartLineInfo(mType, GetHandle());

	return(true);
}

}