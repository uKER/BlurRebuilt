#ifndef PORTAL_ROOMCONTAINMENTSPACE_H
#define PORTAL_ROOMCONTAINMENTSPACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/RoomContainmentSubSpace.h>
#include <misc_shared/Geometry/RegionBSP.h>


namespace Portal
{
	// Call these forward.
	class C_Room;
	class C_System;
	
	//! A list of sub-spaces in a room.
	class C_RoomContainmentSpace
	{
	public:
		//! Default c'tor.
		__forceinline explicit C_RoomContainmentSpace();
		//! D'tor.
		__forceinline ~C_RoomContainmentSpace();

		//! Return true if containment space contains world-space point.
		//@param rkPoint World-space point to check for containment.
		//@param pSystem Pointer to parent portal system object.
		//@return True if the space contains the point.
		NOALIAS const bool								Contains(const Maths::HVec3& rkPoint, C_System* pSystem) const __restrict;
		//! Build debug mesh.
		void											BuildDebugMesh();
		//! Debug render the space.
		//@param pDebugRender Pointer to an instance of the debug renderer.
		//@param rkCameraMat View matrix.
		//@param rkColour Colour to render the space.
		void											DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43 &rkCameraMat, const Maths::RGBA& rkColour);
		//! Deserialise a room containment space.
		//@param rkStream Stream from which to deserialise the space.
		template <typename _T>
		void											Deserialise(_T& rkStream);
		//! Return the end indexd into the sub-spaces vector.
		//@return End index into the sub-space vector.
		__forceinline NOALIAS const U32					GetEndIndex() const __restrict;
		//! Return the number of planes that define the space.
		//@return Number of planes that define this space.
		__forceinline NOALIAS const U32					GetPlaneCount() const __restrict;
		//! Return the start index into the sub-spaces vector.
		//@param Start index into the sub-space vector.
		__forceinline NOALIAS const U32					GetStartIndex() const __restrict;
		//! Return the number of sub-spaces in the space.
		//@return Number of sub-spaces that define this space.
		__forceinline NOALIAS const U32					GetSubSpaceCount() const __restrict;

	private:
		U32										m_iStartSubSpaceIndex;	//!< Start index into sub-spaces vector.
		U32										m_iEndSubSpaceIndex;	//!< End index into sub-spaces vector.
		Btl::Vector<RegionBSP::C_Mesh>			m_vDebugMeshes;			//!< Debug mesh list.
	};

	// Include the implementation of templated methods.
	#include <Portal/RoomContainmentSpace.inl>
};

#endif // PORTAL_ROOMCONTAINMENTSPACE_H
