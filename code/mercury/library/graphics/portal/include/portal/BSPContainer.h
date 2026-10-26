#ifndef PORTAL_BSPCONTAINER_H
#define PORTAL_BSPCONTAINER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/Configuration.h>
#include <Portal/TraceResults.h>
#include <Portal/BSPRegion.h>
#include <Portal/BSPSplitPlane.h>

namespace Portal
{
	// Call these forward:
	class C_Ray;
	class C_BSPSplitPlane;
	class C_Blocker;
	
	//! Container for the BSP tree.
	class ALIGN16 C_BSPContainer
	{
	public:
		//! C'tor.
		__forceinline explicit C_BSPContainer();
		//! D'tor.
		__forceinline ~C_BSPContainer();

		//! Classify a blocker using the BSP container.
		//@param rkBlocker Blocker to classify.
		//@param rkResultsVector Vector of room indices the blocker intersects.
		NOALIAS void								Classify(const C_Blocker& rkBlocker, /*[OUT]*/Btl::Vector<U32>& rkResultsVector) const __restrict;
		//! Classify a vertex using the BSP container.
		//@param rkWorldPosition World-space position to classify.
		//@return Index of the room containing the point.
		NOALIAS const U32							Classify(const Maths::HVec3& rkWorldPosition) const __restrict;

#ifndef PLATFORM_SPU
		//! Deserialise the tree from a stream.
		//@param rkStream Stream from which to deserialise the BSP tree.
		template <typename _T>
		void										Deserialise(_T& rkStream);
#endif // #ifndef PLATFORM_SPU

		//! Return a pointer to split planes vector.
		//@param Pointer to a list of nodes in the tree.
		__forceinline const C_BSPSplitPlane*		GetNodes() const __restrict;
		//! Return the number of elements in the tree.
		//@return The number of elements in the tree.
		__forceinline NOALIAS const U32				GetNodeCount() const __restrict;
#ifdef PLATFORM_SPU
		//! Set the nodes pointer on the object.
		//@param pNodes Pointer to nodes in local store.
		NOALIAS void								SetNodePtr(C_BSPSplitPlane* pNodes) __restrict;
#endif // #ifdef PLATFORM_SPU
		//! Trace a ray into the BSP tree.
		//@param rkRray The ray to trace into the BSP tree.
		//@return Trace results object containing results from the traced ray.
		C_TraceResults								Trace(const C_Ray& rkRay) const;
		
	private:
		C_BSPRegion			m_RootRegion;							//!< Index of root element.
		C_BSPSplitPlane*	m_pElements;							//!< Pointer to array of elements -- make sure of 16byte alignment when deserialising!
		U32					m_iNodeCount;							//!< Size of element array.
		// TODO: [Stove] Serialise nodes in pre-order traversal, better access pattern.

		//! Classify a blocker using the BSP container.
		//@param rkBlocker Blocker to classify.
		//@param pRegion Pointer to region.
		//@param rkResultsVector Vector of room indicies that will be written during the traversal.
		NOALIAS void								Classify(const C_Blocker& rkBlocker, 
															 const C_BSPRegion* pRegion,
															 Btl::Vector<U32>& rkResultsVector) const __restrict;
		//! Classify a vertex using the BSP container.
		//@param rkWorldPosition World-space position to classify.
		//@param pRegion Pointer to region.
		//@return Index of the room containing the point.
		NOALIAS const U32							Classify(const Maths::HVec3& rkWorldPosition, 
															 const C_BSPRegion* pRegion) const __restrict;
		//! Recursive method to traverse tree.
		//@param rkResults Results object to object with results of the trace.
		//@param rkNode Root node of the sub-tree.
		//@param rkRay Ray that should be traced into the BSP tree.
		//@param rkStartPosition World-space starting position of the ray.
		//@param rkEndPosition World-space ending position of the ray.
		//@param rkWorldNormal World-space normal for the ray.
		void										Trace(C_TraceResults& rkResults,
														const C_BSPRegion& rkNode,
														const C_Ray& rkRay,
														const Maths::C_Vertex& rkStartPosition, 
														const Maths::C_Vertex& rkEndPosition,
														const Maths::C_Normal& rkWorldNormal) const;
	};

	// Include implementations of templated methods in C_BSPContainer.
	#include <Portal/BSPContainer.inl>
}

#endif // PORTAL_BSPCONTAINER_H
