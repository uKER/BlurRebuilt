#ifndef PORTAL_DEFERREDBLOCKER_H
#define PORTAL_DEFERREDBLOCKER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Allocators/pool.h>
#include <Btl/Utility/pair.h>
#include <Portal/Blocker.h>
#include <Portal/Configuration.h>
#include <core/threads/Mutex.h>
#include <Portal/Portal.h>

namespace Portal
{
	class C_Portal;
	
	//! Portal blocker manager.
	class ALIGN16 C_DeferredBlocker
	{
	public:
		//! Default c'tor.
		__forceinline NOALIAS explicit C_DeferredBlocker();
		//! D'tor.
		__forceinline NOALIAS ~C_DeferredBlocker();

		//! Add a blocker -- this is quite a slow operation.
		//@param rkWorldPosition World-space position of the blocker.
		//@param rkExtents Extents of the blocker.
		NOALIAS BlockerHandle		CreateBlocker(const Maths::HMat43& rkWorldMatrix, const Maths::HVec3& rkHalfExtents) __restrict;
#ifndef PLATFORM_SPU
		//! Debug render the blocker.
		void						DebugRender(DbUtil::I_DebugRenderPtr pDebugRender);
#endif // #ifndef PLATFORM_SPU
		//! Mark dirty.
		//@param pBlocker Pointer to the blocker to mark as dirty.
		void						MarkDirty(C_Blocker* __restrict const pBlocker);
		//! Remove blocker.
		//@param handle The handle to the blocker to remove.
		NOALIAS void				RemoveBlocker(BlockerHandle handle);
		//! Apply blockers to portal systems.
		NOALIAS void				Update();

	private:
		Btl::StaticPool<C_Blocker, PORTAL_CONFIG_MAXBLOCKERS>	m_Blockers;								//!< Static pool of portal blockers.
		Btl::Vector<const C_Blocker*, Btl::VectorParameters_Static<PORTAL_CONFIG_MAXBLOCKERS> >		
																m_vDirtyBlockers;						//!< Vector of dirty indices.
#ifndef CFG_GOLD
		Btl::Vector<const C_Blocker*>							m_vAllocatedBlockers;					//!< Vector of potentially occluded portals and their system index.
#endif // #ifndef CFG_GOLD
		mutable MT::Mutex										m_Mutex;								//!< Mutex for modification of the blocker vector.
		
		//! Check for intersection between blocker and portal.
		//@param pBlocker Pointer to a blocker.
		//@param pPortal Pointer to a portal.
		//@return True if the portal and blocker intersect one another.
		__forceinline NOALIAS const bool Intersection(const C_Blocker* __restrict pBlocker, const C_Portal* __restrict pPortal) const;
	};

	// Include implementations of inline methods.
	#include "DeferredBlocker.inl"
}

#endif // PORTAL_DEFERREDBLOCKER_H
