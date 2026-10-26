
#ifndef DEFAULTSUBSYSTEM_H
#define DEFAULTSUBSYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "AppInterfaceLayer.h"

enum E_DefaultSubSystemMessage
{
	E_DefaultSubSystemMessage_Mine,
	E_DefaultSubSystemMessage_TestSound

};

enum E_DefaultSubSystemMine
{
	E_DefaultSubSystemMineCreate,
	E_DefaultSubSystemMineExplode,
	E_DefaultSubSystemMineDestroy

};

class DefaultSubsystem : public Audio::Subsystem
{
public:

	struct TrackObject
	{
		AudioEntityHandle m_hEntity; //assume max for now...
		bool m_markForDelete;
		Audio::PagedArrayHandle m_hQueryEntityFinished;
		Maths::HVec3 m_Position;

	};


	DefaultSubsystem() {m_EntityInstanceCount = 0; }
	~DefaultSubsystem() { }

	bool		ProcessSystemMessage(const U32 u32MessageId);

	bool		ProcessCustomMessage(const U32 u32MessageId, U32 u32Data = 0, void* pData = 0);
	//{
	//	return true;
	//}
	bool		SendStateTransition(const U32 u32OutgoingState, const U32 u32IncomingState)
	{
		return true;
	}
	void		Update();


	Audio::XtEntityProxy* m_pMineXtEntity;
	//AudioEntityHandle m_hEntity[60]; //assume max for now...
	Btl::Vector< TrackObject, Btl::VectorParameters_Array< 60 > > m_trackObject;
	S32 m_EntityInstanceCount;



	//typedef struct PerkUpdater
	//{
	//	float mTimer;
	//	float mRadius;
	//	float mTotalRadius;
	//	Actor::C_ActorHandle mOwner;
	//	E_PICKUP_TYPE mType;
	//	int mLevel;
	//	Btl::Vector<Actor::C_ActorHandle> mAlreadyHit;
	//	Maths::HVec3 mPosition;
	//}PerkUpdater;

	//void CreateMine();
	//void DestroyMine();
};

#endif // DEFAULTSUBSYSTEM_H
