#ifndef PORTAL_SYSTEM_H
#define PORTAL_SYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_SPU
#include <Portal/Room.h>
#include <Portal/Portal.h>
#include <Portal/RoomGrid.h>
#include <Portal/RoomContainmentSubSpace.h>
#include <Btl/Containers/vector.h>
#else
#include <Portal/Room.h>
#include <Portal/Portal.h>
#include <Portal/RoomGrid.h>
#include <Portal/DeferredTracer.h>
#include <Portal/DeferredBlocker.h>
#include <Btl/Containers/BitVector.h>
#include <Btl/Containers/algorithm.h>
#include <misc_shared/DataStructure/StringTable.h>
#include <Btl/Utility/string.h>
#include <System/debug/Variables.h>
#endif

// Call these forward.
namespace Display
{
	class C_ViewSettings3D;
};

namespace Sys
{
	class C_FileBlock;
};

namespace Portal
{
	//! Enum for portal data versioning.
	enum E_PortalFileVersion
	{
		E_PortalFileVersion_Initial,				//!< Initial version.
		E_PortalFileVersion_SPURefactor,			//!< Flattened portal data structures for SPUification of point classification.
		E_PortalFileVersion_BSPChunk,				//!< Added BSP chunk for ray-tracing.
		
		//! Current file version.
		E_PortalFileVersion_Current = E_PortalFileVersion_BSPChunk
	};

	extern Debug::Logger &g_Log;

	// Call these forward.
	class C_Builder;
	class C_ViewPolygon;

	//! Main portal system control object, supported multiple instanciations,
	//! now 16 byte aligned due to need to fetch for SPU classification.
	class ALIGN16 C_System
	{
	public:
#ifndef PLATFORM_SPU
		//! Default c'tor.
		__forceinline explicit C_System();
		//! D'tor.
		__forceinline ~C_System();

		//! Block a portal at the specified index.
		//@param iIndex Index of the portal to block.
		__forceinline NOALIAS void						BlockPortalAtIndex(const U32 iIndex);
		//! Return the room index containing a specified point.
		//@param rkPoint The point that is to be classified.
		//@param iLastIndex The room index in which the point was last seen, useful for exploiting temporal and spatial coherence.
		//@return The room index in which the point resides.
		NOALIAS const U32								Classify(const Maths::HVec3& rkPoint, U32 iLastIndex) const __restrict;
		//! Entry point for portal system debug evaluation.
		//@param rkBitfield Bitfield to store results of the portal system room visibility evaluation.
		//@rkViewSettings View settings for the current world view.
		//@pDebugRender Pointer to an instance of a debug renderer.
		void											DebugEvaluate(Btl::BitVector& rkBitfield, const Display::C_ViewSettings3D& rkViewSettings, DbUtil::I_DebugRenderPtr pDebugRender) const;
		//! Debug render the entire portal system.
		//@param pDebugRender Pointer to an instance of a debug renderer.
		//@param rkCameraMat View matrix.
		//@param rkVisibleRoomSet A bit vector of visible rooms.
		void											DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat, const Btl::BitVector& rkVisibleRoomSet);
		//! Entry point for portal system evaluation.
		//@param rkBitfield Bitfield to store results of the portal system room visibility evaluation.
		//@param rkViewSettings View settings for the current world view.
		void											Evaluate(Btl::BitVector& rkBitfield, Btl::Vector<U16>& rkRoomIndexList, const Display::C_ViewSettings3D& rkViewSettings) const;
		//! Return a pointer to room that contained the camera after the last evaluation of the room/portal graph.
		//@return Pointer to the room that contained the camaera after the last evaluation.
		__forceinline const C_Room*						GetLastCameraRoom() const;
		//! Return a plane at specified index.
		//@param iIndex Index of the desired plane.
		//@return Plane for the specified index.
		__forceinline const Maths::C_Plane&				GetPlaneAtIndex(const U32 iIndex) const;
		//! Return portal at specified index.
		//@param iIndex Index of the desired portal.
		//@return Portal at the specified index.
		__forceinline const C_Portal&					GetPortalAtIndex(const U32 iIndex) const;
		//! Return room at specified index.
		//@param iIndex Index of the desired room.
		//@return Room at the specified index.
		__forceinline const C_Room&						GetRoomAtIndex(const U32 iIndex) const;
		//! Return the number of rooms in the portal system object.
		//@return Number of rooms in the portal system.
		__forceinline NOALIAS const U32					GetRoomCount() const __restrict;
		//! Return a room containment space at a given index.
		//@ Space at a specified index.
		__forceinline const C_RoomContainmentSpace&		GetSpaceAtIndex(U32 iIndex) const;
		//! Return sub-space at a specified index.
		//@ Sub-space at a specified index.
		__forceinline const C_RoomContainmentSubSpace&	GetSubSpaceAtIndex(U32 iIndex) const;
		//! Return a string for a given string table index.
		//@ String at a specified index.
		Btl::String										GetStringByIndex(Util::C_StringId iStringId) const;
		//! Return a reference the deferred tracer.
		//@ The deferred tracer.
		__forceinline C_DeferredTracer&					GetTracer();
		//! Return a reference the deferred tracer.
		//@ The deferred tracer.
		__forceinline const C_DeferredTracer&			GetTracer() const;
		//! Kick off deferred raytrace jobs.
		void											KickDeferredRayTraceJobs();
		//! Load from file.
		//@param rkBlock Root file block.
		bool											LoadFromFile(Sys::C_FileBlock& rkBlock);
		//! Process results from deferred raytrace jobs.
		void											ProcessDeferredRayTraceJobs();
		//! Reset the system.
		void											Reset();
		//! Unblock a portal at specified index.
		//@param iIndex Index of the portal to be unblocked.
		__forceinline NOALIAS void						UnblockPortalAtIndex(const U32 iIndex);
		
		//! Get portal system list.
		//@return Vector of portal systems that are currently active.
		static const Btl::Vector<C_System*, Btl::VectorParameters_Static<PORTAL_CONFIG_MAXPORTALSYSTEMS> >&
														GetActivePortalSystems();
		//! Return the deferred blocker.
		static C_DeferredBlocker&						GetDeferredBlocker();
		//! Return true if debug rendering should highlight current room.
		//@return Boolean value indicating if the current room should be highligted when debug rendering.
		static const bool								GetHighlightCurrentRoom();
		//! Return true if we should render debug room names.
		//@return Boolean value indicating if room names should be displayed when debug rendering.
		static const bool								GetRenderDebugRoomNames();
		//! Return true if we should render debug rooms as solid polygons.
		//@return Boolean value indicating if room geometry should be rendered as solid polygons.
		static const bool								GetRenderSolidRooms();
		//! Return true if we should render debug portals as solid polygons.
		//@return Boolean value indicating if the portal geometry should be rendered as solid polygons.
		static const bool								GetRenderSolidPortals();
		
#endif // #ifndef PLATFORM_SPU

	private:
		// Note to future class maintainers, be careful moving around class members and upsetting SPU alignment.
		// TODO: [Stove] Idea for optimisation, sort sub-spaces by volume -- should increase chances of point being in sub-space earlier in the list.
#ifndef PLATFORM_SPU
		C_DeferredTracer						m_Tracer;			//!< Storage for BSP raytracer, includes the BSP tree.
		ALIGN16 Btl::Vector<C_RoomContainmentSubSpace>	
												m_vSubSpaces;		//!< Storage for all sub-spaces in system.
		ALIGN16 Btl::Vector<Maths::C_Plane>				
												m_vPlanes;			//!< Storage for all planes that define our sub-spaces.
		Btl::Vector<C_Room>						m_vRooms;			//!< Storage for all rooms in system.
		Btl::Vector<C_Portal>					m_vPortals;			//!< Storage for all portals in system.
		Btl::Vector<C_RoomContainmentSpace>		m_vSpaces;			//!< Storage for all spaces in system.
		// TODO: [Stove] Make these U16s -- lower LS usage footprint.
		Btl::Vector<U32>						m_vCellSpaceIndices;//!< Storage for indices mapping grid cells to subspaces.
		C_RoomGrid								m_RoomGrid;			//!< Spatial structure of rooms to accelerate room containment queries.
		Util::C_StringTable						m_StringTable;		//!< Storage for all strings (room/portal names) in system *DEBUG ONLY*.
		mutable const C_Room*					m_pLastCameraRoom;	//!< Pointer to room that last contained camera.
		Btl::BitVector							m_vPortalsBlocked;	//!< Bit vector representing if each portal is blocked or not.
		Btl::BitVector							m_vBlockers;		//!< Bitfield indicating if each portal is open or closed.
#endif // #ifndef PLATFORM_SPU
		
#ifndef PLATFORM_SPU
		// Static variables.
		static C_DeferredBlocker	m_DeferredBlocker;				//!< Deferred blocker.
		static Btl::Vector<C_System*, Btl::VectorParameters_Static<PORTAL_CONFIG_MAXPORTALSYSTEMS> >
									m_vPortalSystems;				//!< List of portal systems that are active.
		static Debug::Bool			m_bDebugShowRoomNames;			//!< Set to true to render debug room names.
		static Debug::Int			m_iMaxRecursionLevels;			//!< The number of levels of recursion to allow when rendering portals.
		static Debug::Bool			m_bDebugShowViewPoly;			//!< Set to true to show the view polygons used during traversal.
		static Debug::Float			m_fDebugTestPortalsBorderX;		//!< X - Border around the screen that poly doesn't encompass.
		static Debug::Float			m_fDebugTestPortalsBorderY;		//!< Y - Border around the screen that poly doesn't encompass.
		static Debug::Bool			m_bDebugShowCurrentRoom;		//!< Set to true to highlight the current room containing the camera.
		static Debug::Bool			m_bDebugShowSpaces;				//!< Show the spaces that define the rooms.
		static Debug::Bool			m_bDebugShowPortals;			//!< Debug render the portals.
		static Debug::Bool			m_bDebugShowRooms;				//!< Debug render the rooms.
		static Debug::Bool			m_bDebugShowRays;				//!< Debug render the deferred rays.
		static Debug::Bool			m_bDebugShowBlockers;			//!< Debug render the deferred blockers.
		static Debug::Bool			m_bDebugShowGrid;				//!< Debug render the grid.
		static Debug::Int			m_bDebugSpaceIndex;				//!< Highlight a specific space index.
		static Debug::Bool			m_bDebugSolidRooms;				//!< Debug render rooms with solid geometry as opposed to wireframe.
		static Debug::Bool			m_bDebugDrawOnlyVisibleRooms;	//!< Draw only visible rooms.
		static Debug::Bool			m_bDebugSetWorldRoomBit;		//!< Set the world room bit.
		static Debug::Bool			m_bDebugSolidPortals;			//!< Debug render poratls with solid geometry as opposed to wireframe.
		static Debug::Int			m_iDebugRenderRoomOnly;			//!< Debug render room geometry for a specific room.
		static Debug::Float			m_fDebugMinArea;				//!< Debug value for minimum projected area of a portal during traversal.
		static Debug::Int			m_bDebugSingleRoom;				//!< Render room contents for a specific room.
				
		//! Calculate a room index from a room pointer.
		//@param pRoom Pointer to the room for which an index is desired.
		//@return The index for the pointer, InvalidRoom returned if pointer is out of range of room vector.
		__forceinline NOALIAS const U32					CalculateRoomIndex(const C_Room* pRoom) const __restrict;
		//! Return the room index containing a specified point.
		//@param rkPoint The point that is to be classified.
		//@return The index of the room in which the point resides.
		NOALIAS const U32								Classify(const Maths::HVec3& rkPoint) const __restrict;
		//! Figure out what to draw (debug version).
		//@param rkBitfield Bitfield to store result of the traversal.
		//@param rkView View matrix.
		//@param pRoom Pointer to the room we are currently considering.
		//@param rkViewPoly View polygon used to evaluate visiblity of portals.
		//@param pDebugRender Pointer to an instance of a debug renderer.
		//@param iRecurseLevel Number of recursive calls to this function.
		void											DebugEvaluate(Btl::BitVector& rkBitfield, const Display::C_ViewSettings3D& rkViewSettings, const C_Room* pRoom, const C_ViewPolygon& rkViewPoly, DbUtil::I_DebugRenderPtr pDebugRender, U32 iRecurseLevel = 1) const;
		//! Debug render the blockers.
		//@param pDebugRender Pointer to an instance of a debug renderer.
		void											DebugRenderBlockers(DbUtil::I_DebugRenderPtr pDebugRender);
		//! Debug render the portals only.
		//@param pDebugRender Pointer to an instance of a debug renderer.
		void											DebugRenderPortals(DbUtil::I_DebugRenderPtr pDebugRender);
		//! Debug render the deferred rays.
		//@param pDebugRender Pointer to an instance of a debug renderer.
		//@param rkCameraMat View matrix.
		void											DebugRenderRays(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat);
		//! Debug render the room grid.
		//@param pDebugRender Pointer to an instance of a debug renderer.
		//@param rkCameraMat View matrix.
		void											DebugRenderRoomGrid(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat);
		//! Debug render the rooms.
		//@param pDebugRender Pointer to an instance of a debug renderer.
		//@param rkCameraMat View matrix.
		//@param rkVisibleRoomSet Bit vector of visible rooms.
		void											DebugRenderRooms(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat, const Btl::BitVector& rkVisibleRoomSet);
		//! Debug render the spaces.
		//@param pDebugRender Pointer to an instance of a debug renderer.
		//@param rkCameraMat View matrix.
		void											DebugRenderSpaces(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat);
		//! Figure out what to draw.
		//@param rkBitfield Bitfield to store result of the traversal.
		//@param rkView View matrix.
		//@param pRoom Pointer to the room we are currently considering.
		//@param rkViewPoly View polygon used to evaluate visiblity of portals.
		//@param iRecurseLevel Number of recursive calls to this function.
		void											Evaluate(Btl::BitVector& rkBitfield, Btl::Vector<U16>& rkRoomIndexList, const Display::C_ViewSettings3D& rkViewSettings, const C_Room* pRoom, const C_ViewPolygon& rkViewPoly, U32 iRecurseLevel = 1) const;
		//! Find room containing camera.
		//@param rkCameraMatrix View matrix.
		//@return Pointer to the room containing the camera.
		const C_Room*									FindRoomContainingCamera(const Maths::HMat43& rkCameraMatrix) const;
#else
		// On SPU the query object needs access to the system's internals in
		// order to fetch vectors by their EA. :(
		friend class C_SystemContainmentQuery;
#endif // #ifndef PLATFORM_SPU
	};	

	// Include implementation of inlined methods.
	#include "System.inl"
};

#endif // PORTAL_SYSTEM_H
