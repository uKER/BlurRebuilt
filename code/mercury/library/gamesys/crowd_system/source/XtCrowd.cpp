#include "CrowdSystemPch.h"

#include <crowd_system/XtCrowd.h>
#include <crowd_system/CrowdResources.h>
#include <crowd_system/CrowdPopulation.h>
#include <crowd_system/CrowdDebug.h>

XT_TYPE(AnimatedCrowd::CrowdBehaviourType,	"CrowdBehaviourType")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_HANGING_POST_SITTING,			"HANGING_POST_SITTING")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_HANGING_POST_STANDING,		"HANGING_POST_STANDING")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_HANGING_FENCE_TOP,			"HANGING_FENCE_TOP")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_HANGING_SWING_UNDER,			"HANGING_SWING_UNDER")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_HANGING_LEFT_POST,			"HANGING_LEFT_POST")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_HANGING_RIGHT_POST,			"HANGING_RIGHT_POST")

	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_LEANING_FORWARD_LEDGE,		"LEANING_FORWARD_LEDGE")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_LEANING_BACKWARD_LEDGE,		"LEANING_BACKWARD_LEDGE")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_LEANING_BACKWARD_WALL,		"LEANING_BACKWARD_WALL")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_LEANING_BACKWARD_WALL_IDLE,	"LEANING_BACKWARD_WALL_IDLE")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_LEANING_BACKWARD_CAR,			"LEANING_BACKWARD_CAR")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_LEANING_FORWARD_FENCE,		"LEANING_FORWARD_FENCE")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_LEANING_BACKWARD_FENCE,		"LEANING_BACKWARD_FENCE")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_LEANING_FORWARD_POST,			"LEANING_FORWARD_POST")
	
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_SITTING_CHAIR,				"SITTING_CHAIR")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_SITTING_CHAIR_IDLE,			"SITTING_CHAIR_IDLE")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_SITTING_LEDGE,				"SITTING_LEDGE")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_SITTING_FLOOR,				"SITTING_FLOOR")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_SITTING_SQUAT,				"SITTING_SQUAT")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_SITTING_GRAFFITI,				"SITTING_GRAFFITI")
	
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_STANDING,						"STANDING")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_STANDING_PHONE,				"STANDING_PHONE")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_STANDING_HANDSHIPS,			"STANDING_HANDSHIPS")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_STANDING_HANDSHIPS_PHONE,		"STANDING_HANDSHIPS_PHONE")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_STANDING_IDLE,				"STANDING_IDLE")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_STANDING_BALANCING_SIDE,		"STANDING_BALANCING_SIDE")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_STANDING_BALANCING_FRONT,		"STANDING_BALANCING_FRONT")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_STANDING_BMX,					"STANDING_BMX")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_STANDING_GRAFFITI,			"STANDING_GRAFFITI")
	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_STANDING_SKATEBOARD,			"STANDING_SKATEBOARD")

	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_FLAG_WAVER,					"FLAG_WAVER")

	XT_OPTION(AnimatedCrowd::E_BEHAVIOUR_TYPE_UNKNOWN,						"UNKNOWN")
XT_END_TYPE

XT_TYPE(AnimatedCrowd::CrowdAction,	"CrowdAction")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_IDLE,			"Idle")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_CLAP,			"Clap")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_DISAGREE,		"Disagree")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_CELEBRATE,		"Celebrate")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_LOOK,			"Look")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_RECOIL,			"Recoil")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_TAKE_PHOTO,		"TakePhoto")

	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_UNKNOWN,		"Unknown")
XT_END_TYPE

XT_TYPE(AnimatedCrowd::CrowdActionDirection,	"CrowdActionDirection")
	XT_OPTION(AnimatedCrowd::E_ACTION_DIRECTION_ANY,	"Any")
	XT_OPTION(AnimatedCrowd::E_ACTION_DIRECTION_LEFT,	"Left")
	XT_OPTION(AnimatedCrowd::E_ACTION_DIRECTION_RIGHT,	"Right")
XT_END_TYPE

XT_TYPE(AnimatedCrowd::CrowdActionGroup,	"CrowdActionGroup")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_GROUP_IDLE,				"Idle")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_GROUP_INTERESTED,		"Interested")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_GROUP_EXCITED,			"Excited")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_GROUP_ECSTATIC,			"Ecstatic")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_GROUP_DANGER_NEAR,		"DangerNear")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_GROUP_DANGER_FAR,		"DangerFar")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_GROUP_ANTICIPATE_DANGER,"AnticipateDanger")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_GROUP_UNHAPPY,			"Unhappy")
	XT_OPTION(AnimatedCrowd::E_CROWD_ACTION_GROUP_UNKNOWN,			"Unknown")
XT_END_TYPE

XT_TYPE(AnimatedCrowd::CrowdEffectSystemType,	"CrowdEffectSystemType")
	XT_OPTION(AnimatedCrowd::E_CROWD_EFFECT_SYSTEM_CAMERA_FLASHES,	"CameraFlashes")
XT_END_TYPE

XT_TYPE(AnimatedCrowd::C_XtCrowdEffectSystem, "CrowdEffectSystem")
	XT_FIELD(m_Type,			"Type")
	XT_FIELD(m_ParticleEvent,	"ParticleEvent")
XT_END_TYPE

XT_TYPE(AnimatedCrowd::C_XtCrowdAnimationEventEffect, "CrowdAnimationEventEffect")
	XT_FIELD(m_ParticleEvent,	"ParticleEvent")
	XT_FIELD(m_AnimationEvent,	"AnimationEvent")
	XT_FIELD(m_JointName,		"JointName")
XT_END_TYPE

XT_TYPE(AnimatedCrowd::C_XtCrowdCycle,	"CrowdCycle")
	XT_BASE(ResourceUtils::C_XtCycleResourceDef)
	XT_FIELD(m_Action,			"Action")
	XT_FIELD(m_Direction,		"Direction")
XT_END_TYPE

XT_TYPE(AnimatedCrowd::C_XtCrowdSkeleton,	"CrowdSkeleton")
	XT_BASE(ResourceUtils::C_XtSkeletonResourceDef)
XT_END_TYPE

XT_TYPE(AnimatedCrowd::C_XtCrowdCycleGroupItem,	"CrowdCycleGroupItem")
	XT_CALLBACK(AnimatedCrowd::C_XtCrowdCycleGroupItem::XtCallback)
	XT_FIELD(m_Cycle,			"Cycle")
	XT_FIELD(m_Weight,			"Weight")
XT_END_TYPE

XT_TYPE(AnimatedCrowd::C_XtCrowdCycleGroup,	"CrowdCycleGroup")
	XT_CALLBACK(AnimatedCrowd::C_XtCrowdCycleGroup::XtCallback)
	XT_FIELD(m_Cycles,			"Cycles")
XT_END_TYPE

XT_TYPE(AnimatedCrowd::C_XtCrowdCharacter,	"CrowdCharacter")
	XT_CALLBACK(AnimatedCrowd::C_XtCrowdCharacter::XtCallback)
	XT_FIELD(m_BehaviourType,	"BehaviourType")
	XT_FIELD(m_BehaviourName,	"Behaviour")
	XT_FIELD(m_CycleGroup,		"CycleGroup")
XT_END_TYPE

XT_TYPE(AnimatedCrowd::C_XtCrowdPopulation,	"CrowdPopulation")
	XT_CALLBACK(AnimatedCrowd::C_XtCrowdPopulation::XtCallback)
	XT_FIELD(m_Name,					"Name")
	XT_FIELD(m_Skeleton,				"Skeleton")
	XT_FIELD(m_Characters,				"Characters")
	XT_FIELD(m_AnimationEventEffects,	"AnimationEventEffects")
	XT_FIELD(m_EffectSystems,			"EffectSystems")
XT_END_TYPE

XT_TYPE(AnimatedCrowd::CrowdTextureListUsage, "CrowdTextureListUsage")
	XT_OPTION(AnimatedCrowd::E_TextureListUsage_Flags,			"Flags")
	XT_OPTION(AnimatedCrowd::E_TextureListUsage_Unknown,		"None")
XT_END_TYPE

XT_TYPE(AnimatedCrowd::C_XtCrowdTextureList, "CrowdTextureList")
	XT_CALLBACK(AnimatedCrowd::C_XtCrowdTextureList::XtCallback)
	XT_FIELD(m_Usage,			"Usage")
	XT_FIELD(m_Textures,		"FilenamesMinusExt")
XT_END_TYPE

XT_LIBRARY(CrowdXtLib)
	XT_REGISTER_TYPE(AnimatedCrowd::C_XtCrowdTextureList)
	XT_REGISTER_TYPE(AnimatedCrowd::C_XtCrowdPopulation)
	XT_REGISTER_TYPE(AnimatedCrowd::C_XtCrowdCharacter)
	XT_REGISTER_TYPE(AnimatedCrowd::C_XtCrowdCycleGroup)
	XT_REGISTER_TYPE(AnimatedCrowd::C_XtCrowdCycleGroupItem)
	XT_REGISTER_TYPE(AnimatedCrowd::C_XtCrowdSkeleton)
	XT_REGISTER_TYPE(AnimatedCrowd::C_XtCrowdCycle)
	XT_REGISTER_TYPE(AnimatedCrowd::C_XtCrowdAnimationEventEffect)
	XT_REGISTER_TYPE(AnimatedCrowd::C_XtCrowdEffectSystem)
	XT_REGISTER_TYPE(AnimatedCrowd::CrowdTextureListUsage)
	XT_REGISTER_TYPE(AnimatedCrowd::CrowdBehaviourType)
	XT_REGISTER_TYPE(AnimatedCrowd::CrowdAction)
	XT_REGISTER_TYPE(AnimatedCrowd::CrowdActionDirection)
	XT_REGISTER_TYPE(AnimatedCrowd::CrowdActionGroup)
	XT_REGISTER_TYPE(AnimatedCrowd::CrowdEffectSystemType)
XT_END_LIBRARY

namespace AnimatedCrowd
{

XtDb*				g_XtDb;
XtPack*				g_XtPack;
XtCallbackContext	g_XtContext('CSHS');

void Xt_Open_Callback(XtPackOpenStatus *status,void *context)
{
	LOGWARN( g_CrowdLog, "Xt Open Status: %s", status->message );
}

void CreateXt(XtDb* database)
{
	XtInitialise();
	
	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open("gamedata\\xt\\Crowd.bin", Sys::E_FileAccess_Read);

	if (pFile)
	{
		int length = (int) pFile->Length();
		U8* data = new U8[length];

		if (data)
		{
			if (pFile->Read(data, length))
			{
				XtPackOpenParams params;
				params.callback = Xt_Open_Callback;

				if (!database)
					g_XtDb = XtDbCreate(XtGetApplicationSchema(), 100, &g_XtContext);
				else
					g_XtDb = database;

				BIZ_ASSERT(g_XtDb);
				if (g_XtDb)
					g_XtPack = XtPackOpen(g_XtDb, data, length, &params);

				if (!database)
					XtNetworkStart(g_XtDb);
			}
			delete [] data;
		}

		delete pFile;
	}
}

void DestroyXt()
{
	XtPackClose(g_XtDb, g_XtPack);

	XtDbDestroy(g_XtDb);

	XtShutdown();
}

///////////////////////////////////////////////////////////////////////////////////////

PopulationListenerList	g_PopulationListeners;
PopulationVector		g_Populations;

TextureListListenerList	g_TextureListListeners;
TextureListVector		g_TextureLists;

///////////////////////////////////////////////////////////////////////////////////////

void C_XtCrowdEffectSystem::XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdEffectSystem* pEffectSystem, XtCallbackContext* pContext)
{
	switch (eventType)
	{
		case kXtEtConstruct:
		{
			for (PopulationListenerList::Iterator iter = g_PopulationListeners.Begin(); iter != g_PopulationListeners.End(); ++iter)
			{
				iter->OnXtCrowdEffectSystemCreated(pRecord, pEffectSystem);
			}

			break;
		}
		
		case kXtEtDestruct:
		{
			for (PopulationListenerList::Iterator iter = g_PopulationListeners.Begin(); iter != g_PopulationListeners.End(); ++iter)
			{
				iter->OnXtCrowdEffectSystemDestroyed(pRecord, pEffectSystem);
			}

			break;
		}
			
		default:
			break;
	}
}

bool C_XtCrowdEffectSystem::Build(C_CrowdPopulation& population) const
{
	BIZ_ASSERT(m_Type < E_CROWD_EFFECT_SYSTEM_COUNT);

	if (m_ParticleEvent->pEvent)
		population.SetEffectSystemEvent(m_Type, m_ParticleEvent->pEvent);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////

void C_XtCrowdAnimationEventEffect::XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdAnimationEventEffect* pAnimationEventEffect, XtCallbackContext* pContext)
{
	switch (eventType)
	{
		case kXtEtConstruct:
		{
			for (PopulationListenerList::Iterator iter = g_PopulationListeners.Begin(); iter != g_PopulationListeners.End(); ++iter)
			{
				iter->OnXtCrowdAnimationEventEffectCreated(pRecord, pAnimationEventEffect);
			}

			break;
		}
		
		case kXtEtDestruct:
		{
			for (PopulationListenerList::Iterator iter = g_PopulationListeners.Begin(); iter != g_PopulationListeners.End(); ++iter)
			{
				iter->OnXtCrowdAnimationEventEffectDestroyed(pRecord, pAnimationEventEffect);
			}

			break;
		}
			
		default:
			break;
	}
}

bool C_XtCrowdAnimationEventEffect::Build(C_CrowdPopulation& population) const
{
	if (m_ParticleEvent->pEvent)
	{
		if (m_AnimationEvent.Text() && m_JointName.Text())
			population.AddAnimationEventEffect(m_ParticleEvent, m_AnimationEvent.Text(), m_JointName.Text());
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////

void C_XtCrowdCycleGroupItem::XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdCycleGroupItem* pCycleGroupItem, XtCallbackContext* pContext)
{
	switch (eventType)
	{
		case kXtEtConstruct:
		{
			for (PopulationListenerList::Iterator iter = g_PopulationListeners.Begin(); iter != g_PopulationListeners.End(); ++iter)
			{
				iter->OnXtCrowdCycleGroupItemCreated(pRecord, pCycleGroupItem);
			}

			break;
		}
		
		case kXtEtDestruct:
		{
			for (PopulationListenerList::Iterator iter = g_PopulationListeners.Begin(); iter != g_PopulationListeners.End(); ++iter)
			{
				iter->OnXtCrowdCycleGroupItemDestroyed(pRecord, pCycleGroupItem);
			}

			break;
		}
			
		default:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void C_XtCrowdCycleGroup::XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdCycleGroup* pCycleGroup, XtCallbackContext* pContext)
{
	switch (eventType)
	{
		case kXtEtConstruct:
		{
			for (PopulationListenerList::Iterator iter = g_PopulationListeners.Begin(); iter != g_PopulationListeners.End(); ++iter)
			{
				iter->OnXtCrowdCycleGroupCreated(pRecord, pCycleGroup);
			}

			break;
		}
		
		case kXtEtDestruct:
		{
			for (PopulationListenerList::Iterator iter = g_PopulationListeners.Begin(); iter != g_PopulationListeners.End(); ++iter)
			{
				iter->OnXtCrowdCycleGroupDestroyed(pRecord, pCycleGroup);
			}

			break;
		}
			
		default:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void C_XtCrowdCharacter::XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdCharacter* pCharacter, XtCallbackContext* pContext)
{
	switch (eventType)
	{
		case kXtEtConstruct:
		{
			for (PopulationListenerList::Iterator iter = g_PopulationListeners.Begin(); iter != g_PopulationListeners.End(); ++iter)
			{
				iter->OnXtCrowdCharacterCreated(pRecord, pCharacter);
			}

			break;
		}
		
		case kXtEtDestruct:
		{
			for (PopulationListenerList::Iterator iter = g_PopulationListeners.Begin(); iter != g_PopulationListeners.End(); ++iter)
			{
				iter->OnXtCrowdCharacterDestroyed(pRecord, pCharacter);
			}

			break;
		}
			
		default:
			break;
	}
}

bool C_XtCrowdCharacter::Build(C_CrowdPopulation& population) const
{
	//GetMemberTypeName(m_BehaviourType);

	if (!m_CycleGroup || !m_CycleGroup->m_Cycles)
	{
		if (!strlen(m_BehaviourName.Text()))
			LOGWARN( g_CrowdLog, "C_XtCrowdCharacter::Build() - Problem with character '%s'", GetBehaviourTypeName(m_BehaviourType) );
		else
			LOGWARN( g_CrowdLog, "C_XtCrowdCharacter::Build() - Problem with character '%s'", m_BehaviourName.Text() );
		return false;
	}

	if (!m_CycleGroup->m_Cycles.count)
	{
		if (!strlen(m_BehaviourName.Text()))
			LOGWARN( g_CrowdLog, "C_XtCrowdCharacter::Build() - Character '%s' has no cycles.", GetBehaviourTypeName(m_BehaviourType) );
		else
			LOGWARN( g_CrowdLog, "C_XtCrowdCharacter::Build() - Character '%s' has no cycles.", m_BehaviourName.Text() );
		return false;
	}

	for (S32 i = 0, count = m_CycleGroup->m_Cycles.count; i < count; ++i)
	{
		if (m_CycleGroup->m_Cycles[i].m_Cycle)
		{
			if (!strlen(m_BehaviourName.Text()))
				population.AddCycle(GetBehaviourTypeName(m_BehaviourType), m_CycleGroup->m_Cycles[i].m_Cycle->GetFileName(), m_CycleGroup->m_Cycles[i].m_Weight, m_CycleGroup->m_Cycles[i].m_Cycle->m_Action);
			else
				population.AddCycle(m_BehaviourName.Text(), m_CycleGroup->m_Cycles[i].m_Cycle->GetFileName(), m_CycleGroup->m_Cycles[i].m_Weight, m_CycleGroup->m_Cycles[i].m_Cycle->m_Action);
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////

void C_XtCrowdPopulation::XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdPopulation* pPopulation, XtCallbackContext* pContext)
{
	switch (eventType)
	{
		case kXtEtConstruct:
		{
			// Inform objects listening for this population that it has been created.
			for (PopulationListenerList::Iterator iter = g_PopulationListeners.Begin(); iter != g_PopulationListeners.End(); ++iter)
			{
				iter->OnXtCrowdPopulationCreated(pRecord, pPopulation);
			}

			g_Populations.PushBack(pPopulation);
			
			break;
		}
		
		case kXtEtDestruct:
		{
			// Inform objects listening for this population that it has been destroyed.
			for (PopulationListenerList::Iterator iter = g_PopulationListeners.Begin(); iter != g_PopulationListeners.End(); ++iter)
			{
				iter->OnXtCrowdPopulationDestroyed(pRecord, pPopulation);
			}
			
			bool found = false;
			for (S32 i = 0, count = g_Populations.Size(); i < count; ++i)
			{
				if (g_Populations[i] == pPopulation)
				{
					g_Populations.EraseByIndexNonOrdered(i);
					found = true;
					break;
				}				
			}
			assert(found);
			
			break;
		}
			
		default:
			break;
	}
}

bool C_XtCrowdPopulation::Build(C_CrowdPopulation& population) const
{
	population.Clear();

	if (!m_Skeleton || !m_Characters)
		return false;

	population.SetSkeleton(m_Skeleton->GetFileName());

	for (S32 i = 0, count = m_Characters.count; i < count; ++i)
	{
		if (m_Characters[i])
		{
			if (!m_Characters[i]->Build(population))
				LOGWARN( g_CrowdLog, "C_XtCrowdPopulation::Build() - Problem with character '%d'", i );
		}
	}

	return true;
}

void C_XtCrowdPopulation::PostLoadResources(C_CrowdPopulation& population) const
{
	for (S32 i = 0, count = m_AnimationEventEffects.count; i < count; ++i)
	{
		if (m_AnimationEventEffects[i])
		{
			if (!m_AnimationEventEffects[i]->Build(population))
				LOGWARN( g_CrowdLog, "C_XtCrowdPopulation::Build() - Problem with animation event effect '%d'", i );
		}
	}

	for (S32 i = 0, count = m_EffectSystems.count; i < count; ++i)
	{
		if (m_EffectSystems[i])
		{
			if (!m_EffectSystems[i]->Build(population))
				LOGWARN( g_CrowdLog, "C_XtCrowdPopulation::Build() - Problem with effect system '%d'", i );
		}
	}
}

void C_XtCrowdPopulation::AddListener(C_XtCrowdPopulationListener* pListener)
{
	g_PopulationListeners.PushBack(*pListener);
}

void C_XtCrowdPopulation::RemoveListener(C_XtCrowdPopulationListener* pListener)
{
	for (PopulationListenerList::Iterator iter = g_PopulationListeners.Begin(); iter != g_PopulationListeners.End(); ++iter)
	{
		if (&(*iter) == pListener)
		{
			g_PopulationListeners.Erase(iter);
			return;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void C_XtCrowdTextureList::XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdTextureList* pTextureList, XtCallbackContext* pContext)
{
	switch (eventType)
	{
		case kXtEtConstruct:
		{
			// Inform objects listening for this population that it has been created.
			for (TextureListListenerList::Iterator iter = g_TextureListListeners.Begin(); iter != g_TextureListListeners.End(); ++iter)
			{
				iter->OnXtCrowdTextureListCreated(pRecord, pTextureList);
			}

			g_TextureLists.PushBack(pTextureList);
			
			break;
		}
		
		case kXtEtDestruct:
		{
			// Inform objects listening for this population that it has been destroyed.
			for (TextureListListenerList::Iterator iter = g_TextureListListeners.Begin(); iter != g_TextureListListeners.End(); ++iter)
			{
				iter->OnXtCrowdTextureListDestroyed(pRecord, pTextureList);
			}
			
			bool found = false;
			for (S32 i = 0, count = g_TextureLists.Size(); i < count; ++i)
			{
				if (g_TextureLists[i] == pTextureList)
				{
					g_TextureLists.EraseByIndexNonOrdered(i);
					found = true;
					break;
				}				
			}
			assert(found);
			
			break;
		}
			
		default:
			break;
	}
}

bool C_XtCrowdTextureList::Build(C_CrowdResources& resources) const
{
	/*population.Clear();

	if (!m_Skeleton || !m_Characters)
		return false;

	population.SetSkeleton(m_Skeleton->GetFileName());

	for (S32 i = 0, count = m_Characters.count; i < count; ++i)
	{
		if (m_Characters[i])
		{
			if (!m_Characters[i]->Build(population))
				LOGWARN( g_CrowdLog, "C_XtCrowdPopulation::Build() - Problem with character '%d'", i );
		}
	}

	for (S32 i = 0, count = m_AnimationEventEffects.count; i < count; ++i)
	{
		if (m_AnimationEventEffects[i])
		{
			if (!m_AnimationEventEffects[i]->Build(population))
				LOGWARN( g_CrowdLog, "C_XtCrowdPopulation::Build() - Problem with animation event effect '%d'", i );
		}
	}

	for (S32 i = 0, count = m_EffectSystems.count; i < count; ++i)
	{
		if (m_EffectSystems[i])
		{
			if (!m_EffectSystems[i]->Build(population))
				LOGWARN( g_CrowdLog, "C_XtCrowdPopulation::Build() - Problem with effect system '%d'", i );
		}
	}*/

	return true;
}

void C_XtCrowdTextureList::AddListener(C_XtCrowdTextureListListener* pListener)
{
	g_TextureListListeners.PushBack(*pListener);
}

void C_XtCrowdTextureList::RemoveListener(C_XtCrowdTextureListListener* pListener)
{
	for (TextureListListenerList::Iterator iter = g_TextureListListeners.Begin(); iter != g_TextureListListeners.End(); ++iter)
	{
		if (&(*iter) == pListener)
		{
			g_TextureListListeners.Erase(iter);
			return;
		}
	}
}

}; // namespace AnimatedCrowd
