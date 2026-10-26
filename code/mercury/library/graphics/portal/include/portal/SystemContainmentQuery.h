#ifndef PORTAL_SYSTEMCONTAINMENTQUERY_H
#define PORTAL_SYSTEMCONTAINMENTQUERY_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_SPU
#include <Portal/Configuration.h>
#include <Portal/RoomContainmentSubSpace.h>
#endif // #ifdef PLATFORM_SPU

namespace Portal
{
	// Call this forward.
	class C_System;

	//! Portal system containment query object.
	class C_SystemContainmentQuery
	{
	public:
		//! Construct a query object from a portal system BSP object.
		//@param pTreeEA The effective address of the BSP tree.
		explicit C_SystemContainmentQuery(const C_BSPContainer* pTreeEA);
		//! D'tor.
		~C_SystemContainmentQuery();

		//! Classify the point with respect to the portal system object, returns a room index.
		//@param rkPoint The world-space point that is to be classified.
		//@param iLastSeen [Optional] The index of the room in which the point was last seen.
		//@return The index of the room in the portal system that point resides within.
		NOALIAS const U32							Classify(const Maths::HVec3& rkPoint, const U32 iLastSeen = 0) const __restrict;

	private:
		const C_BSPContainer*						m_pTree;				//!< Pointer to underlying system object in main memory, used for PPU-based queries/SPU Dma ops.
	};
}

#endif // PORTAL_SYSTEMCONTAINMENTQUERY_H
