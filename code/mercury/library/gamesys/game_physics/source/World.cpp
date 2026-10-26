#include "GamePhysicsPch.h"
#include <game_physics/World.h>

#include <game_physics/BodyUserData.h>
#include <game_physics/BodyUserDataTypes.h>
#include <game_physics/CollisionEvent.h>
#include <game_physics/CollisionResult.h>
#include <game_physics/FluidEvent.h>
#include <gravity/sdk/PhysicsSDK.h>
#include <gravity/SharedPhysics/ContactStreamIterator.h>




namespace GamePhysics
{

static Debug::Bool g_DebugGamePhysicsShowCollisions("GamePhysics.Contacts.ShowCollisions", false);
static Debug::Bool g_DebugGamePhysicsPersistCollisions("GamePhysics.Contacts.PersistCollisions", false);




class C_WorldImpl : public Gravity::C_CollisionCallback
{
public:
	explicit C_WorldImpl(Gravity::C_Scene* pGravityScene);
	virtual ~C_WorldImpl();

	virtual void ProcessCollisions(SharedPhysics::C_ContactStreamIterator& iter, SharedPhysics::TCollisionCallbackPhase phase); // from Gravity::C_CollisionCallback
	virtual void ProcessFluidEvents(Gravity::C_FluidEvent* pFluidEvents, U32 numFluidEvents); // from Gravity::C_CollisionCallback

	void SetCollisionResultAdjuster(const U32 typeUid, F_CollisionResultAdjuster pFn);
	void SetCollisionResultAdjuster(const U32 typeUidA, const U32 typeUidB, F_CollisionResultAdjuster pFn);

	void SetCollisionEventHandler(const U32 typeUid, F_CollisionEventHandler pFn);
	void SetCollisionEventHandler(const U32 typeUidA, const U32 typeUidB, F_CollisionEventHandler pFn);

	void SetFluidEventHandler(const U32 typeUid, F_FluidEventHandler pFn);

private:
	struct C_DoubleDispatchEntry;
	struct C_SingleDispatchEntry;

	Gravity::C_Scene*		m_pGravityScene; ///< weak ref
	U32						m_NumBodyTypes; ///< number of body types in dispatch tables
	C_SingleDispatchEntry*	m_pSingleDispatchTable; ///< dispatch table for single body events
	C_DoubleDispatchEntry*	m_pDoubleDispatchTable; ///< dispatch table for double body events

	C_WorldImpl(const C_WorldImpl& copy); // can not copy
	C_WorldImpl& operator = (const C_WorldImpl& copy); // can not copy

	C_SingleDispatchEntry&	GetSingleDispatchEntry(const U32 typeUid);
	C_DoubleDispatchEntry&	GetDoubleDispatchEntry(const U32 typeUidA, const U32 typeUidB);

};

struct C_WorldImpl::C_SingleDispatchEntry
{
	F_CollisionResultAdjuster	pCollisionResultAdjusterFn;
	F_CollisionEventHandler		pCollisionEventHandlerFn;
	F_FluidEventHandler			pFluidEventHandlerFn;
};

struct C_WorldImpl::C_DoubleDispatchEntry
{
	bool						bReverseCollisionResultAdjuster;
	F_CollisionResultAdjuster	pCollisionResultAdjusterFn;
	bool						bReverseCollisionEventHandler;
	F_CollisionEventHandler		pCollisionEventHandlerFn;
};

C_WorldImpl::C_WorldImpl(Gravity::C_Scene* pGravityScene)
	: Gravity::C_CollisionCallback()
	, m_pGravityScene(pGravityScene)
	, m_NumBodyTypes(0)
	, m_pSingleDispatchTable(NULL)
	, m_pDoubleDispatchTable(NULL)
{
	const U32 numBodyTypes = C_BodyUserDataTypes::NumTypes();
	m_NumBodyTypes = numBodyTypes;

	m_pSingleDispatchTable = new C_SingleDispatchEntry [numBodyTypes];
	memset(m_pSingleDispatchTable, 0, numBodyTypes * sizeof(C_SingleDispatchEntry));

	m_pDoubleDispatchTable = new C_DoubleDispatchEntry [numBodyTypes * numBodyTypes];
	memset(m_pDoubleDispatchTable, 0, numBodyTypes * numBodyTypes * sizeof(C_DoubleDispatchEntry));

	pGravityScene->RegisterCollisionCallback(this);
}

C_WorldImpl::~C_WorldImpl()
{
	if(m_pGravityScene)
		m_pGravityScene->RegisterCollisionCallback(NULL);

	delete [] m_pSingleDispatchTable;
	delete [] m_pDoubleDispatchTable;
	m_pGravityScene = NULL;
}

C_WorldImpl::C_SingleDispatchEntry& C_WorldImpl::GetSingleDispatchEntry(const U32 typeUid)
{
	BIZ_ASSERT(typeUid < m_NumBodyTypes);
	return m_pSingleDispatchTable[typeUid];
}

C_WorldImpl::C_DoubleDispatchEntry& C_WorldImpl::GetDoubleDispatchEntry(const U32 typeUidA, const U32 typeUidB)
{
	BIZ_ASSERT(typeUidA < m_NumBodyTypes);
	BIZ_ASSERT(typeUidB < m_NumBodyTypes);
	return m_pDoubleDispatchTable[(typeUidA * m_NumBodyTypes) + typeUidB];
}

void C_WorldImpl::ProcessCollisions(SharedPhysics::C_ContactStreamIterator& iter, SharedPhysics::TCollisionCallbackPhase phase)
{
	while (iter.hasNextContact())
	{
		SharedPhysics::C_ContactStreamResult* pContactStreamResult = iter.nextContact();

		// get bodies and corresponding user data
		Gravity::C_Body* pBodyA = pContactStreamResult->m_pNodeA->m_pBody;
		Gravity::C_Body* pBodyB = pContactStreamResult->m_pNodeB->m_pBody;

		BIZ_ASSERT(pBodyA != NULL);
		BIZ_ASSERT(pBodyB != NULL);

		C_BodyUserData* pDataA = pBodyA->GetUserData<C_BodyUserData>();
		C_BodyUserData* pDataB = pBodyB->GetUserData<C_BodyUserData>();

		// obtain the dispatchers
		const C_DoubleDispatchEntry* pEntryD = NULL;
		const C_SingleDispatchEntry* pEntryA = NULL;
		const C_SingleDispatchEntry* pEntryB = NULL;

		if (pDataA != NULL)
		{
			pEntryA = &GetSingleDispatchEntry(pDataA->GetTypeUid());

			if (pDataB != NULL)
			{
				pEntryD = &GetDoubleDispatchEntry(pDataA->GetTypeUid(), pDataB->GetTypeUid());
			}
		}
		
		if (pDataB != NULL)
		{
			pEntryB = &GetSingleDispatchEntry(pDataB->GetTypeUid());
		}

		// adjust result, possibly ignore it altogether
		bool processResult = false;

		{
			C_CollisionResult collisionResult;

			collisionResult.pIter = &iter;
			collisionResult.pResult = pContactStreamResult;

			// if we have a double-dispatch handler then use that
			if ((pEntryD != NULL) && (pEntryD->pCollisionResultAdjusterFn != NULL))
			{
				if (pEntryD->bReverseCollisionResultAdjuster)
				{
					collisionResult.pBodyA = pBodyB;
					collisionResult.pBodyB = pBodyA;
					collisionResult.pDataA = pDataB;
					collisionResult.pDataB = pDataA;
				}
				else
				{
					collisionResult.pBodyA = pBodyA;
					collisionResult.pBodyB = pBodyB;
					collisionResult.pDataA = pDataA;
					collisionResult.pDataB = pDataB;
				}

				processResult = (*pEntryD->pCollisionResultAdjusterFn)(collisionResult);
			}
			else
			{
				bool anyAdjusted = false;

				// do we have a single dispatch for the first body
				if ((pDataA != NULL) && (pEntryA->pCollisionResultAdjusterFn != NULL))
				{
					collisionResult.pBodyA = pBodyA;
					collisionResult.pBodyB = NULL;
					collisionResult.pDataA = pDataA;
					collisionResult.pDataB = NULL;

					processResult |= (*pEntryA->pCollisionResultAdjusterFn)(collisionResult);
					anyAdjusted = true;
				}

				// do we have a single dispatch for the second body
				if ((pDataB != NULL) && (pEntryB->pCollisionResultAdjusterFn != NULL))
				{
					collisionResult.pBodyA = pBodyB;
					collisionResult.pBodyB = NULL;
					collisionResult.pDataA = pDataB;
					collisionResult.pDataB = NULL;

					processResult |= (*pEntryB->pCollisionResultAdjusterFn)(collisionResult);
					anyAdjusted = true;
				}

				processResult |= !anyAdjusted;
			}
		}

		if (processResult)
		{
			C_CollisionEvent collisionEvent;

			while (iter.hasNextPatch())
			{
				SharedPhysics::C_ContactStreamPatch* pContactStreamPatch = iter.nextPatch();

				const Maths::HVec3 patchNormal = Maths::Load<Maths::HVec3>(&pContactStreamPatch->m_Normal);

				while (iter.hasNextPoint())
				{
					SharedPhysics::C_ContactStreamPoint* pContactStreamPoint = iter.nextPoint();

					// determine if either body is interested in the collision
					bool processEvent = false;
					Maths::HF32 impulseMagnitude;

					{
						const bool forceBActive = (static_cast<F32>(pContactStreamResult->m_pNodeB->Deactivator().m_StaticActivateThresholdSq) > 0.0f);
						iter.GetImpulseAndFriction(impulseMagnitude, collisionEvent.friction, forceBActive);
						collisionEvent.impulseMagnitude = impulseMagnitude;
					}

					collisionEvent.contactType = static_cast<U32>(iter.GetContactType());

					if (pDataA != NULL)
					{
						processEvent |= pDataA->GetCollisionFilter().Match(collisionEvent.contactType, collisionEvent.impulseMagnitude);
					}

					if (pDataB != NULL)
					{
						processEvent |= pDataB->GetCollisionFilter().Match(collisionEvent.contactType, collisionEvent.impulseMagnitude);
					}

					if (processEvent)
					{
						collisionEvent.position = Maths::Load<Maths::HVec3>(&pContactStreamPoint->m_ContactPoint);
						collisionEvent.penetrationDepth = pContactStreamPoint->m_PenetrationDepth;

						// if we have a double dispatch then use it
						if ((pEntryD != NULL) && (pEntryD->pCollisionEventHandlerFn != NULL))
						{
							if (pEntryD->bReverseCollisionResultAdjuster)
							{
								collisionEvent.normal = -patchNormal;
								collisionEvent.pBodyA = pBodyB;
								collisionEvent.pBodyB = pBodyA;
								collisionEvent.pDataA = pDataB;
								collisionEvent.pDataB = pDataA;
								collisionEvent.effectMaterialA = pContactStreamPoint->m_EffectMaterialIndices >> 16;
								collisionEvent.effectMaterialB = pContactStreamPoint->m_EffectMaterialIndices & 0xFFFF;
							}
							else
							{
								collisionEvent.normal = patchNormal;
								collisionEvent.pBodyA = pBodyA;
								collisionEvent.pBodyB = pBodyB;
								collisionEvent.pDataA = pDataA;
								collisionEvent.pDataB = pDataB;
								collisionEvent.effectMaterialA = pContactStreamPoint->m_EffectMaterialIndices & 0xFFFF;
								collisionEvent.effectMaterialB = pContactStreamPoint->m_EffectMaterialIndices >> 16;
							}

							(*pEntryD->pCollisionEventHandlerFn)(collisionEvent);
						}
						else
						{
							// single dispatch for first body
							if ((pEntryA != NULL) && (pEntryA->pCollisionEventHandlerFn != NULL))
							{
								collisionEvent.normal = patchNormal;
								collisionEvent.pBodyA = pBodyA;
								collisionEvent.pBodyB = pBodyB;
								collisionEvent.pDataA = pDataA;
								collisionEvent.pDataB = NULL;
								collisionEvent.effectMaterialA = pContactStreamPoint->m_EffectMaterialIndices & 0xFFFF;
								collisionEvent.effectMaterialB = 0;

								(*pEntryA->pCollisionEventHandlerFn)(collisionEvent);
							}

							// single dispatch for second body
							if ((pEntryB != NULL) && (pEntryB->pCollisionEventHandlerFn != NULL))
							{
								collisionEvent.normal = -patchNormal;
								collisionEvent.pBodyA = pBodyB;
								collisionEvent.pBodyB = pBodyA;
								collisionEvent.pDataA = pDataB;
								collisionEvent.pDataB = NULL;
								collisionEvent.effectMaterialA = pContactStreamPoint->m_EffectMaterialIndices >> 16;
								collisionEvent.effectMaterialB = 0;

								(*pEntryB->pCollisionEventHandlerFn)(collisionEvent);
							}
						}
					}

					if (g_DebugGamePhysicsShowCollisions)
					{
						const Maths::HVec3 position = Maths::Load<Maths::HVec3>(&pContactStreamPoint->m_ContactPoint);
						DbUtil::I_DebugRenderPtr dbgRend = g_DebugGamePhysicsPersistCollisions ? DbUtil::DefaultDebugRenderSys()->Persistent() : DbUtil::DefaultDebugRenderSys()->Deferred();
						dbgRend->Draw3dCross(position, 0.1f, Maths::RGBA(255,0,0));
						dbgRend->Draw3dArrow(position, patchNormal, impulseMagnitude * 0.05f, 0.05f, Maths::RGBA(0,255,0));
					}
				}
			}
		}
	}
}

void C_WorldImpl::ProcessFluidEvents(Gravity::C_FluidEvent* pFluidEvents, U32 numFluidEvents)
{
	C_FluidEvent dst;

	for (U32 i = 0; i < numFluidEvents; ++i)
	{
		const Gravity::C_FluidEvent& src = pFluidEvents[i];

		if ((src.m_pBody != NULL) && (src.m_pBody->m_pBody != NULL))
		{
			C_BodyUserData* pUserData = src.m_pBody->m_pBody->GetUserData<C_BodyUserData>();

			if ((pUserData != NULL) && (pUserData->GetCollisionFilter().MatchFluid()))
			{
				const C_SingleDispatchEntry& entry = GetSingleDispatchEntry(pUserData->GetTypeUid());

				if (entry.pFluidEventHandlerFn != NULL)
				{
					dst.submergedCentroid = Maths::Load<Maths::HVec3>(&src.m_SubmergedCentroid);
					dst.surfacePoint = Maths::Load<Maths::HVec3>(&src.m_SurfacePoint);
					dst.submergedVolume = src.m_SubmergedVolume;
					dst.pBody = src.m_pBody->m_pBody;
					dst.pData = pUserData;

					(entry.pFluidEventHandlerFn)(dst);
				}
			}
		}
	}
}

void C_WorldImpl::SetCollisionResultAdjuster(const U32 typeUid, F_CollisionResultAdjuster pFn)
{
	GetSingleDispatchEntry(typeUid).pCollisionResultAdjusterFn = pFn;
}

void C_WorldImpl::SetCollisionResultAdjuster(const U32 typeUidA, const U32 typeUidB, F_CollisionResultAdjuster pFn)
{
	C_DoubleDispatchEntry& ddAB = GetDoubleDispatchEntry(typeUidA, typeUidB);
	C_DoubleDispatchEntry& ddBA = GetDoubleDispatchEntry(typeUidB, typeUidA);

	ddAB.bReverseCollisionResultAdjuster = false;
	ddAB.pCollisionResultAdjusterFn = pFn;

	ddBA.bReverseCollisionResultAdjuster = true;
	ddBA.pCollisionResultAdjusterFn = pFn;
}

void C_WorldImpl::SetCollisionEventHandler(const U32 typeUid, F_CollisionEventHandler pFn)
{
	GetSingleDispatchEntry(typeUid).pCollisionEventHandlerFn = pFn;
}

void C_WorldImpl::SetCollisionEventHandler(const U32 typeUidA, const U32 typeUidB, F_CollisionEventHandler pFn)
{
	C_DoubleDispatchEntry& ddAB = GetDoubleDispatchEntry(typeUidA, typeUidB);
	C_DoubleDispatchEntry& ddBA = GetDoubleDispatchEntry(typeUidB, typeUidA);

	ddAB.bReverseCollisionEventHandler = false;
	ddAB.pCollisionEventHandlerFn = pFn;

	ddBA.bReverseCollisionEventHandler = true;
	ddBA.pCollisionEventHandlerFn = pFn;
}

void C_WorldImpl::SetFluidEventHandler(const U32 typeUid, F_FluidEventHandler pFn)
{
	GetSingleDispatchEntry(typeUid).pFluidEventHandlerFn = pFn;
}




C_World::C_World(Gravity::C_Scene* pGravityScene)
	: m_pImpl(new C_WorldImpl(pGravityScene))
{
}

C_World::~C_World()
{
	delete m_pImpl;
}

void C_World::SetCollisionResultAdjuster(const U32 typeUid, F_CollisionResultAdjuster pFn)
{
	m_pImpl->SetCollisionResultAdjuster(typeUid, pFn);
}

void C_World::SetCollisionResultAdjuster(const U32 typeUidA, const U32 typeUidB, F_CollisionResultAdjuster pFn)
{
	m_pImpl->SetCollisionResultAdjuster(typeUidA, typeUidB, pFn);
}

void C_World::SetCollisionEventHandler(const U32 typeUid, F_CollisionEventHandler pFn)
{
	m_pImpl->SetCollisionEventHandler(typeUid, pFn);
}

void C_World::SetCollisionEventHandler(const U32 typeUidA, const U32 typeUidB, F_CollisionEventHandler pFn)
{
	m_pImpl->SetCollisionEventHandler(typeUidA, typeUidB, pFn);
}

void C_World::SetFluidEventHandler(const U32 typeUid, F_FluidEventHandler pFn)
{
	m_pImpl->SetFluidEventHandler(typeUid, pFn);
}

} // namespace GamePhysics
