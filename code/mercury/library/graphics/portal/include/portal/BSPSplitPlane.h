#ifndef PORTAL_BSPSPLITPLANE_H
#define PORTAL_BSPSPLITPLANE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/BSPRegion.h>

namespace Portal
{
	//! A split plane (node data for the tree).
	class C_BSPSplitPlane
	{
	public:
		//! Default c'tor.
		__forceinline explicit C_BSPSplitPlane();
		//!< D'tor.
		__forceinline ~C_BSPSplitPlane();

		//! Deserialise a split plane from a stream.
		//@param rkStream Stream from which to deserialise a split plane object.
		template <typename _T>
		void							Deserialise(_T& rkStream);
		//! Return the back region.
		//@return The region at the back of the plane.
		__forceinline NOALIAS const C_BSPRegion&		GetBack() const __restrict;
		//! Return the front region.
		//@ return The region at the front of the plane.
		__forceinline NOALIAS const C_BSPRegion&		GetFront() const __restrict;
		//! Return the plane at that sub-divides the space.
		//@ return The plane that divides the space at this node.
		__forceinline NOALIAS const Maths::C_Plane&	GetPlane() const __restrict;
		
	private:
		Maths::C_Plane		m_Plane;					//!< Physical plane that divides the space.
		C_BSPRegion			m_Front;					//!< Front region.
		C_BSPRegion			m_Back;						//!< Back region.
	};

	// Include implementation of templated methods.
	#include "BSPSplitPlane.inl"
}

#endif // PORTAL_BSPSPLITPLANE_H
