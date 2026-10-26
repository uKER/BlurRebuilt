#ifndef PORTAL_BLOCKER_H
#define PORTAL_BLOCKER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/Configuration.h>
#include <Btl/Allocators/pool.h>
#include <Btl/Utility/pair.h>


namespace Portal
{
	// Call this forward:
	class C_DeferredBlocker;

	//! Portal blocker object.
	class ALIGN16 C_Blocker
	{
	public:
		//! Uninitialised blocker.
		__forceinline NOALIAS explicit C_Blocker() 
			{ return; }
		//! New blocker with world-space position and extents.
		__forceinline NOALIAS explicit C_Blocker(const Maths::HMat43& rkWorldMatrix, const Maths::HVec3& rkExtents);
		//! D'tor.
		__forceinline NOALIAS ~C_Blocker()
			{ return; }

		//! Add an affected portal to the blocker.
		//@param iSystemIndex Index of the portal system in the static system list.
		//@param iPortalIndex Index of the portal w.r.t to it's containing system.
		__forceinline NOALIAS void					AddAffectedPortal(U32 iSystemIndex, U32 iPortalIndex);
#ifndef PLATFORM_SPU
		//! Debug render the blocker.
		void										DebugRender(DbUtil::I_DebugRenderPtr pDebugRender) const;
#endif // #ifndef PLATFORM_SPU
		//! Return index pair of affected portal at specified index.
		//@return Pair containing system index and portal index.
		__forceinline NOALIAS const Btl::Pair<U16, U16>& 
													GetAffectedPortalIndexPairAtIndex(const U32 iIndex) const;
		//! Return the number of portals this blocker affects.
		//@return Number of portals affected by the blocker.
		__forceinline NOALIAS const U32				GetAffectedPortalCount() const;
		//! Return the extents of the blocker.
		//@return The extents of the blocker.
		__forceinline NOALIAS const Maths::HVec3&	GetHalfExtents() const __restrict;
		//! Return the world-space position of the blocker.
		//@return The world-space position of the blocker.
		__forceinline NOALIAS const Maths::HMat43&	GetWorldMatrix() const __restrict;
		//! Return true if blocker is active.
		__forceinline NOALIAS const bool			IsActive() const __restrict;
		//! Turn the blocker on.
		NOALIAS void								TurnOn();
		//! Turn the blocker off.
		NOALIAS void								TurnOff();
		
	private:
		
		Btl::Vector<Btl::Pair<U16, U16>, Btl::VectorParameters_Static<PORTAL_CONFIG_MAXAFFECTEDPORTALS> >
							m_vAffectedPortals;		//!< Portals that this blocker affects.
		Maths::HMat43		m_WorldMatrix;			//!< World matrix of the blocker.
		Maths::HVec3		m_HalfExtents;			//!< Half extents of the blocker.
		bool				m_bActive;				//!< Flag indicating if the blocker is active or not.
		
		//! Copy c'tor.
		__forceinline NOALIAS						C_Blocker(const C_Blocker& rkCopy)
			{ return; }
		//! Assignment operator.
		__forceinline NOALIAS const C_Blocker		operator = (const C_Blocker& rkCopy)
			{ BIZ_ASSERT_FAIL("Shouldn't be called!"); return *this; }
		//! Update the blocker.
		NOALIAS void								Update();
	};

	//! Handle to a blocker.
	class ALIGN16 BlockerHandle
	{
	public:
		//! Construct a blocker handle from a pointer to a blocker.
		//@param pBlocker Pointer to the blocker.
		//@param pDeferredBlocker Pointer to the deferred blocker.
		__forceinline NOALIAS explicit BlockerHandle(C_Blocker* pBlocker, C_DeferredBlocker* pDeferredBlocker);
		//! D'tor.
		__forceinline NOALIAS ~BlockerHandle()
			{ return; }
		
		
		//! Turn on.
		NOALIAS void			On() __restrict;
		//! Turn off.
		NOALIAS void			Off() __restrict;
	
	private:
		// There exists a close coupling between this class and the deferred blocker.
		friend class C_DeferredBlocker;
		
		C_Blocker*			m_pPointer;				//!< Pointer to blocker.
		C_DeferredBlocker*	m_pDeferredBlocker;		//!< Pointer to deferred blocker.
		
		//! Default c'tor -- disallow calls to this.
		__forceinline NOALIAS explicit BlockerHandle() {};
	};

	// Include implementations of inline methods.
	#include "Blocker.inl"
}

#endif // PORTAL_BLOCKER_H
