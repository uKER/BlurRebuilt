#ifndef PORTAL_BUILDER_H
#define PORTAL_BUILDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Utility/String.h>
#include <Btl/Containers/bitvector.h>
#include <Btl/Containers/map.h>
#include <misc_shared/DataStructure/StringTable.h>
#include <Portal/BuilderPortal.h>
#include <Portal/BuilderRoom.h>
#include <Portal/BuilderRoomGrid.h>
#include <Portal/BuilderRoomContainmentSpace.h>
#include <misc_shared/Geometry/RegionBSP.h>

// Call these forward.
namespace Aftk
{
	class C_SceneMeshNode;
	class C_Polygon;
	class C_Bounds;
};

namespace Portal
{
	// Call these forward.
	class C_System;
	
	//! Portal-system builder object.
	class C_Builder
	{
	public:
		//! Default c'tor.
		explicit C_Builder();
		//! D'tor.
		~C_Builder();

		//! Build the room BSP tree.
		void											BuildRoomBSP();
		//! Calculate containment bit vector for a polygon and return it's index.
		//@param rkNode Scene mesh node that contains polygon.
		//@param rkPoly Polygon for which bit vector is required.
		//@return Index into list of bit vectors.
		const U32										CalculateContainmentBitVector(const Aftk::C_SceneMeshNode& rkNode, const Aftk::C_Polygon& rkPoly);
		//! Calculate containment bit vector for a bounding box and return it's index.
		//@param rkBounds Bounding volume for which bitvector is desired.
		//@return Index into list of bit vectors.
		const U32										CalculateContainmentBitVector(const Aftk::C_Bounds& rkBounds);
		//! Create a new edge.
		//@param pVert1 Pointer to first vertex in the edge.
		//@param pVert2 Pointer to second vertex in the edge.
		//@return Pointer to newly created edge.
		C_BuilderEdge*									CreateEdge(C_BuilderVertex* pVert1, C_BuilderVertex* pVert2);
		//! Create a new portal and return a pointer to it.
		//@param pParentRoom Pointer to room that contains portal.
		//@return Pointer to newly created portal.
		C_BuilderPortal*								CreatePortal(C_BuilderRoom* pParentRoom);
		//! Create a new room and return a pointer to it.
		//@param pName Name of the room.
		//@return Pointer to newly created room.
		C_BuilderRoom*									CreateRoom(const char* pName);
		//! Create a new vertex.
		//@param rkPos World-space position of the vertex.
		//@return Pointer to the newly created vertex.
		C_BuilderVertex*								CreateVertex(const Maths::HVec3& rkPos);
		//! Return the index for a specified portal.
		//@param pPortal Pointer to the portal.
		//@return Index for the portal in the builder's portal list.
		const U32										GetIndexForPortal(const C_BuilderPortal* pPortal) const;
		//! Return the index for a specified room.
		//@param pRoom Pointer to room.
		//@return Index for the room in the builder's room list.
		const U32										GetIndexForRoom(const C_BuilderRoom* pRoom) const;
		//! Return the index for a specified space.
		//@param pSpace Pointer to space.
		//@return Index for the space in the builder's space list.
		const U32										GetIndexForSpace(const C_BuilderRoomContainmentSpace* pSpace) const;
		//! Return the rooms bitvector at a specified index.
		//@param iIndex Index into bit vector list.
		//@return Bit vector at the specified index.
		const Btl::BitVector&							GetPortalBitsAtIndex(const U32 iIndex) const;
		//! Return the number of portals in the builder object.
		//@return Number of portals in the portal system object.
		NOALIAS const U32								GetPortalCount() const __restrict;
		//! Return portal list.
		//@return Reference to the list of portals.
		Btl::List<C_BuilderPortal>&						GetPortalList();
		//! Return the room BSP tree.
		//@return Reference to the BSP tree.
		RegionBSP::C_Tree&								GetRoomBSPTree();
		//! Return the number of rooms in the builder object.
		//@return Number of rooms in the portal system object.
		NOALIAS const U32								GetRoomCount() const __restrict;
		//! Return the room grid object.
		//@return Pointer to the room grid.
		C_BuilderRoomGrid*								GetRoomGrid();
		//! Return room list.
		//@return Reference to the list of rooms.
		Btl::List<C_BuilderRoom>&						GetRoomList();
		//! Get room index by Region Id.
		//@param iRegionId BSP-tree region ID.
		//@return Room index.
		const U32										GetRoomIndexByRegionId(S32 iRegionId) const;
		//! Return the number of spaces in the builder object.
		//@return Number of spaces in the space list.
		NOALIAS const U32								GetSpaceCount() const __restrict;
		//! Return space list.
		//@return Reference to the list of spaces.
		Btl::List<C_BuilderRoomContainmentSpace>&		GetSpaceList();
		//! Get the string builder object.
		//@return Reference to the string table builder.
		Util::T_StringTableBuilder<Btl::String>&		GetStringBuilder();
		//! Merge two containment bit vectors to form a new bitvector, returns index of merged vector.
		//@param iBV1 Index of first bitvector to merge.
		//@param iBV2 Index of second bitvector to merge.
		//@return Merge two bitvectors into a new bitvector.
		const U32										MergeBitVectors(U32 iBV1, U32 iBV2);
		//! Register a string with the string table, returns index.
		//@param rkString Name of the string to register with the string table.
		//@return Index of the string in string table.
		const U32										RegisterString(const Btl::String& rkString);
		//! Setup the room grid.
		//@param fCellSize Size of a cell in the grid in world units.
		void											SetupRoomGrid(F32 fCellSize);

		//! Return a bitvector that makes the polygon always visible.
		//@return Index of world-room only bitvector.
		static const U32								GetWorldRoomOnlyBitVector();

	private:
		Btl::List<C_BuilderRoom>					m_vRooms;						//!< List of rooms in the portal system.
		Btl::List<C_BuilderPortal>					m_vPortals;						//!< List of portals in the portal system.
		Btl::List<C_BuilderVertex>					m_vVertices;					//!< Vertex data for all polygons.
		Btl::List<C_BuilderEdge>					m_vEdges;						//!< Edge data for all edges.
		C_BuilderRoomGrid*							m_pRoomGrid;					//!< Grid containing all the rooms.
		Util::T_StringTableBuilder<Btl::String>		m_StringTableBuilder;			//!< String table builder.
		Btl::List<C_BuilderRoomContainmentSpace>	m_vSpaces;						//!< Containment spaces.
		Btl::Vector<Btl::BitVector>					m_vPrimitiveContainment;		//!< Vector of bitvectors that indicate which rooms a polygon or instance is in.
		Btl::Map<Btl::BitVector, U32>				m_vPrimitiveContainmentMap;		//!< Maps primitives onto their indices, accelerates queries of containment vector.
		RegionBSP::C_Tree							m_RoomBSPTree;					//!< BSP tree for rooms.
		static U32									m_iAlwaysVisibleIndex;			//!< Index into primitive containment vector for bitfield representing always visible polygon.
		static bool									m_bAlwaysVisibleIndexCreated;	//!< Set to true when the bitfield pointed to by the above index is created.
		
		//! Return a pointer to the room that contains a point.
		//@param rkPoint Point for which the containing room is desired.
		//@return Pointer to the room that contains the pointer, NULL if non.
		C_BuilderRoom*									FindContainingRoom(const Maths::HVec3& rkPoint);
		//! Epsilon test for two floating-point numbers that encompasses both absolute and relative tests.
		//@param fX First floating point value in the test.
		//@param fY Second floating point value in the test.
		//@param fAbsTolerance Absolute tolerance in the test.
		//@return True if floating values are equivalent.
		const bool										FloatEqual(const F32 fX, const F32 fY, const F32 fAbsTolerance, const F32 fRelTolerance) const;
	};
};

#endif // PORTAL_BUILDER_H
