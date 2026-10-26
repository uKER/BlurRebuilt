#ifndef PORTAL_BUILDERROOM_H
#define PORTAL_BUILDERROOM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/BuilderPolygon.h>


namespace Portal
{
	// Call these forward.
	class C_Builder;
	class C_BuilderPortal;
	class C_BuilderRoomContainmentSpace;
	
	//! Portal-system room builder object.
	class C_BuilderRoom
	{
	public:
		//! Specified c'tor, takes string name of the room.
		//@param pName Name of the room.
		//@param pBuilder Pointer to the parent builder object.
		explicit C_BuilderRoom(const char* pName, C_Builder* pBuilder);
		//! D'tor.
		~C_BuilderRoom();

		//! Add a portal to the room.
		//@param pPortal Pointer to the portal to add to the room.
		void									AddPortal(C_BuilderPortal* pPortal);
		//! Compute the axis-aligned box that minimally contains the room.
		void									ComputeExtents();
		//! Create a new room polygon and return a pointer to it.
		//@param iBaseIndex Base index for the polygon.
		//@return Pointer to newly created room polygon.
		C_BuilderPolygon*						CreatePolygon(U32 iBaseIndex, U32 iAftkPolygonIndex, Aftk::C_SceneMeshNode* pAftkSceneMeshNode);
		//! Get the builder object that owns this room.
		//@return Pointer to the parent builder object.
		C_Builder*								GetBuilder();
		//! Return the centroid of the room.
		//@return Centroid of the room.
		const Maths::HVec3&						GetCentroid() const;
		//! Return the axis-aligned bounding box that minimally contains the room.
		//@return Axis-aligned box that minimally contains the room.
		const Maths::C_AxisBox&					GetExtents() const;
		//! Return the name of the room.
		//@return Name of the room.
		const Btl::String&						GetName() const;
		//! Return the number of polygons in the room.
		//@return Number of polygons in the room.
		NOALIAS U32								GetPolygonCount() const __restrict;
		//! Return polygon list.
		//@return Reference to the polygon list.
		Btl::List<C_BuilderPolygon>&			GetPolygonList();
		//! Return polygon list.
		//@return Reference to the polygon list.
		const Btl::List<C_BuilderPolygon>&		GetPolygonList() const;
		//! Return the number of portals in this room.
		//@return Number of portals in the room.
		NOALIAS U32								GetPortalCount() const __restrict;
		//! Return the list of portals.
		//@return Reference to the portal list.
		const Btl::List<C_BuilderPortal*>&		GetPortalList() const;
		//! Return the list of portals.
		//@return Reference to the portal list.
		Btl::List<C_BuilderPortal*>&			GetPortalList();
		//! Return the Region Id.
		//@return The region ID in the BSP tree.
		const S32								GetRegionId() const;
		//! Return pointer to the space that defines this room.
		//@return Pointer to the space that defines this room.
		C_BuilderRoomContainmentSpace*			GetSpace();
		//! Return pointer to the space that defines this room.
		//@return Pointer to the space that defines this room.
		const C_BuilderRoomContainmentSpace*	GetSpace() const;
		//! Return the number of sub-spaces the room uses.
		//@return The number of sub-spaces in the room.
		const U32								GetSubSpaceCount() const;
		//! Return true if ray intersects room.
		//@param rkOrigin Origin of the ray that intersects the room.
		//@param rkDir Direction of the vector.
		//@param fMaxT Maximum length of the ray.
		//@param pOutDistance The distance of the ray.
		//@param pOutNormal World-space normal at the point of intersection.
		//@return True if an intersection occured.
		const bool								Intersects(const Maths::HVec3& rkOrigin, const Maths::HVec3& rkDir, F32 fMaxT, F32* pOutDistance, Maths::HVec3* pOutNormal) const;
		//! Remove polygon from the room.
		//@param pPoly Pointer to polygon to remove from the room.
		Btl::List<C_BuilderPolygon>::Iterator	RemovePolygon(C_BuilderPolygon* pPoly);
		//! Set the region Id for the room.
		//@param iRegionId Region Id for the BSP mapping.
		void									SetRegionId(S32 iRegionId);
		//! Set the space on the room.
		//@param pSpace Pointer to the space that defines the room.
		void									SetSpace(C_BuilderRoomContainmentSpace* pSpace);

	private:
		C_Builder*						m_pBuilder;			//!< Builder object.
		Btl::List<C_BuilderPolygon>		m_vPolys;			//!< Polygons for the room.
		Btl::List<C_BuilderPortal*>		m_vPortals;			//!< Portals in this room.
		Btl::String						m_Name;				//!< Name of the room.
		C_BuilderRoomContainmentSpace*	m_pSpace;			//!< Pointer to space that defines this room.
		Maths::HVec3					m_Centroid;			//!< Room's centroid.
		Maths::C_AxisBox				m_Extents;			//!< Room's extents, used for quick grid rejection.
		S32								m_iRegionId;		//!< Region Id that matches up with Room BSP tree in Builder object.

		// Disallow calls to these:
		explicit C_BuilderRoom();
	};
};

#endif // PORTAL_BUILDERROOM_H
