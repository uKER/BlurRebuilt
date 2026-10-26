#ifndef COLLISION_CALLBACK_H
#define COLLISION_CALLBACK_H

#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/sharedphysics/ContactStream.h"
#include "gravity/sdk/scene.h"

namespace SharedPhysics
{
	class C_ContactStream;
	class C_ContactStreamIterator;

	enum TCollisionCallbackPhase
	{
		E_CollisionCallbackPhase_Colliders=0,
		E_CollisionCallbackPhase_Woken
	};
};

namespace Gravity
{

	struct C_FluidEvent;
	class C_CollisionCallback
	{
	public:
		C_CollisionCallback()
		{
		}
		virtual ~C_CollisionCallback()
		{
		}
		virtual void ProcessCollisions(SharedPhysics::C_ContactStreamIterator& iter, SharedPhysics::TCollisionCallbackPhase phase) = 0;

		virtual void ProcessFluidEvents(C_FluidEvent* m_pFluidEvents, U32 m_NumFluidEvents) = 0;
	};

	//A class which writes a contact into the contact stream...
	class C_ContactStreamWriter
	{
		Gravity::C_Scene* m_pScene;
		SharedPhysics::C_ContactStream* m_pStream;
		U8* m_pCurrentWritePoint;
		public:
		C_ContactStreamWriter(Gravity::C_Scene* pScene, SharedPhysics::C_ContactStream* pStream) : m_pScene(pScene), m_pStream(pStream)
		{
		}

		~C_ContactStreamWriter()
		{
		}

		template<int MaxPatches, int MaxContacts>
		void EncodeContacts(Collision::C_CollisionResult<MaxPatches, MaxContacts>& results)
		{
			SharedPhysics::C_Collider* pCollider = m_pScene->CreateCollider(results.m_pObjectA->m_pNodePtr, results.m_pObjectB->m_pNodePtr);
			results.m_pCurrentCollider = pCollider;
			Collision::EncodeContacts<MaxPatches, MaxContacts>(results, *m_pStream, pCollider);
			//Add sentinel values
			SharedPhysics::C_ContactStreamResult* pSentinel = (SharedPhysics::C_ContactStreamResult*)m_pStream->m_pCurrentWritePoint;
			pSentinel->m_ValidData = SharedPhysics::E_NullData;
		}
		
	};

};

#endif
