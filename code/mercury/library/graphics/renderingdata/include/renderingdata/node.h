#ifndef RD_NODE_H
#define RD_NODE_H
#ifdef _MSC_VER
#pragma once
#endif

#include "resourceindex.h"
#include "bounds.h"

#if !PLATFORM_SPU
#include "btl\Allocators\stackallocator.h"
#endif

namespace RenderingData
{
	class Scene;
	namespace Build {class ResourceBlock; class Scene;}

	// For SPU fetch.
	struct SrcData
	{
		const U8* pMain;
		U32 size;
	};

#if !PLATFORM_SPU

	class AutoStackAllocator
	{
	public:
		AutoStackAllocator() : m_pAllocator(0) {}
		~AutoStackAllocator() {Btl::StackAllocatorSystem::Instance().Free(m_pAllocator);}

		void Set(Btl::DynamicStackAllocator* p) {assert(m_pAllocator == 0); m_pAllocator = p;}
		void Free() {Btl::StackAllocatorSystem::Instance().Free(m_pAllocator); m_pAllocator = 0;}
		void Start() {Btl::StackAllocatorSystem::Instance().Start(m_pAllocator);}
		void End() {Btl::StackAllocatorSystem::Instance().End(m_pAllocator);}

		Btl::DynamicStackAllocator* GetAllocatorPtr() {return m_pAllocator;}
		const Btl::DynamicStackAllocator* GetAllocatorPtr() const {return m_pAllocator;}
		
		S32 BytesUsed() const {if (m_pAllocator) return m_pAllocator->BytesUsed(); else return 0;}

		// Immune to copying.
		AutoStackAllocator(const AutoStackAllocator& other) : m_pAllocator(0) {}
		AutoStackAllocator& operator = (const AutoStackAllocator& other) {return *this;}

	private:
		Btl::DynamicStackAllocator* m_pAllocator;
	};

#endif

	enum NodeType
	{
		NODE_TYPE_EMPTY,
		NODE_TYPE_ROOT,
		NODE_TYPE_CULL,
		NODE_TYPE_RENDER,
		NODE_TYPE_GROUP,
		NODE_TYPE_LOD,
		NODE_TYPE_PORTAL,	// deprecated.
		NODE_TYPE_RENDER_LIST,
		NODE_TYPE_RENDER_LIST_PLATFORM, // on target platform only
		NODE_TYPE_RENDER_LIST_BUILD, // while building
		NODE_TYPE_MATERIAL,
		NODE_TYPE_CUSTOM = 100 // user types start from here
	};

	// Base type for nodes.
	class Node
	{
	public:
		Node();
		virtual ~Node();

		const Btl::String& GetName() const;

		virtual NodeType GetNodeType() const; // The actual type of the node.
		virtual bool IsNode(NodeType t) const; // True if the node is type t or derived from it.
		virtual Node* Clone() const; // Return a copy of the node.

		ResourceIndex GetParent() const;
		S32 GetNumSubNodes() const;
		ResourceIndex GetSubNode(S32 i) const;

		void SetName(const Btl::String& name);
		S32 AddSubNode(const ResourceIndex& i); // Add a node to the end of the list.
		ResourceIndex EraseSubNode(S32 i); // Erase a sub-node and return the index of the erased node. Subsequent nodes move down in the list.
		ResourceIndex ClearSubNode(S32 i); // Clear the sub-node, leaving a blank space in the list.
		void CleanSubNodes(); // Remove all blank sub-nodes.
		void ReplaceSubNode(S32 i, const ResourceIndex& r); // Replaces sub-node i with r.

		Scene* GetScenePtr() const; // Scene that the node belongs to. 
		const ResourceIndex& GetLocation() const; // Returns the resource index of the node.
		
		FRIEND_SERIALISE(Node)
		VIRTUAL_SERIALISATION_KEY_DECL

		friend class Build::ResourceBlock;
		friend class Build::Scene;
		void SetScenePtr(Scene* pScene); // For internal use.

	protected:
		void SetParent(const ResourceIndex& i);
		void SetLocation(S32 block, S32 index);

	private:
		Btl::Vector<ResourceIndex> m_subNodes;
#if !PLATFORM_SPU
		Btl::String m_name;
#endif
		ResourceIndex m_parent;
		ResourceIndex m_location;
		Scene* m_pScene;
	};

	//////////////////////////////////////////////
	// Nodes

	class CullNode : public Node
	{
	public:
		CullNode();

		virtual NodeType GetNodeType() const;
		virtual bool IsNode(NodeType t) const;
		virtual CullNode* Clone() const;

		// Bounds on a cull node are initialised to infinite size.
		// Objects within the node may also have bounds, e.g. primitives and sub-nodes.
		// Once the tree is complete, the bounds will be recalculated in Scene::Finish(),
		// which reduces the bounds to the union of the bounds of the contained objects.
		const Bounds& GetBounds() const;
		Bounds& GetBounds();
		void SetBounds(const Bounds& bounds);

		const Btl::BitVector& GetPvsBits() const;
		Btl::BitVector& GetPvsBits();

		const Btl::BitVector& GetPortalBits() const;
		Btl::BitVector& GetPortalBits();

		S32 GetGroupIndex() const;
		void SetGroupIndex(S32 i);

		FRIEND_SERIALISE(CullNode)
		VIRTUAL_SERIALISATION_KEY_DECL;

	private:
		Bounds m_bounds;				// for culling
		Btl::BitVector m_pvsBits;		// 1 bit per PVS zone
		Btl::BitVector m_portalBits;	// 1 bit per room
		S32 m_groupIndex;
	};

	class RootNode : public CullNode
	{
	public:
		RootNode();// {}
		//explicit RootNode(Scene* pScene);

		friend class Scene;

		virtual NodeType GetNodeType() const;
		virtual bool IsNode(NodeType t) const;
		virtual RootNode* Clone() const;
	};

	// Node that puts its sub-nodes into a group.
	class GroupNode : public CullNode
	{
	public:
		GroupNode() {}

		virtual NodeType GetNodeType() const;
		virtual bool IsNode(NodeType t) const;
		virtual GroupNode* Clone() const;

		FRIEND_SERIALISE(GroupNode)
		VIRTUAL_SERIALISATION_KEY_DECL

	private:
		Btl::String m_groupName;
	};

	// Node that interprets sub-nodes as lods.
	class LodNode : public CullNode
	{
	public:
		LodNode() {}

		virtual NodeType GetNodeType() const;
		virtual bool IsNode(NodeType t) const;
		virtual LodNode* Clone() const;

		FRIEND_SERIALISE(LodNode)
		VIRTUAL_SERIALISATION_KEY_DECL

	private:
		Btl::Vector<float> m_lodThresholds;
	};

	#include "noninterface/node.inl"

}

#endif
