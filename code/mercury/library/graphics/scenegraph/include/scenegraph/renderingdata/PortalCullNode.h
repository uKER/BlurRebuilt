#ifndef SCENE_GRAPH_PORTAL_CULL_NODE_H
#define SCENE_GRAPH_PORTAL_CULL_NODE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SceneGraph/renderingdata/ModelInstanceCullNode.h>
#include <Btl/Utility/pair.h>

namespace SceneGraph
{
	/// Cull node for dynamic objects using portal data.
	// Note to this class' future clients: The first element in the probes vector should 
	// always be the world-space position of the world sphere for the parent ModelInstance,
	// in order to reduce the overhead of continual clearing and setting of the probe vector
	// elements.
	//  -- STOVE, 03032009
	class ALIGN16 C_PortalCullNode : public C_ModelInstanceBasicCullNode
	{
	private:
		/// Spatial probe sub-type.
		class C_SpatialProbe
		{
		public:
			/// Portal system/room index pair.
			typedef Btl::Pair<U32, U32> RoomSystemIndexPair;

			/// Default c'tor.
			explicit C_SpatialProbe() :
				m_iLastPortalAndRoom(Btl::Pair<U32, U32>(static_cast<U32>(-1), static_cast<U32>(-1)))
			{
				return;
			}
			/// C'tor.
			explicit C_SpatialProbe(const Maths::HVec3& rkPosition) :
				m_Position(rkPosition),
				m_iLastPortalAndRoom(Btl::Pair<U32, U32>(static_cast<U32>(-1), static_cast<U32>(-1)))
			{ 
				return;
			}
			/// D'tor.
			~C_SpatialProbe()
			{ return; }

			/// Return the last room the index of the last room the spatial probe was 
			/// classified as being in.
			NOALIAS const RoomSystemIndexPair	GetLastPortalAndRoomIndex() const __restrict
			{ return m_iLastPortalAndRoom; }
			/// Return position of the spatial probe.
			NOALIAS const Maths::HVec3&			GetPosition() const
			{ return m_Position; }
			/// Set the last room index.
			void								SetLastRoomIndex(const RoomSystemIndexPair& rkIndexPair)
			{
				m_iLastPortalAndRoom = rkIndexPair;
				return;
			}
			/// Set the position.
			void								SetPosition(const Maths::HVec3& rkPosition)
			{
				m_Position = rkPosition;
				return;
			}

		private:
			Maths::HVec3				m_Position;					// Position in world.
			RoomSystemIndexPair			m_iLastPortalAndRoom;		// Last portal system and room the spatial probe was in.
		};

	public:
		/// C'tor.
		explicit C_PortalCullNode(const C_ModelInstance* pModelInstance) :
			C_ModelInstanceBasicCullNode(pModelInstance)
		{
			m_vProbes.Resize(1);	// Make sure we always have 1 spatial probe available.
			return; 
		}
		/// D'tor -- placeholder method.
		~C_PortalCullNode()
		{ return; }

		/// Accept a visitor object.
		void							Accept(I_Visitor& rkVisitor);
		/// Add a spatial probe to the cull node.
		NOALIAS void					AddSpatialProbe(const Maths::HVec3& rkPosition)
		{ 
			m_vProbes.PushBack(C_SpatialProbe(rkPosition));
			return;
		}
		/// Remove all spatial probes from the cull node.
		void							ClearSpatialProbes();
		/// Gather this cull node.
		void							Gather(const C_View& rkView) const;
		/// Gather this cull node for multiple views.
		void							Gather(const C_MultiView& rkMultiView) const;
		/// Return the last portal system index at a specified index.
		const U32						GetLastPortalIndexAtIndex(const U32 iIndex) const
		{ return m_vProbes.At(iIndex).GetLastPortalAndRoomIndex().second; }
		/// Return the last room index at a specified index.
		const U32						GetLastRoomIndexAtIndex(const U32 iIndex) const
		{ return m_vProbes.At(iIndex).GetLastPortalAndRoomIndex().first; }
		/// Get spatial probe at a specified index.
		const Maths::HVec3&				GetSpatialProbeAtIndex(const U32 iIndex) const
		{ return m_vProbes.At(iIndex).GetPosition(); }
		/// Return the number of spatial probes in the cull node.
		NOALIAS const U32				GetSpatialProbeCount() const __restrict
		{ return static_cast<U32>(m_vProbes.Size()); }
		const ModelInstanceCullNodeType	GetType() const __restrict
		{
			return I_ModelInstanceCullNode::MODELINSTANCECULLNODE_TYPE_PORTAL;
		}
		/// Set the spatial probe at index.
		void							SetSpatialProbeAtIndex(const U32 iIndex, const Maths::HVec3& rkPosition)
		{
			m_vProbes.At(0).SetPosition(rkPosition);
			return;
		}
		/// Set the last room index.
		void							SetLastRoomIndex(const U32 iLastPortalIndex, const U32 iLastRoomIndex)
		{
			C_SpatialProbe::RoomSystemIndexPair pair(iLastPortalIndex, iLastRoomIndex);
			m_vProbes.At(0).SetLastRoomIndex(pair);
			return;
		}
		
	private:
		Btl::Vector<C_SpatialProbe>		m_vProbes;				// List of probes on the cull node.
	
		void							OnGather(const C_View& rkView) const;
		void							OnGather(const C_MultiView& rkMultiView) const;

		// Disallow calls to this:
		explicit C_PortalCullNode();
	};

}

#endif // SCENE_GRAPH_PORTAL_CULL_NODE_H
